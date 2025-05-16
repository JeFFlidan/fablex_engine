#pragma once

#include "resource_metadata.h"
#include "json.hpp"

namespace fe::renderer
{

class RenderContext;
class ShaderManager;

class RenderGraphMetadata
{
public:
    RenderGraphMetadata(const RenderContext* renderContext);

    void deserialize(const std::string& path);

    const TextureMetadata* get_texture_metadata(ResourceName textureName) const;
    const RenderPassMetadata* get_render_pass_metadata(RenderPassName renderPassName) const;
    const PipelineMetadata* get_pipeline_metadata(PipelineName pipelineName) const;
    const PushConstantsMetadata* get_push_constants_metadata(PushConstantsName pushConstantsName) const;

    const std::vector<RenderPassMetadata*>& get_render_passes_metadata() const { return m_renderPassesMetadata; }

private:
    std::unordered_map<Name, ResourceMetadataHandle> m_resourceMetadataByName;
    std::vector<RenderPassMetadata*> m_renderPassesMetadata;

    ShaderManager* m_shaderManager = nullptr;

    template<typename MetadataType>
    MetadataType& add_metadata(Name name)
    {
        auto it = m_resourceMetadataByName.emplace(name, new MetadataType());
        return *static_cast<MetadataType*>(it.first->second.get());
    }

    void parse_flags(const nlohmann::json& metadataJson, ResourceMetadataWithFlags<Name>& metadata);
};

}