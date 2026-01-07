file(GLOB_RECURSE ENGINE_SOURCES ../engine/*.cpp)
file(GLOB_RECURSE COMMON_SOURCES ../common/*.cpp)
file(GLOB_RECURSE CLIENT_SOURCES *.cpp)

set(CLIENT_SOURCES
    ${CLIENT_SOURCES}
    ${ENGINE_SOURCES}
    ${COMMON_SOURCES}
)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

add_executable(r-type_client ${CLIENT_SOURCES})

target_include_directories(r-type_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../engine ../common)

target_compile_options(r-type_client PRIVATE -Wall -Wextra)
target_compile_features(r-type_client PRIVATE cxx_std_17)
target_link_libraries(r-type_client PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
    $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
    yaml-cpp::yaml-cpp
)

set_target_properties(r-type_client PROPERTIES OUTPUT_NAME "r-type_client")
