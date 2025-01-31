#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Error handling
set -e

echo -e "${GREEN}Building Command Clipboard Manager...${NC}\n"

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo -e "${YELLOW}Creating build directory...${NC}"
    mkdir build
fi

# Navigate to build directory
cd build

# Check if running in Debug or Release mode
BUILD_TYPE="Release"
if [ "$1" = "debug" ]; then
    BUILD_TYPE="Debug"
    echo -e "${YELLOW}Building in Debug mode...${NC}"
else
    echo -e "${YELLOW}Building in Release mode...${NC}"
fi

# Configure with CMake
echo -e "${YELLOW}Configuring CMake...${NC}"
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..

# Build the project
echo -e "${YELLOW}Compiling...${NC}"
cmake --build .

# Check if build was successful
if [ $? -eq 0 ]; then
    echo -e "\n${GREEN}Build successful!${NC}"
    echo -e "${YELLOW}Executable is located in the root directory${NC}"
    cd ..
    
    # Make the executable executable (if on Unix)
    if [ -f "clipman" ]; then
        chmod +x clipman
        echo -e "${GREEN}Execute with: ${YELLOW}./clipman${NC}"
    fi
else
    echo -e "\n${RED}Build failed!${NC}"
    exit 1
fi
