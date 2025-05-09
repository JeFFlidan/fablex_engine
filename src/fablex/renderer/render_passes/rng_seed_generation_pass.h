#pragma once

#include "render_pass.h"

namespace fe::renderer
{

class RNGSeedGenerationPass : public RenderPass
{
    FE_DECLARE_OBJECT(RNGSeedGenerationPass);

public:
    virtual void create_pipeline() override;
    virtual void execute(rhi::CommandBuffer* cmd) override;
};

}