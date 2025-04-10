
== Prepare schematics for documentation ==
To generate wiring.svg, follow these steps:
- Compile the LaTeX file
  - Run xelatex figure.tex to generate figure.pdf.
- Open the PDF in Inkscape
  - Launch Inkscape.
  - Go to File → Open and select figure.pdf.
  - In the PDF Import Settings dialog:
    - Page Settings: Select "All" (if multi-page, but ensure your diagram is on the first page).
    - Import Settings: Choose "Internal" (preserves vector paths instead of rasterizing).
      - Replace PDF fonts by closest installed fonts.
      - Embed Images: Enable this to ensure embedded graphics are retained.
- Save as SVG
  - Go to File → Save As.
  - Choose Plain SVG (*.svg) as the file type.
  - Name the file wiring.svg and save it.
