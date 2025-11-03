#pragma once

#include "fwd.h"
#include "rhi/resources/shader.h"
#include "handles/shader.h"
#include "core/task_types.h"

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <mutex>

namespace fe::renderer
{

struct ShaderInputInfo
{
    std::string path;
    ShaderType type = ShaderType::UNDEFINED;
    ShaderFormat format = ShaderFormat::UNDEFINED;
    HLSLShaderModel minHlslShaderModel = HLSLShaderModel::SM_6_5;
    std::string entryPoint = "main";
    std::vector<std::string> defines;
    std::vector<std::string> includePaths;
};

struct ShaderOutputInfo
{
    std::shared_ptr<void> internalBlob;
    const uint8_t* data{ nullptr };
    uint64_t dataSize;
    std::unordered_set<std::string> dependencies;
};

class IShaderCompiler
{
public:
    virtual ~IShaderCompiler() = default;
    virtual void compile(ShaderInputInfo& inputInfo, ShaderOutputInfo& outputInfo) = 0;
};

class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();
    
    ShaderRef get_shader(const rg::ShaderMetadata& shaderMetadata);
    
    void request_shader_loading(const rg::ShaderMetadata& shaderMetadata);
    void wait_shaders_loading();

private:
    std::unique_ptr<IShaderCompiler> m_shaderCompiler = nullptr;
    std::unordered_map<uint64, ShaderHandle> m_shaderByMetadataHash{};
    TaskGroup* m_taskGroup = nullptr;
    std::mutex m_mutex{};

    // Relative path must not include src/fablex/shaders, only file names or subfolders of shaders + file names
    ShaderRef load_shader(
        const rg::ShaderMetadata& metadata,
        HLSLShaderModel shaderModel = HLSLShaderModel::SM_6_7
    );

    uint64 get_hash(const rg::ShaderMetadata& metadata) const;
};

}