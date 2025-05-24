#pragma once

#include "core/event.h"

namespace fe::engine
{

class ModelEntityCreationRequest : public IEvent
{
public:
    FE_DECLARE_EVENT(ModelCreationRequest);
};

}