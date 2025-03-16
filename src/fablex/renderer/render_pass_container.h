#pragma once

#include "common.h"
#include <unordered_map>

namespace fe::renderer
{

class RenderPass;
class RenderContext;

class RenderPassContainer
{
public:
    using RenderPassMap = std::unordered_map<RenderPassName, RenderPass*>;

    RenderPassContainer(const RenderContext* renderContext);
    RenderPass* add_render_pass(const RenderPassMetadata& metadata);
    void cleanup();

    RenderPass* get_render_pass(RenderPassName name) const;
    RenderPassMap& get_render_passes() { return m_renderPassByName; }
    const RenderPassMap& get_render_pass_map() const { return m_renderPassByName; }
    const RenderContext* get_render_context() const { return m_renderContext; }

private:
    const RenderContext* m_renderContext;
    RenderPassMap m_renderPassByName;

    RenderPass* create_render_pass(const RenderPassMetadata& metadata);
};

}