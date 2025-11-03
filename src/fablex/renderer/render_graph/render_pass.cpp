#include "render_pass.h"
#include "render_context.h"
#include "rendering_infos.h"
#include "render_graph_metadata.h"
#include "render_graph.h"
#include "resource_manager.h"
#include "resource_scheduler.h"
#include "globals.h"
#include "utils.h"
#include "rhi/utils.h"
#include "rhi/rhi.h"

namespace fe::renderer::rg
{

FE_DEFINE_OBJECT(RenderPass, Object)

constexpr const char* RESOURCE_NAME_FORMAT_STR = "{}-{}";

void RenderPass::init(const RenderPassMetadata& metadata, const RenderContext* renderContext)
{
    FE_CHECK(renderContext);
    m_renderContext = renderContext;
    m_metadata = &metadata;
}

void RenderPass::create_pipelines()
{
    PipelineManager* pipelineManager = m_renderContext->pipeline_manager();
    PipelineName pipelineName = m_metadata->pipelineName;
    uint32 index = 0;

    while (true)
    {
        std::string pipelineNameStr = get_name_at_index(pipelineName, index++);

        if (pipelineNameStr.empty())
            break;

        const PipelineMetadata& pipelineMetadata = get_pipeline_metadata(pipelineNameStr);
        const ShaderMetadata& shaderMetadata = pipelineMetadata.shadersMetadata.at(0);

        if (shaderMetadata.type == ShaderType::COMPUTE)
            pipelineManager->create_compute_pipeline(pipelineMetadata);
        else if (rhi::is_rt_shader(shaderMetadata.type))
            pipelineManager->create_ray_tracing_pipeline(pipelineMetadata);
        else
            pipelineManager->create_compute_pipeline(pipelineMetadata);  
    }
}

void RenderPass::schedule_resources()
{
    FE_CHECK(m_metadata);

    for (ResourceName textureName : m_metadata->inputTextureNames)
    {
        const TextureMetadata& textureMetadata = get_texture_metadata(textureName);

        if (textureMetadata.has_flag(ResourceMetadataFlag::CROSS_FRAME_READ))
        {
            auto [prevFrameName, curFrameName] = get_resource_names_xfr(textureMetadata.name);
            ResourceScheduler::read_texture(name(), prevFrameName);
            ResourceScheduler::read_texture(name(), curFrameName);
        }
        else if (textureMetadata.has_flag(ResourceMetadataFlag::CROSS_FRAME_READ_NO_HISTORY))
        {
            ResourceScheduler::read_previous_texture(name(), &textureMetadata);
        }
        else if (textureMetadata.has_flag(ResourceMetadataFlag::PING_PONG))
        {
            ResourceName pingPong0 = textureName.to_string() + "0";
            ResourceScheduler::read_texture(name(), pingPong0);
        }
        else
        {
            ResourceScheduler::read_texture(name(), textureName);
        }
    }

    for (const RenderTargetMetadata& renderTargetMetadata : m_metadata->renderTargetsMetadata)
    {
        if (renderTargetMetadata.textureName.is_valid())
        {     
            const TextureMetadata& textureMetadata = get_texture_metadata(renderTargetMetadata.textureName);
            
            if (textureMetadata.has_flag(ResourceMetadataFlag::CROSS_FRAME_READ))
            {
                auto [prevFrameName, curFrameName] = get_resource_names_xfr(textureMetadata.name);
                if (rhi::is_depth_stencil_format(textureMetadata.format))
                {
                    ResourceScheduler::create_depth_stencil(name(), &textureMetadata, prevFrameName);
                    ResourceScheduler::create_depth_stencil(name(), &textureMetadata, curFrameName);
                }
                else
                {
                    ResourceScheduler::create_render_target(name(), &textureMetadata, prevFrameName);
                    ResourceScheduler::create_render_target(name(), &textureMetadata, curFrameName);
                }
            }
            else
            {
                if (rhi::is_depth_stencil_format(textureMetadata.format))
                {
                    ResourceScheduler::create_depth_stencil(name(), &textureMetadata);
                }
                else
                {
                    ResourceScheduler::create_render_target(name(), &textureMetadata);
                }
            }
        }
        else
        {
            ResourceScheduler::write_to_back_buffer(name());
        }
    }

    for (ResourceName textureName : m_metadata->outputStorageTextureNames)
    {
        const TextureMetadata& textureMetadata = get_texture_metadata(textureName);

        if (textureMetadata.has_flag(ResourceMetadataFlag::CROSS_FRAME_READ))
        {
            auto [prevFrameName, curFrameName] = get_resource_names_xfr(textureName);
            ResourceScheduler::read_previous_texture(name(), &textureMetadata, prevFrameName);
            ResourceScheduler::create_storage_texture(name(), &textureMetadata, curFrameName);
        }
        else if (textureMetadata.has_flag(ResourceMetadataFlag::CROSS_FRAME_READ_NO_HISTORY))
        {
            ResourceScheduler::create_storage_texture(name(), &textureMetadata);
        }
        else if (textureMetadata.has_flag(ResourceMetadataFlag::PING_PONG))
        {
            ResourceName pingPong0 = textureName.to_string() + "0";
            ResourceName pingPong1 = textureName.to_string() + "1";
            ResourceScheduler::create_storage_texture(name(), &textureMetadata, pingPong0);
            ResourceScheduler::create_storage_texture(name(), &textureMetadata, pingPong1);

            s_pingPongResourceRegistry[textureName] = {pingPong0, pingPong1};
        }
        else
        {
            ResourceScheduler::create_storage_texture(name(), &textureMetadata);
        }
    }
}

RenderPassInfo RenderPass::info() const
{
    return RenderPassInfo(
        m_metadata->name,
        m_metadata->pipelineName,
        m_metadata->type
    );
}

void RenderPass::begin_rendering(CommandBufferRef cmd)
{
    OffscreenPassRenderingInfo renderingInfo; 

    for (const RenderTargetMetadata& renderTargetMetadata : m_metadata->renderTargetsMetadata)
    {
        const Texture& texture = get_texture(renderTargetMetadata.textureName);
        const TextureMetadata& textureMetadata = get_texture_metadata(renderTargetMetadata.textureName);
        renderingInfo.add_render_target(texture, textureMetadata, renderTargetMetadata);
    }
    
    cmd.begin_rendering(renderingInfo);
}

void RenderPass::begin_rendering(CommandBufferRef cmd, SwapChainRef swapChain)
{
    SwapChainPassRenderingInfo renderingInfo(swapChain);

    for (const RenderTargetMetadata& renderTargetMetadata : m_metadata->renderTargetsMetadata)
    {
        renderingInfo.set_render_target(renderTargetMetadata);
        break;
    }

    cmd.begin_rendering(renderingInfo);
}

void RenderPass::end_rendering(CommandBufferRef cmd)
{
    cmd.end_rendering(nullptr);
}

void RenderPass::end_rendering(CommandBufferRef cmd, SwapChainRef swapChain)
{
    cmd.end_rendering(swapChain);
}

RenderPassName RenderPass::name() const
{
    return m_metadata->name;
}

void RenderPass::create_compute_pipeline()
{
    m_renderContext->pipeline_manager()->create_compute_pipeline(get_pipeline_metadata());
}
    
void RenderPass::create_graphics_pipeline()
{
    m_renderContext->pipeline_manager()->create_graphics_pipeline(get_pipeline_metadata());
}

void RenderPass::create_graphics_pipeline(const PipelineManager::GraphicsPipelineConfigurator& configurator)
{
    m_renderContext->pipeline_manager()->create_graphics_pipeline(get_pipeline_metadata(), configurator);
}

void RenderPass::create_ray_tracing_pipeline()
{
    m_renderContext->pipeline_manager()->create_ray_tracing_pipeline(get_pipeline_metadata());
}

void RenderPass::create_ray_tracing_pipeline(const PipelineManager::RayTracingPipelineConfigurator& configurator)
{
    m_renderContext->pipeline_manager()->create_ray_tracing_pipeline(get_pipeline_metadata(), configurator);
}

SceneManager* RenderPass::scene_manager() const
{
    return m_renderContext->scene_manager();
}

void RenderPass::bind_pipeline(CommandBufferRef cmd)
{
    m_renderContext->pipeline_manager()->bind_pipeline(cmd, m_metadata->pipelineName);
}

void RenderPass::bind_pipeline(CommandBufferRef cmd, uint32 pipelineIndex)
{
    m_renderContext->pipeline_manager()->bind_pipeline(cmd, get_name_at_index(m_metadata->pipelineName, pipelineIndex));
}

void RenderPass::push_constants(CommandBufferRef cmd, void* data)
{
    m_renderContext->pipeline_manager()->push_constants(cmd, m_metadata->pipelineName, data);
}

void RenderPass::push_constants(CommandBufferRef cmd, void* data, uint32 pipelineIndex)
{
    m_renderContext->pipeline_manager()->push_constants(cmd, get_name_at_index(m_metadata->pipelineName, pipelineIndex), data);
}

void RenderPass::set_default_viewport_and_scissor(CommandBufferRef cmd) const
{
    m_renderContext->render_surface().set_default_viewport(cmd);
    m_renderContext->render_surface().set_default_scissor(cmd);
}

void RenderPass::set_viewport_and_scissor_by_window(CommandBufferRef cmd) const
{
    m_renderContext->render_surface().set_viewport_by_window(cmd);
    m_renderContext->render_surface().set_scissor_by_window(cmd);
}

void RenderPass::fill_dispatch_rays_info(DispatchRaysInfo& outInfo) const
{
    m_renderContext->pipeline_manager()->fill_dispatch_rays_info(m_metadata->pipelineName, outInfo);
}

DispatchSizes dispatch_group_count(const DispatchSizes& threadCounts, const DispatchSizes& groupSizes)
{
    uint32 x = std::max(ceilf((float)threadCounts.x / groupSizes.x), 1.0f);
    uint32 y = std::max(ceilf((float)threadCounts.y / groupSizes.y), 1.0f);
    uint32 z = std::max(ceilf((float)threadCounts.z / groupSizes.z), 1.0f);

    return {x, y, z};
}

void RenderPass::dispatch(CommandBufferRef cmd, const RenderSurface& surface, const DispatchSizes& groupSizes)
{
    DispatchSizes groupCount = dispatch_group_count({surface.width, surface.height, 1}, groupSizes);
    cmd.dispatch(groupCount);
}

const RenderGraphMetadata& RenderPass::get_render_graph_metadata() const
{
    return *m_renderContext->render_graph()->metadata();
}

const PipelineMetadata& RenderPass::get_pipeline_metadata() const
{
    const RenderGraphMetadata* renderGraphMetadata = m_renderContext->render_graph()->metadata();
    const PipelineMetadata* pipelineMetadata = renderGraphMetadata->get_pipeline_metadata(m_metadata->pipelineName);
    if (!pipelineMetadata)
        FE_LOG(LogRenderer, FATAL, "No pipeline metadata for name {}", m_metadata->pipelineName);
    return *pipelineMetadata;
}

const PipelineMetadata& RenderPass::get_pipeline_metadata(Name pipelineName) const
{
    const RenderGraphMetadata* renderGraphMetadata = m_renderContext->render_graph()->metadata();
    const PipelineMetadata* pipelineMetadata = renderGraphMetadata->get_pipeline_metadata(pipelineName);
    if (!pipelineMetadata)
        FE_LOG(LogRenderer, FATAL, "No pipeline metadata for name {}", pipelineName);
    return *pipelineMetadata;
}

const TextureMetadata& RenderPass::get_texture_metadata(ResourceName textureName) const
{
    FE_CHECK(textureName.is_valid());

    const RenderGraphMetadata* renderGraphMetadata = m_renderContext->render_graph()->metadata();
    const TextureMetadata* textureMetadata = renderGraphMetadata->get_texture_metadata(textureName);
    if (!textureMetadata)
        FE_LOG(LogRenderer, FATAL, "No metadata for resource with name {}", textureName);

    return *textureMetadata;
}

const Texture& RenderPass::get_texture(ResourceName textureName) const
{
    FE_CHECK(textureName.is_valid());
    const Texture& texture = m_renderContext->render_graph_resource_manager()->get_resource(textureName)->texture();
    FE_CHECK(texture.handle());

    return texture;
}

RenderPass::ResourceNamesXFR RenderPass::get_resource_names_xfr(ResourceName baseName) const
{
    return ResourceNamesXFR{
        get_prev_frame_resource_name(baseName),
        get_curr_frame_resource_name(baseName)
    };
}

ResourceName RenderPass::get_prev_frame_resource_name(ResourceName baseName) const
{
    return ResourceName(Utils::create_resource_name(RESOURCE_NAME_FORMAT_STR, baseName.to_string(), get_prev_frame_index()));
}

ResourceName RenderPass::get_curr_frame_resource_name(ResourceName baseName) const
{
    return ResourceName(Utils::create_resource_name(RESOURCE_NAME_FORMAT_STR, baseName.to_string(), get_curr_frame_index()));
}

std::string RenderPass::get_name_at_index(Name name, uint32 index) const
{
    uint32 start = 0;
    uint32 currentIndex = 0;

    std::string nameStr = name.to_string();

    for (uint32 i = 0; i <= nameStr.size(); ++i) 
    {
        if (i == nameStr.size() || nameStr[i] == '|') 
        {
            if (currentIndex == index)
                return nameStr.substr(start, i - start);

            start = i + 1;
            ++currentIndex;
        }
    }

    return {};
}

void RenderPass::fill_push_constants(PushConstantsName pushConstantsName, void* data) const
{
    const RenderGraphMetadata& renderGraphMetadata = get_render_graph_metadata();
    const PushConstantsMetadata* pushConstantsMetadata = renderGraphMetadata.get_push_constants_metadata(pushConstantsName);

    if (!pushConstantsMetadata)
    {
        FE_LOG(LogRenderer, WARNING, "No push constants metadata with name {}", pushConstantsName);
        return;
    }

    uint8* typedData = static_cast<uint8*>(data);
    uint64 offset = 0;

    ResourceManager* resourceManager = m_renderContext->render_graph_resource_manager();
    std::unordered_set<ResourceName> pingPongResources;

    for (const auto& resourceMetadata : pushConstantsMetadata->resourcesMetadata)
    {
        const TextureMetadata* textureMetadata = renderGraphMetadata.get_texture_metadata(resourceMetadata.name);
        if (!textureMetadata)
            FE_LOG(LogRenderer, FATAL, "No texture metadata with name {}", resourceMetadata.name);

        ResourceName resourceName = resourceMetadata.name;

        if (textureMetadata->has_flag(ResourceMetadataFlag::CROSS_FRAME_READ))
        {
            if (resourceMetadata.has_flag(ResourceMetadataFlag::PREVIOUS_FRAME))
                resourceName = get_prev_frame_resource_name(resourceName);
            else
                resourceName = get_curr_frame_resource_name(resourceName);
        }

        bool isWritePingPong = false;

        if (textureMetadata->has_flag(ResourceMetadataFlag::PING_PONG))
        {
            isWritePingPong = true;

            if (resourceMetadata.has_flag(ResourceMetadataFlag::PING_PONG_0))
            {
                pingPongResources.insert(resourceName);
                resourceName = get_ping_pong_0(resourceName);
            }
            else if (resourceMetadata.has_flag(ResourceMetadataFlag::PING_PONG_1))
            {
                pingPongResources.insert(resourceName);
                resourceName = get_ping_pong_1(resourceName);
            }
            else
            {
                isWritePingPong = false;
                resourceName = resourceName.to_string() + "0";
            }
        }

        uint32 descriptor = ~0u;

        if (resourceMetadata.has_flag(ResourceMetadataFlag::WRITABLE) || isWritePingPong)
            descriptor = resourceManager->texture_uav_descriptor(name(), resourceName);
        else
            descriptor = resourceManager->texture_srv_descriptor(name(), resourceName);

        memcpy(typedData + offset, &descriptor, sizeof(uint32));
        offset += sizeof(uint32);
    }

    for (ResourceName name : pingPongResources)
        swap_ping_pong(name);
}

ResourceName RenderPass::get_ping_pong_0(ResourceName baseName) const
{
    return get_ping_pong_names(baseName).first;
}

ResourceName RenderPass::get_ping_pong_1(ResourceName baseName) const
{
    return get_ping_pong_names(baseName).second;
}

void RenderPass::swap_ping_pong(ResourceName baseName) const
{
    auto& pingPongNames = get_ping_pong_names(baseName);
    std::swap(pingPongNames.first, pingPongNames.second);
}

RenderPass::ResourcePingPongNames& RenderPass::get_ping_pong_names(ResourceName baseName) const
{
    auto it = s_pingPongResourceRegistry.find(baseName);
    if (it == s_pingPongResourceRegistry.end())
        FE_LOG(LogRenderer, FATAL, "Failed to find ping pong for resource {}", baseName);

    return it->second;
}

}