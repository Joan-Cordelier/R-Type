file(GLOB_RECURSE ENGINE_SOURCES ../engine/*.cpp)
file(GLOB_RECURSE COMMON_SOURCES ../common/*.cpp)

set(CLIENT_SOURCES
    main.cpp
    ${ENGINE_SOURCES}
    ${COMMON_SOURCES}
)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)

add_executable(r-type_client ${CLIENT_SOURCES})

target_include_directories(r-type_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../engine)

target_compile_options(r-type_client PRIVATE -Wall -Wextra)
target_compile_features(r-type_client PRIVATE cxx_std_17)
target_link_libraries(r-type_client PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
)

set_target_properties(r-type_client PROPERTIES OUTPUT_NAME "r-type_client")
