#pragma once

#include "core/logger.h"
#include <vector>
#include <string>

namespace fe::asset
{

class Model;
class Texture;
class Material;

struct ModelImportContext
{
    std::string path;

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

struct TextureImportContext
{
    std::string path;
};

struct TextureImportResult
{
    Texture* texture = nullptr;
};

FE_DEFINE_LOG_CATEGORY(LogAssetManager);

}