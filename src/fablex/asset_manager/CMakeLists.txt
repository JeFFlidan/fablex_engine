file(GLOB ASSET_MANAGER_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ASSET_MANAGER_SRC})

add_library(asset_manager STATIC ${ASSET_MANAGER_SRC})

target_link_libraries(asset_manager 
    core
    rhi
    stb
    basisu
    tinygltf
    ddsktx
)

target_include_directories(asset_manager 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
