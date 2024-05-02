#include "plutobook.hpp"

int main()
{
    plutobook::Book book(plutobook::PageSize::A4);
    book.loadHtml("<b> Hello World </b>");
    book.writeToPdf("hello.pdf");
    return 0;
}
