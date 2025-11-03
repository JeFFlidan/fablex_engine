#pragma once

#include "render_graph/render_pass.h"

namespace fe::renderer
{

class TrianglePass : public rg::RenderPass
{
    FE_DECLARE_OBJECT(TrianglePass)

public:
    virtual void create_pipeline() override;
    virtual void execute(CommandBufferRef cmd) override;
};

}