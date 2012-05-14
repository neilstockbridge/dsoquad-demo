 
This is a demonstration of a proportional typeface for DSO Quad applications along with a utility to generate embedded font data from a glyph sheet.


## Making a typeface

Draw a glyph sheet with 3 rows of glyphs with 32 glyphs in each row and save it in PNM P6 ( RGB binary) format.  The layout of the glyph sheet is chosen so that upper and lower-case letters line up with each other on the sheet.  An example sheet can be found in `fonts/8x10-example.pnm`.

To generate a typeface suitable for embedding, invoke:

    util/mkfont.rb  nice_font < fonts/8x10-example.pnm

..where *nice_font* is the name of the variable in the generated source code.

Note that this demo is 2826 bytes in size and installs to page 18 by default, which requires [tiny-menu] to launch.

  [tiny-menu]: ../tiny-menu
