cmake_minimum_required(VERSION 3.10)
project(command_clipboard_manager 
        VERSION 1.0
        DESCRIPTION "A cross-platform clipboard manager for commands"
        LANGUAGES CXX)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Output directories - place executable in root directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})

# Compiler-specific options
if(MSVC)
    # Visual Studio specific flags
    add_compile_options(/W4 /fp:fast)
    
    # Enable multi-processor compilation
    add_compile_options(/MP)
    
    # Disable specific warnings
    add_compile_options(/wd4100) # unreferenced formal parameter
else()
    # GCC/Clang flags
    add_compile_options(-Wall -Wextra -Wpedantic)
    
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3)
    endif()
endif()

# Platform specific configuration
if(WIN32)
    # Windows specific settings
    add_compile_definitions(_WIN32)
    set(PLATFORM_LIBS "")
elseif(APPLE)
    # macOS specific settings
    add_compile_definitions(__APPLE__)
    find_library(CORE_FOUNDATION CoreFoundation)
    set(PLATFORM_LIBS ${CORE_FOUNDATION})
else()
    # Linux specific settings
    add_compile_definitions(__linux__)
    set(PLATFORM_LIBS "")
endif()

# Add executable
add_executable(clipman src/main.cpp)

# Set executable name based on platform
if(WIN32)
    set_target_properties(clipman 
        PROPERTIES 
        OUTPUT_NAME "clipman"
        SUFFIX ".exe"
    )
else()
    set_target_properties(clipman 
        PROPERTIES 
        OUTPUT_NAME "clipman"
    )
endif()

# Link platform-specific libraries
target_link_libraries(clipman PRIVATE ${PLATFORM_LIBS})

# Add filesystem library for older compilers if needed
target_link_libraries(clipman PRIVATE $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.1>>:stdc++fs>)

# Install rules (optional)
include(GNUInstallDirs)
install(TARGETS clipman
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Testing configuration
enable_testing()

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary")
message(STATUS "--------------------")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Generator: ${CMAKE_GENERATOR}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
if(WIN32)
    message(STATUS "Platform: Windows")
elseif(APPLE)
    message(STATUS "Platform: macOS")
else()
    message(STATUS "Platform: Linux")
endif()
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "")
