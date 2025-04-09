#include "plutobook.hpp"

int main()
{
    plutobook::Book book(plutobook::PageSize::A4);
    book.loadUrl("invoice.html");
    book.writeToPdf("invoice.pdf");
    book.writeToPng("invoice.png");
    return 0;
}
