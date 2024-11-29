#include "plutobook.hpp"

int main()
{
    plutobook::Book book(plutobook::PageSize::A4);
    book.loadUrl("/home/sammycage/Projects/hello.html");
    book.writeToPng("hello.png");
    return 0;
}
