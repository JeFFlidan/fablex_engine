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

    RenderGraphMetadata(const RenderContext* renderContext);

    void deserialize(const std::string& path);

    const TextureMetadata* get_texture_metadata(ResourceName textureName) const;
    const RenderPassMetadata* get_render_pass_metadata(RenderPassName renderPassName) const;
    const PipelineMetadata* get_pipeline_metadata(PipelineName pipelineName) const;

    const RenderPassMetadataMap& get_render_pass_metadata_map() const { return m_renderPassMetadataByName; }

private:
    TextureMetadataMap m_renderTextureMetadataByName;
    RenderPassMetadataMap m_renderPassMetadataByName;
    PipelineMetadataMap m_pipelineMetadataByName;

    ShaderManager* m_shaderManager = nullptr;
};

}