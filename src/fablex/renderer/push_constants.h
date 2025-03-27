#pragma once

#include "common.h"
#include "core/math.h"
#include <variant>
#include <unordered_map>

namespace fe::renderer
{

class PushConstants
{
public:
    using FieldValueVariant = std::variant<
        float,
        int32,
        uint32,
        Float2,
        Float3,
        Float4,
        Int2,
        Int3,
        Int4,
        UInt2,
        UInt3,
        UInt4
    >;

    struct FieldValueInfo
    {
        uint32 size = 0;
        uint32 offset = 0;
        PushConstantType type;
    };

    PushConstants(const PushConstantsMetadata& metadata);

    void set_value(FieldName fieldName, const FieldValueVariant& value);

    const void* get_memory() const { return m_memory.data(); }
    uint64 get_size() const { return m_memory.size(); }

private:
    std::vector<uint8> m_memory;
    std::unordered_map<FieldName, FieldValueInfo> m_fieldValueInfoByName;

    uint32 get_value_size(PushConstantType type);
    const FieldValueInfo* get_field_value_info(FieldName name) const;
    bool is_variant_type_valid(const FieldValueVariant& value, const FieldValueInfo& valueInfo) const;
};

}