#include "tlas.h"
#include "device.h"
#include "debugger_config.h"
#include "scene_manager.h"
#include "gpu_model.h"
#include "engine/entity/entity.h"

namespace fe::renderer
{

TLAS::TLAS(SceneManager* sceneManager) : m_sceneManager(sceneManager)
{
    m_uploadBuffers.set_debug_name(TLAS_UPLOAD_BUFFER_NAME);
}

TLAS::~TLAS()
{

}

void TLAS::allocate(uint32 inObjectCount)
{
    if (!m_tlas || m_tlas->count <= inObjectCount)
    {
        uint32 newObjectCount = (inObjectCount + 1) * 2;

        m_sceneManager->enqueue_destruction(m_instanceBuffer);
        m_sceneManager->enqueue_destruction(m_tlas);

        m_instanceBuffer = BufferHandle(
            BufferCreateInfo
            {
                .size = newObjectCount * instance_size(),
                .bufferUsage = ResourceUsage::STORAGE_BUFFER | ResourceUsage::TRANSFER_DST,
                .memoryUsage = MemoryUsage::GPU,
                .flags = ResourceFlags::RAY_TRACING
            }
        );

        m_tlas = TLASHandle(
            AccelerationStructureCreateInfo
            {
                .flags = AccelerationStructureCreateInfo::Flags::PREFER_FAST_BUILD,
                .type = AccelerationStructureCreateInfo::TOP_LEVEL,
                .tlas = 
                {
                    .instanceBuffer = m_instanceBuffer,
                    .count = newObjectCount
                }
            }
        );

        m_tlas.set_name(MAIN_TLAS_NAME);
        m_tlas.set_instance_buffer_name(TLAS_INSTANCE_BUFFER_NAME);
    }

    m_uploadBuffers.allocate(m_tlas->count);
    m_uploadBuffers.memset(0);
}

void TLAS::write(const GPUModel* model, const engine::Entity* entity, uint32 instanceIndex) const
{
    TLASInstance instance
    {
        .instanceID = instanceIndex,
        .instanceMask = 1 << 0,
        .instanceContributionToHitGroupIndex = 0,
        .blas = model->blases().at(0),
        .flags = TLASInstance::Flags::TRIANGLE_CULL_DISABLE
    };

    Matrix remapMat = model->aabb().get_unorm_remap_matrix();
    Float4x4 transformMat = remapMat * entity->get_world_transform();

    for (uint32 i = 0; i != ARRAYSIZE(instance.transform); ++i)
        for (uint32 j = 0; j != ARRAYSIZE(instance.transform[i]); ++j)
            instance.transform[i][j] = transformMat.m[j][i];

    void* dst = m_uploadBuffers.data() + instanceIndex * instance_size();

    m_tlas.write_instance(instance, dst);
}

void TLAS::build()
{
    m_sceneManager->record_compute_cmd([this](CommandBufferRef cmd)
    {
        cmd.copy_buffer(m_uploadBuffers.active_buffer(), m_tlas->instanceBuffer);
        cmd.build_tlas(m_tlas, nullptr);
    });
}

uint64 TLAS::instance_size() const
{
    return Device::acceleration_structure_instance_size();
}

void TLASUploadBuffersAllocator::allocate(BufferVector& inOutBuffers, uint32 entryCount, const char* debugName)
{
    uint64 instanceSize = Device::acceleration_structure_instance_size();

    if (inOutBuffers.size() < g_frameIndex + 1
        || inOutBuffers.at(g_frameIndex)->size / instanceSize < entryCount
    )
    {
        if (inOutBuffers.size() < g_frameIndex + 1)
            inOutBuffers.emplace();

        inOutBuffers[g_frameIndex].init(
            BufferCreateInfo
            {
                .size = entryCount * instanceSize,
                .bufferUsage = ResourceUsage::TRANSFER_SRC,
                .memoryUsage = MemoryUsage::CPU
            }
        );

        Utils::set_debug_name(inOutBuffers.at(g_frameIndex), debugName);
    }
};

}