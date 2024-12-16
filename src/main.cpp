#include <iostream>
#include <span>
#include "subprocess.h"

void start_process(std::span<char const *> command) {
    subprocess_s subprocess{};
    if (int const result{subprocess_create(command.data(), 0, &subprocess)}; result != 0) {
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
#define CA65 "../bin/ca65.exe"
#define LD65 "../bin/ld65.exe"
#else
#define CA65 "../bin/ca65"
#define LD65 "../bin/ld65"
#endif

int main() {
    std::cout << "Hello, World!" << std::endl;

    {
        std::array<char const *, 6> command{"bin/make", "clean", "-C", "neslides", "OUT_DIR=../output", nullptr};
        start_process(command);
    }

    {
        std::array<char const *, 8> command{"bin/make", "all", "-C", "neslides", "CA65=" CA65, "LD65=" LD65, "OUT_DIR=../output", nullptr};
        start_process(command);
    }

    return 0;
}