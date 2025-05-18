#pragma once

#include "render_pass.h"

namespace fe::renderer
{

class SVGFPass : public RenderPass
{
    FE_DECLARE_OBJECT(SVGFPass);

public:
    virtual void create_pipeline() override;
    virtual void execute(rhi::CommandBuffer* cmd) override;
};

}