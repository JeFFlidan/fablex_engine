file(GLOB MAIN_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h    
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${MAIN_SRC})

add_executable(main ${MAIN_SRC})

set_target_name_exe(main "fablex")
set_engine_out_dir(main ${CMAKE_SOURCE_DIR}/bin)

target_link_options(main PRIVATE "/WHOLEARCHIVE:core.lib")
target_link_options(main PRIVATE "/WHOLEARCHIVE:engine.lib")
target_link_options(main PRIVATE "/WHOLEARCHIVE:renderer.lib")
target_link_libraries(main core rhi renderer engine asset_manager)

target_include_directories(main 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
