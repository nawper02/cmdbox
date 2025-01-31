# Command Clipboard Manager

A cross-platform terminal-based utility for managing frequently used commands and text snippets. Organize your commands in folders and quickly copy them to your clipboard.

## Features

- 📋 Quick clipboard access to stored commands
- 📁 Folder organization for commands
- 🎯 Easy navigation with keyboard shortcuts
- 🖥️ Cross-platform support (Windows, macOS, Linux)
- 🎨 Color-coded interface (on supported terminals)
- 📝 Create and edit commands directly in the terminal
- 🚀 Fast and lightweight

## Requirements

### All Platforms
- C++17 or later
- CMake 3.10 or later
- A C++ compiler (gcc, clang, or MSVC)

### Platform-Specific Requirements

#### Linux
- xclip or xsel for clipboard support
- A terminal that supports ANSI escape sequences

#### macOS
- No additional requirements (uses pbcopy for clipboard)

#### Windows
- Windows 10 or later for ANSI color support
- Visual Studio 2019 or later (recommended)

## Building from Source

1. Clone the repository:
```bash
git clone https://github.com/yourusername/command-clipboard-manager.git
cd command-clipboard-manager
```

2. Create build directory:
```bash
mkdir build
cd build
```

3. Build the project:
```bash
cmake ..
cmake --build .
```

## Usage

### Basic Navigation
- Use letters (a-z) to select items
- `ESC` to go back/quit
- `←` or `Backspace` to navigate to parent directory

### Commands
- `N` - Create new folder
- `C` - Create new command
- `D` - Delete item
- `M` - Move item
- Select any command to copy it to clipboard

### File Management
- Commands are stored in the `commands` directory
- Commands are saved with `.cmd` extension
- Folders can be nested for better organization

## Project Structure

```
command-clipboard-manager/
├── src/
│   └── main.cpp
├── CMakeLists.txt
├── README.md
└── commands/         # Created on first run
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

### Development Guidelines
1. Follow the existing code style
2. Maintain cross-platform compatibility
3. Test on all supported platforms before submitting PR
4. Update documentation as needed

## Building for Different Platforms

### Windows
```bash
cmake -G "Visual Studio 16 2019" ..
cmake --build . --config Release
```

### macOS/Linux
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

## Known Issues

1. Windows Command Prompt has limited color support
2. Some terminals may not display Unicode characters correctly

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Thanks to the C++ Standard Library contributors for filesystem support
- Inspired by various clipboard managers and command-line tools
