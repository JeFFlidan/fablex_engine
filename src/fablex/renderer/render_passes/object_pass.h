#pragma once

#include "render_graph/render_pass.h"

namespace fe::renderer
{

class ObjectPass : public rg::RenderPass
{
    FE_DECLARE_OBJECT(ObjectPass)

public:
    virtual void create_pipeline() override;
    virtual void execute(CommandBufferRef cmd) override;
};

}