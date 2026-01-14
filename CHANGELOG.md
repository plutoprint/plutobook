# Changelog

## PlutoBook 0.13.0 (2026-01-14)

- Support repeating table headers and footers
- Support column group background painting and border resolution
- Support absolute and fixed positioning inside page margin boxes
- Fix table row and cell height sizing
- Fix table column and row span width and padding calculations
- Fix collapsed table border resolution order for adjacent columns
- Prevent page breaks inside table rows
- Remove table padding for collapsed border
- Relicense under the Mozilla Public License 2.0 (MPL-2.0)

Backers and sponsors:

- [Peter Nguyen](https://github.com/jupetern)
- [Ashish Kulkarni](https://github.com/ashkulz)

## PlutoBook 0.12.0 (2025-12-23)

- Add support for the `width` style attribute on `td`, `th`, `col`, and `colgroup` elements
- Support outline painting for table rows and sections
- Fix unnecessary pseudo-element box generation when content is `none` or `normal`

Backers and sponsors:

- [Peter Nguyen](https://github.com/jupetern)
- [Ashish Kulkarni](https://github.com/ashkulz)

## PlutoBook 0.11.3 (2025-12-11)

- Fix GCC 12 compilation error
- Refactor SSL verification settings in curl
- Remove incorrect `isElementNode` assertion in `BoxView::build()`

Backers and sponsors:

- [Ashish Kulkarni](https://github.com/ashkulz)

## PlutoBook 0.11.2 (2025-12-05)

- Fix `:has()` matching by preventing premature return so all sub-selectors are evaluated
- Handle UTF-8 filenames on Windows when opening output files
- Skip zero-width spaces during text rendering to prevent invisible characters in PDF output

Backers and sponsors:

- [Ashish Kulkarni](https://github.com/ashkulz)

## PlutoBook 0.11.1 (2025-11-23)

- Fix URL resolution for Windows absolute paths by mapping them to proper `file://` URLs
- Add `cairo-fix-font-options-leaks.patch` to address memory leaks in Cairo

Backers and sponsors:

- [Ashish Kulkarni](https://github.com/ashkulz)

## PlutoBook 0.11.0 (2025-11-17)

- Add support for `<base>` tag to resolve relative URLs
- Add support for `font-variant-emoji`
- Add support for `min-content`, `max-content`, and `fit-content` in `flex` shorthand
- Add support for `:local-link` selector
- Add support for `:has` selector
- Add support for `:where` selector
- Add `line-height: normal` to `::marker` to prevent inherited `line-height` issues
- Fix `:nth-of-type` and `:nth-last-of-type` sibling counting
- Fix `:nth()` page selector matching
- Fix CSS `:lang()` selector matching
- Fix CSS selector specificity calculation to match W3C specification
- Fix Windows Fontconfig failing to load its default config files
- Refactor `align` and `hidden`  presentational attributes
- Reset form control `font-size` to match most browsers
- Fully implement `format()` support in `@font-face` to skip unsupported font sources
- Add some subset of ready-made counter styles (`binary`, `octal`, `lower-hexadecimal`, `upper-hexadecimal`)
- Add `plutobook_set_fontconfig_path()` to set the Fontconfig configuration directory
- Enable FreeType error strings for clearer diagnostic messages
- Fix default border value for table elements
- Account for relative positioning offsets when computing static distances
- Handle RTL direction when computing horizontal relative offsets

Backers and sponsors:

- [Ashish Kulkarni](https://github.com/ashkulz)

## PlutoBook 0.10.0 (2025-10-03)

- Add support for running headers and footers
- Add support for CSS `min()`, `max()` and `clamp()` functions
- Add support for `unicode-range` in `@font-face` for selective font coverage
- Add support for `type` and `fallback` in `attr()` function
- Prioritize color emoji fonts during font selection
- Use `serif` as the last-resort fallback font
- Handle UTF-8 BOM

Backers and sponsors:

- [Woza Labs](https://github.com/wozalabs)
- [Ashish Kulkarni](https://github.com/ashkulz)
- [Nap2016](https://github.com/Nap2016)

## PlutoBook 0.9.0 (2025-09-20)

- Add support for CSS Custom Properties
- Add support for CSS `calc()` function with length values
- Add support for extended `rgb()` and `hsl()` functions with whitespace and alpha slash syntax
- Add support for CSS `hwb()` color function
- Add support for CSS wide keyword `unset`

Backers and sponsors:

- [Woza Labs](https://github.com/wozalabs)
- [Ashish Kulkarni](https://github.com/ashkulz)

## PlutoBook 0.8.0 (2025-09-09)

- Add support for `space-evenly` in flex layout
- Add support for presentational attributes on `<li>` and `<ol>`
- Fix table height computation for positioned tables
- Ensure empty list items with outside markers generate boxes
- PlutoBook is now available via Homebrew :)

## PlutoBook 0.7.0 (2025-08-30)

- Add support for `row-gap`, `column-gap`, and `gap` in flex layout
- Add support for CSS hex alpha notation
- Fix flex layout to avoid shrinking table boxes below min preferred width
- Fix flex layout to avoid shrinking table height
- Fix table section height calculation to avoid double-counting border spacing
- Fix preferred width calculation for replaced boxes

## PlutoBook 0.6.0 (2025-08-27)

- Add support for `-pluto-qrcode()` in CSS `content` property for embedding QR codes
- Fix uninitialized table members causing large cell `padding` and `border`

## PlutoBook 0.5.0 (2025-08-26)

- Add support for `overflow-wrap` in inline line-breaking algorithm
- Add CLI options `--info` / `-i` for build metadata inspection
- Fix `text-indent` offset calculation in block-level inline formatting
- Fix parser for `text-decoration-line` to return `nullptr` when no values are consumed
- Fix luminance mask computation

## PlutoBook 0.4.0 (2025-08-24)

- Add support for `text-orientation` and `writing-mode`
- PNG export outputs a single continuous image (no pagination)
- Add page range and metadata options to `html2pdf` cli

## PlutoBook 0.3.0 (2025-08-19)

- Replace the `format` parameter with `width` and `height` parameters in `Book::writeToPng` and related functions.
- Add `html2png` command-line tool.
- Extend `html2pdf` command-line tool with new options:
  - `--size`
  - `--margin`
  - `--media`
  - `--orientation`
  - `--width`
  - `--height`
  - `--margin-top`
  - `--margin-right`
  - `--margin-bottom`
  - `--margin-left`

## PlutoBook 0.2.0 (2025-08-17)

- Add runtime autodetection of CA bundles and directories for curl.
- Enhance `DefaultResourceFetcher` with configurable network settings:
  - New setter methods:
    - `setCAInfo`
    - `setCAPath`
    - `setVerifyPeer`
    - `setVerifyHost`
    - `setFollowRedirects`
    - `setMaxRedirects`
    - `setTimeout`
  - Corresponding C API functions:
    - `plutobook_set_ssl_cainfo`
    - `plutobook_set_ssl_capath`
    - `plutobook_set_ssl_verify_peer`
    - `plutobook_set_ssl_verify_host`
    - `plutobook_set_http_follow_redirects`
    - `plutobook_set_http_max_redirects`
    - `plutobook_set_http_timeout`

## PlutoBook 0.1.0 (2025-08-12)

- This is the first release. Everything is new. Enjoy!
