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

## Simple E‑Book Viewer

This example loads **Alice’s Adventures in Wonderland** (from Project Gutenberg) and shows how to render the first three pages as PNGs, then bundle them into a PDF.

_Source: Project Gutenberg – [Alice’s Adventures in Wonderland](https://www.gutenberg.org/ebooks/11)_

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
        PLUTOBOOK_MEDIA_TYPE_SCREEN
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

Example output:

| `page-1` | `page-2` | `page-3` |
| --- | --- | --- |
| ![page-1](https://github.com/user-attachments/assets/c9c26c07-e283-487e-a2e8-ab77c79bdbb5) | ![page-2](https://github.com/user-attachments/assets/43bdf6dc-21fc-427f-a9c6-e54510b88fbd) | ![page-3](https://github.com/user-attachments/assets/6bff4046-7877-4a89-9723-920bdb5799c0) |

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
