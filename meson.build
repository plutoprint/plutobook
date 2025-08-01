project('plutobook', 'cpp',
    version: '0.0.1',
    license: 'MIT',
    meson_version: '>=0.64.0',
    default_options: ['cpp_std=c++20']
)

expat_dep = dependency('expat',
    required: true,
    version: '>=2.1.0'
)

icu_dep = dependency('icu-uc',
    required: true,
    version: '>=49.0'
)

freetype_dep = dependency('freetype2',
    required: true,
    version: '>=9.7.3'
)

fontconfig_dep = dependency('fontconfig',
    required: true,
    version: '>=2.12.5'
)

harfbuzz_dep = dependency('harfbuzz',
    required: true,
    version: '>=2.0.0'
)

cairo_required_version = '>=1.14.0'
cairo_dep = dependency('cairo',
    required: true,
    version: cairo_required_version
)

plutobook_deps = [
    expat_dep,
    icu_dep,
    freetype_dep,
    fontconfig_dep,
    harfbuzz_dep,
    cairo_dep
]

cairo_required_features = [
    'cairo-pdf',
    'cairo-png',
    'cairo-ft',
    'cairo-fc'
]

foreach feature: cairo_required_features
    plutobook_deps += dependency(feature,
        required: true,
        version: cairo_required_version
    )
endforeach

curl_dep = dependency('libcurl',
    required: get_option('curl'),
    version: '>=7.71.0'
)

plutobook_cpp_args = ['-DPLUTOBOOK_BUILD']

if curl_dep.found()
    plutobook_deps += [curl_dep]
    plutobook_cpp_args += ['-DPLUTOBOOK_HAS_CURL']
endif

jpeg_dep = dependency('libturbojpeg',
    required: get_option('turbojpeg')
)

if jpeg_dep.found()
    plutobook_deps += [jpeg_dep]
    plutobook_cpp_args += ['-DPLUTOBOOK_HAS_TURBOJPEG']
endif

webp_dep = dependency('libwebp',
    required: get_option('webp')
)

if webp_dep.found()
    plutobook_deps += [webp_dep]
    plutobook_cpp_args += ['-DPLUTOBOOK_HAS_WEBP']
endif

plutobook_sources = [
    'source/graphics/color.cpp',
    'source/graphics/geometry.cpp',
    'source/graphics/graphicscontext.cpp',
    'source/graphics/textshape.cpp',

    'source/layout/blockbox.cpp',
    'source/layout/borderpainter.cpp',
    'source/layout/box.cpp',
    'source/layout/boxlayer.cpp',
    'source/layout/boxstyle.cpp',
    'source/layout/boxview.cpp',
    'source/layout/contentbox.cpp',
    'source/layout/flexiblebox.cpp',
    'source/layout/formcontrolbox.cpp',
    'source/layout/fragmentbuilder.cpp',
    'source/layout/inlinebox.cpp',
    'source/layout/linebox.cpp',
    'source/layout/linelayout.cpp',
    'source/layout/listitembox.cpp',
    'source/layout/multicolumnbox.cpp',
    'source/layout/pagebox.cpp',
    'source/layout/replacedbox.cpp',
    'source/layout/svgboxmodel.cpp',
    'source/layout/svgcontainerbox.cpp',
    'source/layout/svggeometrybox.cpp',
    'source/layout/svglinelayout.cpp',
    'source/layout/svgreplacedbox.cpp',
    'source/layout/svgresourcebox.cpp',
    'source/layout/svgtextbox.cpp',
    'source/layout/tablebox.cpp',
    'source/layout/textbox.cpp',

    'source/resource/fontresource.cpp',
    'source/resource/imageresource.cpp',
    'source/resource/resource.cpp',
    'source/resource/textresource.cpp',
    'source/resource/url.cpp',

    'source/counters.cpp',
    'source/cssparser.cpp',
    'source/cssrule.cpp',
    'source/cssstylesheet.cpp',
    'source/csstokenizer.cpp',
    'source/document.cpp',
    'source/globalstring.cpp',
    'source/htmldocument.cpp',
    'source/htmlentityparser.cpp',
    'source/htmlparser.cpp',
    'source/htmltokenizer.cpp',
    'source/plutobook.cc',
    'source/plutobook.cpp',
    'source/svgdocument.cpp',
    'source/svgproperty.cpp',
    'source/textbreakiterator.cpp',
    'source/xmldocument.cpp',
    'source/xmlparser.cpp'
]

plutobook_public_headers = [
    'include/plutobook.h',
    'include/plutobook.hpp'
]

plutobook_include_dirs = include_directories(
    'include',
    'source',
    'source/graphics',
    'source/layout',
    'source/resource'
)

cpp = meson.get_compiler('cpp')

plutobook_cpp_args += cpp.get_supported_arguments(
    '-Wno-sign-compare',
    '-Wno-unused-function',
    '-Wno-unused-variable'
)

plutobook_compile_args = []

if get_option('default_library') == 'static'
    plutobook_compile_args += ['-DPLUTOBOOK_BUILD_STATIC']
    plutobook_cpp_args += ['-DPLUTOBOOK_BUILD_STATIC']
endif

plutobook_lib = library('plutobook', plutobook_sources,
    include_directories: plutobook_include_dirs,
    version: meson.project_version(),
    dependencies: plutobook_deps,
    cpp_args: plutobook_cpp_args,
    install: true
)

plutobook_dep = declare_dependency(
    link_with: plutobook_lib,
    dependencies: plutobook_deps,
    include_directories: include_directories('include'),
    compile_args: plutobook_compile_args
)

meson.override_dependency('plutobook', plutobook_dep)
install_headers(plutobook_public_headers, subdir: 'plutobook')

if not get_option('examples').disabled()
    subdir('examples')
endif

pkgmod = import('pkgconfig')
pkgmod.generate(plutobook_lib,
    name: 'PlutoBook',
    description: 'Paged HTML rendering library',
    filebase: 'plutobook',
    subdirs: 'plutobook'
)
