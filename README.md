# PlutoBook

PlutoBook is a robust C++ HTML rendering library specifically designed for paged media.

## Example

```cpp
#include <plutobook.hpp>

int main()
{
    plutobook::Book book(plutobook::PageSize::A4);
    book.loadHtml("<b> Hello World </b>");
    book.writeToPdf("hello.pdf");
    book.writeToPng("hello.png");
    return 0;
}
```

## Installation

Install [Meson](http://mesonbuild.com) and [Ninja](http://ninja-build.org) if they are not already installed.

```bash
git clone https://github.com/plutoprint/plutobook.git
cd plutobook
meson setup build
ninja -C build
ninja -C build install
```

## Contributions

Contributions to PlutoBook are welcome! Feel free to open issues for bug reports, feature requests, or submit pull requests with enhancements.

## License

PlutoBook is licensed under the [MIT License](LICENSE), allowing for both personal and commercial use.
