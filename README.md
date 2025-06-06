# PlutoBook

PlutoBook is a robust HTML rendering library tailored for paged media. It takes HTML or XML as input, applies CSS stylesheets, and lays out elements across one or more pages, which can then be rendered as Bitmap images or PDF documents.

> [!NOTE]
> PlutoBook implements its own rendering engine and does **not** depend on rendering engines like Chromium, WebKit, or Gecko.  
> The engine is designed to be robust, lightweight, and memory-efficient, leveraging modern C++17 features such as [`std::pmr::monotonic_buffer_resource`](https://en.cppreference.com/w/cpp/memory/monotonic_buffer_resource) to minimize memory fragmentation and optimize allocation performance.

## Basic Usage

PlutoBook is designed to be easy to get started with. To understand where to begin, it's helpful to first look at how PlutoBook works.

The `Book` class is the core API class of the PlutoBook library. It serves as the main entry point for working with documents. A `Book` instance can load content in HTML or XML format, and provides a high-level interface for rendering, exporting to PDF or PNG, and interacting with page structure and metadata.

To create and render a document, you typically start by instantiating a `Book`, specifying parameters such as the page size (e.g., A4, Letter), page margins, and the media type (`MediaType::Print` or `MediaType::Screen`) to control how styles and layouts are interpreted. These parameters define the physical layout and styling context of the document. After that, you load your content using one of the `load` methods (such as `loadHtml()` or `loadXml()`), and then render it to a canvas or export it as needed.

---

### Quick Start

This example demonstrates the simplest way to use PlutoBook: creating a `Book` instance with a standard page size (A4), loading a small HTML snippet, and exporting the result as a PDF file named `hello.pdf`. It showcases the core workflow of loading content and generating a document in just a few lines of code.

```cpp
#include <plutobook.hpp>

int main() {
    plutobook::Book book(plutobook::PageSize::A4);
    book.loadHtml("<b> Hello World </b>");
    book.writeToPdf("hello.pdf");
    return 0;
}
```

---

### Working with Viewport Size

In PlutoBook, the **viewport** refers to the area of the page available for laying out content after accounting for margins. It functions similarly to the browser viewport in web development, where layout calculations are made relative to a visible content area.

The viewport size is automatically determined using:

* **Viewport Width** = Page Width − Left Margin − Right Margin
* **Viewport Height** = Page Height − Top Margin − Bottom Margin

You can access the computed dimensions using the methods `Book::viewportWidth()` and `Book::viewportHeight()`, which return values in CSS pixels (`px`).

#### Viewport Units: `vw` and `vh`

PlutoBook supports CSS viewport-relative units:

* `1vw` is equal to 1% of the viewport width
* `1vh` is equal to 1% of the viewport height

For example:

```css
div {
  width: 100vw;
  height: 50vh;
}
```

This creates a `<div>` that spans the full width and half the height of the available viewport space.

These units help create responsive layouts that adapt smoothly to different page sizes and margin settings, ensuring your content looks great regardless of the document’s physical dimensions.

#### Setting and Retrieving Viewport Size

```cpp
#include <plutobook.hpp>

#include <iostream>

int main()
{
    // Define page size (A4: 210 x 297 mm)
    plutobook::PageSize size = plutobook::PageSize::A4;

    // Define page margins (Narrow: 1 inch on all sides)
    plutobook::PageMargins margins = plutobook::PageMargins::Narrow;

    // Create a Book instance with specified page size and margins
    plutobook::Book book(size, margins);

    // Output the viewport size in pixels (content area after margins)
    std::cout << "Viewport Width: " << book.viewportWidth() << " px\n";
    std::cout << "Viewport Height: " << book.viewportHeight() << " px\n";

    return 0;
}
```

Expected output:
```log
Viewport Width: 697.701 px
Viewport Height: 1026.52 px
```

---

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
