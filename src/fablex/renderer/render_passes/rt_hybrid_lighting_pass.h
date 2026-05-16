#pragma once

#include "render_graph/render_pass.h"

namespace fe::renderer
{

class RTHybridLightingPass : public rg::RenderPass
{
    FE_DECLARE_OBJECT(RTHybridLightingPass)

public:
    virtual void create_pipeline() override;
    virtual void schedule_resources() override;
    virtual void execute(CommandBufferRef cmd) override;
};

}