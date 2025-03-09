#include "test_pass.h"
#include "core/logger.h"

namespace fe::renderer
{

FE_DEFINE_OBJECT(TestPass, RenderPass)

void TestPass::execute(rhi::CommandBuffer* cmd)
{
    FE_LOG(LogRenderer, INFO, "Test Pass is active !!! {}", s_typeInfo.get_name());
}

}
