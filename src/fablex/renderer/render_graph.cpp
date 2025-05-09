#include "render_graph.h"
#include "render_pass_container.h"

namespace fe::renderer
{

void RenderGraph::load_from_metadata(const std::string& metadataPath, RenderPassContainer* renderPassContainer)
{
    FE_LOG(LogRenderer, INFO, "Starting loading render graph from metadata.");

    clear();
    
    m_metadata = std::make_unique<RenderGraphMetadata>(renderPassContainer->get_render_context());
    m_metadata->deserialize(metadataPath);

    const RenderGraphMetadata::RenderPassMetadataMap& renderPassMetadataMap = 
        m_metadata->get_render_pass_metadata_map();

    for (const auto& it : renderPassMetadataMap)
    {
        RenderPass* renderPass = renderPassContainer->add_render_pass(it.second);
        add_node(renderPass->get_info());
    }

    FE_LOG(LogRenderer, INFO, "Loading render graph from metadata completed.");
}

void RenderGraph::add_node(const RenderPassInfo& info)
{
    if (m_nodeIndexByName.find(info.renderPassName) != m_nodeIndexByName.end())
        return;

    Node& node = m_passNodes.emplace_back(info, &m_globalWriteDependencyRegistry);
    node.m_indexInUnorderedArray = m_passNodes.size() - 1;
    m_nodeIndexByName[info.renderPassName] = (uint32)node.m_indexInUnorderedArray;
}

void RenderGraph::build()
{
    build_adjacency_lists();
    topological_sort();
    build_dependency_levels();
    finalize_dependency_levels();
    remove_redundant_syncs();
}

void RenderGraph::clear()
{
    m_globalWriteDependencyRegistry.clear();
    m_dependencyLevels.clear();
    m_queueNodeCounters.clear();
    m_topologicallySortedNodes.clear();
    m_nodesInGlobalExecOrder.clear();
    m_adjacencyArrays.clear();
    m_detectedQueueCount = 1;
    m_nodesPerQueue.clear();
    
    for (Node& node : m_passNodes)
        node.clear();
}

RenderGraph::Node* RenderGraph::get_node(RenderPassName renderPassName) const
{
    auto it = m_nodeIndexByName.find(renderPassName);
    if (it == m_nodeIndexByName.end())
        return nullptr;

    return const_cast<Node*>(&m_passNodes.at(it->second));
}

RenderGraph::ViewName RenderGraph::encode_view_name(ResourceName resourceName, uint32 viewIndex)
{
    ViewName viewName = resourceName.to_id();
    viewName <<= 32;
    viewName |= viewIndex;
    return viewName;
}

std::pair<ResourceName, uint32> RenderGraph::decode_view_name(ViewName viewName)
{
    return {ResourceName{uint32(viewName >> 32)}, uint32(viewName & 0x0000FFFF)};
}

void RenderGraph::build_adjacency_lists()
{
    m_adjacencyArrays.resize(m_passNodes.size());

    for (uint32 nodeIdx = 0; nodeIdx != m_passNodes.size(); ++nodeIdx)
    {
        Node& node = m_passNodes.at(nodeIdx);
        if (!node.has_any_dependency())
            continue;

        const ViewNameSet& curNodeWrittenViews = node.get_written_views(); 
        std::vector<uint32>& adjacentNodeIndices = m_adjacencyArrays[nodeIdx];

        for (uint32 otherNodeIdx = 0; otherNodeIdx != m_passNodes.size(); ++otherNodeIdx)
        {
            if (nodeIdx == otherNodeIdx)
                continue;

            Node& otherNode = m_passNodes.at(otherNodeIdx);

            auto detectAdjacency = [&](ViewName viewName)
            {
                bool otherDependsOnCurrentNode = 
                    curNodeWrittenViews.find(viewName) != curNodeWrittenViews.end();

                if (!otherDependsOnCurrentNode)
                    return false;

                adjacentNodeIndices.push_back(otherNodeIdx);
                if (node.m_queueIndex != otherNode.m_queueIndex)
                {
                    node.m_syncSignalRequired = true;
                    otherNode.m_nodesToSyncWith.push_back(&node);
                }

                return true;
            };

            for (ViewName viewName : otherNode.m_readViews)
                if (detectAdjacency(viewName)) break;
        }
    }
}

void RenderGraph::depth_first_search(uint64 nodeIdx, std::vector<bool>& visited, std::vector<bool>& onStack, bool& isCyclic)
{
    if (isCyclic)
        return;

    visited[nodeIdx] = true;
    onStack[nodeIdx] = true;
    
    uint64 adjacencyArrayIdx = m_passNodes.at(nodeIdx).m_indexInUnorderedArray;
    AdjacencyArray adjacencyArray = m_adjacencyArrays.at(adjacencyArrayIdx);

    for (uint32 neighbor : adjacencyArray)
    {
        if (visited[neighbor] && onStack[neighbor])
        {
            isCyclic = true;
            return;
        }

        if (!visited[neighbor])
            depth_first_search(nodeIdx, visited, onStack, isCyclic);
    }

    onStack[nodeIdx] = false;
    m_topologicallySortedNodes.push_back(&m_passNodes.at(nodeIdx));
}

void RenderGraph::topological_sort()
{
    std::vector<bool> visitedNodes(m_passNodes.size(), false);
    std::vector<bool> onStackNodes(m_passNodes.size(), false);

    bool isCyclic = false;

    for (uint32 nodeIdx = 0; nodeIdx != m_passNodes.size(); ++nodeIdx)
    {
        const Node& node = m_passNodes[nodeIdx];
        if (!visitedNodes[nodeIdx] && node.has_any_dependency())
        {
            depth_first_search(nodeIdx, visitedNodes, onStackNodes, isCyclic);
            if (isCyclic)
                FE_LOG(LogRenderer, FATAL, "Detected cyclic dependency in pass {}", node.get_info().renderPassName);
        }
    }

    std::reverse(m_topologicallySortedNodes.begin(), m_topologicallySortedNodes.end());
}

void RenderGraph::build_dependency_levels()
{
    uint32 dependencyLevelCount = 1;
    std::vector<uint32> longestDistances(m_passNodes.size(), 0);

    for (uint32 nodeIdx = 0; nodeIdx != m_topologicallySortedNodes.size(); ++nodeIdx)
    {
        uint32 originalIdx = m_topologicallySortedNodes[nodeIdx]->m_indexInUnorderedArray;
        const AdjacencyArray& adjacencyArray = m_adjacencyArrays.at(originalIdx);

        for (uint32 adjacencyNodeIdx : adjacencyArray)
        {
            if (longestDistances[adjacencyNodeIdx] < longestDistances[originalIdx] + 1)
            {
                uint32 newLongestDistance = longestDistances[originalIdx] + 1;
                longestDistances[adjacencyNodeIdx] = newLongestDistance;
                dependencyLevelCount = std::max(newLongestDistance + 1, dependencyLevelCount);
            }
        }
    }

    m_dependencyLevels.resize(dependencyLevelCount);
    m_detectedQueueCount = 1;

    for (uint32 nodeIdx = 0; nodeIdx != m_topologicallySortedNodes.size(); ++nodeIdx)
    {
        Node* node = m_topologicallySortedNodes[nodeIdx];
        uint32 levelIdx = longestDistances[node->m_indexInUnorderedArray];
        DependencyLevel& dependencyLevel = m_dependencyLevels[levelIdx];
        dependencyLevel.add_node(node);
        dependencyLevel.m_levelIndex = levelIdx;
        node->m_dependencyLevelIndex = levelIdx;
        m_detectedQueueCount = std::max(m_detectedQueueCount, node->m_queueIndex + 1);
    }
}

void RenderGraph::finalize_dependency_levels()
{
    uint64 globalExecIdx = 0;

    m_nodesInGlobalExecOrder.resize(m_topologicallySortedNodes.size(), nullptr);
    m_nodesPerQueue.resize(m_detectedQueueCount);
    NodePtrArray perQueuePreviousNode(m_detectedQueueCount, nullptr);

    for (DependencyLevel& dependencyLevel : m_dependencyLevels)
    {
        uint64 localExecIdx = 0;

        std::unordered_map<ViewName, std::unordered_set<QueueIndex>> resourceReadingQueueTracker;
        dependencyLevel.m_nodesPerQueue.resize(m_detectedQueueCount);

        for (Node* node : dependencyLevel.m_nodes)
        {
            for (ViewName viewName : node->get_read_views())
                resourceReadingQueueTracker[viewName].insert(node->m_queueIndex);

            node->m_globalExecIndex = globalExecIdx;
            node->m_dependencyLevelLocalExecIdx = localExecIdx;
            node->m_dependencyLevelIndex = dependencyLevel.get_level_index();
            node->m_queueLocalExecIdx = m_queueNodeCounters[node->m_queueIndex]++;

            m_nodesInGlobalExecOrder[globalExecIdx] = node;

            dependencyLevel.m_nodesPerQueue[node->m_queueIndex].push_back(node);
            m_nodesPerQueue[node->m_queueIndex].push_back(node);

            if (perQueuePreviousNode[node->m_queueIndex])
                node->m_nodesToSyncWith.push_back(perQueuePreviousNode[node->m_queueIndex]);
            
            perQueuePreviousNode[node->m_queueIndex] = node;

            ++globalExecIdx;
            ++localExecIdx;
        }

        for (auto& [viewName, queueIndices] : resourceReadingQueueTracker)
        {
            if (queueIndices.size() > 1)
            {
                for (QueueIndex queueIndex : queueIndices)
                {
                    dependencyLevel.m_queuesInvolvedInCrossQueueResourceReads.insert(queueIndex);
                    dependencyLevel.m_viewsReadByMultipleQueues.insert(viewName);
                }
            }
        }
    }
}

void RenderGraph::remove_redundant_syncs()
{
    struct SyncCoverage
    {
        const Node* nodeToSyncWith = nullptr;
        uint32 nodeToSyncWithIdx = 0;
        std::vector<uint32> syncedQueueIndices;
    };

    for (Node& node : m_passNodes)
        node.m_syncIndices.resize(m_detectedQueueCount, Node::s_invalidSyncIndex);

    for (DependencyLevel& dependencyLevel : m_dependencyLevels)
    {
        for (Node* node : dependencyLevel.m_nodes)
        {
            NodePtrArray closestNodesToSyncWith(m_detectedQueueCount, nullptr);

            for (const Node* dependencyNode : node->m_nodesToSyncWith)
            {
                const Node* currentClosestNode = closestNodesToSyncWith.at(dependencyNode->m_queueIndex);

                if (!currentClosestNode || dependencyNode->m_queueLocalExecIdx > currentClosestNode->m_queueLocalExecIdx)
                    closestNodesToSyncWith[dependencyNode->m_queueIndex] = dependencyNode;
            }

            node->m_nodesToSyncWith.clear();

            for (uint32 queueIdx = 0; queueIdx != m_detectedQueueCount; ++queueIdx)
            {
                const Node* closestNode = closestNodesToSyncWith.at(queueIdx);

                if (!closestNode)
                {
                    const Node* prevNode = closestNodesToSyncWith[node->m_queueIndex];
                    if (prevNode)
                    {
                        uint32 syncIndexFromPrevNode = prevNode->m_syncIndices[queueIdx];
                        node->m_syncIndices[queueIdx] = syncIndexFromPrevNode;
                    }
                }
                else
                {
                    if (closestNode->m_queueIndex != node->m_queueIndex)    
                        node->m_syncIndices[closestNode->m_queueIndex] = closestNode->m_queueLocalExecIdx;
                    node->m_nodesToSyncWith.push_back(closestNode);
                }
            }

            node->m_syncIndices[node->m_queueIndex] = node->m_queueLocalExecIdx;
        }

        for (Node* node : dependencyLevel.m_nodes)
        {
            std::unordered_set<uint32> queuesToSyncWith;
            std::vector<SyncCoverage> syncCoverages;
            std::vector<const Node*> optimalNodesToSyncWith;

            for (const Node* nodeToSync : node->m_nodesToSyncWith)
                queuesToSyncWith.insert(nodeToSync->m_queueIndex);

            while (!queuesToSyncWith.empty())
            {
                uint64 syncsCoveredByNodeMaxCount = 0;

                for (uint32 dependencyNodeIdx = 0; dependencyNodeIdx != node->m_nodesToSyncWith.size(); ++dependencyNodeIdx)
                {
                    const Node* dependencyNode = node->m_nodesToSyncWith[dependencyNodeIdx];
                    std::vector<uint32> syncedQueueIndices;

                    for (uint32 queueIdx : queuesToSyncWith)
                    {
                        uint32 curNodeDesiredSyncIdx = node->m_syncIndices.at(queueIdx);
                        uint32 dependencyNodeSyncIdx = dependencyNode->m_syncIndices.at(queueIdx);

                        FE_CHECK(curNodeDesiredSyncIdx != Node::s_invalidSyncIndex);

                        if (queueIdx == node->m_queueIndex)
                            curNodeDesiredSyncIdx -= 1; // Because same queue has current node sync index 

                        if (dependencyNodeSyncIdx != Node::s_invalidSyncIndex && dependencyNodeSyncIdx >= curNodeDesiredSyncIdx)
                            syncedQueueIndices.push_back(queueIdx);
                    }

                    syncCoverages.emplace_back(dependencyNode, dependencyNodeIdx, syncedQueueIndices);
                    syncsCoveredByNodeMaxCount = std::max(syncsCoveredByNodeMaxCount, syncedQueueIndices.size());
                }

                for (const SyncCoverage& syncCoverage : syncCoverages)
                {
                    const Node* nodeToSyncWith = syncCoverage.nodeToSyncWith;

                    if (syncsCoveredByNodeMaxCount >= syncCoverage.syncedQueueIndices.size())
                    {
                        if (nodeToSyncWith->m_queueIndex != node->m_queueIndex)
                        {
                            optimalNodesToSyncWith.push_back(syncCoverage.nodeToSyncWith);
                            node->m_syncIndices[nodeToSyncWith->m_queueIndex] = nodeToSyncWith->m_queueIndex;
                        }

                        for (uint32 syncedQueueIdx : syncCoverage.syncedQueueIndices)
                            queuesToSyncWith.erase(syncedQueueIdx);
                    }

                }

                for (auto syncCoverageIt = syncCoverages.rbegin(); syncCoverageIt != syncCoverages.rend(); ++syncCoverageIt)
                    node->m_nodesToSyncWith.erase(node->m_nodesToSyncWith.begin() + syncCoverageIt->nodeToSyncWithIdx);
            }

            node->m_nodesToSyncWith = optimalNodesToSyncWith;
        }
    }
}

RenderGraph::Node::Node(const RenderPassInfo& info, WriteDependencyRegistry* writeDependencyRegistry)
    : m_renderPassInfo(info), m_writeDependencyRegistry(writeDependencyRegistry)
{
    m_queueIndex = std::underlying_type_t<RenderPassType>(m_renderPassInfo.type);
}

void RenderGraph::Node::add_read_dependency(ResourceName resourceName, uint32 viewCount)
{
    FE_CHECK(viewCount > 0);
    add_read_dependency(resourceName, 0, viewCount - 1);
}

void RenderGraph::Node::add_read_dependency(ResourceName resourceName, uint32 firstViewIndex, uint32 lastViewIndex)
{
    for (uint32 i = firstViewIndex; i <= lastViewIndex; ++i)
    {
        ViewName viewName = encode_view_name(resourceName, i);
        m_readViews.insert(viewName);
        m_allViews.insert(viewName);
    }
}

void RenderGraph::Node::add_write_dependency(ResourceName resourceName, uint32 viewCount)
{
    FE_CHECK(viewCount > 0);
    add_write_dependency(resourceName, 0, viewCount - 1);
}

void RenderGraph::Node::add_write_dependency(ResourceName resourceName, uint32 firstViewIndex, uint32 lastViewIndex)
{
    for (uint32 i = firstViewIndex; i <= lastViewIndex; ++i)
    {
        ViewName viewName = encode_view_name(resourceName, i);
        check_single_write_dependency(viewName);
        m_writtenViews.insert(viewName);
        m_allViews.insert(viewName);
    }
}

void RenderGraph::Node::check_single_write_dependency(ViewName name)
{
    std::pair<ResourceName, uint32> resourceName = decode_view_name(name);
    auto it = m_writeDependencyRegistry->find(name);
    if (it != m_writeDependencyRegistry->end())
    {
        FE_LOG(LogRenderer, FATAL, "Resource {} can't be written in {} because it already has a write dependency in {}.", 
            resourceName.first, m_renderPassInfo.renderPassName, it->second);    
    }

    m_writeDependencyRegistry->insert({name, m_renderPassInfo.renderPassName});
}

void RenderGraph::Node::clear()
{
    m_readViews.clear();
    m_writtenViews.clear();
    m_allViews.clear();
    m_syncIndices.clear();
    m_nodesToSyncWith.clear();
    m_globalExecIndex = 0;
    m_dependencyLevelIndex = 0;
    m_queueLocalExecIdx = 0;
    m_dependencyLevelLocalExecIdx = 0;
    m_syncSignalRequired = false;
}

bool RenderGraph::Node::has_dependency(ResourceName resourceName, uint32 viewIndex) const
{
    return has_dependency(encode_view_name(resourceName, viewIndex));
}

bool RenderGraph::Node::has_dependency(ViewName viewName) const
{
    return m_allViews.find(viewName) != m_allViews.end();
}

bool RenderGraph::Node::has_any_dependency() const
{
    return !m_allViews.empty();
}

void RenderGraph::DependencyLevel::add_node(Node* node)
{
    m_nodes.push_back(node);
}

RenderGraph::Node* RenderGraph::DependencyLevel::remove_node(NodeListIterator it)
{
    Node* node = *it;
    m_nodes.erase(it);
    return node;
}

}