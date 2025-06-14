# PlutoBook

PlutoBook is a robust HTML rendering library tailored for paged media. It takes HTML or XML as input, applies CSS stylesheets, and lays out elements across one or more pages, which can then be rendered as Bitmap images or PDF documents.

> [!NOTE]
> PlutoBook implements its own rendering engine and does **not** depend on rendering engines like Chromium, WebKit, or Gecko.  
> The engine is designed to be robust, lightweight, and memory-efficient, leveraging modern C++ features such as [`std::pmr::monotonic_buffer_resource`](https://en.cppreference.com/w/cpp/memory/monotonic_buffer_resource) to minimize memory fragmentation and optimize allocation performance.

---

## Table of Contents

- [Introduction](#plutobook)
- [Motivation](#motivation)
- [Features](FEATURES.md)
- [Quick Start](#quick-start)
  - [Basic Usage](#basic-usage)
  - [Loading From a URL](#loading-from-a-url)
  - [Loading From a File](#loading-from-a-file)
  - [Rendering to a Canvas](#rendering-to-a-canvas)
  - [Working with Viewport Size](#working-with-viewport-size)
- [Practical Uses](#practical-uses)
  - [Email Client HTML Renderer](#email-client-html-renderer)
  - [Text Editor Print Renderer](#text-editor-print-renderer)
  - [Automated Report Generation](#automated-report-generation)
  - [Custom E-Book or Document Viewer](#custom-e-book-or-document-viewer)
  - [Offline Help and Documentation](#offline-help-and-documentation)
- [Installation Guide](#installation-guide)
  - [Prerequisites](#prerequisites)
  - [Build and Installation](#build-and-installation)
- [API Documentation](#api-documentation)
- [Contributions](#contributions)
- [License](#license)

---

## Motivation

Programmatically generating PDFs can be surprisingly difficult. Many libraries aim to simplify the task, but they often require learning specialized APIs or handling low-level layout details. In contrast, HTML and CSS are tools that most developers already know. They are expressive, easy to write, and well-suited for describing visual layouts.

You might ask, "Wait, aren't tools like Playwright or Puppeteer already solving this?" They are powerful and feature-rich, but also come with significant overhead. Browser-based renderers are resource-heavy, often requiring gigabytes of memory and complex configurations. They're designed to handle the unpredictable nature of the modern web, including dynamic, JavaScript-heavy content. That makes them ideal for browsing and automation, but excessive for static document rendering.

No one uses a bulldozer to mow a lawn. Not because it can’t, but because it’s overkill. Likewise, PlutoBook isn’t trying to replace your browser. It’s a focused tool designed specifically for high-quality rendering of paginated documents from static HTML or XML sources.

PlutoBook is lightweight, robust, and easy to install. It supports a broad set of modern HTML and CSS features for intuitive layout, while maintaining efficiency and precision. Installation is straightforward and dependency-light. It uses well-established libraries like Cairo, FreeType, HarfBuzz, Fontconfig, Expat, and ICU. Optional support for Curl, TurboJPEG, and WebP extends its capabilities, without the need for a full browser engine.

If your goal is to generate beautiful, paginated documents with precision and control, PlutoBook might be the right tool for the job.

---

## Quick Start

PlutoBook is designed to be easy to get started with. To understand where to begin, it's helpful to first look at how PlutoBook works.

The `Book` class is the core API class of the PlutoBook library. It serves as the main entry point for working with documents. A `Book` instance can load content in HTML or XML format, and provides a high-level interface for rendering, exporting to PDF or PNG, and interacting with page structure and metadata.

To create and render a document, you typically start by instantiating a `Book`, specifying parameters such as the page size (e.g., A4, Letter), page margins, and the media type (`MediaType::Print` or `MediaType::Screen`) to control how styles and layouts are interpreted. These parameters define the physical layout and styling context of the document. After that, you load your content using one of the `load` methods (such as `loadHtml` or `loadXml`), and then render it to a canvas or export it as needed.

---

### Basic Usage

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

### Loading From a URL

PlutoBook supports loading web content directly from remote or local URLs using the `loadUrl` method. This is useful when rendering existing online documents, integrating live web content, or referencing local assets. Simply provide the URL to the desired resource, and PlutoBook will fetch and parse the content for layout and rendering.

> [!NOTE]
> Resource loading is single-threaded and occurs inline with the layout process, which may be slower than in web browsers. JavaScript execution is **currently not supported**, so some websites may not render correctly.
> Loading resources over protocols other than `file:` and `data:` (for example, HTTP, HTTPS, or FTP) requires `libcurl`.

Example of loading a webpage from a remote URL:

```cpp
#include <plutobook.hpp>

int main()
{
    // Create a Book with A4 page size and no margins
    plutobook::Book book(plutobook::PageSize::A4, plutobook::PageMargins::None);

    // Load content from a remote URL
    book.loadUrl("https://en.wikipedia.org/wiki/Bjarne_Stroustrup");

    // Export the rendered content to a PDF file
    book.writeToPdf("Bjarne_Stroustrup.pdf");

    return 0;
}
```

Example output:

<p align="center"><img src="https://github.com/user-attachments/assets/1f563034-9575-4345-8a64-c781267b06b6" alt="Bjarne_Stroustrup_Page" width="500"></p>

### Loading From a File

You can load local HTML, SVG, images, or other supported file formats directly by specifying their file paths. This is useful when working with existing documents stored on your filesystem or when generating content offline.

The path can be relative to the current working directory or an absolute path.

Examples:

```cpp
// Load a file in the current directory
book.loadUrl("hello.html");
```

```cpp
// Load a file using a relative path
book.loadUrl("../hello.html");
```

```cpp
// Load a file using an absolute path
book.loadUrl("/home/sammycage/Projects/hello.html");
```

### Rendering to a Canvas

PlutoBook uses an abstract drawing interface called `Canvas`, which supports rendering document content to multiple output targets. Two built-in implementations are provided: `ImageCanvas` and `PDFCanvas`.

`ImageCanvas` renders to an in-memory bitmap. It is well-suited for generating PNG images, creating visual previews, or integrating with other graphics systems. The output can be exported to PNG files, or the raw pixel buffer can be accessed for further processing.

`PDFCanvas`, on the other hand, renders directly to a vector-based PDF stream. It preserves exact layout fidelity, supports selectable text and vector graphics, and is ideal for producing high-quality documents for print, digital publication, or long-term archiving.

Below is a simple example that renders the first few pages of a web article into a single PNG image. It lays the pages side-by-side on one canvas, then saves the result as an image file:

```cpp
#include <plutobook.hpp>

#include <cmath>
#include <algorithm>

int main()
{
    // Create a document with A4 pages and no margins
    plutobook::Book book(plutobook::PageSize::A4, plutobook::PageMargins::None);

    // Load content from Wikipedia
    book.loadUrl("https://en.wikipedia.org/wiki/Bjarne_Stroustrup");

    // Convert page size to pixel dimensions
    const plutobook::PageSize& pageSize = book.pageSize();
    int pageWidth = std::ceil(pageSize.width() / plutobook::units::px);
    int pageHeight = std::ceil(pageSize.height() / plutobook::units::px);

    // Only render up to 3 pages
    uint32_t pageCount = std::min(3u, book.pageCount());

    // Create a canvas wide enough to hold all pages side by side
    plutobook::ImageCanvas canvas(pageCount * pageWidth, pageHeight);
    canvas.clearSurface(1, 1, 1, 1); // white background

    // Loop through pages and render each onto the canvas
    for(uint32_t pageIndex = 0; pageIndex < pageCount; ++pageIndex) {
        canvas.saveState();
        canvas.translate(pageIndex * pageWidth, 0); // shift canvas to next page slot
        book.renderPage(canvas, pageIndex);
        canvas.restoreState();
    }

    // Save the final image
    canvas.writeToPng("Bjarne_Stroustrup_Pages.png");
    return 0;
}
```

Example output:

<p align="center"><img src="https://github.com/user-attachments/assets/c06a42c2-113a-45d0-a123-45ac5d97c5c8" alt="Bjarne_Stroustrup_Pages" width="800"></p>

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

## Practical Uses
PlutoBook provides a robust HTML and CSS rendering engine that can be integrated into a wide range of workflows. The examples below illustrate how PlutoBook can be used in real-world scenarios.

### Email Client HTML Renderer

PlutoBook can render HTML content safely and consistently within an email client. With PlutoBook, developers can implement a custom rendering layer that displays email layouts and styles accurately, without needing a full web browser engine.

For example, the screenshot below shows PlutoBook rendering the first page of a **New York Times** newsletter:

<p align="center"><img src="https://github.com/user-attachments/assets/72c644d6-0b95-4326-8ad4-ea7dd9afed51" alt="New_York_Times_Newsletter" width="500"></p>

### Text Editor Print Renderer

PlutoBook can be embedded in text editors such as **Sublime Text** to render HTML output for printing or print preview. This allows developers to generate clean, styled print layouts for their code, Markdown, or other text documents directly within the editor, without needing a separate browser or external PDF tool.

For example, the screenshot below shows PlutoBook rendering a **Sublime Text** document prepared for printing:

<p align="center"><img src="https://github.com/user-attachments/assets/0340eca9-43ed-4fb6-8eb7-1d8b51c40d15" alt="Sublime_Text_Print_Preview" width="500"></p>

### Automated Report Generation

PlutoBook can be integrated into backend services to automatically generate high-quality PDFs or static HTML reports from dynamic data. This is useful for creating invoices, statements, or compliance documents on demand. Designers can style reports using familiar HTML and CSS, making templates easy to maintain.

<p align="center"><img src="https://github.com/user-attachments/assets/49b0d5bf-ecd0-4c3e-b48a-7beefa09bd43" alt="Invoice" width="700"></p>

### Custom E-Book or Document Viewer

PlutoBook enables developers to build lightweight e-book readers or document viewers that display paginated, static HTML content with consistent styling, images, and fonts. This is ideal for publishing tools, educational apps, or offline manuals where predictable layout is important.

### Offline Help and Documentation

PlutoBook can power built-in help systems or static documentation viewers within applications. It renders static HTML and CSS quickly and securely, making it a practical choice for embedding user guides or offline documentation without the overhead of a full web browser.

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
