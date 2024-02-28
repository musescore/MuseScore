
#include "import-font.h"

#include <cstdlib>
#include <queue>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#if defined(_WIN32) && !defined(MSDFGEN_NO_PRAGMA_LIB)
    #pragma comment(lib, "freetype.lib")
#endif

namespace msdfgen {

#define REQUIRE(cond) { if (!(cond)) return false; }

class FreetypeHandle {
    friend FreetypeHandle * initializeFreetype();
    friend void deinitializeFreetype(FreetypeHandle *library);
    friend FontHandle * loadFont(FreetypeHandle *library, const char *filename);

    FT_Library library;

};

class FontHandle {
    friend FontHandle * loadFont(FreetypeHandle *library, const char *filename);
    friend void destroyFont(FontHandle *font);
    friend bool getFontScale(double &output, FontHandle *font);
    friend bool getFontWhitespaceWidth(double &spaceAdvance, double &tabAdvance, FontHandle *font);
    friend Shape loadGlyph(FontHandle *font, int unicode, double *advance);
    friend bool getKerning(double &output, FontHandle *font, int unicode1, int unicode2);

    FT_Face face;

};

struct FtContext {
    Point2 position;
    Shape *shape;
    Contour *contour;
};

static Point2 ftPoint2(const FT_Vector &vector) {
    return Point2(vector.x/64., vector.y/64.);
}

static int ftMoveTo(const FT_Vector *to, void *user) {
    FtContext *context = reinterpret_cast<FtContext *>(user);
    context->contour = &context->shape->addContour();
    context->position = ftPoint2(*to);
    return 0;
}

static int ftLineTo(const FT_Vector *to, void *user) {
    FtContext *context = reinterpret_cast<FtContext *>(user);
    context->contour->addEdge(LinearSegment(context->position, ftPoint2(*to)));
    context->position = ftPoint2(*to);
    return 0;
}

static int ftConicTo(const FT_Vector *control, const FT_Vector *to, void *user) {
    FtContext *context = reinterpret_cast<FtContext *>(user);
    context->contour->addEdge(QuadraticSegment(context->position, ftPoint2(*control), ftPoint2(*to)));
    context->position = ftPoint2(*to);
    return 0;
}

static int ftCubicTo(const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *user) {
    FtContext *context = reinterpret_cast<FtContext *>(user);
    context->contour->addEdge(CubicSegment(context->position, ftPoint2(*control1), ftPoint2(*control2), ftPoint2(*to)));
    context->position = ftPoint2(*to);
    return 0;
}

FreetypeHandle * initializeFreetype() {
    FreetypeHandle *handle = new FreetypeHandle;
    FT_Error error = FT_Init_FreeType(&handle->library);
    if (error) {
        delete handle;
        return NULL;
    }
    return handle;
}

void deinitializeFreetype(FreetypeHandle *library) {
    FT_Done_FreeType(library->library);
    delete library;
}

FontHandle * loadFont(FreetypeHandle *library, const char *filename) {
    if (!library)
        return NULL;
    FontHandle *handle = new FontHandle;
    FT_Error error = FT_New_Face(library->library, filename, 0, &handle->face);
    if (error) {
        delete handle;
        return NULL;
    }
    return handle;
}

void destroyFont(FontHandle *font) {
    FT_Done_Face(font->face);
    delete font;
}

bool getFontScale(double &output, FontHandle *font) {
    output = font->face->units_per_EM/64.;
    return true;
}

bool getFontWhitespaceWidth(double &spaceAdvance, double &tabAdvance, FontHandle *font) {
    FT_Error error = FT_Load_Char(font->face, ' ', FT_LOAD_NO_SCALE);
    if (error)
        return false;
    spaceAdvance = font->face->glyph->advance.x/64.;
    error = FT_Load_Char(font->face, '\t', FT_LOAD_NO_SCALE);
    if (error)
        return false;
    tabAdvance = font->face->glyph->advance.x/64.;
    return true;
}

Shape loadGlyph(FontHandle *font, int unicode, double *advance) {
    if (!font)
        return Shape();
    FT_Error error = FT_Load_Char(font->face, unicode, FT_LOAD_NO_SCALE);
    if (error)
        return Shape();

    return loadGlyphSlot(font->face->glyph, advance);
}

Shape loadGlyphSlot(FT_GlyphSlot glyph, double *advance) {
    if (!glyph)
        return Shape();

	Shape output;
	output.inverseYAxis = false;
    if (advance)
        *advance = glyph->advance.x/64.;

    FtContext context = { };
    context.shape = &output;
    FT_Outline_Funcs ftFunctions;
    ftFunctions.move_to = &ftMoveTo;
    ftFunctions.line_to = &ftLineTo;
    ftFunctions.conic_to = &ftConicTo;
    ftFunctions.cubic_to = &ftCubicTo;
    ftFunctions.shift = 0;
    ftFunctions.delta = 0;
    FT_Error error = FT_Outline_Decompose(&glyph->outline, &ftFunctions, &context);
    if (error)
        return Shape();
    return output;
}

bool getKerning(double &output, FontHandle *font, int unicode1, int unicode2) {
    FT_Vector kerning;
    if (FT_Get_Kerning(font->face, FT_Get_Char_Index(font->face, unicode1), FT_Get_Char_Index(font->face, unicode2), FT_KERNING_UNSCALED, &kerning)) {
        output = 0;
        return false;
    }
    output = kerning.x/64.;
    return true;
}

}
