#include "push_constants.h"
#include "common.h"
#include "core/logger.h"

namespace fe::renderer
{

PushConstants::PushConstants(const PushConstantsMetadata& metadata)
{
    uint64 size = 0;

    for (const PushConstantsMetadata::FieldMetadata& fieldMetadata : metadata.fieldsMetadata)
    {
        FieldValueInfo& fieldValueInfo = m_fieldValueInfoByName[fieldMetadata.name];
        fieldValueInfo.type = fieldMetadata.type;
        fieldValueInfo.offset = size;
        fieldValueInfo.size = get_value_size(fieldValueInfo.type);

        size += fieldValueInfo.size;
    }

    m_memory.resize(size);
}

void PushConstants::set_value(FieldName fieldName, const FieldValueVariant& value)
{
    const FieldValueInfo* fieldValueInfo = get_field_value_info(fieldName);
    FE_CHECK(fieldValueInfo);

    if (!is_variant_type_valid(value, *fieldValueInfo))
        FE_LOG(LogRenderer, FATAL, "Type mismatch for push constant field {}", fieldName);

    const void* srcData = std::visit([](auto& val)->const void*{ return &val; }, value);
    uint8* dstPtr = m_memory.data() + fieldValueInfo->offset;
    memcpy(dstPtr, srcData, fieldValueInfo->size);
}

uint32 PushConstants::get_value_size(PushConstantType type)
{
    switch (type)
    {
    case PushConstantType::FLOAT: return (uint32)sizeof(float);
    case PushConstantType::INT32: return (uint32)sizeof(int32);
    case PushConstantType::UINT32: return (uint32)sizeof(uint32);
    case PushConstantType::FLOAT2: return (uint32)sizeof(math::Float2);
    case PushConstantType::FLOAT3: return (uint32)sizeof(math::Float3);
    case PushConstantType::FLOAT4: return (uint32)sizeof(math::Float4);
    case PushConstantType::INT2: return (uint32)sizeof(math::Int2);
    case PushConstantType::INT3: return (uint32)sizeof(math::Int3);
    case PushConstantType::INT4: return (uint32)sizeof(math::Int4);
    case PushConstantType::UINT2: return (uint32)sizeof(math::UInt2);
    case PushConstantType::UINT3: return (uint32)sizeof(math::UInt3);
    case PushConstantType::UINT4: return (uint32)sizeof(math::UInt4);
    default: return 0;
    }
}

const PushConstants::FieldValueInfo* PushConstants::get_field_value_info(FieldName name) const
{
    auto it = m_fieldValueInfoByName.find(name);
    if (it == m_fieldValueInfoByName.end())
        return nullptr;
    return &it->second;
}

bool PushConstants::is_variant_type_valid(
    const PushConstants::FieldValueVariant& value, 
    const PushConstants::FieldValueInfo& valueInfo
) const
{
    if (std::get_if<float>(&value))
        return valueInfo.type == PushConstantType::FLOAT;
    if (std::get_if<uint32>(&value))
        return valueInfo.type == PushConstantType::UINT32;
    if (std::get_if<int32>(&value))
        return valueInfo.type == PushConstantType::INT32;
    if (std::get_if<math::Float2>(&value))
        return valueInfo.type == PushConstantType::FLOAT2;
    if (std::get_if<math::Float3>(&value))
        return valueInfo.type == PushConstantType::FLOAT3;
    if (std::get_if<math::Float4>(&value))
        return valueInfo.type == PushConstantType::FLOAT4;
    if (std::get_if<math::Int2>(&value))
        return valueInfo.type == PushConstantType::INT2;
    if (std::get_if<math::Int3>(&value))
        return valueInfo.type == PushConstantType::INT3;
    if (std::get_if<math::Int4>(&value))
        return valueInfo.type == PushConstantType::INT4;
    if (std::get_if<math::UInt2>(&value))
        return valueInfo.type == PushConstantType::UINT2;
    if (std::get_if<math::UInt3>(&value))
        return valueInfo.type == PushConstantType::INT3;
    if (std::get_if<math::UInt4>(&value))
        return valueInfo.type == PushConstantType::INT4;

    return false;
}

}