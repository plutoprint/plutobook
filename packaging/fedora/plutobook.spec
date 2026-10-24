Name:           libplutobook
Version:        0.17.0
Release:        %{autorelease}
Summary:        Paged HTML Rendering Library

License:        MPL-2.0
URL:            https://github.com/plutoprint/plutobook
Source0:        %{url}/archive/v%{version}/plutobook-%{version}.tar.gz

BuildRequires:  gcc-c++
BuildRequires:  meson
BuildRequires:  ninja-build
BuildRequires:  pkgconfig(cairo)
BuildRequires:  pkgconfig(expat)
BuildRequires:  pkgconfig(fontconfig)
BuildRequires:  pkgconfig(freetype2)
BuildRequires:  pkgconfig(harfbuzz)
BuildRequires:  pkgconfig(icu-uc)
BuildRequires:  pkgconfig(icu-i18n)
BuildRequires:  pkgconfig(libcurl)
BuildRequires:  pkgconfig(libturbojpeg)
BuildRequires:  pkgconfig(libwebp)

%description
PlutoBook is a robust HTML rendering library tailored for paged media. It takes
HTML or XML as input, applies CSS stylesheets, and lays out elements across one
or more pages, which can then be rendered as Bitmap images or PDF documents.


%package        devel
Summary:        Development files for libplutobook
Requires:       libplutobook%{?_isa} = %{version}-%{release}

%description    devel
The libplutobook-devel package contains libraries and header files for
developing applications that use libplutobook.

%prep
%autosetup -n plutobook-%{version}


%build
%meson
%meson_build


%install
%meson_install

%check
# Smoke test
LD_LIBRARY_PATH=%{buildroot}%{_libdir}:${LD_LIBRARY_PATH} %{buildroot}%{_bindir}/html2pdf --help
LD_LIBRARY_PATH=%{buildroot}%{_libdir}:${LD_LIBRARY_PATH} %{buildroot}%{_bindir}/html2png --help

%files
%license LICENSE
%doc README.md
%doc FEATURES.md
%doc CHANGELOG.md
%doc CREDITS
%{_bindir}/html2pdf
%{_bindir}/html2png
%{_libdir}/libplutobook.so.0{,.*}

%files devel
%{_includedir}/plutobook/
%{_libdir}/libplutobook.so
%{_libdir}/pkgconfig/plutobook.pc

%changelog
%autochangelog 
