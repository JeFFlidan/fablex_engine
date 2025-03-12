#pragma once

#include "common.h"
#include "core/object/object.h"

namespace fe::renderer
{

class RenderContext;

struct RenderPassInfo
{
    RenderPassName name;
    RenderPassType type;
};

class RenderPass : public Object
{
    FE_DECLARE_OBJECT(RenderPass)

public:
    virtual ~RenderPass() = default;

    void init(const RenderPassMetadata& metadata, const RenderContext* renderContext);

    virtual void create_pipelines() { }
    virtual void allocate_push_constants() { }
    virtual void schedule_resources() { }
    virtual void execute(rhi::CommandBuffer* cmd) { }

    const RenderPassInfo& get_info() const { return m_info; }

private:
    RenderPassInfo m_info;
    const RenderContext* m_renderContext = nullptr;
};

}