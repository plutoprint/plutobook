argparser_lib = static_library('argparser', 'argparser.cpp',
    dependencies: plutobook_dep
)

foreach tool : ['html2pdf', 'html2png']
    executable(tool, tool + '.cpp',
        link_with: argparser_lib,
        dependencies: plutobook_dep,
        install: true
    )
endforeach
