#pragma once

#include "engine/fwd.h"
#include "core/fwd.h"
#include "core/logger.h"
#include <unordered_set>
#include <vector>

namespace fe::editor
{

using EntitySet = std::unordered_set<engine::Entity*>;
using PropertyArray = std::vector<Property*>;

FE_DEFINE_LOG_CATEGORY(LogEditor);

}