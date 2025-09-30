#pragma once

#include "render_graph/render_pass.h"

namespace fe::renderer
{

class RTTestPass : public rg::RenderPass
{
    FE_DECLARE_OBJECT(RTTestPass)

public:
    virtual void create_pipeline() override;
    virtual void schedule_resources() override;
    virtual void execute(rhi::CommandBuffer* cmd) override;
};

}