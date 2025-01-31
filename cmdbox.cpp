#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <iomanip>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

namespace fs = std::filesystem;

// Platform-specific definitions
#ifdef _WIN32
    const std::string CLEAR_COMMAND = "cls";
    const char KEY_ESCAPE = 27;
    const char KEY_BACKSPACE = 8;
#else
    const std::string CLEAR_COMMAND = "clear";
    const char KEY_ESCAPE = 27;
    const char KEY_BACKSPACE = 127;
#endif

enum class Mode {
    NORMAL,
    CREATE_FOLDER,
    CREATE_COMMAND,
    DELETE,
    MOVE,
    MOVE_NAVIGATE,
    QUIT
};

class TerminalUI {
public:
    #ifdef _WIN32
        static const std::string RESET;
        static const std::string BLUE;
        static const std::string GREEN;
        static const std::string YELLOW;
        static const std::string RED;
        static const std::string CYAN;
        static const std::string BOLD;
        static const std::string DIM;
    #else
        static const std::string RESET;
        static const std::string BLUE;
        static const std::string GREEN;
        static const std::string YELLOW;
        static const std::string RED;
        static const std::string CYAN;
        static const std::string BOLD;
        static const std::string DIM;
    #endif
};

#ifdef _WIN32
    const std::string TerminalUI::RESET = "";
    const std::string TerminalUI::BLUE = "";
    const std::string TerminalUI::GREEN = "";
    const std::string TerminalUI::YELLOW = "";
    const std::string TerminalUI::RED = "";
    const std::string TerminalUI::CYAN = "";
    const std::string TerminalUI::BOLD = "";
    const std::string TerminalUI::DIM = "";
#else
    const std::string TerminalUI::RESET = "\033[0m";
    const std::string TerminalUI::BLUE = "\033[34m";
    const std::string TerminalUI::GREEN = "\033[32m";
    const std::string TerminalUI::YELLOW = "\033[33m";
    const std::string TerminalUI::RED = "\033[31m";
    const std::string TerminalUI::CYAN = "\033[36m";
    const std::string TerminalUI::BOLD = "\033[1m";
    const std::string TerminalUI::DIM = "\033[2m";
#endif

class ClipboardManager {
private:
    fs::path current_path;
    std::vector<fs::path> current_items;
    const std::string root_dir = "commands";
    const int TERM_WIDTH = 80;
    Mode current_mode = Mode::NORMAL;
    fs::path item_to_move;
    std::string moving_item_name;

