#pragma once

#include "renderer/render_graph/render_pass.h"

namespace fe::renderer
{

class VBufferPass : public rg::RenderPass
{
    FE_DECLARE_OBJECT(VBufferPass)

public:
    virtual void create_pipeline() override;
    virtual void execute(CommandBufferRef cmd) override;

};

}