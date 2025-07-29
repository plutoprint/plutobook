[![Actions](https://github.com/plutoprint/plutobook/actions/workflows/main.yml/badge.svg)](https://github.com/plutoprint/plutobook/actions)
[![Releases](https://img.shields.io/badge/Version-0.0.1-orange.svg)](https://github.com/plutoprint/plutobook/releases)
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](https://github.com/plutoprint/plutobook/blob/master/LICENSE)
[![Sponsor](https://img.shields.io/badge/Sponsor-Github-orange.svg)](https://github.com/sponsors/plutoprint)
[![Donate](https://img.shields.io/badge/Donate-PayPal-blue.svg)](https://www.paypal.me/sammycage)

> Prefer Python? Try [PlutoPrint](https://github.com/plutoprint/plutoprint) — a Python library built on PlutoBook for easy paged HTML rendering.

# PlutoBook

PlutoBook is a robust HTML rendering library tailored for paged media. It takes HTML or XML as input, applies CSS stylesheets, and lays out elements across one or more pages, which can then be rendered as Bitmap images or PDF documents.

> [!NOTE]
> PlutoBook implements its own rendering engine and does **not** depend on rendering engines like Chromium, WebKit, or Gecko.  
> The engine is designed to be robust, lightweight, and memory-efficient, leveraging modern C++ features such as [`std::pmr::monotonic_buffer_resource`](https://en.cppreference.com/w/cpp/memory/monotonic_buffer_resource) to minimize memory fragmentation and optimize allocation performance.

---

## Quick Start

```cpp
static const char kHTMLContent[] = R"HTML(
<!DOCTYPE html>
<html lang="la">
<head>
  <meta charset="UTF-8">
  <title>Magnum Scopulum Corallinum</title>
  <style>
    body { font-family: "Segoe UI", sans-serif; line-height: 1.6; margin: 40px auto; max-width: 800px; color: #222; }
    h1 { font-size: 2.5em; margin-bottom: 20px; }
    img { width: 100%; border-radius: 6px; margin-bottom: 20px; }
    p { font-size: 1.05em; text-align: justify; }
  </style>
</head>
<body>
  <h1>Magnum Scopulum Corallinum</h1>
  <img src="https://picsum.photos/800/400?random=2" alt="Magnum Scopulum Corallinum">
  <p>Magnum Scopulum Corallinum est maximum systema scopulorum corallinorum in mundo, quod per plus quam 2,300 chiliometra oram septentrionalem-orientalem Australiae extenditur. Ex milibus scopulorum individualium et centenis insularum constat, e spatio videri potest et inter mirabilia naturalia mundi numeratur.</p>
  <p>Domus est incredibili diversitati vitae marinae, cum plus quam 1,500 speciebus piscium, 400 generibus corallii, et innumerabilibus aliis organismis. Partem vitalem agit in salute oecosystematis marini conservanda et sustentat victum communitatum litoralium per otium et piscationem.</p>
  <p>Quamquam pulchritudinem ac significationem oecologicam praebet, Magnum Scopulum Corallinum minas continenter patitur ex mutatione climatis, pollutione, et nimia piscatione. Eventus albi corallii ex temperaturis marinis crescentibus magnam partem scopuli nuper laeserunt. Conatus conservatorii toto orbe suscipiuntur ad hunc magnificum oecosystema subaquaneum tuendum et restaurandum.</p>
</body>
</html>
)HTML";
```

### C++

```cpp
#include <plutobook.hpp>

int main()
{
    plutobook::Book book(plutobook::PageSize::A4, plutobook::PageMargins::Narrow);
    book.loadHtml(kHTMLContent);
    book.writeToPdf("hello.pdf");
    return 0;
}
```

### C

```c
#include <plutobook.h>

int main()
{
    plutobook_t* book = plutobook_create(
        PLUTOBOOK_PAGE_SIZE_A4,
        PLUTOBOOK_PAGE_MARGINS_NARROW,
        PLUTOBOOK_MEDIA_TYPE_PRINT
    );

    plutobook_load_html(book, kHTMLContent, -1, "", "", "");
    plutobook_write_to_pdf(book, "hello.pdf");
    plutobook_destroy(book);
    return 0;
}
```

Example output:

<p align="center"><img src="https://github.com/user-attachments/assets/dc069903-2dad-47a2-ac5a-9854a617f7ae" alt="hello.pdf" width="650"></p>

---

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

<p align="center">
  <img src="https://github.com/user-attachments/assets/f2f1be09-8d7b-4317-801e-0d27787c8a58" alt="Alice’s Adventures in Wonderland" width="800">
  <br>
  <em>Rendered page preview of <a href="https://www.gutenberg.org/ebooks/11">Alice’s Adventures in Wonderland</a> generated by PlutoBook</em>
</p>

---

## Installation Guide

To build and install **PlutoBook**, you will need [Meson](http://mesonbuild.com) and [Ninja](http://ninja-build.org) installed on your system.

### Prerequisites

PlutoBook depends on the following external libraries:

- **Required:** `cairo`, `freetype`, `harfbuzz`, `fontconfig`, `expat`, `icu`
- **Optional:** `curl`, `turbojpeg`, `webp` (enable additional features)

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
meson compile -C build
meson install -C build
```

## API Documentation

Detailed information on PlutoBook's functionalities can be found in the header files:

* [plutobook.h](https://github.com/plutoprint/plutobook/blob/main/include/plutobook.h) (C API)
* [plutobook.hpp](https://github.com/plutoprint/plutobook/blob/main/include/plutobook.hpp) (C++ API)

## Contributions

Contributions to PlutoBook are welcome! Feel free to open issues for bug reports, feature requests, or submit pull requests with enhancements.

## License

PlutoBook is licensed under the [MIT License](https://github.com/plutoprint/plutobook/blob/main/LICENSE), allowing for both personal and commercial use.
