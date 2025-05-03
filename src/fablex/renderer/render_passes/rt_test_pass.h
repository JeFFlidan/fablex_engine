#pragma once

#include "render_pass.h"

namespace fe::renderer
{

class RTTestPass : public RenderPass
{
    FE_DECLARE_OBJECT(RTTestPass)

public:
    virtual void create_pipeline() override;
    virtual void execute(rhi::CommandBuffer* cmd) override;

protected:
    virtual void schedule_resources_internal() override;
};

}