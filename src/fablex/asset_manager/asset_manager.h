#pragma once

#include "model/model.h"
#include "texture/texture.h"
#include "material/material.h"
#include "core/pool_allocator.h"

namespace fe::asset
{

class Material;

class AssetManager
{
public:
    static Model* create_model(const ModelCreateInfo& createInfo);
    static Texture* create_texture(const TextureCreateInfo& createInfo);
    static Material* create_material(const MaterialCreateInfo& createInfo);

    static bool is_model_format_supported(const std::string& extension);
    static bool is_texture_format_supported(const std::string& extension);

private:
    template<typename T>
    static T* allocate()
    {
        FE_COMPILE_CHECK((std::is_base_of_v<Asset, T>));
        static ThreadSafePoolAllocator<T, AssetPoolSize<T>::poolSize> allocator;
        return allocator.allocate();
    }
};

}