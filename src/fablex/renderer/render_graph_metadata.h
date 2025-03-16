#pragma once

#include "common.h"
#include "json.hpp"

namespace fe::renderer
{

class RenderContext;
class ShaderManager;

class RenderGraphMetadata
{
public:
    using TextureMetadataMap = std::unordered_map<ResourceName, TextureMetadata>;
    using RenderPassMetadataMap = std::unordered_map<RenderPassName, RenderPassMetadata>;
    using PipelineMetadataMap = std::unordered_map<PipelineName, PipelineMetadata>;
    using PushConstantsMetadataMap = std::unordered_map<PushConstantsName, PushConstantsMetadata>;

    RenderGraphMetadata(const RenderContext* renderContext);

    void deserialize(const nlohmann::json& json);

    const TextureMetadata* get_texture_metadata(ResourceName textureName) const;
    const PushConstantsMetadata* get_push_constants_metadata(PushConstantsName pushConstantsName) const;
    const RenderPassMetadata* get_render_pass_metadata(RenderPassName renderPassName) const;
    const PipelineMetadata* get_pipeline_metadata(PipelineName pipelineName) const;

    const RenderPassMetadataMap& get_render_pass_metadata_map() const { return m_renderPassMetadataByName; }

private:
    TextureMetadataMap m_renderTextureMetadataByName;
    PushConstantsMetadataMap m_pushConstantsMetadataByName;
    RenderPassMetadataMap m_renderPassMetadataByName;
    PipelineMetadataMap m_pipelineMetadataByName;

    ShaderManager* m_shaderManager = nullptr;
};

}