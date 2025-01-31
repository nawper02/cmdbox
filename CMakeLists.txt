cmake_minimum_required(VERSION 3.10)
project(command_clipboard_manager VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(clipman src/main.cpp)

# Platform specific settings
if(WIN32)
    target_compile_definitions(clipman PRIVATE _WIN32)
elseif(APPLE)
    target_compile_definitions(clipman PRIVATE __APPLE__)
else()
    target_compile_definitions(clipman PRIVATE __linux__)
endif()

# Installation rules
install(TARGETS clipman DESTINATION bin)

# Enable testing
enable_testing()
