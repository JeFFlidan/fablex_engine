add_library(volk INTERFACE)
target_include_directories(volk INTERFACE volk vulkan)

add_library(vma INTERFACE)
target_include_directories(vma INTERFACE vma)

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