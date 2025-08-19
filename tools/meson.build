tools_dep = declare_dependency(
    dependencies: plutobook_dep,
    sources: ['argparser.cpp']
)

executable('html2pdf', 'html2pdf.cpp', dependencies: tools_dep, install: true)
executable('html2png', 'html2png.cpp', dependencies: tools_dep, install: true)
