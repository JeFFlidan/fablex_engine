#include "shader_manager.h"
#include "common.h"
#include "core/utils.h"
#include "core/logger.h"
#include "core/file_system/file_system.h"
#include "core/task_composer.h"
#include "rhi/rhi.h"

#ifdef WIN32
#define DXCOMPILER_ENABLED

#include <wrl/client.h>
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

#include "dxcapi.h"

#endif // WIN32

#include "json.hpp"

#include <memory>
#include <mutex>
#include <unordered_set>

#define DX_ENSURE(Value) SUCCEEDED(Value)
#define DX_CHECK(Value) FE_CHECK(SUCCEEDED(Value))

FE_DEFINE_LOG_CATEGORY(LogShaderCompiler)

namespace fe::renderer
{

#ifdef DXCOMPILER_ENABLED

class DXIncludeHandler : public IDxcIncludeHandler
{
public:
    DXIncludeHandler(const ShaderInputInfo& inputInfo, ShaderOutputInfo& outputInfo, ComPtr<IDxcUtils>& utils)
        : m_inputInfo(inputInfo), m_outputInfo(outputInfo)
    {
        DX_CHECK(utils->CreateDefaultIncludeHandler(&m_dxcIncludeHandler));
    }

    HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource) override
    {
        HRESULT result = m_dxcIncludeHandler->LoadSource(pFilename, ppIncludeSource);

        if (DX_ENSURE(result))
        {
            std::string path;
            Utils::convert_string(pFilename, path);
            m_outputInfo.dependencies.insert(path);
        }
        return result;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(const IID& riid, void** ppvObject) override
    {
        return m_dxcIncludeHandler->QueryInterface(riid, ppvObject);
    }

    ULONG STDMETHODCALLTYPE AddRef() override { return 0; }
    ULONG STDMETHODCALLTYPE Release() override { return 0; }

private:
    const ShaderInputInfo& m_inputInfo;
    ShaderOutputInfo& m_outputInfo;
    ComPtr<IDxcIncludeHandler> m_dxcIncludeHandler;
};

class DXCompiler : public IShaderCompiler
{
public:
    DXCompiler()
    {
        ComPtr<IDxcCompiler3> dxcCompiler;
        DX_CHECK(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler)));

        ComPtr<IDxcVersionInfo> versionInfo;
        DX_CHECK(dxcCompiler->QueryInterface(IID_PPV_ARGS(&versionInfo)));

        uint32_t majorVer = 0;
        uint32_t minorVer = 0;
        DX_CHECK(versionInfo->GetVersion(&majorVer, &minorVer));

