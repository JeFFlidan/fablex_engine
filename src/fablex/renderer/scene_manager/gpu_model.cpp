#include "gpu_model.h"
#include "vertex.h"
#include "scene_manager.h"
#include "rhi/rhi.h"
#include "rhi/utils.h"
#include "asset_manager/model/model.h"
#include "core/primitives/aabb.h"
#include "shaders/shader_interop_renderer.h"
#include "meshoptimizer.h"

namespace fe::renderer
{

GPUModel::GPUModel(asset::Model* model) : m_model(model)
{
    FE_CHECK(m_model);
}

GPUModel::~GPUModel()
{
    destroy_BLASes();
    destroy_buffer_views();
}

void GPUModel::build(SceneManager* sceneManager, const CommandRecorder& cmdRecorder)
{
    destroy_buffer_views();

    const float targetPrecision = 1.0f / 1000.0f;
    m_positionFormat = VertexPositionWind16Bit::FORMAT;

    const AABB& aabb = get_aabb();

    for (uint32 i = 0; i != m_model->vertex_positions().size(); ++i)
    {
        const Float3& position = m_model->vertex_positions()[i];
        const uint8 wind = m_model->vertex_wind_weights().empty() ? 0xFF : m_model->vertex_wind_weights()[i];

        VertexPositionWind16Bit vertex;
        vertex.from_full(aabb, position, wind);
        const Float3 posAfterCompression = vertex.get_position(aabb);

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

    const float coneWeight = 0.5f;

    const size_t maxMeshlets = meshopt_buildMeshletsBound(
        m_model->index_count(), 
        MESHLET_VERTEX_COUNT, 
        MESHLET_TRIANGLE_COUNT
    );

    std::vector<meshopt_Meshlet> meshoptMeshlets(maxMeshlets);
    std::vector<unsigned int> meshletVertices(maxMeshlets * MESHLET_VERTEX_COUNT);
    std::vector<unsigned char> meshletTriangles(maxMeshlets * MESHLET_TRIANGLE_COUNT * 3);

    m_meshletCount = meshopt_buildMeshlets(
        meshoptMeshlets.data(),
        meshletVertices.data(),
        meshletTriangles.data(),
        m_model->indices().data(),
        m_model->index_count(),
        (float*)m_model->vertex_positions().data(),
        m_model->vertex_count(),
        sizeof(Float3),
        MESHLET_VERTEX_COUNT,
        MESHLET_TRIANGLE_COUNT,
        coneWeight
    );

    std::vector<ShaderMeshlet> shaderMeshlets;
    std::vector<ShaderMeshletBounds> shaderMeshletBounds;

    shaderMeshlets.reserve(m_meshletCount);
    shaderMeshletBounds.reserve(m_meshletCount);

    const meshopt_Meshlet& lastMeshlet = meshoptMeshlets[m_meshletCount - 1];
    meshletVertices.resize(lastMeshlet.vertex_offset + lastMeshlet.vertex_count);
    meshletTriangles.resize(lastMeshlet.triangle_offset + ((lastMeshlet.triangle_count * 3 + 3) & ~3));
    meshoptMeshlets.resize(m_meshletCount);

    for (const meshopt_Meshlet& meshoptMeshlet : meshoptMeshlets)
    {
        meshopt_optimizeMeshlet(
            &meshletVertices[meshoptMeshlet.vertex_offset], 
            &meshletTriangles[meshoptMeshlet.triangle_offset], 
            meshoptMeshlet.triangle_count,
            meshoptMeshlet.vertex_count
        );

        meshopt_Bounds bounds = meshopt_computeMeshletBounds(
            &meshletVertices[meshoptMeshlet.vertex_offset], 
            &meshletTriangles[meshoptMeshlet.triangle_offset], 
            meshoptMeshlet.triangle_count, 
            &m_model->vertex_positions()[0].x, 
            m_model->vertex_count(), 
            sizeof(Float3)
        );

        ShaderMeshletBounds& shaderMeshletBoundsEntry = shaderMeshletBounds.emplace_back();
        shaderMeshletBoundsEntry.bounds.center.x = bounds.center[0];
        shaderMeshletBoundsEntry.bounds.center.y = bounds.center[1];
        shaderMeshletBoundsEntry.bounds.center.z = bounds.center[2];
        shaderMeshletBoundsEntry.bounds.radius = bounds.radius;
        shaderMeshletBoundsEntry.coneAxis.x = bounds.cone_axis[0];
        shaderMeshletBoundsEntry.coneAxis.y = bounds.cone_axis[1];
        shaderMeshletBoundsEntry.coneAxis.z = bounds.cone_axis[2];
        shaderMeshletBoundsEntry.coneCutoff = bounds.cone_cutoff;

        ShaderMeshlet& shaderMeshlet = shaderMeshlets.emplace_back();
        shaderMeshlet.triangleCount = meshoptMeshlet.triangle_count;
        shaderMeshlet.vertexCount = meshoptMeshlet.vertex_count;

        for (size_t i = 0; i != meshoptMeshlet.triangle_count; ++i)
        {
            shaderMeshlet.triangles[i].init(
                meshletTriangles.at(meshoptMeshlet.triangle_offset + i * 3 + 0),
                meshletTriangles.at(meshoptMeshlet.triangle_offset + i * 3 + 1),
                meshletTriangles.at(meshoptMeshlet.triangle_offset + i * 3 + 2)
            );
        }

        for (size_t i = 0; i != meshoptMeshlet.vertex_count; ++i)
            shaderMeshlet.vertices[i] = meshletVertices.at(meshoptMeshlet.vertex_offset + i);
    }

    rhi::BufferInfo bufferInfo;
    bufferInfo.bufferUsage = 
        rhi::ResourceUsage::STORAGE_BUFFER |
        rhi::ResourceUsage::STORAGE_TEXEL_BUFFER | 
        rhi::ResourceUsage::UNIFORM_TEXEL_BUFFER |
        rhi::ResourceUsage::TRANSFER_DST |
        rhi::ResourceUsage::INDEX_BUFFER |
        rhi::ResourceUsage::VERTEX_BUFFER;

    bufferInfo.memoryUsage = rhi::MemoryUsage::GPU;
    bufferInfo.flags = rhi::ResourceFlags::RAY_TRACING;
    
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

    if (!shaderMeshlets.empty())
    {
        bufferInfo.size = rhi::align_to(bufferInfo.size, sizeof(ShaderMeshlet));
        bufferInfo.size = rhi::align_to(bufferInfo.size + shaderMeshlets.size() * sizeof(ShaderMeshlet), alignment);
    }

    if (!shaderMeshletBounds.empty())
    {
        bufferInfo.size = rhi::align_to(bufferInfo.size, sizeof(ShaderMeshletBounds));
        bufferInfo.size = rhi::align_to(bufferInfo.size + shaderMeshletBounds.size() * sizeof(ShaderMeshletBounds), alignment);
    }

    rhi::create_buffer(&m_generalBuffer, &bufferInfo);

    bufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_SRC;
    bufferInfo.memoryUsage = rhi::MemoryUsage::CPU;

    rhi::Buffer* stagingBuffer;
    rhi::create_buffer(&stagingBuffer, &bufferInfo);

    sceneManager->add_staging_buffer(stagingBuffer);

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
            uint8 wind = m_model->vertex_wind_weights().empty() ? 0 : m_model->vertex_wind_weights()[i];
            VertexPositionWind16Bit vertex;
            vertex.from_full(aabb, position, wind);
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

    if (!shaderMeshlets.empty())
    {
        bufferOffset = rhi::align_to(bufferOffset, sizeof(ShaderMeshlet));
        m_meshlets.offset = bufferOffset;
        m_meshlets.size = shaderMeshlets.size() * sizeof(ShaderMeshlet);
        memcpy(bufferData + bufferOffset, shaderMeshlets.data(), m_meshlets.size);
        bufferOffset += rhi::align_to(m_meshlets.size, alignment);
    }

    if (!shaderMeshletBounds.empty())
    {
        bufferOffset = rhi::align_to(bufferOffset, sizeof(ShaderMeshletBounds));
        m_meshletBounds.offset = bufferOffset;
        m_meshletBounds.size = shaderMeshletBounds.size() * sizeof(ShaderMeshletBounds);
        memcpy(bufferData + bufferOffset, shaderMeshletBounds.data(), m_meshletBounds.size);
        bufferOffset += rhi::align_to(m_meshletBounds.size, alignment);
    }

    cmdRecorder.record([&](rhi::CommandBuffer* cmd)
    {
        rhi::copy_buffer(cmd, stagingBuffer, m_generalBuffer, stagingBuffer->size, 0, 0);
    });

    FE_CHECK(m_vertexPositionsWinds.is_valid());
    configure_buffer_view(m_vertexPositionsWinds, m_positionFormat, "Vertices");

    FE_CHECK(m_indices.is_valid());
    configure_buffer_view(m_indices, rhi::Format::R32_UINT, "Indices");

    if (m_meshlets.is_valid())
        configure_buffer_view(m_meshlets, rhi::Format::UNDEFINED, "Meshlets");

    if (m_meshletBounds.is_valid())
        configure_buffer_view(m_meshletBounds, rhi::Format::UNDEFINED, "MeshletBounds");

    if (m_vertexNormals.is_valid())
        configure_buffer_view(m_vertexNormals, VertexNormal::FORMAT, "VertexNormals");

    if (m_vertexTangents.is_valid())
        configure_buffer_view(m_vertexTangents, VertexTangent::FORMAT, "VertexTangents");

    if (m_vertexUVs.is_valid())
        configure_buffer_view(m_vertexUVs, m_uvFormat, "VertexUVs");

    if (m_vertexAtlas.is_valid())
        configure_buffer_view(m_vertexAtlas, VertexUV16Bit::FORMAT, "VertexAtlas");

    if (m_vertexColors.is_valid())
        configure_buffer_view(m_vertexColors, VertexColor::FORMAT, "VertexColors");
}

void GPUModel::build_blas(const CommandRecorder& cmdRecorder)
{
    if (m_BLASes.empty())
    {
        rhi::AccelerationStructureInfo asInfo;
        asInfo.type = rhi::AccelerationStructureInfo::BOTTOM_LEVEL;
        asInfo.flags |= rhi::AccelerationStructureInfo::Flags::PREFER_FAST_TRACE;
        
        rhi::BLAS::Geometry& geometry = asInfo.blas.geometries.emplace_back();
        geometry.triangles.vertexBuffer = m_generalBuffer;
        geometry.triangles.vertexOffset = m_vertexPositionsWinds.offset;
        geometry.triangles.vertexCount = m_model->vertex_count();
        geometry.triangles.vertexFormat = m_positionFormat == VertexPosition32Bit::FORMAT ? rhi::Format::R32G32B32_SFLOAT : m_positionFormat;
        geometry.triangles.vertexStride = rhi::get_format_stride(m_positionFormat);
        geometry.triangles.indexBuffer = m_generalBuffer;
        geometry.triangles.indexCount = m_model->index_count();
        geometry.triangles.indexOffset = m_indices.offset / sizeof(uint32);

        rhi::create_acceleration_structure(&m_BLASes.emplace_back(), &asInfo);
    }

    switch (m_BLASState)
    {
    case BLASState::REQUIRES_REBUILD:
    {
        cmdRecorder.record([&](rhi::CommandBuffer* cmd)
        {
            for (rhi::AccelerationStructure* as : m_BLASes)
                rhi::build_acceleration_structure(cmd, as, nullptr);

            m_BLASState = BLASState::READY;
        });

        break;
    }
    case BLASState::REQUIRES_REFIT:
    {
        cmdRecorder.record([&](rhi::CommandBuffer* cmd)
        {
            for (rhi::AccelerationStructure* as : m_BLASes)
                rhi::build_acceleration_structure(cmd, as, as);

            m_BLASState = BLASState::READY;
        });

        break;
    }
    case BLASState::READY:
    {
        return;
    }
    }
}

void GPUModel::destroy_buffer_views()
{
    m_indices.cleanup();
    m_vertexPositionsWinds.cleanup();
    m_meshlets.cleanup();
    m_meshletBounds.cleanup();
    m_vertexNormals.cleanup();
    m_vertexTangents.cleanup();
    m_vertexUVs.cleanup();
    m_vertexAtlas.cleanup();
    m_vertexColors.cleanup();

    rhi::destroy_buffer(m_generalBuffer);
}

void GPUModel::destroy_BLASes()
{
    for (rhi::AccelerationStructure* as : m_BLASes)
        rhi::destroy_acceleration_structure(as);
    m_BLASes.clear();
    m_BLASState = BLASState::REQUIRES_REBUILD;
}

void GPUModel::fill_shader_model(ShaderModel& outRendererModel)
{
    outRendererModel.indexBuffer = get_srv_indices();
    outRendererModel.vertexBufferPosWind = get_srv_positions_winds();
    outRendererModel.vertexBufferMeshlets = get_srv_meshlets();
    outRendererModel.vertexBufferMeshletBounds = get_srv_meshlet_bounds();
    outRendererModel.vertexBufferNormals = get_srv_normals();
    outRendererModel.vertexBufferTangents = get_srv_tangents();
    outRendererModel.vertexBufferUVs = get_srv_uvs();
    outRendererModel.vertexBufferAtlas = get_srv_atlas();
    outRendererModel.vertexBufferColors = get_srv_colors();

    outRendererModel.aabbMin = get_aabb().minPoint;
    outRendererModel.aabbMax = get_aabb().maxPoint;

    outRendererModel.uvRangeMin = m_uvRangeMin;
    outRendererModel.uvRangeMax = m_uvRangeMax;
}

const AABB& GPUModel::get_aabb() const
{
    return m_model->aabb();
}

uint32 GPUModel::get_thread_group_count_x() const
{
    return static_cast<uint32>(m_meshletCount / 32 + 1);
}

uint64 GPUModel::get_index_count() const
{
    return m_model->indices().size();
}

int32 GPUModel::get_srv_indices() const
{
    return m_indices.srv ? m_indices.srv->descriptorIndex : -1;
}

int32 GPUModel::get_srv_positions_winds() const
{
    return m_vertexPositionsWinds.srv ? m_vertexPositionsWinds.srv->descriptorIndex : -1;
}

int32 GPUModel::get_srv_meshlets() const
{
    return m_meshlets.srv ? m_meshlets.srv->descriptorIndex : -1;
}

int32 GPUModel::get_srv_meshlet_bounds() const
{
    return m_meshletBounds.srv ? m_meshletBounds.srv->descriptorIndex : -1;
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

void GPUModel::configure_buffer_view(BufferView& bufferView, rhi::Format format, std::string debugName, bool requireUAV)
{
    rhi::BufferViewInfo bufferViewInfo;
    bufferViewInfo.newFormat = format;
    bufferViewInfo.offset = bufferView.offset;
    bufferViewInfo.size = bufferView.size;
    bufferViewInfo.type = rhi::ViewType::SRV;
    rhi::create_buffer_view(&bufferView.srv, &bufferViewInfo, m_generalBuffer);
    rhi::set_name(bufferView.srv, m_model->get_name() + debugName + "SRV");

    if (requireUAV)
    {
        bufferViewInfo.type = rhi::ViewType::UAV;
        rhi::create_buffer_view(&bufferView.uav, &bufferViewInfo, m_generalBuffer);
        rhi::set_name(bufferView.srv, m_model->get_name() + debugName + "UAV");
    }
}

void GPUModel::BufferView::cleanup()
{
    rhi::destroy_buffer_view(srv);
    rhi::destroy_buffer_view(uav);
    offset = INVALID;
    size = INVALID;
}

}