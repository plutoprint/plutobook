#include "plutobook.hpp"

int main()
{
    plutobook::Book book(plutobook::PageSize::A4);
    book.loadHtml("<b> Hello World </b>");
    book.writeToPng("hello.png");
    return 0;
}