        FE_LOG(LogShaderCompiler, INFO, "DXCompiler::init(): Initialized DXCompiler. Version: {}.{}", majorVer, minorVer);
    }

    virtual void compile(ShaderInputInfo& inputInfo, ShaderOutputInfo& outputInfo)
    {
        std::vector<std::wstring> compileArgs;
        set_shader_format_flags(inputInfo, compileArgs);
        set_target_profile_flag(inputInfo, compileArgs);
        set_shader_code_flags(inputInfo, compileArgs);

        std::vector<const wchar_t*> compilerArgsPtrs;
        for (auto& compileArg : compileArgs)
            compilerArgsPtrs.push_back(compileArg.c_str());

        ComPtr<IDxcCompiler3> dxcCompiler;
        DX_CHECK(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler)));
        
        ComPtr<IDxcUtils> dxcUtils;
        DX_CHECK(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(dxcUtils.ReleaseAndGetAddressOf())));

        DXIncludeHandler includeHandler(inputInfo, outputInfo, dxcUtils);

        std::vector<uint8_t> rawShaderData;
        FileSystem::read(inputInfo.path, rawShaderData);

        DxcBuffer srcBuffer;
        srcBuffer.Ptr = rawShaderData.data();
        srcBuffer.Size = rawShaderData.size();
        srcBuffer.Encoding = DXC_CP_ACP;

        ComPtr<IDxcResult> dxcResult;
        DX_CHECK(dxcCompiler->Compile(
            &srcBuffer, 
            compilerArgsPtrs.data(),
            compileArgs.size(),
            &includeHandler,
            IID_PPV_ARGS(&dxcResult)));

        ComPtr<IDxcBlobUtf8> dxcErrors = nullptr;
        DX_CHECK(dxcResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&dxcErrors), nullptr));

        if (dxcErrors && dxcErrors->GetStringLength() != 0)
        {
            FE_LOG(LogShaderCompiler, ERROR, "DXCompiler::compile(): {}", dxcErrors->GetStringPointer());
            return;
        }

        ComPtr<IDxcBlob> dxcShaderObject = nullptr;
        DX_CHECK(dxcResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&dxcShaderObject), nullptr));

        if (dxcShaderObject)
        {
            outputInfo.data = (const uint8_t*)dxcShaderObject->GetBufferPointer();
            outputInfo.dataSize = dxcShaderObject->GetBufferSize();
            outputInfo.dependencies.insert(inputInfo.path);

            auto internalBlob = std::make_shared<ComPtr<IDxcBlob>>();
            *internalBlob = dxcShaderObject;
            outputInfo.internalBlob = internalBlob;
        }

        if (inputInfo.format == rhi::ShaderFormat::HLSL6)
        {
            // TODO some features like saving debug and reflection data
        }
    }

