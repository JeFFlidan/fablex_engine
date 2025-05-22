#define FE_MODEL_PROXY
#include "gltf_bridge.h"
#include "asset_manager.h"
#include "model.h"
#include "texture/texture_bridge.h"
#include "asset_manager.h"
#include "material/opaque_material_settings.h"

#include "core/file_system/file_system.h"
#include "core/task_composer.h"
#include "core/platform/platform.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_FS
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

namespace tinygltf
{

struct TextureLoadingContext
{
    fe::TaskGroup taskGroup;
    std::mutex mutex;
    std::unordered_map<std::string, fe::asset::Texture*> textureByURI; 
};

bool FileExists(const std::string& absFilename, void*)
{
    return fe::FileSystem::exists(absFilename);
}

std::string ExpandFilePath(const std::string& filePath, void*)
{
#ifdef _WIN32
    DWORD len = ExpandEnvironmentStringsA(filePath.c_str(), NULL, 0);
    std::string result;
    result.resize(len);
    ExpandEnvironmentStringsA(filePath.c_str(), result.data(), len);
    return result;
#endif
}

bool ReadWholeFile(std::vector<unsigned char>* out, std::string* err, const std::string& filePath, void*)
{
    fe::FileSystem::read(filePath, *out);
    return true;
}

bool WriteWholeFile(
    std::string* err,
    const std::string& filePath,
    const std::vector<unsigned char>& contents, void*)
{
    FE_CHECK_MSG(0, "WriteWholeFile is not implemented");
    return true;
}

bool LoadImageData(Image *image, const int imageIdx, std::string *err, std::string *warn,
    int reqWidth, int reqHeight, const unsigned char *bytes, int size, void *userData)
{
    (void)warn;

    if (image->uri.empty())
    {
        image->uri = "gltf_" + image->name + ".png"; // Random name with extension to determine how to parse texture
    }

    TextureLoadingContext* loadingContext = static_cast<TextureLoadingContext*>(userData);

    fe::asset::TextureImportFromMemoryContext importContext;
    importContext.data = bytes;
    importContext.dataSize = size;
    importContext.name = image->name;
    importContext.originalFilePath = image->uri;
    importContext.projectDirectory = fe::FileSystem::get_project_path();

    fe::TaskComposer::execute(loadingContext->taskGroup, [importContext, loadingContext](fe::TaskExecutionInfo)
    {   
        fe::asset::TextureImportResult importResult;
        std::string uri = importContext.originalFilePath;

        if (!fe::asset::AssetManager::import_texture(importContext, importResult))
        {
            FE_LOG(LogDefault, ERROR, "Failed to load texture {}", uri);
            loadingContext->textureByURI[uri] = nullptr; // temp, must use default texture not to fire material assertion
        }

        std::scoped_lock<std::mutex> locker(loadingContext->mutex);
        loadingContext->textureByURI[uri] = importResult.texture;
    });

    return true;
}

bool WriteImageData(const std::string *basePath, const std::string *fileName, const Image *image,
    bool embedImages, const FsCallbacks* fsCb, const URICallbacks *uriCb, std::string *outUri, void *userPointer)
{
    // TODO
    FE_CHECK_MSG(0, "WriteImageData is not implemented");
    return false;
}

}

