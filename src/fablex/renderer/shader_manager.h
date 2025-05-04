#pragma once

#include "common.h"
#include "rhi/resources.h"
#include "core/task_types.h"

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <mutex>

namespace fe::renderer
{

struct ShaderMetadata;

struct ShaderInputInfo
{
    std::string path;
    rhi::ShaderType type = rhi::ShaderType::UNDEFINED;
    rhi::ShaderFormat format = rhi::ShaderFormat::UNDEFINED;
    rhi::HLSLShaderModel minHlslShaderModel = rhi::HLSLShaderModel::SM_6_5;
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
    
    rhi::Shader* get_shader(const ShaderMetadata& shaderMetadata);
    
    void request_shader_loading(const ShaderMetadata& shaderMetadata);
    void wait_shaders_loading();

private:
    std::unique_ptr<IShaderCompiler> m_shaderCompiler = nullptr;
    std::unordered_map<ShaderMetadata, rhi::Shader*> m_shaderByMetadata{};
    TaskGroup* m_taskGroup = nullptr;
    std::mutex m_mutex{};

    // Relative path must not include src/fablex/shaders, only file names or subfolders of shaders + file names
    rhi::Shader* load_shader(
        const std::string& relativePath, 
        rhi::ShaderType shaderType,
        const std::string& entryPoint = "main",
        rhi::HLSLShaderModel shaderModel = rhi::HLSLShaderModel::SM_6_7,
        const std::vector<std::string>& defines = {}
    );
};

}