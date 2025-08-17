# Changelog

## PlutoBook 0.2.0 (2025-08-17)

- Added runtime autodetection of CA bundles and directories for curl.
- Enhanced `DefaultResourceFetcher` with configurable network settings:
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