private:
    std::unordered_set<std::wstring> m_targetProfilesForShaderModel6_5 = { L"ms", L"as" };

    void set_shader_format_flags(ShaderInputInfo& inputInfo, std::vector<std::wstring>& compileArgs)
    {
        switch (inputInfo.format)
        {
        case rhi::ShaderFormat::UNDEFINED:
            FE_LOG(LogShaderCompiler, FATAL, "DXCompiler::compile(): Can't compile shader with undefined format.");
        case rhi::ShaderFormat::HLSL6:
            // TODO FOR D3D12
            break;
        case rhi::ShaderFormat::HLSL_TO_SPIRV:
            compileArgs.push_back(L"-spirv");
            compileArgs.push_back(L"-fspv-target-env=vulkan1.3");
            compileArgs.push_back(L"-fvk-use-dx-layout");
                
            compileArgs.push_back(L"-fvk-t-shift");
            compileArgs.push_back(L"1000");
            compileArgs.push_back(L"0");

            compileArgs.push_back(L"-fvk-u-shift");
            compileArgs.push_back(L"2000");
            compileArgs.push_back(L"0");
            
            compileArgs.push_back(L"-fvk-s-shift");
            compileArgs.push_back(L"3000"); 
            compileArgs.push_back(L"0");
        
#ifdef DEBUG_SHADER
            compileArgs.push_back(L"/Zi");
#endif
            break;
        }
    }

    void set_target_profile_flag(ShaderInputInfo& inputInfo, std::vector<std::wstring>& compileArgs)
    {
        compileArgs.push_back(L"-T");
        std::wstring& targetProfile = compileArgs.emplace_back();
        switch (inputInfo.type)
        {
        case rhi::ShaderType::VERTEX:
            targetProfile = L"vs";
            break;
        case rhi::ShaderType::FRAGMENT:
            targetProfile = L"ps";
            break;
        case rhi::ShaderType::COMPUTE:
            targetProfile = L"cs";
            break;
        case rhi::ShaderType::TESSELLATION_CONTROL:
            targetProfile = L"hs";
            break;
        case rhi::ShaderType::TESSELLATION_EVALUATION:
            targetProfile = L"ds";
            break;
        case rhi::ShaderType::MESH:
            targetProfile = L"ms";
            break;
        case rhi::ShaderType::TASK:
            targetProfile = L"as";
            break;
        case rhi::ShaderType::LIB:
            targetProfile = L"lib";
            break;
        default:
            FE_LOG(LogShaderCompiler, FATAL, "DXCompiler::compile(): Unsupported shader type.");
        }

        switch (inputInfo.minHlslShaderModel)
        {
        case rhi::HLSLShaderModel::SM_6_0:
            if (m_targetProfilesForShaderModel6_5.find(targetProfile) == m_targetProfilesForShaderModel6_5.end())
                targetProfile += L"_6_0";
            break;
        case rhi::HLSLShaderModel::SM_6_1:
            if (m_targetProfilesForShaderModel6_5.find(targetProfile) == m_targetProfilesForShaderModel6_5.end())
                targetProfile += L"_6_1";
            break;
        case rhi::HLSLShaderModel::SM_6_2:
            if (m_targetProfilesForShaderModel6_5.find(targetProfile) == m_targetProfilesForShaderModel6_5.end())
                targetProfile += L"_6_2";
            break;
        case rhi::HLSLShaderModel::SM_6_3:
            if (m_targetProfilesForShaderModel6_5.find(targetProfile) == m_targetProfilesForShaderModel6_5.end())
                targetProfile += L"_6_3";
            break;
        case rhi::HLSLShaderModel::SM_6_4:
            if (m_targetProfilesForShaderModel6_5.find(targetProfile) == m_targetProfilesForShaderModel6_5.end())
                targetProfile += L"_6_4";
            break;
        case rhi::HLSLShaderModel::SM_6_5:
            targetProfile += L"_6_5";
            break;
        case rhi::HLSLShaderModel::SM_6_6:
            targetProfile += L"_6_6";
            break;
        case rhi::HLSLShaderModel::SM_6_7:
            targetProfile += L"_6_7";
            break;
        }

        std::wstring newProfileName(compileArgs.back());
        std::string profileName;
        Utils::convert_string(newProfileName, profileName);
    }

    void set_shader_code_flags(ShaderInputInfo& inputInfo, std::vector<std::wstring>& compileArgs)
    {
        for (auto& define : inputInfo.defines)
        {
            compileArgs.push_back(L"-D");
            Utils::convert_string(define, compileArgs.emplace_back());
        }

        for (auto& includePath : inputInfo.includePaths)
        {
            compileArgs.push_back(L"-I");
            Utils::convert_string(includePath, compileArgs.emplace_back());
        }
        
        compileArgs.push_back(L"-E");
        Utils::convert_string(inputInfo.entryPoint, compileArgs.emplace_back());

        // Shader name for error messages
        Utils::convert_string(FileSystem::get_file_name(inputInfo.path), compileArgs.emplace_back());
    }
};

#endif // DXCOMPILER_ENABLED

enum class ShaderCacheType
{
    DXIL,
    SPIRV
};

class ShaderCache
{
public:
    static void init()
    {
        switch (rhi::get_api())
        {
        case rhi::API::VK:
            s_cacheType = ShaderCacheType::SPIRV;
            break;
        case rhi::API::D3D12:
            s_cacheType = ShaderCacheType::DXIL;
            break;
        }
    }

