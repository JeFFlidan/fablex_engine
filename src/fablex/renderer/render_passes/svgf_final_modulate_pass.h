#pragma once

#include "render_graph/render_pass.h"

namespace fe::renderer
{

class SVGFFinalModulatePass : public rg::RenderPass
{
    FE_DECLARE_OBJECT(SVGFFinalModulatePass);

public:
    virtual void create_pipeline() override;
    virtual void execute(rhi::CommandBuffer* cmd) override;
};

}