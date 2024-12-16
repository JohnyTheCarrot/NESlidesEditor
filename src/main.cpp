#include <iostream>
#include <span>
#include <array>
#include "subprocess.h"

void start_process(std::span<char const *> command) {
    subprocess_s subprocess{};
    if (int const result{subprocess_create(command.data(), subprocess_option_inherit_environment, &subprocess)}; result != 0) {
        std::cerr << "Failed to create subprocess." << std::endl;
        exit(1);
    }

    int process_return{};
    if (const int result{subprocess_join(&subprocess, &process_return)}; result != 0) {
        std::cerr << "Failed to join subprocess." << std::endl;
        exit(1);
    }

    std::cout << "Process returned: " << process_return << std::endl;
}

#ifdef _WIN32
#define MAKE ".\\bin\\make.exe"
#define OUTPUT_FOLDER "..\\output"
#define CA65 "..\\bin\\ca65.exe"
#define LD65 "..\\bin\\ld65.exe"
#define OS_OPTION "OS=Windows_NT"
#else
#define MAKE "./bin/make"
#define OUTPUT_FOLDER "../output"
#define CA65 "../bin/ca65"
#define LD65 "../bin/ld65"
#define OS_OPTION nullptr
#endif

int main() {
    std::cout << "Hello, World!" << std::endl;

    {
        std::array<char const *, 7> command{MAKE, "clean", "-C", "neslides", "OUT_DIR=" OUTPUT_FOLDER, OS_OPTION, nullptr};
        start_process(command);
    }

    {
        std::array<char const *, 9> command{MAKE, "all", "-C", "neslides", "CA65=" CA65, "LD65=" LD65, "OUT_DIR=" OUTPUT_FOLDER, OS_OPTION, nullptr};
        start_process(command);
    }

    return 0;
}