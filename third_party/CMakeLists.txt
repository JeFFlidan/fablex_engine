add_library(volk INTERFACE)
target_include_directories(volk INTERFACE volk vulkan)

add_library(vma INTERFACE)
target_include_directories(vma INTERFACE vma)

add_library(tinygltf INTERFACE)
target_include_directories(tinygltf INTERFACE tinygltf)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)

add_library(fmt)
target_compile_options(fmt PUBLIC "/utf-8")
set_target_properties(fmt PROPERTIES CXX_SCAN_FOR_MODULES OFF)

target_include_directories(fmt PUBLIC fmt)

file(GLOB FMT_HEADERS fmt/fmt/*.h)
set(FMT_SOURCES fmt/fmt/format.cc)

target_sources(fmt PRIVATE ${FMT_HEADERS} ${FMT_SOURCES})

add_library(spirv_reflect)
target_include_directories(spirv_reflect PUBLIC spirv_reflect)
target_sources(spirv_reflect PRIVATE
    spirv_reflect/spirv_reflect.h
    spirv_reflect/spirv_reflect.c
)

add_library(dxc INTERFACE)
target_include_directories(dxc INTERFACE dxc)
target_link_libraries(dxc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/dxc/dxcompiler.lib)

add_library(json INTERFACE)
target_include_directories(json INTERFACE json)

add_library(lz4 STATIC)
target_include_directories(lz4 PUBLIC lz4)
target_sources(lz4 PRIVATE
    lz4/lz4.c
    lz4/lz4.h
)

add_library(basisu STATIC)
target_include_directories(basisu PUBLIC basis_universal)
file(GLOB BASISU_FILES
    basis_universal/zstd/*.h
    basis_universal/zstd/zstd.с
    basis_universal/encoder/*.h
    basis_universal/encoder/*.cpp
    basis_universal/encoder/*/*.h
    basis_universal/encoder/*/*.cpp
    basis_universal/transcoder/*.h
    basis_universal/transcoder/*.cpp
)
target_sources(basisu PRIVATE ${BASISU_FILES})
target_link_libraries(basisu zstd)
