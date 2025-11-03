#pragma once

#include "rhi_types.h"
#include "json.hpp"

namespace fe::renderer
{

class RendererConfig
{
public:
    void init(const nlohmann::json& engineConfigJson);

    API get_graphics_api() const { return m_graphicsAPI; }
    ValidationMode get_validation_mode() const { return m_validationMode; }

    // Returns relative path
    const std::string& get_render_graph_metadata_path() const { return m_renderGraphMetadataPath; }

private:
    API m_graphicsAPI = API::VK;
    ValidationMode m_validationMode = ValidationMode::ENABLED;
    std::string m_renderGraphMetadataPath;
};

}