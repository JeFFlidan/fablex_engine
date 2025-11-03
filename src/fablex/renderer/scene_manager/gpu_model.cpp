#include "gpu_model.h"
#include "vertex.h"
#include "scene_manager.h"
#include "staging_buffer.h"
#include "rhi/rhi.h"
#include "rhi/utils.h"
#include "rhi/resources/buffer.h"
#include "rhi/resources/buffer_view.h"
#include "rhi/resources/acceleration_structure.h"
#include "asset_manager/model/model.h"
#include "core/primitives/aabb.h"
#include "core/primitives/sphere.h"
#include "engine/entity/entity.h"
#include "shaders/shader_interop_renderer.h"
#include "meshoptimizer.h"

namespace fe::renderer
{

GPUModel::GPUModel(asset::Model* model) : GPUResource(model)
{

}

GPUModel::~GPUModel()
{
    destroy_BLASes();
    destroy_buffer_views();
}

void GPUModel::reset()
{
    Parent::reset();

    m_nextModelInstanceIndex = INVALID_INSTANCE_INDEX;
    m_nextMeshInstanceIndex = INVALID_INSTANCE_INDEX;
}

void GPUModel::build(SceneManager* sceneManager)
{
    destroy_buffer_views();

    const float targetPrecision = 1.0f / 1000.0f;
    m_positionFormat = VertexPositionWind16Bit::FORMAT;

    const AABB& aabb = this->aabb();

    for (uint32 i = 0; i != m_asset->vertex_positions().size(); ++i)
    {
        const Float3& position = m_asset->vertex_positions()[i];
        const uint8 wind = m_asset->vertex_wind_weights().empty() ? 0xFF : m_asset->vertex_wind_weights()[i];

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

        m_positionFormat = VertexPositionWind32Bit::FORMAT;
    }

    const uint32 positionFormatStride = rhi::get_format_stride(m_positionFormat);

    const uint64 uvCount = std::max(m_asset->vertex_uv_set0().size(), m_asset->vertex_uv_set1().size());
    uint64 uvStride = sizeof(VertexUVs16Bit);
    m_uvFormat = VertexUVs16Bit::FORMAT;

    if (!m_asset->vertex_uv_set0().empty() || !m_asset->vertex_uv_set1().empty())
    {
        const std::vector<Float2>& uv0 = m_asset->vertex_uv_set0().empty() ? m_asset->vertex_uv_set1() : m_asset->vertex_uv_set0();
        const std::vector<Float2>& uv1 = m_asset->vertex_uv_set1().empty() ? m_asset->vertex_uv_set0() : m_asset->vertex_uv_set1();

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
        m_asset->index_count(), 
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
        m_asset->indices().data(),
        m_asset->index_count(),
        (float*)m_asset->vertex_positions().data(),
        m_asset->vertex_count(),
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
            &m_asset->vertex_positions()[0].x, 
            m_asset->vertex_count(), 
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

    BufferCreateInfo bufferInfo;
    bufferInfo.bufferUsage = 
        ResourceUsage::STORAGE_BUFFER |
        ResourceUsage::STORAGE_TEXEL_BUFFER | 
        ResourceUsage::UNIFORM_TEXEL_BUFFER |
        ResourceUsage::TRANSFER_DST |
        ResourceUsage::INDEX_BUFFER |
        ResourceUsage::VERTEX_BUFFER;

    bufferInfo.memoryUsage = MemoryUsage::GPU;
    bufferInfo.flags = ResourceFlags::RAY_TRACING;
    
    // TODO: Add bone indices to size when animations will be implemented
    const uint64 alignment = rhi::get_min_offset_alignment(&bufferInfo);
    bufferInfo.size = 
        rhi::align_to(m_asset->vertex_positions().size() * positionFormatStride, alignment) +
        rhi::align_to(m_asset->indices().size() * sizeof(uint32), alignment) +
        rhi::align_to(m_asset->vertex_normals().size() * sizeof(VertexNormal), alignment) +
        rhi::align_to(m_asset->vertex_tangents().size() * sizeof(VertexTangent), alignment) +
        rhi::align_to(uvCount * uvStride, alignment) +
        rhi::align_to(m_asset->vertex_atlas().size() * sizeof(VertexUV16Bit), alignment) +
        rhi::align_to(m_asset->vertex_colors().size() * sizeof(VertexColor), alignment);

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

    m_generalBuffer.init(bufferInfo);

    bufferInfo.bufferUsage = ResourceUsage::TRANSFER_SRC;
    bufferInfo.memoryUsage = MemoryUsage::CPU;

    StagingBuffer stagingBuffer(bufferInfo);

    // uint8* bufferData = stagingBuffer.mapped_data();
    // uint64 bufferOffset = 0;

    switch (m_positionFormat)
    {
    case VertexPositionWind16Bit::FORMAT:
    {
        m_vertexPositionsWinds.offset = stagingBuffer.offset();
        m_vertexPositionsWinds.size = sizeof(VertexPositionWind16Bit) * m_asset->vertex_positions().size();
        
        for (uint64 i = 0; i != m_asset->vertex_positions().size(); ++i)
        {
            const Float3& position = m_asset->vertex_positions()[i];
            uint8 wind = m_asset->vertex_wind_weights().empty() ? 0 : m_asset->vertex_wind_weights()[i];

            VertexPositionWind16Bit vertex;
            vertex.from_full(aabb, position, wind);
            stagingBuffer.write(vertex, i);
        }

        break;
    }
    case VertexPositionWind32Bit::FORMAT:
    {
        m_vertexPositionsWinds.offset = stagingBuffer.offset();
        m_vertexPositionsWinds.size = sizeof(VertexPositionWind32Bit) * m_asset->vertex_positions().size();
        
        for (uint64 i = 0; i != m_asset->vertex_positions().size(); ++i)
        {
            const Float3& position = m_asset->vertex_positions()[i];
            uint8 wind = m_asset->vertex_wind_weights().empty() ? 0 : m_asset->vertex_wind_weights()[i];
            
            VertexPositionWind32Bit vertex;
            vertex.from_full(position, wind);
            stagingBuffer.write(vertex, i);
        }

        break;
    }
    default:
        FE_CHECK(0);
        break;
    }

    stagingBuffer.add_to_offset(rhi::align_to(m_vertexPositionsWinds.size, alignment));

    m_indices.offset = stagingBuffer.offset();
    m_indices.size = sizeof(uint32) * m_asset->indices().size();
    stagingBuffer.write_chunk(m_asset->indices().data(), m_indices.size);
    stagingBuffer.add_to_offset(rhi::align_to(m_indices.size, alignment));

    if (!m_asset->vertex_normals().empty())
    {
        m_vertexNormals.offset = stagingBuffer.offset();
        m_vertexNormals.size = sizeof(VertexNormal) * m_asset->vertex_normals().size();

        for (uint64 i = 0; i != m_asset->vertex_normals().size(); ++i)
        {
            VertexNormal vertex;
            vertex.from_full(m_asset->vertex_normals()[i]);
            stagingBuffer.write(vertex, i);
        }

        stagingBuffer.add_to_offset(rhi::align_to(m_vertexNormals.size, alignment));
    }

    if (!m_asset->vertex_tangents().empty())
    {
        m_vertexTangents.offset = stagingBuffer.offset();
        m_vertexTangents.size = sizeof(VertexTangent) * m_asset->vertex_tangents().size();

        for (uint64 i = 0; i != m_asset->vertex_tangents().size(); ++i)
        {
            VertexTangent vertex;
            vertex.from_full(m_asset->vertex_tangents()[i]);
            stagingBuffer.write(vertex, i);
        }

        stagingBuffer.add_to_offset(rhi::align_to(m_vertexTangents.size, alignment));
    }

    if (!m_asset->vertex_uv_set0().empty() || m_asset->vertex_uv_set1().empty())
    {
        const std::vector<Float2>& uv0 = m_asset->vertex_uv_set0().empty() ? m_asset->vertex_uv_set1() : m_asset->vertex_uv_set0();
        const std::vector<Float2>& uv1 = m_asset->vertex_uv_set1().empty() ? m_asset->vertex_uv_set0() : m_asset->vertex_uv_set1();

        m_vertexUVs.offset = stagingBuffer.offset();
        m_vertexUVs.size = uvCount * uvStride;

        switch (m_uvFormat)
        {
        case VertexUVs16Bit::FORMAT:
        {
            for (uint64 i = 0; i != uvCount; ++i)
            {
                VertexUVs16Bit vertex;
                vertex.uv0.from_full(uv0.at(i), m_uvRangeMin, m_uvRangeMax);
                vertex.uv1.from_full(uv1.at(i), m_uvRangeMin, m_uvRangeMax);
                stagingBuffer.write(vertex, i);
            }

            break;
        }
        case VertexUVs32Bit::FORMAT:
        {
            for (uint64 i = 0; i != uvCount; ++i)
            {
                VertexUVs32Bit vertex;
                vertex.uv0.from_full(uv0.at(i), m_uvRangeMin, m_uvRangeMax);
                vertex.uv1.from_full(uv1.at(i),m_uvRangeMin, m_uvRangeMax);
                stagingBuffer.write(vertex, i);
            }

            break;
        }
        default:
            FE_CHECK(0);
            break;
        }
    }

    stagingBuffer.add_to_offset(rhi::align_to(m_vertexUVs.size, alignment));

    if (!m_asset->vertex_atlas().empty())
    {
        m_vertexAtlas.offset = stagingBuffer.offset();
        m_vertexAtlas.size = m_asset->vertex_atlas().size() * sizeof(VertexUV16Bit);

        for (uint64 i = 0; i != m_asset->vertex_atlas().size(); ++i)
        {
            VertexUV16Bit vertex;
            vertex.from_full(m_asset->vertex_atlas()[i]);
            stagingBuffer.write(vertex, i);
        }

        stagingBuffer.add_to_offset(rhi::align_to(m_vertexAtlas.size, alignment));
    }

    if (!m_asset->vertex_colors().empty())
    {
        m_vertexColors.offset = stagingBuffer.offset();
        m_vertexColors.size = m_asset->vertex_colors().size() * sizeof(VertexColor);

        for (uint64 i = 0; i != m_asset->vertex_colors().size(); ++i)
        {
            VertexColor vertex{m_asset->vertex_colors()[i]};
            stagingBuffer.write(vertex, i);
        }

        stagingBuffer.add_to_offset(rhi::align_to(m_vertexColors.size, alignment));
    }

    if (!shaderMeshlets.empty())
    {
        stagingBuffer.set_offset(rhi::align_to(stagingBuffer.offset(), sizeof(ShaderMeshlet)));
        m_meshlets.offset = stagingBuffer.offset();
        m_meshlets.size = shaderMeshlets.size() * sizeof(ShaderMeshlet);
        stagingBuffer.write_chunk(shaderMeshlets.data(), m_meshlets.size);
        stagingBuffer.add_to_offset(rhi::align_to(m_meshlets.size, alignment));
    }

    if (!shaderMeshletBounds.empty())
    {
        stagingBuffer.set_offset(rhi::align_to(stagingBuffer.offset(), sizeof(ShaderMeshletBounds)));
        m_meshletBounds.offset = stagingBuffer.offset();
        m_meshletBounds.size = shaderMeshletBounds.size() * sizeof(ShaderMeshletBounds);
        stagingBuffer.write_chunk(shaderMeshletBounds.data(), m_meshletBounds.size);
        stagingBuffer.add_to_offset(rhi::align_to(m_meshletBounds.size, alignment));
    }

    sceneManager->record_graphics_cmd([&](CommandBufferRef cmd)
    {
        cmd.copy_buffer(stagingBuffer.handle(), m_generalBuffer);
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

    if (m_BLASes.empty())
    {
        AccelerationStructureCreateInfo asInfo;
        asInfo.type = AccelerationStructureCreateInfo::BOTTOM_LEVEL;
        asInfo.flags |= AccelerationStructureCreateInfo::Flags::PREFER_FAST_TRACE;

        for (const asset::Mesh& mesh : m_asset->meshes())
        {
            asInfo.blas.geometries.emplace_back(
                rhi::BLAS::Geometry
                {
                    .triangles
                    {
                        .vertexBuffer = m_generalBuffer,
                        .indexBuffer = m_generalBuffer,
                        .indexCount = mesh.indexCount,
                        .indexOffset = get_blas_index_offset(mesh),
                        .vertexCount = uint32(m_asset->vertex_count()),
                        .vertexOffset = uint32(m_vertexPositionsWinds.offset),
                        .vertexStride = rhi::get_format_stride(m_positionFormat),
                        .vertexFormat = get_blas_vertex_format(),
                    }
                }
            );
        }
        
        m_BLASes.emplace(asInfo);
    }

    switch (m_BLASState)
    {
    case BLASState::REQUIRES_REBUILD:
    {
        sceneManager->record_compute_cmd([&](CommandBufferRef cmd)
        {
            for (rhi::AccelerationStructure* as : m_BLASes)
                cmd.build_blas(as, nullptr);
                
            m_BLASState = BLASState::READY;
        });

        break;
    }
    case BLASState::REQUIRES_REFIT:
    {
        sceneManager->record_compute_cmd([&](CommandBufferRef cmd)
        {
            for (rhi::AccelerationStructure* as : m_BLASes)
                cmd.build_blas(as, as);    

            m_BLASState = BLASState::READY;
        });

        break;
    }
    case BLASState::READY:
    {
        return;
    }
    }

    sceneManager->enqueue_destruction(stagingBuffer);
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
}

void GPUModel::destroy_BLASes()
{
    m_BLASes.clear();
    m_BLASState = BLASState::REQUIRES_REBUILD;
}

void GPUModel::update_instance_offsets(uint32& inOutModelInstanceOffset, uint32& inOutMeshInstanceOffset)
{
    if (m_nextModelInstanceIndex == INVALID_INSTANCE_INDEX)
    {
        m_nextModelInstanceIndex = inOutModelInstanceOffset;
        inOutModelInstanceOffset += m_refCount;
    }

    if (m_nextMeshInstanceIndex == INVALID_INSTANCE_INDEX)
    {
        m_nextMeshInstanceIndex = inOutMeshInstanceOffset;
        inOutMeshInstanceOffset += m_refCount * asset()->meshes().size();
    }
}

bool GPUModel::upload_to_gpu(const SceneManager* sceneManager)
{
    if (m_refCount == 0)
        return false;

    ShaderModel& shaderModel = sceneManager->model_buffers()[m_indexInBuffer];

    shaderModel.indexBuffer = srv_indices();
    shaderModel.vertexBufferPosWind = srv_positions_winds();
    shaderModel.vertexBufferMeshlets = srv_meshlets();
    shaderModel.vertexBufferMeshletBounds = srv_meshlet_bounds();
    shaderModel.vertexBufferNormals = srv_normals();
    shaderModel.vertexBufferTangents = srv_tangents();
    shaderModel.vertexBufferUVs = srv_uvs();
    shaderModel.vertexBufferAtlas = srv_atlas();
    shaderModel.vertexBufferColors = srv_colors();
    
    shaderModel.aabbMin = aabb().minPoint;
    shaderModel.aabbMax = aabb().maxPoint;
    
    shaderModel.uvRangeMin = m_uvRangeMin;
    shaderModel.uvRangeMax = m_uvRangeMax;

    return true;
}

void GPUModel::upload_model_instance(const SceneManager* sceneManager, engine::Entity* instanceEntity)
{
    engine::MaterialComponent* materialComponent = instanceEntity->get_component<engine::MaterialComponent>();

    const ModelInstanceBuffers& modelInstanceBuffers = sceneManager->model_instance_buffers();
    const MeshInstanceBuffers& meshInstanceBuffers = sceneManager->mesh_instance_buffers();

    ShaderModelInstance& modelInstance = modelInstanceBuffers[m_nextModelInstanceIndex];

    Sphere sphereBounds(aabb());
    modelInstance.sphereBounds.center = sphereBounds.center;
    modelInstance.sphereBounds.radius = sphereBounds.radius;
    modelInstance.meshOffset = m_nextMeshInstanceIndex;

    Matrix remapMat = aabb().get_unorm_remap_matrix();
    Matrix transformMat = instanceEntity->get_world_transform();

    modelInstance.scale = instanceEntity->get_scale();
    modelInstance.transform.set_transfrom(remapMat * transformMat);
    modelInstance.rawTransform.set_transfrom(instanceEntity->get_world_transform());
    modelInstance.prevTransform.set_transfrom(remapMat * instanceEntity->get_prev_world_transform());
    modelInstance.transformInverseTranspose.set_transfrom(transformMat.transpose().inverse());

    for (auto& mesh : m_asset->meshes())
    {
        ShaderMeshInstance& shaderMeshInstance = meshInstanceBuffers[m_nextMeshInstanceIndex++];
        shaderMeshInstance.modelIndex = m_indexInBuffer;

        UUID materialUUID = materialComponent->material_uuids()[mesh.materialIndex];
        shaderMeshInstance.materialIndex = sceneManager->gpu_material(materialUUID)->index();
        shaderMeshInstance.indexOffset = mesh.indexOffset;
    }

    sceneManager->tlas().write(this, instanceEntity, m_nextModelInstanceIndex++);
}

const AABB& GPUModel::aabb() const
{
    return m_asset->aabb();
}

uint32 GPUModel::mesh_count() const
{
    return m_asset->meshes().size();
}

uint32 GPUModel::thread_group_count_x() const
{
    return static_cast<uint32>(m_meshletCount / 32 + 1);
}

uint64 GPUModel::index_count() const
{
    return m_asset->indices().size();
}

int32 GPUModel::srv_indices() const
{
    return m_indices.srv ? m_indices.srv->descriptorIndex : -1;
}

int32 GPUModel::srv_positions_winds() const
{
    return m_vertexPositionsWinds.srv ? m_vertexPositionsWinds.srv->descriptorIndex : -1;
}

int32 GPUModel::srv_meshlets() const
{
    return m_meshlets.srv ? m_meshlets.srv->descriptorIndex : -1;
}

int32 GPUModel::srv_meshlet_bounds() const
{
    return m_meshletBounds.srv ? m_meshletBounds.srv->descriptorIndex : -1;
}

int32 GPUModel::srv_normals() const
{
    return m_vertexNormals.srv ? m_vertexNormals.srv->descriptorIndex : -1;
}

int32 GPUModel::srv_tangents() const
{
    return m_vertexTangents.srv ? m_vertexTangents.srv->descriptorIndex : -1;
}

int32 GPUModel::srv_uvs() const
{
    return m_vertexUVs.srv ? m_vertexUVs.srv->descriptorIndex : -1;
}

int32 GPUModel::srv_atlas() const
{
    return m_vertexAtlas.srv ? m_vertexAtlas.srv->descriptorIndex : -1;
}

int32 GPUModel::srv_colors() const
{
    return m_vertexColors.srv ? m_vertexColors.srv->descriptorIndex : -1;
}

void GPUModel::configure_buffer_view(BufferView& bufferView, rhi::Format format, std::string debugName, bool requireUAV)
{
    BufferViewCreateInfo bufferViewInfo;
    bufferViewInfo.buffer = m_generalBuffer;
    bufferViewInfo.newFormat = format;
    bufferViewInfo.offset = bufferView.offset;
    bufferViewInfo.size = bufferView.size;
    bufferViewInfo.type = rhi::ViewType::SRV;

    bufferView.srv.init(bufferViewInfo);
    bufferView.srv.set_name(m_asset->get_name() + debugName + "SRV");

    if (requireUAV)
    {
        bufferViewInfo.type = rhi::ViewType::UAV;
        bufferView.uav.init(bufferViewInfo);
        bufferView.uav.set_name(m_asset->get_name() + debugName + "UAV");
    }
}

Format GPUModel::get_blas_vertex_format() const
{
    return m_positionFormat == rhi::Format::R32G32B32A32_SFLOAT ? rhi::Format::R32G32B32_SFLOAT : m_positionFormat;
}

uint32 GPUModel::get_blas_index_offset(const asset::Mesh& mesh) const
{
    return m_indices.offset / sizeof(uint32) + mesh.indexOffset;
}

void GPUModel::BufferView::cleanup()
{
    offset = INVALID;
    size = INVALID;
    uav.reset();
    srv.reset();
}

}