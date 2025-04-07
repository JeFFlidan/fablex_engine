#pragma once

#include "core/object.h"
#include "core/uuid.h"

namespace fe::asset
{

class AssetManager;

enum class Type : uint32
{
    MODEL,
    TEXTURE,
    MATERIAL,

    COUNT
};

class Asset : public Object
{
    FE_DECLARE_OBJECT(Asset);
    FE_DECLARE_PROPERTY_REGISTER(Asset);

    friend AssetManager;

public:
    // ========== Begin Object interface ==========

    virtual void serialize(Archive& archive) const override;
    virtual void deserialize(Archive& archive) override;

    // ========== End Object interface ==========

    void make_dirty() { m_isDirty = true; }

    bool is_dirty() const { return m_isDirty; }
    UUID get_uuid() const { return m_uuid; }
    const std::string& get_name() const { return m_name; }
    const std::string& get_path() const { return m_assetPath; }
    const std::string& get_original_file_path() const { return m_originalFilePath; }
    
    virtual Type get_type() const { return Type::COUNT; };

protected:
    UUID m_uuid = UUID();
    std::string m_name;
    std::string m_assetPath;
    std::string m_originalFilePath;
    bool m_isDirty = false;
};

template<typename T>
struct AssetPoolSize { };

#define FE_DEFINE_ASSET_POOL_SIZE(AssetType, PoolSize)  \
    template<>                                          \
    struct AssetPoolSize<AssetType>                     \
    {                                                   \
        static constexpr uint64 poolSize = PoolSize;    \
    };

uint32 get_asset_type_count();

}