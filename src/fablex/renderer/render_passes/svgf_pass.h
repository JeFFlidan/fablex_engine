#pragma once

#include "render_graph/render_pass.h"

namespace fe::renderer
{

class SVGFPass : public rg::RenderPass
{
    FE_DECLARE_OBJECT(SVGFPass);

public:
    virtual void create_pipeline() override;
    virtual void execute(CommandBufferRef cmd) override;
};

}