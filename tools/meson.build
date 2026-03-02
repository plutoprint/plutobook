argparser_lib = static_library('argparser',
    sources: ['argparser.cpp'],
    dependencies: plutobook_dep
)

argparser_dep = declare_dependency(
    link_with: argparser_lib,
    dependencies: plutobook_dep
)

executable('html2pdf', 'html2pdf.cpp', dependencies: argparser_dep, install: true)
executable('html2png', 'html2png.cpp', dependencies: argparser_dep, install: true)