namespace fe::asset
{

bool GLTFBridge::import(const ModelImportContext& inImportContext, ModelImportResult& outImportResult)
{
    tinygltf::TinyGLTF loader;
    std::string tinygltfError;
    std::string tinygltfWarning;
    
    tinygltf::FsCallbacks callbacks{};
    callbacks.ReadWholeFile = tinygltf::ReadWholeFile;
    callbacks.WriteWholeFile = tinygltf::WriteWholeFile;
    callbacks.ExpandFilePath = tinygltf::ExpandFilePath;
    callbacks.FileExists = tinygltf::FileExists;
    
    loader.SetFsCallbacks(callbacks);

    tinygltf::TextureLoadingContext loadingContext;

    loader.SetImageLoader(tinygltf::LoadImageData, &loadingContext);
    loader.SetImageWriter(tinygltf::WriteImageData, nullptr);
    
    std::vector<uint8> fileData;
    FileSystem::read(inImportContext.originalFilePath, fileData);

    std::string extension = FileSystem::get_file_extension(inImportContext.originalFilePath);
    std::string baseDir = tinygltf::GetBaseDir(inImportContext.originalFilePath);

    bool isValid = false;
    tinygltf::Model gltfModel;

    if (extension == "gltf")
    {
        isValid = loader.LoadASCIIFromString(
            &gltfModel,
            &tinygltfError,
            &tinygltfWarning,
            reinterpret_cast<const char*>(fileData.data()),
            static_cast<uint32>(fileData.size()),
            baseDir
        );
    }
    else
    {
        isValid = loader.LoadBinaryFromMemory(
            &gltfModel,
            &tinygltfError,
            &tinygltfWarning,
            fileData.data(),
            static_cast<uint32>(fileData.size()),
            baseDir
        );
    }

    if (!tinygltfWarning.empty())
        FE_LOG(LogAssetManager, WARNING, "GLTFBridge::import(): {}", tinygltfWarning);

    if (!isValid)
        FE_LOG(LogAssetManager, WARNING, "GLTFBridge::import(): {}", tinygltfError);

    auto getTexture = [&](const tinygltf::Parameter& parameter)
    {
        auto& texture = gltfModel.textures[parameter.TextureIndex()];
        return loadingContext.textureByURI.at(gltfModel.images[texture.source].uri);
    };

    outImportResult.materials.reserve(gltfModel.materials.size());

    TaskComposer::wait(loadingContext.taskGroup);

    if (inImportContext.generateMaterials)
    {
        for (auto& material : gltfModel.materials)
        {
            // For now only opaque materials

            asset::OpaqueMaterialCreateInfo createInfo;
            createInfo.name = material.name;
            createInfo.projectDirectory = inImportContext.projectDirectory;
            createInfo.baseColor = Float4(1, 1, 1, 1);
            createInfo.roughness = 1;
            createInfo.metallic = 0;

            auto baseColorTexture = material.values.find("baseColorTexture");
            auto normalTexture = material.additionalValues.find("normalTexture");
            auto metallicRoughnessTexture = material.values.find("metallicRoughnessTexture");
            auto baseColorFactor = material.values.find("baseColorFactor");
            auto roughnessFactor = material.values.find("roughnessFactor");
            auto metallicFactor = material.values.find("metallicFactor");

            if (baseColorTexture != material.values.end())
                createInfo.baseColorTexture = getTexture(baseColorTexture->second);

            if (normalTexture != material.additionalValues.end())
                createInfo.normalTexture = getTexture(normalTexture->second);

            if (metallicRoughnessTexture != material.values.end())
            {
                createInfo.armTexture = getTexture(metallicRoughnessTexture->second);
                createInfo.metallic = 1.0f;
            }

            if (baseColorFactor != material.values.end())
            {
                createInfo.baseColor.x = float(baseColorFactor->second.ColorFactor()[0]);
                createInfo.baseColor.y = float(baseColorFactor->second.ColorFactor()[1]);
                createInfo.baseColor.z = float(baseColorFactor->second.ColorFactor()[2]);
                createInfo.baseColor.w = float(baseColorFactor->second.ColorFactor()[3]);
            }

            if (roughnessFactor != material.values.end())
                createInfo.roughness = float(roughnessFactor->second.Factor());

            if (metallicFactor != material.values.end())
                createInfo.metallic = float(metallicFactor->second.Factor());

            outImportResult.materials.push_back(asset::AssetManager::create_material(createInfo));
        }
    }
    else 
    {
        outImportResult.materials.resize(gltfModel.materials.size(), AssetManager::get_default_material());
    }

    // From WickedEngine
    const size_t indexRemap[] = {0, 2, 1};

    Model* model = nullptr;
    if (inImportContext.mergeMeshes)
    {
        ModelCreateInfo createInfo;
        createInfo.name = gltfModel.meshes.at(0).name;
        model = AssetManager::create_model(createInfo);
        outImportResult.models.push_back(model);
    }
    else
    {
        outImportResult.models.reserve(gltfModel.meshes.size());
    }

    for (auto& gltfMesh : gltfModel.meshes)
    {
        if (!inImportContext.mergeMeshes)
        {
            ModelCreateInfo createInfo;
            createInfo.name = gltfMesh.name;
            model = AssetManager::create_model(createInfo);
            outImportResult.models.push_back(model);
        }

        ModelProxy modelProxy(model);
        modelProxy.meshes.reserve(gltfMesh.primitives.size() + modelProxy.meshes.size());
        modelProxy.materialSlots.reserve(gltfModel.materials.size());
        
        std::vector<MaterialSlot>& materialSlots = modelProxy.materialSlots;
        
        for (auto& primitive : gltfMesh.primitives)
        {
            Mesh& mesh = modelProxy.meshes.emplace_back();
            uint32 vertexOffset = modelProxy.vertexPositions.size();

            if (primitive.indices >= 0)
            {
                const tinygltf::Accessor& accessor = gltfModel.accessors[primitive.indices];
                const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

                int stride = accessor.ByteStride(bufferView);
                size_t indexCount = accessor.count;
                size_t indexOffset = modelProxy.indices.size();
                modelProxy.indices.resize(indexCount + indexOffset);
                mesh.indexCount = indexCount;
                mesh.indexOffset = indexOffset;
                mesh.materialUUID = outImportResult.materials[primitive.material]->get_uuid();

                std::string materialName = gltfModel.materials[primitive.material].name;
                MaterialSlot materialSlot = {materialName, mesh.materialUUID};

                if (std::find(materialSlots.begin(), materialSlots.end(), materialSlot) == materialSlots.end())
                    materialSlots.push_back(materialSlot);

                const uint8* gltfIndices = buffer.data.data() + accessor.byteOffset + bufferView.byteOffset;

                if (stride == sizeof(uint8))
                {
                    for (size_t i = 0; i < indexCount; i += 3)
                    {
                        modelProxy.indices[indexOffset + i + 0] = vertexOffset + gltfIndices[i + indexRemap[0]];
                        modelProxy.indices[indexOffset + i + 1] = vertexOffset + gltfIndices[i + indexRemap[1]];
                        modelProxy.indices[indexOffset + i + 2] = vertexOffset + gltfIndices[i + indexRemap[2]];
                    }
                }
                else if (stride == sizeof(uint16))
                {
                    for (size_t i = 0; i < indexCount; i += 3)
                    {
                        modelProxy.indices[indexOffset + i + 0] = vertexOffset + ((uint16*)gltfIndices)[i + indexRemap[0]];
                        modelProxy.indices[indexOffset + i + 1] = vertexOffset + ((uint16*)gltfIndices)[i + indexRemap[1]];
                        modelProxy.indices[indexOffset + i + 2] = vertexOffset + ((uint16*)gltfIndices)[i + indexRemap[2]];
                    }
                }
                else if (stride == sizeof(uint32))
                {
                    for (size_t i = 0; i < indexCount; i += 3)
                    {
                        modelProxy.indices[indexOffset + i + 0] = vertexOffset + ((uint32*)gltfIndices)[i + indexRemap[0]];
                        modelProxy.indices[indexOffset + i + 1] = vertexOffset + ((uint32*)gltfIndices)[i + indexRemap[1]];
                        modelProxy.indices[indexOffset + i + 2] = vertexOffset + ((uint32*)gltfIndices)[i + indexRemap[2]];
                    }
                }
                else
                {
                    FE_LOG(LogAssetManager, ERROR, "GLTFImporter::import(): Unsupported stride {}", stride);
                    return false;
                }
            }

            for (auto& attribute : primitive.attributes)
            {
                const std::string& attributeName = attribute.first;
                int attributeIndex = attribute.second;

                const tinygltf::Accessor& accessor = gltfModel.accessors[attributeIndex];
                const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

                int stride = accessor.ByteStride(bufferView);
                size_t vertexCount = accessor.count;
                
                if (mesh.indexCount == 0)
                {
                    size_t indexOffset = modelProxy.indices.size();
                    modelProxy.indices.resize(indexOffset + vertexCount);
                    for (size_t i = 0; i < vertexCount; i += 3)
                    {
                        modelProxy.indices[indexOffset + i + 0] = uint32(vertexOffset + i + indexRemap[0]);
                        modelProxy.indices[indexOffset + i + 1] = uint32(vertexOffset + i + indexRemap[1]);
                        modelProxy.indices[indexOffset + i + 2] = uint32(vertexOffset + i + indexRemap[2]);
                    }
                    mesh.indexCount = vertexCount;
                    mesh.indexOffset = indexOffset;
                }

                const uint8* attributeData = buffer.data.data() + accessor.byteOffset + bufferView.byteOffset;

                auto parseUvSet = [attributeData, vertexCount, vertexOffset, stride, &accessor](std::vector<Float2>& uvSet)
                {
                    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                    {
                        for (size_t i = 0; i != vertexCount; ++i)
                        {
                            uvSet[vertexOffset + i] = *(const Float2*)(attributeData + i * stride);
                        }
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        for (size_t i = 0; i != vertexCount; ++i)
                        {
                            const uint8& s = attributeData[i * stride];
                            const uint8& t = attributeData[i * stride + sizeof(uint8)];
                            uvSet[vertexOffset + i].x = s / 255.0f;
                            uvSet[vertexOffset + i].y = t / 255.0f;
                        }
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                    {
                        for (size_t i = 0; i != vertexCount; ++i)
                        {
                            const uint16& s = *(const uint16*)(attributeData + i * stride);
                            const uint16& t = *(const uint16*)(attributeData + i * stride + sizeof(uint16));
                            uvSet[vertexOffset + i].x = s / 65535.0f;
                            uvSet[vertexOffset + i].y = t / 65535.0f;
                        }
                    }
                    else
                    {
                        FE_LOG(LogAssetManager, ERROR, "GLTFImporter::import(): UV set type is unsupported");
                    }
                };

                if (attributeName == "POSITION")
                {
                    modelProxy.vertexPositions.resize(vertexOffset + vertexCount);
                    for (size_t i = 0; i != vertexCount; ++i)
                    {
                        modelProxy.vertexPositions[vertexOffset + i] = *(const Float3*)(attributeData + i * stride);
                    }
                }
                else if (attributeName == "NORMAL")
                {
                    modelProxy.vertexNormals.resize(vertexOffset + vertexCount);
                    for (size_t i = 0; i != vertexCount; ++i)
                    {
                        modelProxy.vertexNormals[vertexOffset + i] = *(const Float3*)(attributeData + i * stride);
                    }
                }
                else if (attributeName == "TANGENT")
                {
                    modelProxy.vertexTangents.resize(vertexOffset + vertexCount);
                    for (size_t i = 0; i != vertexCount; ++i)
                    {
                        modelProxy.vertexTangents[vertexOffset + i] = *(const Float4*)(attributeData + i + stride);
                    }
                }
                else if (attributeName == "TEXCOORD_0")
                {
                    modelProxy.vertexUVSet0.resize(vertexOffset + vertexCount);
                    parseUvSet(modelProxy.vertexUVSet0);
                }
                else if (attributeName == "TEXCOORD_1")
                {
                    modelProxy.vertexUVSet1.resize(vertexOffset + vertexCount);
                    parseUvSet(modelProxy.vertexUVSet1);
                }
                else if (attributeName == "JOINTS_0")
                {
                    modelProxy.vertexBoneIndices.resize(vertexCount + vertexOffset);

                    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        struct TempJoint
                        {
                            uint8 indices[4];
                        };
                        
                        for (size_t i = 0; i != vertexCount; ++i)
                        {
                            const TempJoint& tempJoint = *(const TempJoint*)(attributeData + i * stride);
                            modelProxy.vertexBoneIndices[vertexOffset + i].x = tempJoint.indices[0];
                            modelProxy.vertexBoneIndices[vertexOffset + i].y = tempJoint.indices[1];
                            modelProxy.vertexBoneIndices[vertexOffset + i].z = tempJoint.indices[2];
                            modelProxy.vertexBoneIndices[vertexOffset + i].w = tempJoint.indices[3];
                        }
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                    {
                        struct TempJoint
                        {
                            uint16 indices[4];
                        };

                        for (size_t i = 0; i != vertexCount; ++i)
                        {
                            const TempJoint& tempJoint = *(const TempJoint*)(attributeData + i * stride);
                            modelProxy.vertexBoneIndices[vertexOffset + i].x = tempJoint.indices[0];
                            modelProxy.vertexBoneIndices[vertexOffset + i].y = tempJoint.indices[1];
                            modelProxy.vertexBoneIndices[vertexOffset + i].z = tempJoint.indices[2];
                            modelProxy.vertexBoneIndices[vertexOffset + i].w = tempJoint.indices[3];
                        }
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                    {
                        for (size_t i = 0; i != vertexCount; ++i)
                        {
                            modelProxy.vertexBoneIndices[vertexOffset + i] = *(const UInt4*)(attributeData + i * stride);
                        }
                    }
                    else
                    {
                        FE_LOG(LogAssetManager, ERROR, "GLTFImporter::import(): Joint type is unsupported");
                        return false;
                    }
                }
                else if (attributeName == "WEIGHTS_0")
                {
                    modelProxy.vertexBoneWeights.resize(vertexOffset + vertexCount);
                    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                    {
                        for (size_t i = 0; i != vertexCount; ++i)
                        {
                            modelProxy.vertexBoneWeights[vertexOffset + i] = *(const Float4*)(attributeData + i * stride);
                        }
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        for (size_t i = 0; i != vertexCount; ++i)
                        {
                            const uint8& x = attributeData[i * stride];
                            const uint8& y = attributeData[i * stride + sizeof(uint8)];
                            const uint8& z = attributeData[i * stride + sizeof(uint8) * 2];
                            const uint8& w = attributeData[i * stride + sizeof(uint8) * 3];

                            modelProxy.vertexBoneWeights[vertexOffset + i] = Float4(x / 255.0f, y / 255.0f, z / 255.0f, w / 255.0f);
                        }
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                    {
                        for (size_t i = 0; i != vertexCount; ++i)
                        {
                            const uint16& x = attributeData[i * stride];
                            const uint16& y = attributeData[i * stride + sizeof(uint16)];
                            const uint16& z = attributeData[i * stride + sizeof(uint16) * 2];
                            const uint16& w = attributeData[i * stride + sizeof(uint16) * 3];

                            modelProxy.vertexBoneWeights[vertexOffset + i] = Float4(x / 65535.0f, y / 65535.0f, z / 65535.0f, w / 65535.0f);
                        }
                    }
                    else
                    {
                        FE_LOG(LogAssetManager, ERROR, "GLTFImporter::import(): Weight type is unsupported");
                        return false;
                    }
                }
                else if (attributeName == "COLOR_0")
                {
                    modelProxy.vertexColors.resize(vertexOffset + vertexCount);
                    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && accessor.type == TINYGLTF_TYPE_VEC3)
                    {
                        for (size_t i = 0; i != vertexCount; ++i)
                        {
                            const Float3& color = *(const Float3*)(attributeData + i * stride);
                            modelProxy.vertexColors[vertexOffset + i] = compress_color(color);
                        }
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && accessor.type == TINYGLTF_TYPE_VEC4)
                    {
                        for (size_t i = 0; i != vertexCount; ++i)
                        {
                            const Float4& color = *(const Float4*)(attributeData + i * stride);
                            modelProxy.vertexColors[vertexOffset + i] = compress_color(color);
                        }
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE && accessor.type == TINYGLTF_TYPE_VEC3)
                    {
                        for (size_t i = 0; i != vertexCount; ++i)
                        {
                            const uint8& r = attributeData[i * stride];
                            const uint8& g = attributeData[i * stride + sizeof(uint8)];
                            const uint8& b = attributeData[i * stride + sizeof(uint8) * 2];
                            modelProxy.vertexColors[vertexOffset + i] = compress_color(Float3(r / 255.0f, g / 255.0f, b / 255.0f));
                        }
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE && accessor.type == TINYGLTF_TYPE_VEC4)
                    {
                        for (size_t i = 0; i != vertexCount; ++i)
                        {
                            const uint8& r = attributeData[i * stride];
                            const uint8& g = attributeData[i * stride + sizeof(uint8)];
                            const uint8& b = attributeData[i * stride + sizeof(uint8) * 2];
                            const uint8& a = attributeData[i * stride + sizeof(uint8) * 3];
                            modelProxy.vertexColors[vertexOffset + i] = compress_color(Float4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f));
                        }
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT && accessor.type == TINYGLTF_TYPE_VEC3)
                    {
                        for (size_t i = 0; i != vertexCount; ++i)
                        {
                            const uint16& r = attributeData[i * stride];
                            const uint16& g = attributeData[i * stride + sizeof(uint16)];
                            const uint16& b = attributeData[i * stride + sizeof(uint16) * 2];
                            modelProxy.vertexColors[vertexOffset + i] = compress_color(Float3(r / 65535.0f, g / 65535.0f, b / 65535.0f));
                        }
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT && accessor.type == TINYGLTF_TYPE_VEC4)
                    {
                        for (size_t i = 0; i != vertexCount; ++i)
                        {
                            const uint16& r = attributeData[i * stride];
                            const uint16& g = attributeData[i * stride + sizeof(uint16)];
                            const uint16& b = attributeData[i * stride + sizeof(uint16) * 2];
                            const uint16& a = attributeData[i * stride + sizeof(uint16) * 3];
                            modelProxy.vertexColors[vertexOffset + i] = compress_color(Float4(r / 65535.0f, g / 65535.0f, b / 65535.0f, a / 65535.0f));
                        }
                    }
                    else
                    {
                        FE_LOG(LogAssetManager, ERROR, "GLTFImporter::import(): Color type is unsupported");
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

}