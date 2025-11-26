set(SERVER_SOURCES
    main.cpp
    ../common/Data/Queue.cpp
    ../common/Data/ThreadedQueue.cpp
    ../common/Data/MessageFactory.cpp
    Network/AServer.cpp
    Network/TCPServer.cpp
    Network/UDPServer.cpp
)

add_executable(r-type_server ${SERVER_SOURCES})

target_include_directories(r-type_server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Network
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/Data
)

target_compile_options(r-type_server PRIVATE -Wall -Wextra)
target_compile_features(r-type_server PRIVATE cxx_std_17)

set_target_properties(r-type_server PROPERTIES OUTPUT_NAME "r-type_server")
