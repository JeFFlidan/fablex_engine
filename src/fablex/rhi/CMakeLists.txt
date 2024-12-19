file(GLOB RHI_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${RHI_SRC})

add_library(rhi STATIC ${RHI_SRC})

if (WIN32)
    file(GLOB VULKAN_RHI_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/*.h
    )

    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VULKAN_RHI_SRC})

    target_sources(rhi PRIVATE ${VULKAN_RHI_SRC})
    target_link_libraries(rhi volk)
endif()

target_include_directories(rhi 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

