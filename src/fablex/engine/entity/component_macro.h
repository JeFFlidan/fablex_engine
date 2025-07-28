#pragma once

#include "sparse_set.h"
#include "core/object/object.h"

namespace fe::engine
{

#define FE_DECLARE_COMPONENT(ComponentType)     \
    FE_DECLARE_OBJECT(ComponentType)            \
    FE_DECLARE_PROPERTY_REGISTER(ComponentType) \
    inline static SparseSet<ComponenType> s_sparseSet;\
    static ComponentType* create() \
    {   \
        return sparseSet.insert()\
    }   \


#define FE_DEFINE_COMPONENT(ComponentType, ParentComponentType)  \
    FE_DEFINE_OBJECT(ComponentType, ParentComponentType)         \

}