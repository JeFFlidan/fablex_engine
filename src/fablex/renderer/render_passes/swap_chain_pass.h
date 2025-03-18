#pragma once

#include "render_pass.h"

namespace fe::renderer
{

class SwapChainPass : public RenderPass
{
    FE_DECLARE_OBJECT(SwapChainPass)

public:
    virtual void create_pipeline() override;
    virtual void execute(rhi::CommandBuffer* cmd) override;
};

}