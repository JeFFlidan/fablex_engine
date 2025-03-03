#pragma once

#include "common.h"
#include "json.hpp"

namespace fe::renderer
{

class RenderGraphMetadata
{
public:
    void deserialize(const nlohmann::json& json);

    const TextureMetadata* get_texture_metadata(ResourceName textureName) const;
    const RenderPassMetadata* get_render_pass_metadata(RenderPassName renderPassName) const;
    const PipelineMetadata* get_pipeline_metadata(PipelineName pipelineName) const;

private:
    using TextureMetadataMap = std::unordered_map<ResourceName, TextureMetadata>;
    using RenderPassMetadataMap = std::unordered_map<RenderPassName, RenderPassMetadata>;
    using PipelineMetadataMap = std::unordered_map<PipelineName, PipelineMetadata>;

    TextureMetadataMap m_renderTextureMetadataByName;
    RenderPassMetadataMap m_renderPassMetadataByName;
    PipelineMetadataMap m_pipelineMetadataByName;
};

}