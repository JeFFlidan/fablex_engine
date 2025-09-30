#pragma once

#include "common.h"
#include <unordered_map>

namespace fe::renderer::rg
{

class RenderPassContainer
{
public:
    using RenderPassMap = std::unordered_map<RenderPassName, RenderPass*>;

    RenderPassContainer(const RenderContext* renderContext);
    RenderPass* add_render_pass(const RenderPassMetadata& metadata);
    void cleanup();

    RenderPass* render_pass(RenderPassName name) const;
    RenderPassMap& render_passes() { return m_renderPassByName; }
    const RenderContext* render_context() const { return m_renderContext; }

private:
    const RenderContext* m_renderContext;
    RenderPassMap m_renderPassByName;

    RenderPass* create_render_pass(const RenderPassMetadata& metadata);
};

}