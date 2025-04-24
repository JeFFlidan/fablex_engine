#pragma once

#include "renderer/render_pass.h"

namespace fe::renderer
{

class ObjectMeshTestPass : public RenderPass
{
    FE_DECLARE_OBJECT(ObjectMeshTestPass)

public:
    virtual void create_pipeline() override;
    virtual void execute(rhi::CommandBuffer* cmd) override;

};

}