    // Cross-platform getch implementation
    char getch() {
    #ifdef _WIN32
        return _getch();
    #else
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0) perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0) perror("tcsetattr ICANON");
        if (read(0, &buf, 1) < 0) perror("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0) perror("tcsetattr ~ICANON");
        return buf;
    #endif
    }

    // Cross-platform clipboard implementation
    void copyToClipboard(const std::string& text) {
    #ifdef _WIN32
        if (OpenClipboard(nullptr)) {
            EmptyClipboard();
            HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
            if (hg) {
                memcpy(GlobalLock(hg), text.c_str(), text.size() + 1);
                GlobalUnlock(hg);
                SetClipboardData(CF_TEXT, hg);
            }
            CloseClipboard();
        }
    #elif __APPLE__
        FILE* pipe = popen("pbcopy", "w");
        if (pipe) {
            fwrite(text.c_str(), sizeof(char), text.size(), pipe);
            pclose(pipe);
        }
    #else
        // For Linux, try xclip first, then xsel
        FILE* pipe = popen("which xclip > /dev/null 2>&1 && echo 'xclip' || echo 'xsel'", "r");
        char buf[10];
        fgets(buf, sizeof(buf), pipe);
        pclose(pipe);
        
        std::string cmd;
        if (std::string(buf).find("xclip") != std::string::npos) {
            cmd = "xclip -selection clipboard";
        } else {
            cmd = "xsel -ib";
        }
        
        pipe = popen(cmd.c_str(), "w");
        if (pipe) {
            fwrite(text.c_str(), sizeof(char), text.size(), pipe);
            pclose(pipe);
        }
    #endif
    }

    void clearScreen() {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }

    void sleep(int milliseconds) {
        #ifdef _WIN32
            Sleep(milliseconds);
        #else
            usleep(milliseconds * 1000);
        #endif
    }

    // Rest of the ClipboardManager class remains the same as before...

    void printCentered(const std::string& text, char fill = ' ') {
        int padding = (TERM_WIDTH - text.length()) / 2;
        std::cout << std::string(padding, fill) << text << std::string(padding, fill);
        if ((text.length() + 2 * padding) < TERM_WIDTH) std::cout << fill;
        std::cout << "\n";
    }

    void printBanner(const std::string& subtitle = "") {
        std::string color = TerminalUI::BLUE;
        if (current_mode == Mode::DELETE) color = TerminalUI::RED;
        else if (current_mode != Mode::NORMAL) color = TerminalUI::GREEN;

        std::cout << color << TerminalUI::BOLD;
        printCentered("", '=');
        printCentered(" Command Clipboard Manager ");
        if (!subtitle.empty()) {
            printCentered(subtitle);
        }
        printCentered("", '=');
        std::cout << TerminalUI::RESET << "\n";
    }

    void printPath() {
        std::cout << TerminalUI::DIM << "Location: ";
        std::string path_str = current_path.string();
        std::string relative_path = path_str.substr(path_str.find(root_dir));
        std::cout << TerminalUI::CYAN << relative_path << TerminalUI::RESET << "\n\n";
    }

    void updateCurrentItems() {
        current_items.clear();
        std::vector<fs::path> dirs, files;
        for (const auto& entry : fs::directory_iterator(current_path)) {
            if (fs::is_directory(entry)) {
                dirs.push_back(entry.path());
            } else {
                files.push_back(entry.path());
            }
        }
        std::sort(dirs.begin(), dirs.end());
        std::sort(files.begin(), files.end());
        current_items = dirs;
        current_items.insert(current_items.end(), files.begin(), files.end());
    }

    std::string readFileContents(const fs::path& path) {
        std::ifstream file(path);
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        return content;
    }

    void displayNormalMode() {
        std::cout << TerminalUI::GREEN << " Commands:" << TerminalUI::RESET << "\n";
        std::cout << " [N]ew folder   [C]reate command   [D]elete item   [M]ove item   [ESC] back\n";
        std::cout << " [←] back       [ESC] exit\n\n";
    }

    void displayMoveMode() {
        if (current_mode == Mode::MOVE) {
            std::cout << TerminalUI::YELLOW << " MOVE MODE" << TerminalUI::RESET << "\n";
            std::cout << " Select item to move or [ESC] to cancel\n\n";
        } else if (current_mode == Mode::MOVE_NAVIGATE) {
            std::cout << TerminalUI::YELLOW << " MOVE MODE - SELECTING DESTINATION" << TerminalUI::RESET << "\n";
            std::cout << " Moving: " << moving_item_name << "\n";
            std::cout << " Press [ENTER] to move here, [BACKSPACE] to go back, or select a folder\n\n";
        }
    }

    void displayDeleteMode() {
        std::cout << TerminalUI::RED << " DELETE MODE" << TerminalUI::RESET << "\n";
        std::cout << " Select item to delete or [ESC] to cancel\n\n";
    }

    void displayCreateMode(const std::string& type) {
        std::cout << TerminalUI::GREEN << " CREATE " << type << TerminalUI::RESET << "\n";
        std::cout << " Enter name or [ESC] to cancel\n\n";
        std::cout << " Name: ";
    }

    void displayItems() {
        clearScreen();
        
        switch (current_mode) {
            case Mode::NORMAL:
                printBanner();
                break;
            case Mode::DELETE:
                printBanner(" -- DELETE MODE --");
                break;
            case Mode::CREATE_FOLDER:
                printBanner(" -- CREATE FOLDER --");
                break;
            case Mode::CREATE_COMMAND:
                printBanner(" -- CREATE COMMAND --");
                break;
            case Mode::MOVE:
            case Mode::MOVE_NAVIGATE:
                printBanner(" -- MOVE MODE --");
                break;
            case Mode::QUIT:
                printBanner(" -- QUIT --");
                break;
        }
        
        printPath();
        
        if (current_items.empty()) {
            std::cout << TerminalUI::DIM << "  (empty directory)" << TerminalUI::RESET << "\n\n";
        }
        
        int idx = 0;
        for (const auto& item : current_items) {
            std::string prefix = current_mode == Mode::DELETE ? " [X] " : " ";
            char key = 'a' + idx;
            std::cout << prefix << TerminalUI::BOLD << key << TerminalUI::RESET << " | ";
            if (fs::is_directory(item)) {
                std::cout << TerminalUI::BLUE << "[DIR] " << item.filename().string() << TerminalUI::RESET;
            } else {
                std::cout << item.filename().string();
            }
            std::cout << '\n';
            idx++;
            if (idx >= 26) break;
        }
        
        std::cout << "\n" << std::string(TERM_WIDTH, '-') << "\n\n";
        
        switch (current_mode) {
            case Mode::NORMAL:
                displayNormalMode();
                break;
            case Mode::DELETE:
                displayDeleteMode();
                break;
            case Mode::CREATE_FOLDER:
                displayCreateMode("FOLDER");
                break;
            case Mode::CREATE_COMMAND:
                displayCreateMode("COMMAND");
                break;
            case Mode::MOVE:
            case Mode::MOVE_NAVIGATE:
                displayMoveMode();
                break;
            case Mode::QUIT:
                std::cout << TerminalUI::RED << " QUIT" << TerminalUI::RESET << "\n";
                std::cout << " Are you sure you want to quit? [Y/N]\n\n";
                break;
        }
    }

    void createFolder() {
        std::string name;
        current_mode = Mode::CREATE_FOLDER;
        
        while (true) {
            displayItems();
            char input = getch();
            if (input == KEY_ESCAPE) {
                break;
            } else if (input == '\n' && !name.empty()) {
                fs::create_directory(current_path / name);
                updateCurrentItems();
                break;
            } else if (input == KEY_BACKSPACE && !name.empty()) {
                name.pop_back();
            } else if (isprint(input)) {
                name += input;
            }
            std::cout << "\r Name: " << name << std::string(20, ' ') << "\r Name: " << name;
            std::cout.flush();
        }
        
        current_mode = Mode::NORMAL;
    }

    void createCommand() {
        std::string name, content;
        current_mode = Mode::CREATE_COMMAND;
        bool entering_name = true;
        
        while (true) {
            displayItems();
            if (entering_name) {
                std::cout << "\r Name: " << name << std::string(20, ' ') << "\r Name: " << name;
            } else {
                std::cout << "\r Content: " << content << std::string(20, ' ') << "\r Content: " << content;
            }
            std::cout.flush();
            
            char input = getch();
            if (input == KEY_ESCAPE) {
                break;
            } else if (input == '\n') {
                if (entering_name && !name.empty()) {
                    entering_name = false;
                } else if (!entering_name && !content.empty()) {
                    std::ofstream file(current_path / (name + ".cmd"));
                    file << content;
                    file.close();
                    updateCurrentItems();
                    break;
                }
            } else if (input == KEY_BACKSPACE) {
                if (entering_name && !name.empty()) {
                    name.pop_back();
                } else if (!entering_name && !content.empty()) {
                    content.pop_back();
                }
            } else if (isprint(input)) {
                if (entering_name) {
                    name += input;
                } else {
                    content += input;
                }
            }
        }
        
        current_mode = Mode::NORMAL;
    }

    void enterDeleteMode() {
        current_mode = Mode::DELETE;
        displayItems();
        while (true) {
            char key = getch();
            if (key == KEY_ESCAPE) {
                break;
            }
            int idx = key - 'a';
            if (idx >= 0 && idx < current_items.size()) {
                fs::remove_all(current_items[idx]);
                updateCurrentItems();
                break;
            }
        }
        current_mode = Mode::NORMAL;
    }

    void enterMoveMode() {
        current_mode = Mode::MOVE;
        displayItems();
        
        // First, select the item to move
        char key = getch();
        if (key == KEY_ESCAPE) {
            current_mode = Mode::NORMAL;
            return;
        }
        
        int idx = key - 'a';
        if (idx < 0 || idx >= current_items.size()) {
            current_mode = Mode::NORMAL;
            return;
        }
        
        // Store the item to move and switch to navigation mode
        item_to_move = current_items[idx];
        moving_item_name = item_to_move.filename().string();
        current_mode = Mode::MOVE_NAVIGATE;
        
        fs::path source_path = current_path;
        
        // Navigation loop
        while (true) {
            displayItems();
            
            key = getch();
            if (key == KEY_ESCAPE) {
                break;
            }
            else if (key == '\n') {
                // Only move if we're not in the same directory
                if (current_path != source_path) {
                    fs::path dest_path = current_path / item_to_move.filename();
                    fs::rename(item_to_move, dest_path);
                }
                break;
            }
            else if (key == KEY_BACKSPACE && current_path != fs::absolute(root_dir)) {
                current_path = current_path.parent_path();
                updateCurrentItems();
            }
            else if (key >= 'a' && key < 'a' + current_items.size()) {
                idx = key - 'a';
                if (fs::is_directory(current_items[idx])) {
                    current_path = current_items[idx];
                    updateCurrentItems();
                }
            }
        }
        
        // Reset to original directory
        current_path = source_path;
        updateCurrentItems();
        current_mode = Mode::NORMAL;
    }

    void showNotification(const std::string& message) {
        std::cout << "\n" << TerminalUI::GREEN << " * " << message << TerminalUI::RESET;
        std::cout.flush();
        sleep(1000);
    }

