#include "plutobook.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    if(argc != 3 && argc != 4) {
        std::cerr << "Usage:\n"
                  << "  html2pdf <input> <output> [<style>]\n\n"
                  << "Arguments:\n"
                  << "  input     URL or filename of the HTML input\n"
                  << "  output    Path to the output PDF file\n"
                  << "  style     Optional CSS style string to apply (e.g., \"@page { size: A4; }\")\n";
        return 1;
    }

    const char* input = argv[1];
    const char* output = argv[2];
    const char* style = "";
    if(argc == 4) {
        style = argv[3];
    }

    plutobook::Book book(plutobook::PageSize::A4);
    if(!book.loadUrl(input, style)) {
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