    static bool is_shader_outdated(const std::string& shaderRelativePath)
    {
        std::string shaderName = FileSystem::get_file_name(shaderRelativePath);
        std::string shaderBinObjectRelativePath = "intermediate/shader_cache";

        switch (s_cacheType)
        {
            case ShaderCacheType::DXIL:
                // TODO
                break;
            case ShaderCacheType::SPIRV:
                shaderBinObjectRelativePath += "/spirv/" + shaderName + ".spv";
                break;
        }

        std::string shaderMetadataRelativePath = "intermediate/shader_cache/metadata/" + shaderName + ".femeta";
        
        std::string shaderSourceAbsolutePath = FileSystem::get_absolute_path(shaderRelativePath);
        std::string shaderBinObjectAbsolutePath = FileSystem::get_absolute_path(shaderBinObjectRelativePath);
        std::string shaderBinObjectMetadataPath = FileSystem::get_absolute_path(shaderMetadataRelativePath);

        if (!FileSystem::exists(shaderBinObjectAbsolutePath) || !FileSystem::exists(shaderBinObjectMetadataPath))
            return true;

        uint64_t shaderSourceTimeStamp = FileSystem::get_last_write_time(shaderSourceAbsolutePath);
        uint64_t shaderBinObjectTimeStamp = FileSystem::get_last_write_time(shaderBinObjectAbsolutePath);

        if (shaderBinObjectTimeStamp < shaderSourceTimeStamp)
            return true;
        
        std::vector<uint8_t> outputData;
        FileSystem::read(shaderBinObjectMetadataPath, outputData);
        std::string strMetadata;
        strMetadata.resize(outputData.size());
        memcpy(strMetadata.data(), outputData.data(), outputData.size());

        nlohmann::json shaderBinObjectMetadata = nlohmann::json::parse(strMetadata);
        uint64_t dependencyTimeStamp = 0;
        for (auto& keyAndValue : shaderBinObjectMetadata.items())
        {
            std::string dependencyAbsolutePath = keyAndValue.key();
            if (FileSystem::exists(dependencyAbsolutePath))
            {
                dependencyTimeStamp = FileSystem::get_last_write_time(dependencyAbsolutePath);

                if (shaderBinObjectTimeStamp < dependencyTimeStamp)
                    return true;
            }
        }

        return false;
    }

    static void update_shader_cache(ShaderInputInfo& inputInfo, ShaderOutputInfo& outputInfo)
    {
        std::string shaderName = FileSystem::get_file_name(inputInfo.path);

        std::string shaderBinObjectRelativePath;
        get_shader_object_relative_path(shaderName, shaderBinObjectRelativePath);
        std::string shaderMetadataRelativePath = "intermediate/shader_cache/metadata/" + shaderName + ".aameta";

        std::string shaderBinObjectAbsolutePath = FileSystem::get_absolute_path(shaderBinObjectRelativePath);
        std::string shaderMetadataPath = FileSystem::get_absolute_path(shaderMetadataRelativePath);

        FileSystem::write(shaderBinObjectAbsolutePath, outputInfo.data, outputInfo.dataSize);

        nlohmann::json shaderMetadata;
        for (auto& dependency : outputInfo.dependencies)
        {
            shaderMetadata[dependency] = 0;
        }

        std::string strShaderMetadata = shaderMetadata.dump();
        FileSystem::write(shaderMetadataPath, strShaderMetadata);
    }

    static void load_shader_bin(const std::string& shaderName, std::vector<uint8_t>& outputData)
    {
        std::string relativePath;
        get_shader_object_relative_path(shaderName, relativePath);

        std::string absolutePath = FileSystem::get_absolute_path(relativePath);
        FileSystem::read(absolutePath, outputData);
    }

    static ShaderCacheType get_cache_type()
    {
        return s_cacheType;
    }

private:
    inline static ShaderCacheType s_cacheType;

    static void get_shader_object_relative_path(const std::string& shaderName, std::string& output)
    {
        output= "intermediate/shader_cache";

        switch (s_cacheType)
        {
            case ShaderCacheType::DXIL:
                // TODO
                break;
            case ShaderCacheType::SPIRV:
                output += "/spirv/" + shaderName + ".spv";
            break;
        }
    }
};

ShaderManager::ShaderManager()
{
#ifdef DXCOMPILER_ENABLED
    m_shaderCompiler.reset(new DXCompiler());
#endif // DXCOMPILER_ENABLED
    
    FE_CHECK(m_shaderCompiler);
    
    ShaderCache::init();
    m_taskGroup = TaskComposer::allocate_task_group();
    
    FE_LOG(LogShaderCompiler, INFO, "Shader Compiler initialization completed.");
}

ShaderManager::~ShaderManager()
{
    for (auto& [path, shader] : m_shaderByRelativePath)
        rhi::destroy_shader(shader);
}

