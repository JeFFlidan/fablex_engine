#pragma once

#include "gpu_model_instance.h"
#include "rhi/resources.h"
#include "asset_manager/fwd.h"
#include "engine/entity/fwd.h"
#include "core/primitives/aabb.h"

struct ShaderModel;
struct ShaderModelInstance;
struct ShaderMeshInstance;

namespace fe::renderer
{

class SceneManager;
class CommandRecorder;

enum class BLASState
{
    REQUIRES_REBUILD,
    REQUIRES_REFIT,
    READY
};

class GPUModel
{
public:
    GPUModel(asset::Model* model);
    GPUModel(engine::ModelComponent* modelComponent);
    virtual ~GPUModel();

    void build(SceneManager* sceneManager, const CommandRecorder& cmdRecorder);
    void build_blas(const CommandRecorder& cmdRecorder);
    void destroy_buffer_views();
    void destroy_BLASes();

    void add_instance(engine::Entity* entity);
    void remove_instance(engine::Entity* entity);

    void fill_shader_model(ShaderModel& outShaderModel) const;
    
    void fill_shader_model_and_mesh_instances(
        SceneManager* sceneManager,
        ShaderModelInstance* modelInstanceArray,
        uint64& modelInstanceArrayOffset,
        ShaderMeshInstance* meshInstanceArray,
        uint64& meshInstanceArrayOffset
    );

    asset::Model* model_asset() const { return m_model; }
    const AABB& aabb() const;
    uint32 mesh_count() const;
    rhi::Format position_format() const { return m_positionFormat; }
    rhi::Format uv_format() const { return m_uvFormat; }
    uint64 meshlet_count() const { return m_meshletCount; }
    uint32 thread_group_count_x() const;
    rhi::Buffer* general_buffer() const { return m_generalBuffer; }
    uint64 index_offset() const { return m_indices.offset; }
    uint64 index_count() const; 
    const std::vector<rhi::AccelerationStructure*>& blases() const { return m_BLASes; }
    const std::vector<GPUModelInstance>& instances() const { return m_instances; }
    uint32 instance_count() const { return m_instances.size(); }

    int32 srv_indices() const;
    int32 srv_positions_winds() const;
    int32 srv_meshlets() const;
    int32 srv_meshlet_bounds() const;
    int32 srv_normals() const;
    int32 srv_tangents() const;
    int32 srv_uvs() const;
    int32 srv_atlas() const;
    int32 srv_colors() const;

private:
    struct BufferView
    {
        static constexpr uint64 INVALID = ~0ull;

        uint64 offset = INVALID;
        uint64 size = INVALID;
        rhi::BufferView* uav = nullptr;
        rhi::BufferView* srv = nullptr;

        bool is_valid() const { return offset != INVALID && size != INVALID; }
        void cleanup();
    };

    asset::Model* m_model = nullptr;
    rhi::Format m_positionFormat = rhi::Format::UNDEFINED;
    rhi::Format m_uvFormat = rhi::Format::UNDEFINED;
    Float2 m_uvRangeMin = Float2(0.0f, 0.0f);
    Float2 m_uvRangeMax = Float2(1.0f, 1.0f);
    uint64 m_meshletCount = 0;

    rhi::Buffer* m_generalBuffer = nullptr;
    BufferView m_indices;
    BufferView m_vertexPositionsWinds;
    BufferView m_meshlets;
    BufferView m_meshletBounds;
    BufferView m_vertexNormals;
    BufferView m_vertexTangents;
    BufferView m_vertexUVs;
    BufferView m_vertexAtlas;
    BufferView m_vertexColors;

    BLASState m_BLASState = BLASState::REQUIRES_REBUILD;
    std::vector<rhi::AccelerationStructure*> m_BLASes;

    std::vector<GPUModelInstance> m_instances;

    void configure_buffer_view(BufferView& bufferView, rhi::Format format, std::string debugName, bool requireUAV = false);
};

}