#include "render_pass_container.h"
#include "render_pass.h"

namespace fe::renderer
{

RenderPassContainer::RenderPassContainer(const RenderContext* renderContext)
{
    FE_CHECK(renderContext);
    m_renderContext = renderContext;
}

RenderPass* RenderPassContainer::add_render_pass(const RenderPassMetadata& metadata)
{
    auto it = m_renderPassByName.find(metadata.renderPassName);
    if (it != m_renderPassByName.end())
        return it->second;

    RenderPass* renderPass = create_render_pass(metadata);
    m_renderPassByName[metadata.renderPassName.to_string()] = renderPass;
    return renderPass;
}

void RenderPassContainer::cleanup()
{
    for (auto it : m_renderPassByName)
        destroy_object(it.second);
}

RenderPass* RenderPassContainer::get_render_pass(RenderPassName name) const
{
    auto it = m_renderPassByName.find(name);
    if (it == m_renderPassByName.end())
        return nullptr;
    return it->second;
}

RenderPass* RenderPassContainer::create_render_pass(const RenderPassMetadata& metadata)
{
    std::string name = metadata.renderPassName.to_string();
    if (name.find("Pass") == std::string::npos)
        name += "Pass";

    RenderPass* renderPass = (RenderPass*)create_object(name);
    renderPass->init(metadata, m_renderContext);

    return renderPass;
}

}