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
    RenderPassContainer(const RenderContext* renderContext);
    RenderPass* add_render_pass(const RenderPassMetadata& metadata);
    void cleanup();

    RenderPass* get_render_pass(RenderPassName name) const;

private:
    const RenderContext* m_renderContext;
    std::unordered_map<RenderPassName, RenderPass*> m_renderPassByName;

    RenderPass* create_render_pass(const RenderPassMetadata& metadata);
};

}