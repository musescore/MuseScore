
#pragma once

#include <cstdlib>
#include "../core/Shape.h"

typedef struct FT_GlyphSlotRec_ *FT_GlyphSlot;

namespace msdfgen {

class FreetypeHandle;
class FontHandle;

/// Initializes the FreeType library
FreetypeHandle * initializeFreetype();
/// Deinitializes the FreeType library
void deinitializeFreetype(FreetypeHandle *library);
/// Loads a font file and returns its handle
FontHandle * loadFont(FreetypeHandle *library, const char *filename);
/// Unloads a font file
void destroyFont(FontHandle *font);
/// Returns the size of one EM in the font's coordinate system
bool getFontScale(double &output, FontHandle *font);
/// Returns the width of space and tab
bool getFontWhitespaceWidth(double &spaceAdvance, double &tabAdvance, FontHandle *font);
/// Loads the shape prototype of a glyph from font file
Shape loadGlyph(FontHandle *font, int unicode, double *advance = NULL);
/// Loads the shape prototype of a glyph directly from FT glyph slot
Shape loadGlyphSlot(FT_GlyphSlot glyph, double *advance = NULL);
/// Returns the kerning distance adjustment between two specific glyphs.
bool getKerning(double &output, FontHandle *font, int unicode1, int unicode2);

}
