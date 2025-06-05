file(GLOB ENGINE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/engine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/engine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/events.h
    ${CMAKE_CURRENT_SOURCE_DIR}/fwd.h
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

if (BUILD_TESTS)
    add_executable(engine_tests tests.cpp)
    set_engine_out_dir(engine_tests ${CMAKE_SOURCE_DIR}/bin)
    target_link_libraries(engine_tests engine doctest)

    target_include_directories(engine_tests 
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../
    )
endif()
