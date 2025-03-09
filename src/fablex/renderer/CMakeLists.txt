file(GLOB RENDERER_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${RENDERER_SRC})

add_library(renderer STATIC ${RENDERER_SRC})

target_link_libraries(renderer core rhi dxc)

target_include_directories(renderer 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