rhi::Shader* ShaderManager::load_shader(
    const std::string& relativePath, 
    rhi::ShaderType shaderType,
    const std::string& entryPoint,
    rhi::HLSLShaderModel shaderModel,
    const std::vector<std::string>& defines
)
{
    if (rhi::Shader* shader = get_shader(relativePath))
        return shader;

    rhi::Shader* shader = nullptr;

    if (ShaderCache::is_shader_outdated(relativePath))
    {
        // LOG_INFO("SHADER {} IS OUTDATED", relativeShaderPath.c_str())
        std::string shaderExtension = FileSystem::get_file_extension(relativePath);
        if (shaderExtension != "hlsl")
        {
            FE_LOG(LogShaderCompiler, FATAL, "Can't load shader with extension {}", shaderExtension);

        }

        rhi::ShaderFormat shaderFormat{ rhi::ShaderFormat::UNDEFINED };
        switch (ShaderCache::get_cache_type())
        {
            case ShaderCacheType::SPIRV:
                shaderFormat = rhi::ShaderFormat::HLSL_TO_SPIRV;
                break;
            case ShaderCacheType::DXIL:
                shaderFormat = rhi::ShaderFormat::HLSL6;
                break;
        }
        
        ShaderInputInfo inputInfo;
        inputInfo.path = FileSystem::get_root_path() + "/" + "/src/fablex/shaders/" + relativePath;
        inputInfo.format = shaderFormat;
        inputInfo.type = shaderType;
        inputInfo.minHlslShaderModel = shaderModel;
        inputInfo.defines = defines;
        inputInfo.entryPoint = entryPoint;
        inputInfo.includePaths.push_back(FileSystem::get_root_path() + "/src/fablex/shaders");

        ShaderOutputInfo outputInfo;
        m_shaderCompiler->compile(inputInfo, outputInfo);
        ShaderCache::update_shader_cache(inputInfo, outputInfo);

        rhi::ShaderInfo shaderInfo;
        shaderInfo.shaderType = shaderType;
        shaderInfo.data = const_cast<uint8_t*>(outputInfo.data);
        shaderInfo.size = outputInfo.dataSize;

        rhi::create_shader(&shader, &shaderInfo);
        add_shader(relativePath, shader);
    }
    else
    {
        // LOG_INFO("LOAD SHADER")
        std::vector<uint8_t> shaderData;
        ShaderCache::load_shader_bin(FileSystem::get_file_name(relativePath), shaderData);

        rhi::ShaderInfo shaderInfo;
        shaderInfo.data = shaderData.data();
        shaderInfo.size = shaderData.size();
        shaderInfo.shaderType = shaderType;

        rhi::create_shader(&shader, &shaderInfo);
        add_shader(relativePath, shader);
    }

    return shader;
}

rhi::Shader* ShaderManager::get_shader(const std::string& relativePath)
{
    std::scoped_lock locker(m_mutex);

    auto it = m_shaderByRelativePath.find(relativePath);
    if (it == m_shaderByRelativePath.end())
        return nullptr;
    return it->second;
}

rhi::Shader* ShaderManager::get_shader(const ShaderMetadata& shaderMetadata)
{
    return load_shader(
        shaderMetadata.filePath,
        shaderMetadata.type,
        shaderMetadata.entryPoint,
        rhi::HLSLShaderModel::SM_6_7,
        shaderMetadata.defines
    );
}

void ShaderManager::add_shader(const std::string& relativePath, rhi::Shader* shader)
{
    FE_CHECK(shader);
    std::scoped_lock locker(m_mutex);
    m_shaderByRelativePath[relativePath] = shader;
}

void ShaderManager::request_shader_loading(const ShaderMetadata& shaderMetadata)
{
    TaskComposer::execute(*m_taskGroup, [&shaderMetadata, this](TaskExecutionInfo execInfo)
    {
        get_shader(shaderMetadata);
    });
}

void ShaderManager::wait_shaders_loading()
{
    TaskComposer::wait(*m_taskGroup);
}

}