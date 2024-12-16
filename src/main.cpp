#include <iostream>
#include <span>
#include <array>
#include <fstream>
#include <format>

#include "tinyfiledialogs.h"
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

constexpr std::array c_ExportExtensions{"*.neslides"};

void SaveSlides(Slides const &slides) {
    char const *const file_path{tinyfd_saveFileDialog("Save Slides", "", c_ExportExtensions.size(), c_ExportExtensions.data(), nullptr)};
    if (!file_path)
        return;

    std::ofstream file{file_path, std::ios::binary};

    if (!file.is_open())
        return;

    for (auto const &slide : slides) {
        file.write(slide->c_str(), slide->size());
        file.put('\0');
    }
}

void LoadSlides(Slides &out) {
    char const *const file_path{tinyfd_openFileDialog("Open Slides", "", c_ExportExtensions.size(), c_ExportExtensions.data(), nullptr, 0)};
    if (!file_path)
        return;

    std::ifstream file{file_path, std::ios::binary};

    if (!file.is_open())
        return;

    out.clear();
    std::string slide;
    char c;
    while (file.get(c)) {
        if (c == '\0') {
            out.emplace_back(std::make_unique<std::string>(slide));
            slide.clear();
        } else {
            slide.push_back(c);
        }
    }
}

[[nodiscard]]
bool AskIfSure() {
    return tinyfd_messageBox("Are you sure?", "Are you certain you want to perform this action?", "yesno", "question", 0) == 1;
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
        current_slide_index = static_cast<int>(slides.size()) - 1;
    }};

    auto const export_button = Button("Export", [&] {
        if (Export(slides)) {
            tinyfd_notifyPopup("Success", "Slides exported successfuly. You will find the ROM in the output folder.", "info");
        }
        else
            show_error();
    }, ButtonOption::Ascii());

    auto const new_slide = Button("New Slide", add_slide, ButtonOption::Ascii());
    auto const delete_slide = Button("Delete Slide", [&] {
        if (slides.size() > 1) {
            if (!AskIfSure())
                return;

            slides.erase(slides.begin() + current_slide_index);
            slide_inputs.erase(slide_inputs.begin() + current_slide_index);
            slide_titles.erase(slide_titles.begin() + current_slide_index);
            tabs->ChildAt(current_slide_index)->Detach();

            for (auto title_it{slide_titles.begin() + current_slide_index}; title_it != slide_titles.end(); ++title_it) {
                *title_it = std::format("Slide {}", std::distance(slide_titles.begin(), title_it));
            }
            current_slide_index = static_cast<int>(slides.size()) - 1;
        }
    }, ButtonOption::Ascii());
    auto const reset = Button("Reset", [&] {
        if (!AskIfSure())
            return;

        slides.clear();
        slide_inputs.clear();
        slide_titles.clear();
        tabs->DetachAllChildren();
        add_slide();
    }, ButtonOption::Ascii());

    auto const open = Button("Open", [&] {
        LoadSlides(slides);

        slide_inputs.clear();
        slide_titles.clear();
        tabs->DetachAllChildren();
        for (auto const &slide : slides) {
            slide_inputs.emplace_back(Input(slide.get()) | border);
            tabs->Add(slide_inputs.back());
            slide_titles.emplace_back(std::format("Slide {}", slide_titles.size()));
        }
        current_slide_index = 0;
    }, ButtonOption::Ascii());

    auto const save_as = Button("Save As", [&] {
        SaveSlides(slides);
    }, ButtonOption::Ascii());

    auto const tab_toggle = Toggle(&slide_titles, &current_slide_index);

    auto const component = Container::Vertical({
        export_button,
        save_as,
        open,
        tab_toggle,
        new_slide,
        delete_slide,
        reset,
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
                save_as->Render(),
                open->Render(),
                new_slide->Render(),
                delete_slide->Render(),
                reset->Render(),
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