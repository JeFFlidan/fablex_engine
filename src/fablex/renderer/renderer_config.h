#pragma once

#include "rhi/rhi.h"
#include "json.hpp"

namespace fe::renderer
{

class RendererConfig
{
public:
    void init(const nlohmann::json& engineConfigJson);

    rhi::API get_graphics_api() const { return m_graphicsAPI; }
    rhi::ValidationMode get_validation_mode() const { return m_validationMode; }

    // Returns relative path
    const std::string& get_render_graph_metadata_path() const { return m_renderGraphMetadataPath; }

private:
    rhi::API m_graphicsAPI = rhi::API::VK;
    rhi::ValidationMode m_validationMode = rhi::ValidationMode::ENABLED;
    std::string m_renderGraphMetadataPath;
};

}