#pragma once

#include "render_graph/render_pass.h"

namespace fe::renderer
{

class TestPass : public rg::RenderPass
{
    FE_DECLARE_OBJECT(TestPass)

public:
    virtual void execute(CommandBufferRef cmd) override;
};

}