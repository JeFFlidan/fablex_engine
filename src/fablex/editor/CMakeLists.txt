file(GLOB EDITOR_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*/*.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${EDITOR_SRC})

add_library(editor STATIC ${EDITOR_SRC})

target_include_directories(editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../)
target_link_libraries(editor core renderer engine)

target_include_directories(editor 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
