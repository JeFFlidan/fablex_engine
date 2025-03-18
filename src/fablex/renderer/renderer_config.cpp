#include "renderer_config.h"
#include "rhi/json_serialization.h"

namespace fe::renderer
{

constexpr const char* g_rendererKey = "Renderer";
constexpr const char* g_graphicsAPIKey = "GraphicsAPI";
constexpr const char* g_validationModeKey = "ValidationMode";
constexpr const char* g_renderGraphMetadataPathKey = "RenderGraphConfigPath";

void RendererConfig::init(const nlohmann::json& engineConfigJson)
{
    FE_CHECK(engineConfigJson.contains(g_rendererKey));

    const nlohmann::json& rendererConfigJson = engineConfigJson[g_rendererKey];
    FE_CHECK(rendererConfigJson.contains(g_graphicsAPIKey));
    FE_CHECK(rendererConfigJson.contains(g_renderGraphMetadataPathKey));

    m_graphicsAPI = rendererConfigJson[g_graphicsAPIKey];
    m_renderGraphMetadataPath = rendererConfigJson[g_renderGraphMetadataPathKey];

    // if (rendererConfigJson.contains(g_validationModeKey))
        
}

}