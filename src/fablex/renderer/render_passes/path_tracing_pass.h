#pragma once

#include "render_graph/render_pass.h"

namespace fe::renderer
{

class PathTracingPass : public rg::RenderPass
{
    FE_DECLARE_OBJECT(PathTracingPass);

public:
    PathTracingPass();

    virtual void create_pipeline() override;
    virtual void schedule_resources() override;
    virtual void execute(CommandBufferRef cmd) override;
    
private:
    uint32 m_accumulationFactor = 0;
};

}