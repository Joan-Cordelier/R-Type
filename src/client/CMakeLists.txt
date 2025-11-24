set(CLIENT_SOURCES
    main.cpp
)

add_executable(r-type_client ${CLIENT_SOURCES})

target_include_directories(r-type_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(r-type_client PRIVATE -Wall -Wextra)
target_compile_features(r-type_client PRIVATE cxx_std_17)

set_target_properties(r-type_client PROPERTIES OUTPUT_NAME "r-type_client")
