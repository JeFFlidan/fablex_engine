#pragma once

#include "render_pass.h"

namespace fe::renderer
{

class TrianglePass : public RenderPass
{
    FE_DECLARE_OBJECT(TrianglePass)

public:
    virtual void create_pipeline() override;
    virtual void execute(rhi::CommandBuffer* cmd) override;
};

}