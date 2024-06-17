cmake_minimum_required(VERSION 3.10)
project(Racing)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)

# Set the path to MinGW
set(CMAKE_C_COMPILER "C:/mingw64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "C:/mingw64/bin/g++.exe")

# Set SFML directory
set(SFML_DIR "C:/SFML/lib/cmake/SFML")

# Find SFML package
find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)

# Include directories
include_directories(headers)
include_directories(C:/SFML/include)

# Add the executable
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(Racing ${SOURCES})

# Link SFML libraries
target_link_libraries(Racing sfml-system sfml-window sfml-graphics)

# Custom command to copy DLLs to the build directory
add_custom_command(TARGET Racing POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "C:/SFML/bin"
    $<TARGET_FILE_DIR:Racing>
)
