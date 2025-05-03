#pragma once

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
    
    // Relative path must not include src/fablex/shaders, only file names or subfolders of shaders + file names
    rhi::Shader* load_shader(
        const std::string& relativePath, 
        rhi::ShaderType shaderType,
        const std::string& entryPoint = "main",
        rhi::HLSLShaderModel shaderModel = rhi::HLSLShaderModel::SM_6_7,
        const std::vector<std::string>& defines = {}
    );
    
    // For non-rt and non-lib shaders only
    rhi::Shader* get_shader(const std::string& relativePath);
    rhi::Shader* get_shader(const ShaderMetadata& shaderMetadata);
    
    void request_shader_loading(const ShaderMetadata& shaderMetadata);
    void wait_shaders_loading();

private:
    struct ShaderLib
    {
        rhi::Shader* shader;
        std::string entryPoint;
    };

    using ShaderLibraryArrayHandle = uint32;
    using ShaderVariant = std::variant<rhi::Shader*, ShaderLibraryArrayHandle>;
    using ShaderLibArray = std::vector<ShaderLib>;

    std::unique_ptr<IShaderCompiler> m_shaderCompiler = nullptr;
    std::unordered_map<std::string, ShaderVariant> m_shaderByRelativePath{};
    std::vector<ShaderLibArray> m_shaderLibraries;
    TaskGroup* m_taskGroup = nullptr;
    std::mutex m_mutex{};

    void add_shader(const std::string& relativePath, const std::string& entryPoint, rhi::Shader* shader);
};

}