#pragma once

#include "rhi/resources.h"
#include "asset_manager/fwd.h"
#include "core/primitives/aabb.h"

struct ShaderModel;

namespace fe::renderer
{

class SceneManager;

class GPUModel
{
public:
    GPUModel(asset::Model* model);
    ~GPUModel();

    void build(rhi::CommandBuffer* cmd, SceneManager* sceneManager);
    void destroy();

    void fill_shader_model(ShaderModel& outRendererModel);

    asset::Model* get_model() const { return m_model; }
    AABB get_aabb() const { return m_aabb; }
    rhi::Format get_position_format() const { return m_positionFormat; }
    rhi::Format get_uv_format() const { return m_uvFormat; }
    float get_meshlet_count() const { return m_meshletCount; }
    rhi::Buffer* get_buffer() const { return m_generalBuffer; }
    uint64 get_index_offset() const { return m_indices.offset; }
    uint64 get_index_count() const;

    int32 get_srv_indices() const;
    int32 get_srv_positions_winds() const;
    int32 get_srv_meshlets() const;
    int32 get_srv_meshlet_bounds() const;
    int32 get_srv_normals() const;
    int32 get_srv_tangents() const;
    int32 get_srv_uvs() const;
    int32 get_srv_atlas() const;
    int32 get_srv_colors() const;

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
    AABB m_aabb;
    rhi::Format m_positionFormat = rhi::Format::UNDEFINED;
    rhi::Format m_uvFormat = rhi::Format::UNDEFINED;
    Float2 m_uvRangeMin = Float2(0.0f, 0.0f);
    Float2 m_uvRangeMax = Float2(1.0f, 1.0f);
    float m_meshletCount = 0;

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

    void configure_buffer_view(BufferView& bufferView, rhi::Format format, std::string debugName, bool requireUAV = false);
};

}