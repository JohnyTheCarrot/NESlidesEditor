#include <iostream>
#include <span>
#include <array>
#include <fstream>
#include <format>

#include "subprocess.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

[[nodiscard]]
bool start_process(std::span<char const *> command) {
    subprocess_s subprocess{};
    if (int const result{subprocess_create(command.data(), subprocess_option_inherit_environment, &subprocess)}; result != 0) {
        return false;
    }

    int process_return{};
    if (const int result{subprocess_join(&subprocess, &process_return)}; result != 0) {
        return false;
    }

    return true;
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

using Slides = std::vector<std::unique_ptr<std::string>>;

[[nodiscard]]
bool Export(Slides const &input) {
    std::stringstream stream;
    stream << ".rodata\nslides:\n";

    for (auto it{input.begin()}; it != input.end(); ++it) {
        std::string line;
        std::stringstream input_stream{**it};
        while (std::getline(input_stream, line, '\n')) {
            stream << ".byte ";

            for (auto c : line) {
                stream << toupper(c) << ", ";
            }

            stream << "NEWLINE\n";
        }

        if (it + 1 != input.end()) {
            stream << ".byte NEXT_SLIDE\n";
        } else {
            stream << ".byte LAST_SLIDE\n";
        }
    }

    std::ofstream file{"neslides/src/segments/slides.s65"};
    if (!file.is_open())
        return false;

    file << stream.str();

    file.close();

    std::array<char const *, 7> cleanCmd{MAKE, "clean", "-C", "neslides", "OUT_DIR=" OUTPUT_FOLDER, OS_OPTION, nullptr};
    if (!start_process(cleanCmd))
        return false;

    std::array<char const *, 9> buildCmd{MAKE, "all", "-C", "neslides", "CA65=" CA65, "LD65=" LD65, "OUT_DIR=" OUTPUT_FOLDER, OS_OPTION, nullptr};
    return start_process(buildCmd);
}

[[nodiscard]]
ftxui::Component SuccessModal(std::function<void()> const &okay_clicked) {
    using namespace ftxui;

    auto component = Container::Vertical({
        Button("Okay", okay_clicked),
    });

    component |= Renderer([&](Element inner) {
        return vbox({
            text(L"Success!"),
            separator(),
            std::move(inner),
        })
            | size(WIDTH, GREATER_THAN, 30)
            | border;
    });

    return component;
}

[[nodiscard]]
ftxui::Component ErrorModal(std::function<void()> const &okay_clicked) {
    using namespace ftxui;

    auto component = Container::Vertical({
        Button("Okay", okay_clicked),
    });

    component |= Renderer([&](Element inner) {
        return vbox({
            text(L"Something went wrong."),
            separator(),
            std::move(inner),
        })
            | size(WIDTH, GREATER_THAN, 30)
            | border;
    });

    return component;
}

constexpr int c_MaxColumns{26};
constexpr int c_MaxRows{27};

int main() {
    using namespace ftxui;

    auto screen{ScreenInteractive::Fullscreen()};

    bool success_shown = false;
    auto const show_success{[&]{ success_shown = true; }};
    auto const hide_success{[&]{ success_shown = false; }};

    bool error_shown = false;
    auto const show_error{[&]{ error_shown = true; }};
    auto const hide_error{[&]{ error_shown = false; }};

    Slides slides;
    slides.emplace_back(std::make_unique<std::string>(""));

    std::vector<std::string> slide_titles{"Slide 0"};

    std::vector slide_inputs{Input(slides.back().get()) | border};

    int current_slide_index{0};
    auto tabs{Container::Tab({
        slide_inputs.back()
    }, &current_slide_index)};

    auto const add_slide{[&] {
        slides.emplace_back(std::make_unique<std::string>(""));
        slide_inputs.emplace_back(Input(slides.back().get()) | border);
        tabs->Add(slide_inputs.back());
        slide_titles.emplace_back(std::format("Slide {}", slides.size() - 1));
    }};

    auto const export_button = Button("Export", [&] {
        if (Export(slides))
            show_success();
        else
            show_error();
    }, ButtonOption::Ascii());

    auto const new_slide = Button("New Slide", add_slide, ButtonOption::Ascii());

    auto const tab_toggle = Toggle(&slide_titles, &current_slide_index);

    auto const component = Container::Vertical({
        export_button,
        tab_toggle,
        new_slide,
        tabs
    });

    auto renderer = Renderer(component, [&] {
        auto const current_rows{std::ranges::count(std::as_const(*slides[current_slide_index]), '\n')};
        bool does_exceed_max_rows{current_rows >= c_MaxRows - 1};

        return vbox({
            hbox({
                text("NESlides Editor"),
                separator(),
                export_button->Render(),
                new_slide->Render(),
                separator(),
                tab_toggle->Render()
            }),
            separator(),
            tabs->Render() | size(WIDTH, EQUAL, c_MaxColumns ) | size(HEIGHT, EQUAL, c_MaxRows + 1),
            text(std::format("{} rows remaining", c_MaxRows - current_rows - 2)) | color(does_exceed_max_rows ? Color::Red : Color::White)
        }) | border;
    });

    auto const success_modal{SuccessModal(hide_success)};
    auto const error_modal{ErrorModal(hide_error)};

    renderer |= Modal(success_modal, &success_shown);
    renderer |= Modal(error_modal, &error_shown);

    screen.Loop(renderer);
    return 0;
}