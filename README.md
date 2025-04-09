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

## Installation Guide

To build and install **PlutoBook**, you will need [Meson](http://mesonbuild.com) and [Ninja](http://ninja-build.org) installed on your system.

### Prerequisites

PlutoBook depends on the following external libraries:

- **Required:** `cairo`, `freetype`, `harfbuzz`, `fontconfig`, `expat`, `icu`
- **Optional:** `curl`, `turbojpeg`, `webp` (enable additional features)

> **Note:** For faster builds, it is recommended to install precompiled versions of these libraries through your system's package manager. Otherwise, Meson will build them from source, which may significantly increase build time.

For Ubuntu/Debian users, use the following command to install both required and optional dependencies:

```bash
sudo apt-get install -y libcairo2-dev libexpat1-dev libicu-dev \
    libfreetype6-dev libfontconfig1-dev libharfbuzz-dev \
    libcurl4-openssl-dev libturbojpeg0-dev libwebp-dev \
    ninja-build meson
```

### Build and Installation

Clone the repository, then build and install the project using the following commands:

```bash
git clone https://github.com/plutoprint/plutobook.git
cd plutobook
meson setup build
ninja -C build
sudo ninja -C build install
```

## API Documentation

Detailed information on PlutoBook's functionalities can be found in the header files:

* [plutobook.h](https://github.com/plutoprint/plutobook/blob/main/include/plutobook.h) (C API)
* [plutobook.hpp](https://github.com/plutoprint/plutobook/blob/main/include/plutobook.hpp) (C++ API)

## Contributions

Contributions to PlutoBook are welcome! Feel free to open issues for bug reports, feature requests, or submit pull requests with enhancements.

## License

PlutoBook is licensed under the [MIT License](https://github.com/plutoprint/plutobook/blob/main/LICENSE), allowing for both personal and commercial use.
