#pragma once

#include "render_pass.h"

namespace fe::renderer
{

class PathTracingPass : public RenderPass
{
    FE_DECLARE_OBJECT(PathTracingPass);

public:
    PathTracingPass();

    virtual void create_pipeline() override;
    virtual void schedule_resources() override;
    virtual void execute(rhi::CommandBuffer* cmd) override;
    
private:
    uint32 m_accumulationFactor = 0;
};

}