# Changelog

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
