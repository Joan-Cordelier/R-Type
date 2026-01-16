cmake_minimum_required(VERSION 3.20)
project(RType VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Add subdirectories
add_subdirectory(src/server)
add_subdirectory(src/client)

# Enable testing
enable_testing()

#[[
Build instructions for different platforms

git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh  # On Linux/macOS

sudo dnf install -y ninja-build
sudo apt install autoconf autoconf-archive automake libtool

Linux:
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_INSTALLED_DIR=./vcpkg_installed -G Ninja
cmake --build build --config Release -j$(nproc)

]]
