#pragma once

#include "core/compile_time_hash.h"

namespace fe
{

#define EVENT_TYPE_HASH(x) compile_time_fnv1(#x)

#define FE_DECLARE_EVENT(x)							        \
    enum class EventType : uint64_t							\
    {														\
        TYPE_ID = EVENT_TYPE_HASH(x)						\
    };														\
    static inline constexpr uint64_t get_type_id_static()	\
    {														\
        return static_cast<uint64_t>(EventType::TYPE_ID);	\
    }														\
    virtual uint64_t get_type_id() const override		    \
    {														\
        return static_cast<uint64_t>(EventType::TYPE_ID);	\
    }

    class IEvent
    {
        public:
            virtual ~IEvent() { }
            virtual uint64 get_type_id() const = 0;
    };

}