#include "gpu_model.h"
#include "vertex.h"
#include "rhi/rhi.h"
#include "rhi/utils.h"
#include "asset_manager/model/model.h"
#include "core/primitives/aabb.h"

namespace fe::renderer
{

GPUModel::GPUModel(asset::Model* model) : m_model(model)
{
    FE_CHECK(m_model);
}

GPUModel::~GPUModel()
{
    destroy();
}

void GPUModel::build(rhi::CommandBuffer* cmd)
{
    destroy();

    m_aabb.minPoint = Float3(FLOAT_MAX, FLOAT_MAX, FLOAT_MAX);
    m_aabb.maxPoint = Float3(FLOAT_MIN, FLOAT_MIN, FLOAT_MIN);

    for (const Float3& position : m_model->vertex_positions())
    {
        m_aabb.minPoint = min(position, m_aabb.minPoint);
        m_aabb.maxPoint = max(position, m_aabb.maxPoint);
    }

    const float targetPrecision = 1.0f / 1000.0f;
    m_positionFormat = VertexPositionWind16Bit::FORMAT;

    for (uint32 i = 0; i != m_model->vertex_positions().size(); ++i)
    {
        const Float3& position = m_model->vertex_positions()[i];
        const uint8 wind = m_model->vertex_wind_weights().empty() ? 0xFF : m_model->vertex_wind_weights()[i];

        VertexPositionWind16Bit vertex;
        vertex.from_full(m_aabb, position, wind);
        const Float3 posAfterCompression = vertex.get_position(m_aabb);

        if (
            std::abs(posAfterCompression.x - position.x) <= targetPrecision &&
            std::abs(posAfterCompression.y - position.y) <= targetPrecision &&
            std::abs(posAfterCompression.z - position.z) <= targetPrecision &&
            wind == vertex.get_wind()
        )
        {
            continue;
        }

        m_positionFormat = m_model->vertex_wind_weights().empty() ? VertexPosition32Bit::FORMAT : VertexPositionWind32Bit::FORMAT;
    }

    const uint32 positionFormatStride = rhi::get_format_stride(m_positionFormat);

    const uint64 uvCount = std::max(m_model->vertex_uv_set0().size(), m_model->vertex_uv_set1().size());
    uint64 uvStride = sizeof(VertexUVs16Bit);
    m_uvFormat = VertexUVs16Bit::FORMAT;

    if (!m_model->vertex_uv_set0().empty() || !m_model->vertex_uv_set1().empty())
    {
        const std::vector<Float2>& uv0 = m_model->vertex_uv_set0().empty() ? m_model->vertex_uv_set1() : m_model->vertex_uv_set0();
        const std::vector<Float2>& uv1 = m_model->vertex_uv_set1().empty() ? m_model->vertex_uv_set0() : m_model->vertex_uv_set1();

        m_uvRangeMin = Float2(FLOAT_MAX, FLOAT_MAX);
        m_uvRangeMax = Float2(FLOAT_MIN, FLOAT_MIN);

        for (uint64 i = 0; i != uvCount; ++i)
        {
            m_uvRangeMin = min(m_uvRangeMin, uv0.at(i));
            m_uvRangeMin = min(m_uvRangeMin, uv1.at(i));
            m_uvRangeMax = max(m_uvRangeMax, uv0.at(0));
            m_uvRangeMax = max(m_uvRangeMax, uv1.at(i));
        }

        if (
            std::abs(m_uvRangeMax.x - m_uvRangeMin.x) > 65536 || 
            std::abs(m_uvRangeMax.y - m_uvRangeMin.y) > 65536
        )
        {
            uvStride = sizeof(VertexUVs32Bit);
            m_uvFormat = VertexUVs32Bit::FORMAT;
        }
    }

    rhi::BufferInfo bufferInfo;
    bufferInfo.bufferUsage = 
        rhi::ResourceUsage::STORAGE_BUFFER |
        rhi::ResourceUsage::STORAGE_TEXEL_BUFFER | 
        rhi::ResourceUsage::UNIFORM_TEXEL_BUFFER |
        rhi::ResourceUsage::TRANSFER_DST;

    bufferInfo.memoryUsage = rhi::MemoryUsage::GPU;
    
    // TODO: Add bone indices to size when animations will be implemented
    const uint64 alignment = rhi::get_min_offset_alignment(&bufferInfo);
    bufferInfo.size = 
        rhi::align_to(m_model->vertex_positions().size() * positionFormatStride, alignment) +
        rhi::align_to(m_model->indices().size() * sizeof(uint32), alignment) +
        rhi::align_to(m_model->vertex_normals().size() * sizeof(VertexNormal), alignment) +
        rhi::align_to(m_model->vertex_tangents().size() * sizeof(VertexTangent), alignment) +
        rhi::align_to(uvCount * uvStride, alignment) +
        rhi::align_to(m_model->vertex_atlas().size() * sizeof(VertexUV16Bit), alignment) +
        rhi::align_to(m_model->vertex_colors().size() * sizeof(VertexColor), alignment);


    rhi::create_buffer(&m_generalBuffer, &bufferInfo);

    bufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_SRC;
    bufferInfo.memoryUsage = rhi::MemoryUsage::CPU;

    rhi::Buffer* stagingBuffer;
    rhi::create_buffer(&stagingBuffer, &bufferInfo);

    uint8* bufferData = static_cast<uint8*>(stagingBuffer->mappedData);
    uint64 bufferOffset = 0;

    switch (m_positionFormat)
    {
    case VertexPositionWind16Bit::FORMAT:
    {
        m_vertexPositionsWinds.offset = bufferOffset;
        m_vertexPositionsWinds.size = sizeof(VertexPositionWind16Bit) * m_model->vertex_positions().size();
        
        VertexPositionWind16Bit* vertices = reinterpret_cast<VertexPositionWind16Bit*>(bufferData + bufferOffset);
        bufferOffset += rhi::align_to(m_vertexPositionsWinds.size, alignment);

        for (uint64 i = 0; i != m_model->vertex_positions().size(); ++i)
        {
            const Float3& position = m_model->vertex_positions()[i];
            uint8 wind = m_model->vertex_wind_weights()[i];
            VertexPositionWind16Bit vertex;
            vertex.from_full(m_aabb, position, wind);
            memcpy(vertices + i, &vertex, sizeof(VertexPositionWind16Bit));
        }

        break;
    }
    case VertexPosition32Bit::FORMAT:
    {
        m_vertexPositionsWinds.offset = bufferOffset;
        m_vertexPositionsWinds.size = sizeof(VertexPosition32Bit) * m_model->vertex_positions().size();
        
        VertexPosition32Bit* vertices = reinterpret_cast<VertexPosition32Bit*>(bufferData + bufferOffset);
        bufferOffset += rhi::align_to(m_vertexPositionsWinds.size, alignment);

        for (uint64 i = 0; i != m_model->vertex_positions().size(); ++i)
        {
            const Float3& position = m_model->vertex_positions()[i];
            VertexPosition32Bit vertex;
            vertex.from_full(position);
            memcpy(vertices + i, &vertex, sizeof(VertexPosition32Bit));
        }

        break;
    }
    case VertexPositionWind32Bit::FORMAT:
    {
        m_vertexPositionsWinds.offset = bufferOffset;
        m_vertexPositionsWinds.size = sizeof(VertexPositionWind32Bit) * m_model->vertex_positions().size();
        
        VertexPositionWind32Bit* vertices = reinterpret_cast<VertexPositionWind32Bit*>(bufferData + bufferOffset);
        bufferOffset += rhi::align_to(m_vertexPositionsWinds.size, alignment);

        for (uint64 i = 0; i != m_model->vertex_positions().size(); ++i)
        {
            const Float3& position = m_model->vertex_positions()[i];
            uint8 wind = m_model->vertex_wind_weights()[i];
            VertexPositionWind32Bit vertex;
            vertex.from_full(position, wind);
            memcpy(vertices + i, &vertex, sizeof(VertexPositionWind32Bit));
        }

        break;
    }
    default:
        FE_CHECK(0);
        break;
    }

    m_indices.offset = bufferOffset;
    m_indices.size = sizeof(uint32) * m_model->indices().size();
    uint32* indexData = reinterpret_cast<uint32*>(bufferData + bufferOffset);
    bufferOffset += rhi::align_to(m_indices.size, alignment);
    memcpy(indexData, m_model->indices().data(), m_indices.size);

    if (!m_model->vertex_normals().empty())
    {
        m_vertexNormals.offset = bufferOffset;
        m_vertexNormals.size = sizeof(VertexNormal) * m_model->vertex_normals().size();

        VertexNormal* vertices = reinterpret_cast<VertexNormal*>(bufferData + bufferOffset);
        bufferOffset += rhi::align_to(m_vertexNormals.size, alignment);

        for (uint64 i = 0; i != m_model->vertex_normals().size(); ++i)
        {
            VertexNormal vertex;
            vertex.from_full(m_model->vertex_normals()[i]);
            memcpy(vertices + i, &vertex, sizeof(VertexNormal));
        }
    }

    if (!m_model->vertex_tangents().empty())
    {
        m_vertexTangents.offset = bufferOffset;
        m_vertexTangents.size = sizeof(VertexTangent) * m_model->vertex_tangents().size();

        VertexTangent* vertices = reinterpret_cast<VertexTangent*>(bufferData + bufferOffset);
        bufferOffset += rhi::align_to(m_vertexTangents.size, alignment);

        for (uint64 i = 0; i != m_model->vertex_tangents().size(); ++i)
        {
            VertexTangent vertex;
            vertex.from_full(m_model->vertex_tangents()[i]);
            memcpy(vertices + i, &vertex, sizeof(VertexTangent));
        }
    }

    if (!m_model->vertex_uv_set0().empty() || m_model->vertex_uv_set1().empty())
    {
        const std::vector<Float2>& uv0 = m_model->vertex_uv_set0().empty() ? m_model->vertex_uv_set1() : m_model->vertex_uv_set0();
        const std::vector<Float2>& uv1 = m_model->vertex_uv_set1().empty() ? m_model->vertex_uv_set0() : m_model->vertex_uv_set1();

        m_vertexUVs.offset = bufferOffset;
        m_vertexUVs.size = uvCount * uvStride;

        switch (m_uvFormat)
        {
        case VertexUV16Bit::FORMAT:
        {
            VertexUVs16Bit* vertices = reinterpret_cast<VertexUVs16Bit*>(bufferData + bufferOffset);
            bufferOffset += rhi::align_to(m_vertexUVs.size, alignment);

            for (uint64 i = 0; i != uvCount; ++i)
            {
                VertexUVs16Bit vertex;
                vertex.uv0.from_full(uv0.at(i), m_uvRangeMin, m_uvRangeMax);
                vertex.uv1.from_full(uv1.at(i), m_uvRangeMin, m_uvRangeMax);
                memcpy(vertices + i, &vertex, sizeof(VertexUVs16Bit));
            }

            break;
        }
        case VertexUV32Bit::FORMAT:
        {
            VertexUVs32Bit* vertices = reinterpret_cast<VertexUVs32Bit*>(bufferData + bufferOffset);
            bufferOffset += rhi::align_to(m_vertexUVs.size, alignment);

            for (uint64 i = 0; i != uvCount; ++i)
            {
                VertexUVs32Bit vertex;
                vertex.uv0.from_full(uv0.at(i), m_uvRangeMin, m_uvRangeMax);
                vertex.uv1.from_full(uv1.at(i),m_uvRangeMin, m_uvRangeMax);
                memcpy(vertices + i, &vertex, sizeof(VertexUVs32Bit));
            }

            break;
        }
        default:
            FE_CHECK(0);
            break;
        }
    }

    if (!m_model->vertex_atlas().empty())
    {
        m_vertexAtlas.offset = bufferOffset;
        m_vertexAtlas.size = m_model->vertex_atlas().size() * sizeof(VertexUV16Bit);

        VertexUV16Bit* vertices = reinterpret_cast<VertexUV16Bit*>(bufferData + bufferOffset);
        bufferOffset += rhi::align_to(m_vertexAtlas.size, alignment);

        for (uint64 i = 0; i != m_model->vertex_atlas().size(); ++i)
        {
            VertexUV16Bit vertex;
            vertex.from_full(m_model->vertex_atlas()[i]);
            memcpy(vertices + i, &vertex, sizeof(VertexUV16Bit));
        }
    }

    if (!m_model->vertex_colors().empty())
    {
        m_vertexColors.offset = bufferOffset;
        m_vertexColors.size = m_model->vertex_colors().size() * sizeof(VertexColor);

        VertexColor* vertices = reinterpret_cast<VertexColor*>(bufferData + bufferOffset);
        bufferOffset += rhi::align_to(m_vertexColors.size, alignment);

        for (uint64 i = 0; i != m_model->vertex_colors().size(); ++i)
        {
            VertexColor vertex{m_model->vertex_colors()[i]};
            memcpy(vertices + i, &vertex, sizeof(VertexColor));
        }
    }

    rhi::copy_buffer(cmd, stagingBuffer, m_generalBuffer, stagingBuffer->size, 0, 0);

    FE_CHECK(m_vertexPositionsWinds.is_valid());
    configure_buffer_view(m_vertexPositionsWinds, m_positionFormat);

    FE_CHECK(m_indices.is_valid());
    configure_buffer_view(m_indices, rhi::Format::R32_UINT);

    if (m_vertexNormals.is_valid())
        configure_buffer_view(m_vertexNormals, VertexNormal::FORMAT);

    if (m_vertexTangents.is_valid())
        configure_buffer_view(m_vertexTangents, VertexTangent::FORMAT);

    if (m_vertexUVs.is_valid())
        configure_buffer_view(m_vertexUVs, m_uvFormat);

    if (m_vertexAtlas.is_valid())
        configure_buffer_view(m_vertexAtlas, VertexUV16Bit::FORMAT);

    if (m_vertexColors.is_valid())
        configure_buffer_view(m_vertexColors, VertexColor::FORMAT);
}

void GPUModel::destroy()
{
    rhi::destroy_buffer_view(m_indices.srv);
    rhi::destroy_buffer_view(m_indices.uav);
    rhi::destroy_buffer_view(m_vertexPositionsWinds.srv);
    rhi::destroy_buffer_view(m_vertexPositionsWinds.uav);
    rhi::destroy_buffer_view(m_vertexNormals.srv);
    rhi::destroy_buffer_view(m_vertexNormals.uav);
    rhi::destroy_buffer_view(m_vertexTangents.srv);
    rhi::destroy_buffer_view(m_vertexTangents.uav);
    rhi::destroy_buffer_view(m_vertexUVs.srv);
    rhi::destroy_buffer_view(m_vertexUVs.uav);
    rhi::destroy_buffer_view(m_vertexAtlas.srv);
    rhi::destroy_buffer_view(m_vertexAtlas.uav);
    rhi::destroy_buffer_view(m_vertexAtlas.srv);
    rhi::destroy_buffer_view(m_vertexAtlas.uav);

    rhi::destroy_buffer(m_generalBuffer);
}

int32 GPUModel::get_srv_indices() const
{
    return m_indices.srv ? m_indices.srv->descriptorIndex : -1;
}

int32 GPUModel::get_srv_positions_winds() const
{
    return m_vertexPositionsWinds.srv ? m_vertexPositionsWinds.srv->descriptorIndex : -1;
}

int32 GPUModel::get_srv_normals() const
{
    return m_vertexNormals.srv ? m_vertexNormals.srv->descriptorIndex : -1;
}

int32 GPUModel::get_srv_tangents() const
{
    return m_vertexTangents.srv ? m_vertexTangents.srv->descriptorIndex : -1;
}

int32 GPUModel::get_srv_uvs() const
{
    return m_vertexUVs.srv ? m_vertexUVs.srv->descriptorIndex : -1;
}

int32 GPUModel::get_srv_atlas() const
{
    return m_vertexAtlas.srv ? m_vertexAtlas.srv->descriptorIndex : -1;
}

int32 GPUModel::get_srv_colors() const
{
    return m_vertexColors.srv ? m_vertexColors.srv->descriptorIndex : -1;
}

void GPUModel::configure_buffer_view(BufferView& bufferView, rhi::Format format, bool requireUAV)
{
    rhi::BufferViewInfo bufferViewInfo;
    bufferViewInfo.newFormat = format;
    bufferViewInfo.offset = bufferView.offset;
    bufferViewInfo.size = bufferView.size;
    bufferViewInfo.type = rhi::ViewType::SRV;
    rhi::create_buffer_view(&bufferView.srv, &bufferViewInfo, m_generalBuffer);

    if (requireUAV)
    {
        bufferViewInfo.type = rhi::ViewType::UAV;
        rhi::create_buffer_view(&bufferView.uav, &bufferViewInfo, m_generalBuffer);
    }
}

}