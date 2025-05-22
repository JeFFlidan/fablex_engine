#pragma once

#include "asset_manager/asset.h"
#include "asset_manager/common.h"
#include "core/primitives/aabb.h"

namespace fe::asset
{

struct ModelProxy;
class Model;
class Texture;
class Material;

struct ModelCreateInfo : public CreateInfo
{
    
};

struct ModelImportContext : public ImportContext
{
    bool mergeMeshes = true;
    bool loadTextures = true;
    bool generateMaterials = false;
    bool generateTangents = false;
};

struct ModelImportResult
{
    std::vector<Model*> models;
    std::vector<Texture*> textures;
    std::vector<Material*> materials;
};

struct Mesh
{
    uint32 indexCount = 0;
    uint32 indexOffset = 0;
    UUID materialUUID = 0;
};

struct MaterialSlot
{
    std::string name;
    UUID materialUUID;

    bool operator==(const MaterialSlot& other) const
    {
        return name == other.name;
    }
};

class Model : public Asset
{
    FE_DECLARE_OBJECT(Model);
    FE_DECLARE_PROPERTY_REGISTER(Model);

    friend AssetManager;
    friend ModelProxy;

public:
    // ========== Begin Object interface ==========

    virtual void serialize(Archive& archive) const override;
    virtual void deserialize(Archive& archive) override;

    // ========== End Object interface ==========

    uint64 index_count() const { return m_indices.size(); }
    uint64 vertex_count() const { return m_vertexPositions.size(); }

    const std::vector<uint32>& indices() const { return m_indices; }
    const std::vector<Float3>& vertex_positions() const { return m_vertexPositions; }
    const std::vector<Float3>& vertex_normals() const { return m_vertexNormals; }
    const std::vector<Float4>& vertex_tangents() const { return m_vertexTangents; }
    const std::vector<Float2>& vertex_uv_set0() const { return m_vertexUVSet0; }
    const std::vector<Float2>& vertex_uv_set1() const { return m_vertexUVSet1; }
    const std::vector<UInt4>& vertex_bone_indices() const { return m_vertexBoneIndices; }
    const std::vector<Float4>& vertex_bone_weights() const { return m_vertexBoneWeights; }
    const std::vector<Float2>& vertex_atlas() const { return m_vertexAtlas; }
    const std::vector<uint32>& vertex_colors() const { return m_vertexColors; }
    const std::vector<uint8>& vertex_wind_weights() const { return m_vertexWindWeights; }
    const std::vector<Mesh>& meshes() const { return m_meshes; }
    const std::vector<MaterialSlot>& material_slots() const { return m_materialSlots; }
    const AABB& aabb() const { return m_aabb; }

    // ========== Begin Asset interface ==========

    virtual Type get_type() const override { return Type::MODEL; }

    // ========== End Asset interface ==========

protected:
    std::vector<uint32> m_indices;
    std::vector<Float3> m_vertexPositions;
    std::vector<Float3> m_vertexNormals;
    std::vector<Float4> m_vertexTangents;
    std::vector<Float2> m_vertexUVSet0;
    std::vector<Float2> m_vertexUVSet1;
    std::vector<UInt4> m_vertexBoneIndices;
    std::vector<Float4> m_vertexBoneWeights;
    std::vector<Float2> m_vertexAtlas;
    std::vector<uint32> m_vertexColors;
    std::vector<uint8> m_vertexWindWeights;
    std::vector<Mesh> m_meshes;
    std::vector<MaterialSlot> m_materialSlots;
    
    AABB m_aabb;
};

FE_DEFINE_ASSET_POOL_SIZE(Model, 512);

#ifdef FE_MODEL_PROXY

struct ModelProxy
{
    ModelProxy(Model* model) :
        indices(model->m_indices),
        vertexPositions(model->m_vertexPositions),
        vertexNormals(model->m_vertexNormals),
        vertexTangents(model->m_vertexTangents),
        vertexUVSet0(model->m_vertexUVSet0),
        vertexUVSet1(model->m_vertexUVSet1),
        vertexBoneIndices(model->m_vertexBoneIndices),
        vertexBoneWeights(model->m_vertexBoneWeights),
        vertexAtlas(model->m_vertexAtlas),
        vertexColors(model->m_vertexColors),
        vertexWindWeights(model->m_vertexWindWeights),
        meshes(model->m_meshes),
        materialSlots(model->m_materialSlots),
        aabb(model->m_aabb)
    {

    }

    std::vector<uint32>& indices;
    std::vector<Float3>& vertexPositions;
    std::vector<Float3>& vertexNormals;
    std::vector<Float4>& vertexTangents;
    std::vector<Float2>& vertexUVSet0;
    std::vector<Float2>& vertexUVSet1;
    std::vector<UInt4>& vertexBoneIndices;
    std::vector<Float4>& vertexBoneWeights;
    std::vector<Float2>& vertexAtlas;
    std::vector<uint32>& vertexColors;
    std::vector<uint8>& vertexWindWeights;
    std::vector<Mesh>& meshes;
    std::vector<MaterialSlot>& materialSlots;
    
    AABB& aabb;
};

#endif // FE_MODEL_PROXY

}