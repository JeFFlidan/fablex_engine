file(GLOB CORE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*/*.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${CORE_SRC})

add_library(core STATIC ${CORE_SRC})

target_include_directories(core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../)
