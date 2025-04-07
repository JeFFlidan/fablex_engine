#pragma once

#include "common.h"
#include "core/event.h"
#include "model/model.h"
#include "texture/texture.h"
#include "material/material.h"

namespace fe::asset
{

template<typename T>
class AssetEvent
{
public:
    AssetEvent(T* asset) : m_asset(asset) { }

    T* get_handle() const { return m_asset; }

private:
    T* m_asset = nullptr;
};

template<typename T>
class AssetImportRequestEvent
{
public:
    AssetImportRequestEvent(const T& importContext)
        : m_importContext(importContext) { }

    const T& get_import_context() const { return m_importContext; }

private:
    T m_importContext;
};

template<typename T>
class AssetImportedEvent { };

template<typename T>
class AssetCreatedEvent { };

template<typename T>
class AssetLoadedEvent { };

#define FE_DEFINE_ASSET_IMPORTED_EVENT(AssetType)                                       \
    template<>                                                                          \
    class AssetImportedEvent<AssetType> : public IEvent, public AssetEvent<AssetType>   \
    {                                                                                   \
    public:                                                                             \
        FE_DECLARE_EVENT(AssetType##ImportedEvent);                                     \
        using AssetEvent<AssetType>::AssetEvent;                                        \
    };                                                                                  \
    template<>                                                                          \
    class AssetImportRequestEvent<AssetType##ImportContext> : public IEvent             \
    {                                                                                   \
    public:                                                                             \
        FE_DECLARE_EVENT(AssetType##ImportRequestEvent);                                \
                                                                                        \
        AssetImportRequestEvent(const AssetType##ImportContext& importContext)          \
            : m_importContext(importContext) { }                                        \
        const AssetType##ImportContext& get_import_context() const                      \
            { return m_importContext; }                                                 \
    private:                                                                            \
        AssetType##ImportContext m_importContext;                                       \
    };

#define FE_DEFINE_ASSET_CREATED_EVENT(AssetType)                                        \
    template<>                                                                          \
    class AssetCreatedEvent<AssetType> : public IEvent, public AssetEvent<AssetType>    \
    {                                                                                   \
    public:                                                                             \
        FE_DECLARE_EVENT(AssetType##CreatedEvent);                                      \
        AssetCreatedEvent(AssetType* asset) : AssetEvent<AssetType>(asset) { }          \
    };

#define FE_DEFINE_ASSET_LOADED_EVENT(AssetType)                                         \
    template<>                                                                          \
    class AssetLoadedEvent<AssetType> : public IEvent, public AssetEvent<AssetType>     \
    {                                                                                   \
    public:                                                                             \
        FE_DECLARE_EVENT(AssetType##LoadedEvent);                                       \
        AssetLoadedEvent(AssetType* asset) : AssetEvent<AssetType>(asset) { }           \
    };

FE_DEFINE_ASSET_IMPORTED_EVENT(Model);
FE_DEFINE_ASSET_IMPORTED_EVENT(Texture);

FE_DEFINE_ASSET_CREATED_EVENT(Material);

FE_DEFINE_ASSET_LOADED_EVENT(Model);
FE_DEFINE_ASSET_LOADED_EVENT(Texture);
FE_DEFINE_ASSET_LOADED_EVENT(Material);

}