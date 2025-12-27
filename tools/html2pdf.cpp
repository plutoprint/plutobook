#include "plutobook.hpp"
#include "argparser.h"

#include <iostream>

using namespace plutobook;

enum class PageSizeType {
    A3,
    A4,
    A5,
    B4,
    B5,
    Letter,
    Legal,
    Ledger
};

static PageSize getPageSize(PageSizeType sizeType)
{
    switch(sizeType) {
    case PageSizeType::A3:
        return PageSize::A3;
    case PageSizeType::A4:
        return PageSize::A4;
    case PageSizeType::A5:
        return PageSize::A5;
    case PageSizeType::B4:
        return PageSize::B4;
    case PageSizeType::B5:
        return PageSize::B5;
    case PageSizeType::Letter:
        return PageSize::Letter;
    case PageSizeType::Legal:
        return PageSize::Legal;
    case PageSizeType::Ledger:
        return PageSize::Ledger;
    }

    return PageSize::A4;
}

static bool size_func(void* closure, const char* value)
{
    static const ArgEnum<PageSizeType> choices[] = {
        {"a3", PageSizeType::A3},
        {"a4", PageSizeType::A4},
        {"a5", PageSizeType::A5},
        {"b4", PageSizeType::B4},
        {"b5", PageSizeType::B5},
        {"letter", PageSizeType::Letter},
        {"legal", PageSizeType::Legal},
        {"ledger", PageSizeType::Ledger},
    };

    return parseArgChoices(closure, value, choices, std::size(choices));
}

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

    PageSizeType size = PageSizeType::A4;
    MediaType media = MediaType::Print;
    Orientation orientation = Orientation::None;

    float width = -1;
    float height = -1;

    float margin = 72;
    float margin_top = -1;
    float margin_right = -1;
    float margin_bottom = -1;
    float margin_left = -1;

    int page_start = kMinPageCount;
    int page_end = kMaxPageCount;
    int page_step = 1;

    const char* title = "";
    const char* subject = "";
    const char* author = "";
    const char* keywords = "";
    const char* creator = "";

    ArgDesc args[] = {
        {"input", ArgType::String, &input, nullptr, "Specify the input HTML filename or URL"},
        {"output", ArgType::String, &output, nullptr, "Specify the output PDF filename"},

        {"--size", ArgType::Choice, &size, size_func, "Specify the page size (eg. A4)"},
        {"--margin", ArgType::Length, &margin, nullptr, "Specify the page margin (eg. 72pt)"},
        {"--media", ArgType::Choice, &media, media_func, "Specify the media type (eg. print, screen)"},
        {"--orientation", ArgType::Choice, &orientation, orientation_func, "Specify the page orientation (eg. portrait, landscape)"},

        {"--width", ArgType::Length, &width, nullptr, "Specify the page width (eg. 210mm)"},
        {"--height", ArgType::Length, &height, nullptr, "Specify the page height (eg. 297mm)"},

        {"--margin-top", ArgType::Length, &margin_top, nullptr, "Specify the page margin top (eg. 72pt)"},
        {"--margin-right", ArgType::Length, &margin_right, nullptr, "Specify the page margin right (eg. 72pt)"},
        {"--margin-bottom", ArgType::Length, &margin_bottom, nullptr, "Specify the page margin bottom (eg. 72pt)"},
        {"--margin-left", ArgType::Length, &margin_left, nullptr, "Specify the page margin left (eg. 72pt)"},

        {"--page-start", ArgType::Int, &page_start, nullptr, "Specify the first page number to print"},
        {"--page-end", ArgType::Int, &page_end, nullptr, "Specify the last page number to print"},
        {"--page-step", ArgType::Int, &page_step, nullptr, "Specify the page step value"},

        {"--user-style", ArgType::String, &user_style, nullptr, "Specify the user-defined CSS style"},
        {"--user-script", ArgType::String, &user_script, nullptr, "Specify the user-defined JavaScript"},

        {"--title", ArgType::String, &title, nullptr, "Set PDF document title"},
        {"--subject", ArgType::String, &subject, nullptr, "Set PDF document subject"},
        {"--author", ArgType::String, &author, nullptr, "Set PDF document author"},
        {"--keywords", ArgType::String, &keywords, nullptr, "Set PDF document keywords"},
        {"--creator", ArgType::String, &creator, nullptr, "Set PDF document creator"},
        {nullptr}
    };

    parseArgs("html2pdf", "Convert HTML to PDF", args, argc, argv);

    PageSize pageSize(getPageSize(size));
    if(width >= 0)
        pageSize.setWidth(width);
    if(height >= 0) {
        pageSize.setHeight(height);
    }

    if(orientation == Orientation::Portrait) {
        pageSize = pageSize.portrait();
    } else if(orientation == Orientation::Landscape) {
        pageSize = pageSize.landscape();
    }

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

    Book book(pageSize, margins, media);

    book.setTitle(title);
    book.setSubject(subject);
    book.setAuthor(author);
    book.setKeywords(keywords);
    book.setCreator(creator);

    if(!book.loadUrl(input, user_style, user_script)) {
        std::cerr << "ERROR: " << plutobook_get_error_message() << std::endl;
        return 2;
    }

    if(!book.writeToPdf(output, page_start, page_end, page_step)) {
        std::cerr << "ERROR: " << plutobook_get_error_message() << std::endl;
        return 3;
    }

    std::cout << "Generated PDF file: " << output << std::endl;
    return 0;
}
