#pragma once

#include "render_pass.h"

namespace fe::renderer
{

class TestPass : public RenderPass
{
    FE_DECLARE_OBJECT(TestPass)

public:
    virtual void execute(rhi::CommandBuffer* cmd) override;
};

}