public:
    ClipboardManager() {
        #ifdef _WIN32
            // Enable ANSI escape sequences for Windows 10 and later
            HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD dwMode = 0;
            GetConsoleMode(hOut, &dwMode);
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        #endif

        current_path = fs::absolute(root_dir);
        if (!fs::exists(current_path)) {
            fs::create_directory(current_path);
        }
        updateCurrentItems();
    }

    void run() {
        while (true) {
            displayItems();
            char input = getch();

            if (input == KEY_ESCAPE) {
                if (current_mode != Mode::NORMAL && current_mode != Mode::QUIT) {
                    current_mode = Mode::NORMAL;
                    continue;
                }
                if (current_mode == Mode::QUIT) {
                    displayItems();
                    char confirm = getch();
                    if (confirm == 'Y' || confirm == 'y') {
                        clearScreen();
                        break;
                    }
                    current_mode = Mode::NORMAL;
                } else if (current_path != fs::absolute(root_dir)) {
                    current_path = current_path.parent_path();
                    updateCurrentItems();
                } else {
                    current_mode = Mode::QUIT;
                    displayItems();
                }
            }
            else if (input == 'N' && current_mode == Mode::QUIT) {
                current_mode = Mode::NORMAL;
            }
            else if (input == 'Y' && current_mode == Mode::QUIT) {
                clearScreen();
                break;
            }
            else if (input == 'N' && current_mode == Mode::NORMAL) {
                createFolder();
            }
            else if (input == 'C' && current_mode == Mode::NORMAL) {
                createCommand();
            }
            else if (input == 'D' && current_mode == Mode::NORMAL) {
                enterDeleteMode();
            }
            else if (input == 'M' && current_mode == Mode::NORMAL) {
                enterMoveMode();
            }
            else if (input >= 'a' && input < 'a' + current_items.size()) {
                int idx = input - 'a';
                if (current_mode == Mode::DELETE) {
                    fs::remove_all(current_items[idx]);
                    updateCurrentItems();
                    current_mode = Mode::NORMAL;
                } else if (current_mode == Mode::NORMAL) {
                    if (fs::is_directory(current_items[idx])) {
                        current_path = current_items[idx];
                        updateCurrentItems();
                    } else {
                        std::string command = readFileContents(current_items[idx]);
                        copyToClipboard(command);
                        showNotification("Copied to clipboard: " + command);
                    }
                }
            }
        }
    }
};

int main() {
    ClipboardManager manager;
    manager.run();
    return 0;
}
