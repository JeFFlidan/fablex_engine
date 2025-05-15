#pragma once

#include "render_pass.h"

namespace fe::renderer
{

class SVGFAtrousPass : public RenderPass
{
    FE_DECLARE_OBJECT(SVGFAtrousPass);
    
public:
    virtual void create_pipeline() override;
    virtual void execute(rhi::CommandBuffer* cmd) override;
};

}