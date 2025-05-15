#pragma once

#include "render_pass.h"

namespace fe::renderer
{

class SVGFFilterMomentsPass : public RenderPass
{
    FE_DECLARE_OBJECT(SVGFFilterMomentsPass);

public:
    virtual void create_pipeline() override;
    virtual void execute(rhi::CommandBuffer* cmd) override;
};

}