fs = import('fs')

fs.copyfile('invoice.css')
fs.copyfile('invoice.html')

executable('html2pdf', 'html2pdf.cpp', dependencies: plutobook_dep)
executable('html2png', 'html2png.cpp', dependencies: plutobook_dep)
executable('invoice', 'invoice.cpp', dependencies: plutobook_dep)
