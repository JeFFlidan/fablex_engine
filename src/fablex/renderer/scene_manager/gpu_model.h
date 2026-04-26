#pragma once

#include "gpu_resource.h"
#include "handles/buffer.h"
#include "handles/buffer_view.h"
#include "handles/acceleration_structure.h"
#include "handles/handle_vector.h"

#include "asset_manager/model/model.h"
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

class GPUModel : public GPUResource<GPUModel, asset::Model>
{
public:
    using Parent = GPUResource<GPUModel, asset::Model>;

    GPUModel(asset::Model* model);
    ~GPUModel();

    virtual void reset() override;

    virtual void build(SceneManager* sceneManager) override;
    void destroy_buffer_views();
    void destroy_BLASes();

    uint32 next_model_instance_index() const { return m_nextModelInstanceIndex; } 
    void update_instance_offsets(uint32& inOutModelInstanceOffset, uint32& inOutMeshInstanceOffset);

    virtual bool upload_to_gpu(const SceneManager* sceneManager) override;
    void upload_model_instance(const SceneManager* sceneManager, engine::Entity* instanceEntity);

    const AABB& aabb() const;
    uint32 mesh_count() const;
    Format position_format() const { return m_positionFormat; }
    Format uv_format() const { return m_uvFormat; }
    uint64 meshlet_count() const { return m_meshletCount; }
    uint32 thread_group_count_x() const;
    BufferRef general_buffer() const { return m_generalBuffer; }
    uint64 index_offset() const { return m_indices.offset; }
    uint64 index_count() const; 
    const HandleVector<BLASHandle>& blases() const { return m_BLASes; }
    uint32 instance_count() const { return m_refCount; }

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
        BufferViewHandle uav;
        BufferViewHandle srv;

        void cleanup();
        bool is_valid() const { return offset != INVALID && size != INVALID; }

    private:
        std::string get_uav_name(const std::string& debugBaseName) const;
        std::string get_srv_name(const std::string& debugBaseName) const;
    };

    struct MeshMeshletsInfo
    {
        uint32_t meshletCount = 0;
        uint32_t meshletOffset = 0;
    };

    static constexpr uint32 INVALID_INSTANCE_INDEX = ~0u;

    Format m_positionFormat = Format::UNDEFINED;
    Format m_uvFormat = Format::UNDEFINED;
    Float2 m_uvRangeMin = Float2(0.0f, 0.0f);
    Float2 m_uvRangeMax = Float2(1.0f, 1.0f);
    uint64 m_meshletCount = 0;

    uint32 m_nextModelInstanceIndex = INVALID_INSTANCE_INDEX;
    uint32 m_nextMeshInstanceIndex = INVALID_INSTANCE_INDEX;

    BufferHandle m_generalBuffer;
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
    HandleVector<BLASHandle> m_BLASes;

    std::vector<MeshMeshletsInfo> m_meshMeshletsInfos;

    void configure_buffer_view(BufferView& bufferView, Format format, std::string debugName, bool requireUAV = false);
    Format get_blas_vertex_format() const;
    uint32 get_blas_index_offset(const asset::Mesh& mesh) const;
};

}