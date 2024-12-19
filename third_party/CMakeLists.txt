add_library(volk INTERFACE)
add_library(vma INTERFACE)

target_include_directories(volk INTERFACE volk vulkan)
target_include_directories(vma INTERFACE vma)
