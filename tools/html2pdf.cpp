#include "plutobook.hpp"
#include "argparser.h"

#include <iostream>

using namespace plutobook;

static bool media_func(void* closure, const char* value)
{
    static const ArgEnum<MediaType> choices[] = {
        {"print", MediaType::Print},
        {"screen", MediaType::Screen}
    };

    return parseArgChoices(closure, value, choices, std::size(choices));
}

enum class Orientation {
    None,
    Portrait,
    Landscape
};

static bool orientation_func(void* closure, const char* value)
{
    static const ArgEnum<Orientation> choices[] = {
        {"portrait", Orientation::Portrait},
        {"landscape", Orientation::Landscape}
    };

    return parseArgChoices(closure, value, choices, std::size(choices));
}

int main(int argc, char* argv[])
{
    const char* input = "";
    const char* output = "";

    const char* user_style = "";
    const char* user_script = "";

    MediaType media = MediaType::Print;
    Orientation orientation = Orientation::None;

    float margin = 72.f;
    float margin_top = -1;
    float margin_right = -1;
    float margin_bottom = -1;
    float margin_left = -1;

    ArgDesc args[] = {
        {"input", ArgType::String, &input, nullptr, "Specify the input HTML filename or URL"},
        {"output", ArgType::String, &output, nullptr, "Specify the output PDF filename"},

        {"--margin", ArgType::Length, &margin, nullptr, "Specify the page margin (eg. 72pt)"},
        {"--media", ArgType::Choice, &media, media_func, "Specify the media type (eg. print, screen)"},
        {"--orientation", ArgType::Choice, &orientation, orientation_func, "Specify the page orientation (eg. portrait, landscape)"},

        {"--margin-top", ArgType::Length, &margin_top, nullptr, "Specify the page margin top (eg. 72pt)"},
        {"--margin-right", ArgType::Length, &margin_right, nullptr, "Specify the page margin right (eg. 72pt)"},
        {"--margin-bottom", ArgType::Length, &margin_bottom, nullptr, "Specify the page margin bottom (eg. 72pt)"},
        {"--margin-left", ArgType::Length, &margin_left, nullptr, "Specify the page margin left (eg. 72pt)"},

        {"--user-style", ArgType::String, &user_style, nullptr, "Specify the user-defined CSS style"},
        {"--user-script", ArgType::String, &user_script, nullptr, "Specify the user-defined JavaScript"},
        {nullptr}
    };

    parseArgs("html2pdf", "Convert HTML to PDF", args, argc, argv);

    PageMargins margins(margin, margin, margin, margin);
    if(margin_top >= 0)
        margins.setTop(margin_top);
    if(margin_right >= 0)
        margins.setRight(margin_right);
    if(margin_bottom >= 0)
        margins.setBottom(margin_bottom);
    if(margin_left >= 0) {
        margins.setLeft(margin_left);
    }

    Book book(PageSize::A4, margins, media);
    if(!book.loadUrl(input, user_style, user_script)) {
        std::cerr << "Error: " << plutobook_get_error_message() << std::endl;
        return 2;
    }

    if(!book.writeToPdf(output)) {
        std::cerr << "Error: " << plutobook_get_error_message() << std::endl;
        return 3;
    }

    std::cout << "Generated PDF file: " << output << std::endl;
    return 0;
}
