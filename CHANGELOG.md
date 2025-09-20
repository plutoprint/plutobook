# Changelog

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
