#pragma once

#include "render_pass.h"
#include "render_graph_metadata.h"
#include <unordered_map>
#include <unordered_set>

namespace fe::renderer
{

class RenderPassContainer;

class RenderGraph
{
public:
    using ViewName = uint64;
    using QueueIndex = uint64;
    using WriteDependencyRegistry = std::unordered_map<ViewName, RenderPassName>;
    using ViewNameSet = std::unordered_set<ViewName>;
    using QueueIndexSet = std::unordered_set<QueueIndex>;

    class Node
    {
        friend RenderGraph;

    public:
        using NodePtrArray = std::vector<const Node*>;
        constexpr static uint32 s_invalidSyncIndex = ~0u;

        Node(const RenderPassInfo& info, WriteDependencyRegistry* writeDependencyRegistry);

        void add_read_dependency(ResourceName resourceName, uint32 viewCount);
        void add_read_dependency(ResourceName resourceName, uint32 firstViewIndex, uint32 lastViewIndex);

        void add_write_dependency(ResourceName resourceName, uint32 viewCount);
        void add_write_dependency(ResourceName resourceName, uint32 firstViewIndex, uint32 lastViewIndex);

        void clear();

        bool has_dependency(ResourceName resourceName, uint32 viewIndex) const;
        bool has_dependency(ViewName viewName) const;
        bool has_any_dependency() const;

        const RenderPassInfo& get_info() const { return m_renderPassInfo; }
        const ViewNameSet& get_read_views() const { return m_readViews; }
        const ViewNameSet& get_written_views() const { return m_writtenViews; }
        const ViewNameSet& get_all_views() const { return m_allViews; }
        bool is_sync_required() const { return m_syncSignalRequired; }
        const NodePtrArray& get_nodes_to_sync_with() const { return m_nodesToSyncWith; }
        uint32 get_dependency_level_index() const { return m_dependencyLevelIndex; }
        uint32 get_queue_index() const { return m_queueIndex; }

    private:
        using SyncIndexArray = std::vector<uint32>;

        RenderPassInfo m_renderPassInfo;

        ViewNameSet m_readViews;
        ViewNameSet m_writtenViews;
        ViewNameSet m_allViews;

        WriteDependencyRegistry* m_writeDependencyRegistry = nullptr;

        uint32 m_indexInUnorderedArray = 0;
        uint32 m_globalExecIndex = 0;
        uint32 m_dependencyLevelLocalExecIdx = 0;
        uint32 m_queueLocalExecIdx = 0;
        uint32 m_dependencyLevelIndex = 0;
        uint32 m_queueIndex = 0;

        SyncIndexArray m_syncIndices;
        bool m_syncSignalRequired = false;
        NodePtrArray m_nodesToSyncWith;

        void check_single_write_dependency(ViewName name);
    };

    using NodeArray = std::vector<Node>;
    using NodePtrArray = Node::NodePtrArray;
    using OrderedNodeArray = std::vector<Node*>;

    class DependencyLevel
    {
        friend RenderGraph;

    public:
        using NodeList = std::list<Node*>;
        using NodeListIterator = NodeList::iterator;

        uint32 get_level_index() const { return m_levelIndex; }
        const NodeList& get_nodes() const { return m_nodes; }
        const NodePtrArray& get_nodes_for_queue(QueueIndex index) const { return m_nodesPerQueue.at(index); }
        const ViewNameSet& get_views_read_by_multiple_queues() { return m_viewsReadByMultipleQueues;}
        const QueueIndexSet& get_queues_involved_in_cross_queue_resource_reads() const 
            { return m_queuesInvolvedInCrossQueueResourceReads; }

        const NodePtrArray& get_nodes_for_queue(RenderPassType renderPassType)
        {
            return m_nodesPerQueue.at(std::underlying_type_t<RenderPassType>(renderPassType));
        }

    private:
        NodeList m_nodes;
        std::vector<NodePtrArray> m_nodesPerQueue;
        uint32 m_levelIndex = 0;

        ViewNameSet m_viewsReadByMultipleQueues;
        QueueIndexSet m_queuesInvolvedInCrossQueueResourceReads;

        void add_node(Node* node);
        Node* remove_node(NodeListIterator it);
    };

    void load_from_metadata(const std::string& metadataPath, RenderPassContainer* renderPassContainer);

    void add_node(const RenderPassInfo& info);

    void build();
    void clear();

    Node* get_node(RenderPassName renderPassName) const;
    const RenderGraphMetadata* get_metadata() const { return m_metadata.get(); }
    const OrderedNodeArray& get_nodes_in_global_exec_order() const { return m_nodesInGlobalExecOrder; }
    const NodeArray& get_nodes() const { return m_passNodes; }
    NodeArray& get_nodes() { return m_passNodes; }
    uint32 get_detected_queue_count() const { return m_detectedQueueCount; }

private:
    using NodeMap = std::unordered_map<RenderPassName, uint32>;
    using AdjacencyArray = std::vector<uint32>;
    using AdjacencyArrays = std::vector<AdjacencyArray>;
    using DependencyLevelArray = std::vector<DependencyLevel>;
    using QueueNodeCounters = std::unordered_map<QueueIndex, uint32>;

    std::unique_ptr<RenderGraphMetadata> m_metadata;

    NodeMap m_nodeIndexByName;
    NodeArray m_passNodes;
    AdjacencyArrays m_adjacencyArrays;
    DependencyLevelArray m_dependencyLevels;

    WriteDependencyRegistry m_globalWriteDependencyRegistry;

    QueueNodeCounters m_queueNodeCounters;
    OrderedNodeArray m_topologicallySortedNodes;
    OrderedNodeArray m_nodesInGlobalExecOrder;
    std::vector<NodePtrArray> m_nodesPerQueue;
    uint32 m_detectedQueueCount = 1;

    void build_adjacency_lists();
    void depth_first_search(uint64 nodeIdx, std::vector<bool>& visited, std::vector<bool>& onStack, bool& isCyclic);
    void topological_sort();
    void build_dependency_levels();
    void finalize_dependency_levels();
    void remove_redundant_syncs();

    static ViewName encode_view_name(ResourceName resourceName, uint32 viewIndex);
    static std::pair<ResourceName, uint32> decode_view_name(ViewName viewName);
};

}