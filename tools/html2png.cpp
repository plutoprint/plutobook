#include "plutobook.hpp"
#include "argparser.h"

#include <iostream>

using namespace plutobook;

int main(int argc, char* argv[])
{
    const char* input = "";
    const char* output = "";

    const char* user_style = "";
    const char* user_script = "";

    float viewport_width = 1280 * units::px;
    float viewport_height = 720 * units::px;

    float width = -1;
    float height = -1;

    ArgDesc args[] = {
        {"input", ArgType::String, &input, nullptr, "Specify the input HTML filename or URL"},
        {"output", ArgType::String, &output, nullptr, "Specify the output PNG filename"},

        {"--viewport-width", ArgType::Length, &viewport_width, nullptr, "Specify the viewport width (eg. 1280px)"},
        {"--viewport-height", ArgType::Length, &viewport_height, nullptr, "Specify the viewport height (eg. 720px)"},

        {"--width", ArgType::Length, &width, nullptr, "Specify the output image width (eg. 800px)"},
        {"--height", ArgType::Length, &height, nullptr, "Specify the output image height (eg. 600px)"},

        {"--user-style", ArgType::String, &user_style, nullptr, "Specify the user-defined CSS style"},
        {"--user-script", ArgType::String, &user_script, nullptr, "Specify the user-defined JavaScript"},
        {nullptr}
    };

    parseArgs("html2png", "Convert HTML to PNG", args, argc, argv);

    PageSize size(viewport_width, viewport_height);
    Book book(size, PageMargins::None, MediaType::Screen);
    if(!book.loadUrl(input, user_style, user_script)) {
        std::cerr << "ERROR: " << plutobook_get_error_message() << std::endl;
        return 2;
    }

    if(!book.writeToPng(output, width / units::px, height / units::px)) {
        std::cerr << "ERROR: " << plutobook_get_error_message() << std::endl;
        return 3;
    }

    std::cout << "Generated PNG file: " << output << std::endl;
    return 0;
}
