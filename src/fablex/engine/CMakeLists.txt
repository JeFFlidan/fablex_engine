file(GLOB ENGINE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_SRC})

add_library(engine STATIC ${ENGINE_SRC})

target_link_libraries(engine core asset_manager)

target_include_directories(engine 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
