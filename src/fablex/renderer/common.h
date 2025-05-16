#pragma once

#include "core/name.h"
#include "core/logger.h"

namespace fe::renderer
{

using RenderPassName = Name;
using ResourceName = Name;
using PipelineName = Name;
using FieldName = Name;
using PushConstantsName = Name;

FE_DEFINE_LOG_CATEGORY(LogRenderer);

enum class RenderPassType
{
    GRAPHICS,
    COMPUTE
};

}
