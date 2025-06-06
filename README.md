# PlutoBook

PlutoBook is a robust HTML rendering library tailored for paged media. It takes HTML or XML as input, applies CSS stylesheets, and lays out elements across one or more pages, which can then be rendered as Bitmap images or PDF documents.

> [!NOTE]
> PlutoBook implements its own rendering engine and does **not** depend on rendering engines like Chromium, WebKit, or Gecko.  
> The engine is designed to be robust, lightweight, and memory-efficient, leveraging modern C++17 features such as [`std::pmr::monotonic_buffer_resource`](https://en.cppreference.com/w/cpp/memory/monotonic_buffer_resource) to minimize memory fragmentation and optimize allocation performance.

## Basic Usage

PlutoBook is designed to be easy to get started with. To understand where to begin, it's helpful to first look at how PlutoBook works.

The `Book` class is the core API class of the PlutoBook library. It serves as the main entry point for working with documents. A `Book` instance can load content in HTML or XML format, and provides a high-level interface for rendering, exporting to PDF or PNG, and interacting with page structure and metadata.

To create and render a document, you typically start by instantiating a `Book`, specifying parameters such as the page size (e.g., A4, Letter), page margins, and the media type (`MediaType::Print` or `MediaType::Screen`) to control how styles and layouts are interpreted. These parameters define the physical layout and styling context of the document. After that, you load your content using one of the `load` methods (such as `loadHtml()` or `loadXml()`), and then render it to a canvas or export it as needed.

### Simple Example

This example demonstrates the simplest way to use PlutoBook: creating a `Book` instance with common settings (A4 page size, normal margins, and print media type), loading a small HTML snippet, and exporting the result as a PDF file named `hello.pdf`. It showcases the core workflow of loading content and generating a document in just a few lines of code.

```cpp
#include <plutobook.hpp>

int main() {
    plutobook::Book book(plutobook::PageSize::A4, plutobook::PageMargins::Normal, plutobook::MediaType::Print);
    book.loadHtml("<b> Hello World </b>");
    book.writeToPdf("hello.pdf");
    return 0;
}
```

## Installation Guide

To build and install **PlutoBook**, you will need [Meson](http://mesonbuild.com) and [Ninja](http://ninja-build.org) installed on your system.

### Prerequisites

PlutoBook depends on the following external libraries:

- **Required:** `cairo`, `freetype`, `harfbuzz`, `fontconfig`, `expat`, `icu`
- **Optional:** `curl`, `turbojpeg`, `webp` (enable additional features)

> [!NOTE]
> For faster builds, it is recommended to install precompiled versions of these libraries through your system's package manager. Otherwise, Meson will build them from source, which may significantly increase build time.

For **Ubuntu/Debian** users, use the following command to install both required and optional dependencies:

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
