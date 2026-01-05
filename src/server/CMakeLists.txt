file(GLOB_RECURSE COMMON_SOURCES ../common/*.cpp)
file(GLOB_RECURSE SERVER_SOURCES *.cpp)

# Remove SDL-dependent systems from server build
list(REMOVE_ITEM COMMON_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/../common/ecs/systems/button_system.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../common/ecs/systems/sprite_system.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../common/ecs/systems/spritesheet_system.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../common/ecs/systems/label_system.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../common/ecs/systems/slider_system.cpp"
)

add_executable(r-type_server
    ${SERVER_SOURCES}
    ${COMMON_SOURCES}
)

target_include_directories(r-type_server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Network
    ${CMAKE_CURRENT_SOURCE_DIR}/Session
    ${CMAKE_CURRENT_SOURCE_DIR}/Handler
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/Data
)

target_include_directories(r-type_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../common)

target_compile_options(r-type_server PRIVATE -Wall -Wextra)
target_compile_features(r-type_server PRIVATE cxx_std_17)
target_compile_definitions(r-type_server PRIVATE BUILD_SERVER)

set_target_properties(r-type_server PROPERTIES OUTPUT_NAME "r-type_server")
