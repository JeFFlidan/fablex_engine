#pragma once

#include "enums.h"
#include "core/logger.h"
#include <string>

namespace fe::asset
{

class Model;
class Texture;
class Material;

struct CreateInfo
{
    std::string name;
    std::string projectDirectory;
    AssetFlag flags = AssetFlag::UNDEFINED;
};

struct ImportContext
{
    std::string originalFilePath;
    std::string projectDirectory;
    AssetFlag flags = AssetFlag::UNDEFINED;
};

FE_DEFINE_LOG_CATEGORY(LogAssetManager);

}