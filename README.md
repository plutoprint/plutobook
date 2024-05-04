# PlutoBook

PlutoBook is a robust HTML rendering library specifically designed for paged media.

## Example Usage:

PlutoBook offers APIs for both C and C++. Here are basic examples to get you started:

### C++ Example:

```c++
#include <plutobook.hpp>

int main() {
    plutobook::Book book(plutobook::PageSize::A4);
    book.loadHtml("<b> Hello World </b>");
    book.writeToPdf("hello.pdf");
    return 0;
}
```

### C Example:

```c
#include <plutobook.h>

int main() {
    plutobook_t* book = plutobook_create(PLUTOBOOK_PAGE_SIZE_A4, PLUTOBOOK_PAGE_MARGINS_NORMAL, PLUTOBOOK_MEDIA_TYPE_PRINT);
    plutobook_load_html(book, "<b> Hello World </b>", -1, "", "", "");
    plutobook_write_to_pdf(book, "hello.pdf");
    plutobook_destroy(book);
    return 0;
}
```

## Installation

Ensure you have [Meson](http://mesonbuild.com) and [Ninja](http://ninja-build.org) installed.

```bash
git clone https://github.com/plutoprint/plutobook.git
cd plutobook
meson setup build
ninja -C build
ninja -C build install
```

## Dependencies

PlutoBook relies on several external libraries for functionality. Make sure you have the following installed:

* Required:
    * Cairo: https://www.cairographics.org
    * Expat: https://libexpat.github.io
    * ICU: http://icu-project.org
    * FreeType: https://www.freetype.org
    * FontConfig: https://www.freedesktop.org/wiki/Software/fontconfig
    * HarfBuzz: https://harfbuzz.github.io
* Optional:
    * Curl: https://curl.se
    * TurboJPEG: https://libjpeg-turbo.org
    * WebP: https://developers.google.com/speed/webp

## API Documentation (Work in Progress)

Detailed information on PlutoBook's functionalities can be found in the header files:

* [plutobook.h](include/plutobook.h) (C API)
* [plutobook.hpp](include/plutobook.hpp) (C++ API)

## Contributions

Contributions to PlutoBook are welcome! Feel free to open issues for bug reports, feature requests, or submit pull requests with enhancements.

## License

PlutoBook is licensed under the [MIT License](LICENSE), allowing for both personal and commercial use.
