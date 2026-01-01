[![Actions](https://img.shields.io/github/actions/workflow/status/plutoprint/plutobook/main.yml)](https://github.com/plutoprint/plutobook/actions)
[![Releases](https://img.shields.io/github/v/release/plutoprint/plutobook)](https://github.com/plutoprint/plutobook/releases)
[![License](https://img.shields.io/github/license/plutoprint/plutobook)](https://github.com/plutoprint/plutobook/blob/main/LICENSE)
[![Sponsors](https://img.shields.io/github/sponsors/plutoprint)](https://github.com/sponsors/plutoprint)
[![Packages](https://repology.org/badge/tiny-repos/plutobook.svg)](https://repology.org/project/plutobook/versions)

> Prefer Python? Try [PlutoPrint](https://github.com/plutoprint/plutoprint) — a Python library built on PlutoBook for easy paged HTML rendering.

# PlutoBook

PlutoBook is a robust HTML rendering library tailored for paged media. It takes HTML or XML as input, applies CSS stylesheets, and lays out elements across one or more pages, which can then be rendered as Bitmap images or PDF documents.

> [!NOTE]
> PlutoBook implements its own rendering engine and does **not** depend on rendering engines like Chromium, WebKit, or Gecko.  
> The engine is designed to be robust, lightweight, and memory-efficient, leveraging modern C++ features such as [`std::pmr::monotonic_buffer_resource`](https://en.cppreference.com/w/cpp/memory/monotonic_buffer_resource) to minimize memory fragmentation and optimize allocation performance.

**Testimonial**  
> *"PlutoBook is incredibly fast and lightweight. In my invoicing app (built in 2010), I replaced headless Chrome with PlutoBook and saw at least a **10× speed improvement** and **10× lower memory usage**, while achieving similar output quality."*  
> — **Nikola Radovanović**

---

## Basic Usage

This example creates a PDF from inline HTML using **plutobook**. It sets up a book with A4 page size and **narrow margins** (36 points or 0.5 inches on all sides), then writes the output to `hello.pdf`.

```cpp
#include <plutobook.hpp>

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
  <img src="https://picsum.photos/id/128/800/400" alt="Magnum Scopulum Corallinum">
  <p>Magnum Scopulum Corallinum est maximum systema scopulorum corallinorum in mundo, quod per plus quam 2,300 chiliometra oram septentrionalem-orientalem Australiae extenditur. Ex milibus scopulorum individualium et centenis insularum constat, e spatio videri potest et inter mirabilia naturalia mundi numeratur.</p>
  <p>Domus est incredibili diversitati vitae marinae, cum plus quam 1,500 speciebus piscium, 400 generibus corallii, et innumerabilibus aliis organismis. Partem vitalem agit in salute oecosystematis marini conservanda et sustentat victum communitatum litoralium per otium et piscationem.</p>
  <p>Quamquam pulchritudinem ac significationem oecologicam praebet, Magnum Scopulum Corallinum minas continenter patitur ex mutatione climatis, pollutione, et nimia piscatione. Eventus albi corallii ex temperaturis marinis crescentibus magnam partem scopuli nuper laeserunt. Conatus conservatorii toto orbe suscipiuntur ad hunc magnificum oecosystema subaquaneum tuendum et restaurandum.</p>
</body>
</html>
)HTML";

int main()
{
    plutobook::Book book(plutobook::PageSize::A4, plutobook::PageMargins::Narrow);
    book.loadHtml(kHTMLContent);
    book.writeToPdf("hello.pdf");
    return 0;
}
```

<details>
<summary><b>Equivalent in C</b></summary>

```c
#include <plutobook.h>

static const char kHTMLContent[] =
    "<!DOCTYPE html>\n"
    "<html lang=\"la\">\n"
    "<head>\n"
    "  <meta charset=\"UTF-8\">\n"
    "  <title>Magnum Scopulum Corallinum</title>\n"
    "  <style>\n"
    "    body { font-family: \"Segoe UI\", sans-serif; line-height: 1.6; margin: 40px auto; max-width: 800px; color: #222; }\n"
    "    h1 { font-size: 2.5em; margin-bottom: 20px; }\n"
    "    img { width: 100%; border-radius: 6px; margin-bottom: 20px; }\n"
    "    p { font-size: 1.05em; text-align: justify; }\n"
    "  </style>\n"
    "</head>\n"
    "<body>\n"
    "  <h1>Magnum Scopulum Corallinum</h1>\n"
    "  <img src=\"https://picsum.photos/id/128/800/400\" alt=\"Magnum Scopulum Corallinum\">\n"
    "  <p>Magnum Scopulum Corallinum est maximum systema scopulorum corallinorum in mundo, quod per plus quam 2,300 chiliometra oram septentrionalem-orientalem Australiae extenditur. Ex milibus scopulorum individualium et centenis insularum constat, e spatio videri potest et inter mirabilia naturalia mundi numeratur.</p>\n"
    "  <p>Domus est incredibili diversitati vitae marinae, cum plus quam 1,500 speciebus piscium, 400 generibus corallii, et innumerabilibus aliis organismis. Partem vitalem agit in salute oecosystematis marini conservanda et sustentat victum communitatum litoralium per otium et piscationem.</p>\n"
    "  <p>Quamquam pulchritudinem ac significationem oecologicam praebet, Magnum Scopulum Corallinum minas continenter patitur ex mutatione climatis, pollutione, et nimia piscatione. Eventus albi corallii ex temperaturis marinis crescentibus magnam partem scopuli nuper laeserunt. Conatus conservatorii toto orbe suscipiuntur ad hunc magnificum oecosystema subaquaneum tuendum et restaurandum.</p>\n"
    "</body>\n"
    "</html>\n";

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

</details>

Example output:

<p align="center"><img src="https://github.com/user-attachments/assets/dc069903-2dad-47a2-ac5a-9854a617f7ae" alt="hello.pdf" width="650"></p>

---

## Page Rendering

PlutoBook supports precise page-level rendering, allowing individual pages to be drawn onto different canvas types, including bitmap canvases and PDF outputs. For custom rendering workflows, it integrates directly with Cairo, enabling advanced use cases like drawing onto custom surfaces or embedding within existing rendering pipelines. This page-specific approach improves efficiency by avoiding full document processing and is well-suited for previews, selective exports, and on-demand rendering.

This example loads [**Alice’s Adventures in Wonderland**](https://www.gutenberg.org/ebooks/11) from Project Gutenberg, renders the first three pages as PNG images, and also exports them as a PDF.

```cpp
#include <plutobook.hpp>
#include <cmath>

int main()
{
    // Create a plutobook instance with A4 page size, narrow margins, and print media type
    plutobook::Book book(plutobook::PageSize::A4, plutobook::PageMargins::Narrow, plutobook::MediaType::Print);

    // Load the HTML content from file
    book.loadUrl("Alice’s Adventures in Wonderland.html");

    // Get page size in points and convert to pixel dimensions
    const plutobook::PageSize& pageSize = book.pageSize();
    int pageWidth = std::ceil(pageSize.width() / plutobook::units::px);
    int pageHeight = std::ceil(pageSize.height() / plutobook::units::px);

    // Create a canvas to render pages as images
    plutobook::ImageCanvas canvas(pageWidth, pageHeight);

    // Render the first 3 pages to PNG files
    for(int pageIndex = 0; pageIndex < 3; ++pageIndex) {
        auto filename = "page-" + std::to_string(pageIndex + 1) + ".png";

        // Clear the canvas to white before rendering each page
        canvas.clearSurface(1, 1, 1, 1);

        // Render the page onto the canvas
        book.renderPage(canvas, pageIndex);

        // Save the canvas to a PNG file
        canvas.writeToPng(filename);
    }

    // Export pages 1 to 3 (inclusive) to PDF with step=1 (every page in order)
    book.writeToPdf("Alice’s Adventures in Wonderland.pdf", 1, 3, 1);
    return 0;
}
```

<details>
<summary><b>Equivalent in C</b></summary>

```c
#include <plutobook.h>
#include <stdio.h>
#include <math.h>

int main()
{
    // Create a plutobook instance with A4 page size, narrow margins, and print media type
    plutobook_t* book = plutobook_create(
        PLUTOBOOK_PAGE_SIZE_A4,
        PLUTOBOOK_PAGE_MARGINS_NARROW,
        PLUTOBOOK_MEDIA_TYPE_PRINT
    );

    // Load the HTML content from file
    plutobook_load_url(book, "Alice’s Adventures in Wonderland.html", "", "");

    // Get page size in points and convert to pixel dimensions
    plutobook_page_size_t page_size = plutobook_get_page_size(book);
    int page_width = (int)ceilf(page_size.width / PLUTOBOOK_UNITS_PX);
    int page_height = (int)ceilf(page_size.height / PLUTOBOOK_UNITS_PX);

    // Create a canvas to render pages as images
    plutobook_canvas_t* canvas = plutobook_image_canvas_create(
        page_width,
        page_height,
        PLUTOBOOK_IMAGE_FORMAT_ARGB32
    );

    // Render the first 3 pages to PNG files
    for(int page_index = 0; page_index < 3; ++page_index) {
        char filename[64];
        sprintf(filename, "page-%d.png", page_index + 1);

        // Clear the canvas to white before rendering each page
        plutobook_canvas_clear_surface(canvas, 1, 1, 1, 1);

        // Render the page onto the canvas
        plutobook_render_page(book, canvas, page_index);

        // Save the canvas to a PNG file
        plutobook_image_canvas_write_to_png(canvas, filename);
    }

    // Export pages 1 to 3 (inclusive) to PDF with step=1 (every page in order)
    plutobook_write_to_pdf_range(book, "Alice’s Adventures in Wonderland.pdf", 1, 3, 1);

    // Clean up resources
    plutobook_canvas_destroy(canvas);
    plutobook_destroy(book);
    return 0;
}
```

</details>

Example output:

| `page-1.png` | `page-2.png` | `page-3.png` |
| --- | --- | --- |
| ![page-1](https://github.com/user-attachments/assets/c9c26c07-e283-487e-a2e8-ab77c79bdbb5) | ![page-2](https://github.com/user-attachments/assets/43bdf6dc-21fc-427f-a9c6-e54510b88fbd) | ![page-3](https://github.com/user-attachments/assets/6bff4046-7877-4a89-9723-920bdb5799c0) |

---

## Document Rendering

PlutoBook supports full-document rendering, drawing the entire content flow as a single continuous layout. This is ideal for generating scrollable previews, long-form visual exports, or cases where the overall structure needs to be viewed or processed at once. It also supports rendering specific rectangular regions of the document, which is useful for partial redraws or focused exports. Both full and partial rendering are available across all supported canvas types, including bitmap outputs, PDF surfaces, and Cairo contexts.

The example below demonstrates how to perform a full-document render of an HTML file into a bitmap image. The document's actual rendered dimensions are measured first, and then a canvas of that size is created to ensure the entire layout is captured without clipping. Finally, the rendered result is saved as a PNG image.

<details>
<summary><code>Explore Life Through Moments.html</code></summary>

```html
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Explore Life Through Moments</title>
  <style>
    body {
      display: flex;
      background: #f7f7f7;
      font-family: Arial, sans-serif;
      margin: 0;
    }

    .section {
      flex: 1;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      padding: 32px;
    }

    .section h2 {
      margin: 0 0 12px;
      font-size: 2rem;
      color: #333;
    }

    .section p {
      margin: 0 0 24px;
      font-size: 1.1rem;
      color: #555;
      text-align: center;
      max-width: 400px;
    }

    .section img {
      max-width: 100%;
      max-height: 350px;
      border-radius: 10px;
      object-fit: cover;
    }
  </style>
</head>
<body>
  <section class="section">
    <h2>Nature</h2>
    <p>Relax in the beauty of untouched landscapes and flourishing greenery. Nature inspires and calms the mind.</p>
    <img src="https://picsum.photos/id/190/500/350" alt="Nature">
  </section>
  <section class="section">
    <h2>City Life</h2>
    <p>Discover the energy and excitement of urban environments. Cities pulse with opportunity and culture.</p>
    <img src="https://picsum.photos/id/392/500/350" alt="City">
  </section>
  <section class="section">
    <h2>Adventure</h2>
    <p>Seek new horizons with every journey. Adventure is about embracing the unknown and living boldly.</p>
    <img src="https://picsum.photos/id/177/500/350" alt="Adventure">
  </section>
</body>
</html>
```

</details>

```cpp
#include <plutobook.hpp>
#include <cmath>

int main()
{
    // Define a custom page size in pixel units used as the viewport for layout
    const plutobook::PageSize pageSize(1800 * plutobook::units::px, 600 * plutobook::units::px);

    // Create a plutobook instance with the custom page size, no page margins, and screen media type
    plutobook::Book book(pageSize, plutobook::PageMargins::None, plutobook::MediaType::Screen);

    // Load the HTML content from file with a custom user style
    book.loadUrl("Explore Life Through Moments.html", /*userStyle=*/"body { border: 1px solid gray }");

    // Compute the full document dimensions after layout
    int width = std::ceil(book.documentWidth());
    int height = std::ceil(book.documentHeight());

    // Create a canvas large enough to render the entire document at once
    plutobook::ImageCanvas canvas(width, height);

    // Render the full document content to the canvas
    book.renderDocument(canvas);

    // Export the rendered canvas to a PNG file
    canvas.writeToPng("Explore Life Through Moments.png");
    return 0;
}
```

<details>
<summary><b>Equivalent in C</b></summary>

```c
#include <plutobook.h>
#include <math.h>

int main()
{
    // Define a custom page size in pixel units used as the viewport for layout
    const plutobook_page_size_t page_size = {1800 * PLUTOBOOK_UNITS_PX, 600 * PLUTOBOOK_UNITS_PX};

    // Create a plutobook instance with the custom page size, no page margins, and screen media type
    plutobook_t* book = plutobook_create(page_size, PLUTOBOOK_PAGE_MARGINS_NONE, PLUTOBOOK_MEDIA_TYPE_SCREEN);

    // Load the HTML content from file with a custom user style
    plutobook_load_url(book, "Explore Life Through Moments.html", /*user_style=*/"body { border: 1px solid gray }", "");

    // Compute the full document dimensions after layout
    int width = (int)ceilf(plutobook_get_document_width(book));
    int height = (int)ceilf(plutobook_get_document_height(book));

    // Create a canvas large enough to render the entire document at once
    plutobook_canvas_t* canvas = plutobook_image_canvas_create(width, height, PLUTOBOOK_IMAGE_FORMAT_ARGB32);

    // Render the full document content to the canvas
    plutobook_render_document(book, canvas);

    // Export the rendered canvas to a PNG file
    plutobook_image_canvas_write_to_png(canvas, "Explore Life Through Moments.png");

    // Clean up resources
    plutobook_canvas_destroy(canvas);
    plutobook_destroy(book);
    return 0;
}
```

</details>

Example output:

<img width="1800" height="550" alt="Explore Life Through Moments.png" src="https://github.com/user-attachments/assets/10c956c4-aa3b-4504-b9f2-b6e5f3cce25f" />

---

## Features

**PlutoBook** is a high-performance document renderer designed for static layout and print-ready output. It supports a broad set of modern web standards, including most of `CSS 3` and parts of `CSS 4`, with input from `HTML5`, `XHTML`, `SVG`, and common image formats like `JPG`, `PNG`, `WEBP`, `GIF`, `BMP`, and `TGA`. Output can be saved directly to `PDF`, image files, or any format supported by `Cairo` (e.g. `SVG`, `PostScript`). It includes robust support for international text layout via `ICU` and `HarfBuzz`, including Arabic, Hebrew, Hindi, and more. Emoji rendering (bitmap and vector) is fully supported. Font handling is powered by `Fontconfig` and `FreeType`, enabling access to all installed system fonts and major font formats. PlutoBook supports `file:` and `data:` URLs out of the box. Remote resources over `http`, `https`, and `ftp` are supported via `libcurl`, and you can plug in a custom fetcher for full control over resource loading.

For a full breakdown of supported features, see [`FEATURES.md`](FEATURES.md):

* [Fonts](FEATURES.md#fonts)
* [Color](FEATURES.md#color)
* [Backgrounds and Borders](FEATURES.md#backgrounds-and-borders)
* [Outlines](FEATURES.md#outlines)
* [Box Model](FEATURES.md#box-model)
* [Box Sizing](FEATURES.md#box-sizing)
* [Display](FEATURES.md#display)
* [Positioning](FEATURES.md#positioning)
* [Floats](FEATURES.md#floats)
* [Lists and Counters](FEATURES.md#lists-and-counters)
* [Counter Styles](FEATURES.md#counter-styles)
* [Tables](FEATURES.md#tables)
* [Multiple Columns](FEATURES.md#multiple-columns)
* [Flexible Box](FEATURES.md#flexible-box)
* [Custom Properties](FEATURES.md#custom-properties)
* [Values and Units](FEATURES.md#values-and-units)
* [Transforms](FEATURES.md#transforms)
* [Media Queries](FEATURES.md#media-queries)
* [Paged Media](FEATURES.md#paged-media)
* [Scalable Vector Graphics](FEATURES.md#scalable-vector-graphics)

---

## Roadmap

PlutoBook is designed to grow into a powerful, flexible tool for static HTML rendering and high-quality print output. The following features are not yet available, but they are part of our long-term vision:

* **JavaScript Support:** We plan to embed a lightweight JavaScript engine (like Duktape or QuickJS) so authors can use `<script>` blocks to generate dynamic content such as charts, diagrams, or custom visuals with libraries like `Chart.js`.

* **Accessibility Improvements:** We aim to support full PDF/UA compliance, allowing PlutoBook to automatically tag structure like headings, lists, and figures for screen readers and assistive technologies.

* **CSS Grid Layout:** We plan to support full CSS Grid features including auto-placement, named lines, and implicit tracks. This will give authors greater control over page layout without extra markup or layout workarounds.

* **CSS Logical Properties:** We intend to support logical CSS properties such as `margin-inline-start` and `padding-block-end`. These allow layouts to adapt more naturally to different writing modes, languages, and directions.

* **CSS Exclusions:** We plan to support CSS Exclusions, which allow inline content to wrap around custom shapes defined by properties such as `shape-outside`. This makes it possible to flow text around non-rectangular elements, enabling more expressive and magazine-style layouts.

Your support keeps PlutoBook moving forward as a fast, modern tool for static HTML rendering. If you’d like to help, consider becoming a sponsor: [https://github.com/sponsors/plutoprint](https://github.com/sponsors/plutoprint)

Every contribution, no matter the size, helps move the project forward.

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

<p align="center"><img src="https://github.com/user-attachments/assets/3e6b0768-43fd-4047-a3bc-903deed9c5ea" alt="Alice’s Adventures in Wonderland" width="800"></p>

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
meson compile -C build
meson install -C build
```

On **macOS** and **Linux**, you can also install PlutoBook directly with [Homebrew](https://formulae.brew.sh/formula/plutobook):

```bash
brew update
brew install plutobook
```

---

## Packages

[![Packaging status](https://repology.org/badge/vertical-allrepos/plutobook.svg)](https://repology.org/project/plutobook/versions)

## API Documentation

Detailed information on PlutoBook's functionalities can be found in the header files:

* [plutobook.h](https://github.com/plutoprint/plutobook/blob/main/include/plutobook.h) (C API)
* [plutobook.hpp](https://github.com/plutoprint/plutobook/blob/main/include/plutobook.hpp) (C++ API)

## Contributions

Contributions to PlutoBook are welcome! Feel free to open issues for bug reports, feature requests, or submit pull requests with enhancements.

## License

PlutoBook is licensed under the [Mozilla Public License Version 2.0](https://github.com/plutoprint/plutobook/blob/main/LICENSE), allowing for both personal and commercial use.
