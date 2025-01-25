#pragma once

#include "rhi/resources.h"

namespace fe::renderer
{

void init_shader_compiler();

// Relative path must not include src/fablex/shaders, only file names or subfolders of shaders + file names
rhi::Shader* load_shader(
    const std::string& relativePath, 
    rhi::ShaderType shaderType, 
    rhi::HLSLShaderModel shaderModel = rhi::HLSLShaderModel::SM_6_7,
    const std::vector<std::string>& defines = {}
);

rhi::Shader* get_shader(const std::string& relativePath);

}