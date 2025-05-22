#include "model.h"
#include "core/file_system/archive.h"

namespace fe::asset
{

FE_DEFINE_OBJECT(Model, Asset);

FE_BEGIN_PROPERTY_REGISTER(Model)
{

}
FE_END_PROPERTY_REGISTER(Model);

void Model::serialize(Archive& archive) const
{
    Asset::serialize(archive);

    archive << m_indices;
    archive << m_vertexPositions;
    archive << m_vertexTangents;
    archive << m_vertexUVSet0;
    archive << m_vertexUVSet1;
    archive << m_vertexBoneIndices;
    archive << m_vertexBoneWeights;
    archive << m_vertexAtlas;
    archive << m_vertexColors;
    
    archive << m_meshes.size();
    for (const Mesh& mesh : m_meshes)
    {
        archive << mesh.indexCount;
        archive << mesh.indexOffset;
        archive << mesh.materialUUID;
    }

    archive << m_materialSlots.size();
    for (const MaterialSlot& slot : m_materialSlots)
    {
        archive << slot.name;
        archive << slot.materialUUID;
    }

    archive << m_aabb.minPoint;
    archive << m_aabb.maxPoint;
}

void Model::deserialize(Archive& archive)
{
    Asset::deserialize(archive);

    archive >> m_indices;
    archive >> m_vertexPositions;
    archive >> m_vertexTangents;
    archive >> m_vertexUVSet0;
    archive >> m_vertexUVSet1;
    archive >> m_vertexBoneIndices;
    archive >> m_vertexBoneWeights;
    archive >> m_vertexAtlas;
    archive >> m_vertexColors;
    
    uint64 size = 0;
    archive >> size;
    m_meshes.resize(size);
    
    for (Mesh& mesh : m_meshes)
    {
        archive >> mesh.indexCount;
        archive >> mesh.indexOffset;
        archive >> mesh.materialUUID;
    }

    archive >> size;
    m_materialSlots.resize(size);
    
    for (MaterialSlot& slot : m_materialSlots)
    {
        archive >> slot.name;
        archive >> slot.materialUUID;
    }

    archive >> m_aabb.minPoint;
    archive >> m_aabb.maxPoint;
}

}