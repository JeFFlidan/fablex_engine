#include "tlas.h"
#include "scene_manager.h"
#include "gpu_model.h"
#include "engine/entity/entity.h"

namespace fe::renderer
{

constexpr const char* TLAS_UPLOAD_BUFFER_NAME = "TLASUploadBuffer";

TLAS::TLAS(SceneManager* sceneManager) : m_sceneManager(sceneManager)
{
    m_uploadBuffers.set_debug_name(TLAS_UPLOAD_BUFFER_NAME);
}

TLAS::~TLAS()
{
    if (m_tlas)
    {
        rhi::destroy_buffer(instance_buffer());
        rhi::destroy_acceleration_structure(m_tlas);
    }
}

void TLAS::allocate(uint32 inObjectCount)
{
    // if (m_tlas)
    //     FE_LOG(LogDefault, INFO, "Object Count: {}; TLAS instance count: {}", objectCount, m_tlas->info.tlas.count);

    if (!m_tlas || m_tlas->info.tlas.count <= inObjectCount)
    {
        if (m_tlas)
        {
            rhi::AccelerationStructure* oldTLAS = m_tlas;

            m_sceneManager->enqueue_destruction([oldTLAS]()
            {
                rhi::destroy_buffer(oldTLAS->info.tlas.instanceBuffer);
                rhi::destroy_acceleration_structure(oldTLAS);
            });
        }

        uint64 newObjectCount = (inObjectCount + 1) * 2;
    
        rhi::AccelerationStructureInfo info;
        info.flags = rhi::AccelerationStructureInfo::Flags::PREFER_FAST_BUILD;
        info.type = rhi::AccelerationStructureInfo::TOP_LEVEL;
        info.tlas.count = newObjectCount;

        m_uploadBuffers.entry_count(newObjectCount);
    
        rhi::BufferInfo bufferInfo;
        bufferInfo.bufferUsage = 
            rhi::ResourceUsage::STORAGE_BUFFER |
            rhi::ResourceUsage::TRANSFER_DST;
        bufferInfo.memoryUsage = rhi::MemoryUsage::GPU;
        bufferInfo.size = info.tlas.count * instance_size();
        bufferInfo.flags = rhi::ResourceFlags::RAY_TRACING;
    
        rhi::create_buffer(&info.tlas.instanceBuffer, &bufferInfo);
        rhi::create_acceleration_structure(&m_tlas, &info);

        rhi::set_name(m_tlas->info.tlas.instanceBuffer, "TLASInstanceBuffer");
        rhi::set_name(m_tlas, "MainTLAS");
    }

    m_uploadBuffers.allocate();
    m_uploadBuffers.memset(0);
}

void TLAS::write(const GPUModel* model, const engine::Entity* entity, uint32 instanceIndex) const
{
    rhi::TLAS::Instance instance;
    instance.instanceID = instanceIndex;

    instance.blas = model->blases().at(0);  // zero lod for now
    instance.instanceMask = 1 << 0; // TEMP
    instance.instanceContributionToHitGroupIndex = 0;
    instance.flags = rhi::TLAS::Instance::Flags::TRIANGLE_CULL_DISABLE;

    Matrix remapMat = model->aabb().get_unorm_remap_matrix();
    Float4x4 transformMat = remapMat * entity->get_world_transform();

    for (uint32 i = 0; i != ARRAYSIZE(instance.transform); ++i)
        for (uint32 j = 0; j != ARRAYSIZE(instance.transform[i]); ++j)
            instance.transform[i][j] = transformMat.m[j][i];

    void* dst = m_uploadBuffers.data() + instanceIndex * instance_size();

    rhi::write_top_level_acceleration_structure_instance(&instance, dst);
}

void TLAS::build()
{
    m_sceneManager->record_compute_cmd([this](rhi::CommandBuffer* cmd)
    {
        rhi::Buffer* uploadBuffer = m_uploadBuffers.active_buffer();

        rhi::copy_buffer(cmd, uploadBuffer, instance_buffer(), uploadBuffer->size, 0, 0);
        rhi::build_acceleration_structure(cmd, m_tlas, nullptr);
    });
}

uint64 TLAS::instance_size() const
{
    return rhi::get_acceleration_structure_instance_size();
}

rhi::Buffer* TLAS::instance_buffer() const
{
    return m_tlas->info.tlas.instanceBuffer;
}

}