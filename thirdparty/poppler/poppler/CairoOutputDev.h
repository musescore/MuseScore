//========================================================================
//
// CairoOutputDev.h
//
// Copyright 2003 Glyph & Cog, LLC
// Copyright 2004 Red Hat, INC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005-2008 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2005, 2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2005 Nickolay V. Shmyrev <nshmyrev@yandex.ru>
// Copyright (C) 2006-2011, 2013 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2008, 2009, 2011-2015 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2008 Michael Vrable <mvrable@cs.ucsd.edu>
// Copyright (C) 2010-2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2015 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
// Copyright (C) 2016 Jason Crain <jason@aquaticape.us>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef CAIROOUTPUTDEV_H
#define CAIROOUTPUTDEV_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include <cairo-ft.h>
#include "OutputDev.h"
#include "TextOutputDev.h"
#include "GfxState.h"

class PDFDoc;
class GfxState;
class GfxPath;
class Gfx8BitFont;
struct GfxRGB;
class CairoFontEngine;
class CairoFont;

//------------------------------------------------------------------------

//------------------------------------------------------------------------
// CairoImage
//------------------------------------------------------------------------
class CairoImage {
public:
  // Constructor.
  CairoImage (double x1, double y1, double x2, double y2);

  // Destructor.
  ~CairoImage ();

  // Set the image cairo surface
  void setImage (cairo_surface_t *image);
  
  // Get the image cairo surface
  cairo_surface_t *getImage () const { return image; }

  // Get the image rectangle
  void getRect (double *xa1, double *ya1, double *xa2, double *ya2)
	  { *xa1 = x1; *ya1 = y1; *xa2 = x2; *ya2 = y2; }
  
private:
  cairo_surface_t *image;  // image cairo surface
  double x1, y1;          // upper left corner
  double x2, y2;          // lower right corner
};


//------------------------------------------------------------------------
// CairoOutputDev
//------------------------------------------------------------------------

class CairoOutputDev: public OutputDev {
public:

  // Constructor.
  CairoOutputDev();

  // Destructor.
  virtual ~CairoOutputDev();

  //----- get info about output device

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  virtual GBool upsideDown() { return gTrue; }

  // Does this device use drawChar() or drawString()?
  virtual GBool useDrawChar() { return gTrue; }

  // Does this device use tilingPatternFill()?  If this returns false,
  // tiling pattern fills will be reduced to a series of other drawing
  // operations.
  virtual GBool useTilingPatternFill() { return gTrue; }

  // Does this device use functionShadedFill(), axialShadedFill(), and
  // radialShadedFill()?  If this returns false, these shaded fills
  // will be reduced to a series of other drawing operations.
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0)
  virtual GBool useShadedFills(int type) { return type <= 7; }
#else
  virtual GBool useShadedFills(int type) { return type > 1 && type < 4; }
#endif

  // Does this device use FillColorStop()?
  virtual GBool useFillColorStop() { return gTrue; }

  // Does this device use beginType3Char/endType3Char?  Otherwise,
  // text in Type 3 fonts will be drawn with drawChar/drawString.
  virtual GBool interpretType3Chars() { return gFalse; }

  // Does this device need to clip pages to the crop box even when the
  // box is the crop box?
  virtual GBool needClipToCropBox() { return gTrue; }

  //----- initialization and control

  // Start a page.
  virtual void startPage(int pageNum, GfxState *state, XRef *xref);

  // End a page.
  virtual void endPage();

  //----- save/restore graphics state
  virtual void saveState(GfxState *state);
  virtual void restoreState(GfxState *state);

  //----- update graphics state
  virtual void updateAll(GfxState *state);
  virtual void setDefaultCTM(double *ctm);
  virtual void updateCTM(GfxState *state, double m11, double m12,
			 double m21, double m22, double m31, double m32);
  virtual void updateLineDash(GfxState *state);
  virtual void updateFlatness(GfxState *state);
  virtual void updateLineJoin(GfxState *state);
  virtual void updateLineCap(GfxState *state);
  virtual void updateMiterLimit(GfxState *state);
  virtual void updateLineWidth(GfxState *state);
  virtual void updateFillColor(GfxState *state);
  virtual void updateStrokeColor(GfxState *state);
  virtual void updateFillOpacity(GfxState *state);
  virtual void updateStrokeOpacity(GfxState *state);
  virtual void updateFillColorStop(GfxState *state, double offset);
  virtual void updateBlendMode(GfxState *state);

  //----- update text state
  virtual void updateFont(GfxState *state);

  //----- path painting
  virtual void stroke(GfxState *state);
  virtual void fill(GfxState *state);
  virtual void eoFill(GfxState *state);
  virtual void clipToStrokePath(GfxState *state);
  virtual GBool tilingPatternFill(GfxState *state, Gfx *gfx, Catalog *cat, Object *str,
				  double *pmat, int paintType, int tilingType, Dict *resDict,
				  double *mat, double *bbox,
				  int x0, int y0, int x1, int y1,
				  double xStep, double yStep);
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0)
  virtual GBool functionShadedFill(GfxState *state, GfxFunctionShading *shading);
#endif
  virtual GBool axialShadedFill(GfxState *state, GfxAxialShading *shading, double tMin, double tMax);
  virtual GBool axialShadedSupportExtend(GfxState *state, GfxAxialShading *shading);
  virtual GBool radialShadedFill(GfxState *state, GfxRadialShading *shading, double sMin, double sMax);
  virtual GBool radialShadedSupportExtend(GfxState *state, GfxRadialShading *shading);
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0)
  virtual GBool gouraudTriangleShadedFill(GfxState *state, GfxGouraudTriangleShading *shading);
  virtual GBool patchMeshShadedFill(GfxState *state, GfxPatchMeshShading *shading);
#endif

  //----- path clipping
  virtual void clip(GfxState *state);
  virtual void eoClip(GfxState *state);

  //----- text drawing
  void beginString(GfxState *state, GooString *s);
  void endString(GfxState *state);
  void drawChar(GfxState *state, double x, double y,
		double dx, double dy,
		double originX, double originY,
		CharCode code, int nBytes, Unicode *u, int uLen);
  void beginActualText(GfxState *state, GooString *text);
  void endActualText(GfxState *state);

  virtual GBool beginType3Char(GfxState *state, double x, double y,
			       double dx, double dy,
			       CharCode code, Unicode *u, int uLen);
  virtual void endType3Char(GfxState *state);
  virtual void beginTextObject(GfxState *state);
  virtual void endTextObject(GfxState *state);

  //----- image drawing
  virtual void drawImageMask(GfxState *state, Object *ref, Stream *str,
			     int width, int height, GBool invert, GBool interpolate,
			     GBool inlineImg);
  virtual void setSoftMaskFromImageMask(GfxState *state,
					Object *ref, Stream *str,
					int width, int height, GBool invert,
					GBool inlineImg, double *baseMatrix);
  virtual void unsetSoftMaskFromImageMask(GfxState *state, double *baseMatrix);
  void drawImageMaskPrescaled(GfxState *state, Object *ref, Stream *str,
			      int width, int height, GBool invert, GBool interpolate,
			      GBool inlineImg);
  void drawImageMaskRegular(GfxState *state, Object *ref, Stream *str,
			    int width, int height, GBool invert, GBool interpolate,
			    GBool inlineImg);

  virtual void drawImage(GfxState *state, Object *ref, Stream *str,
			 int width, int height, GfxImageColorMap *colorMap,
			 GBool interpolate, int *maskColors, GBool inlineImg);
  virtual void drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str,
				   int width, int height,
				   GfxImageColorMap *colorMap,
				   GBool interpolate,
				   Stream *maskStr,
				   int maskWidth, int maskHeight,
				   GfxImageColorMap *maskColorMap,
				   GBool maskInterpolate);

  virtual void drawMaskedImage(GfxState *state, Object *ref, Stream *str,
			       int width, int height,
			       GfxImageColorMap *colorMap,
			       GBool interpolate,
			       Stream *maskStr,
			       int maskWidth, int maskHeight,
			       GBool maskInvert, GBool maskInterpolate);

  //----- transparency groups and soft masks
  virtual void beginTransparencyGroup(GfxState * /*state*/, double * /*bbox*/,
                                      GfxColorSpace * /*blendingColorSpace*/,
                                      GBool /*isolated*/, GBool /*knockout*/,
                                      GBool /*forSoftMask*/);
  virtual void endTransparencyGroup(GfxState * /*state*/);
  void popTransparencyGroup();
  virtual void paintTransparencyGroup(GfxState * /*state*/, double * /*bbox*/);
  virtual void setSoftMask(GfxState * /*state*/, double * /*bbox*/, GBool /*alpha*/,
                           Function * /*transferFunc*/, GfxColor * /*backdropColor*/);
  virtual void clearSoftMask(GfxState * /*state*/);

  //----- Type 3 font operators
  virtual void type3D0(GfxState *state, double wx, double wy);
  virtual void type3D1(GfxState *state, double wx, double wy,
      double llx, double lly, double urx, double ury);

  //----- special access
  
  // Called to indicate that a new PDF document has been loaded.
  void startDoc(PDFDoc *docA, CairoFontEngine *fontEngine = NULL);
 
  GBool isReverseVideo() { return gFalse; }
  
  void setCairo (cairo_t *cr);
  void setTextPage (TextPage *text);
  void setPrinting (GBool printing) { this->printing = printing; needFontUpdate = gTrue; }
  void setAntialias(cairo_antialias_t antialias);

  void setInType3Char(GBool inType3Char) { this->inType3Char = inType3Char; }
  void getType3GlyphWidth (double *wx, double *wy) { *wx = t3_glyph_wx; *wy = t3_glyph_wy; }
  GBool hasType3GlyphBBox () { return t3_glyph_has_bbox; }
  double *getType3GlyphBBox () { return t3_glyph_bbox; }

protected:
  void doPath(cairo_t *cairo, GfxState *state, GfxPath *path);
  cairo_surface_t *downscaleSurface(cairo_surface_t *orig_surface);
  void getScaledSize(const cairo_matrix_t *matrix,
                     int orig_width, int orig_height,
		     int *scaledWidth, int *scaledHeight);
  cairo_filter_t getFilterForSurface(cairo_surface_t *image,
				     GBool interpolate);
  GBool getStreamData (Stream *str, char **buffer, int *length);
  void setMimeData(GfxState *state, Stream *str, Object *ref,
		   GfxImageColorMap *colorMap, cairo_surface_t *image);
  void fillToStrokePathClip(GfxState *state);
  void alignStrokeCoords(GfxSubpath *subpath, int i, double *x, double *y);
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 14, 0)
  GBool setMimeDataForJBIG2Globals (Stream *str, cairo_surface_t *image);
#endif
  void setAntialias(cairo_t *cr, cairo_antialias_t antialias);

  GfxRGB fill_color, stroke_color;
  cairo_pattern_t *fill_pattern, *stroke_pattern;
  double fill_opacity;
  double stroke_opacity;
  GBool stroke_adjust;
  GBool adjusted_stroke_width;
  GBool align_stroke_coords;
  CairoFont *currentFont;
  XRef *xref;

  struct StrokePathClip {
    GfxPath *path;
    cairo_matrix_t ctm;
    double line_width;
    double *dashes;
    int dash_count;
    double dash_offset;
    cairo_line_cap_t cap;
    cairo_line_join_t join;
    double miter;
    int ref_count;
  } *strokePathClip;

  PDFDoc *doc;			// the current document

  static FT_Library ft_lib;
  static GBool ft_lib_initialized;

  CairoFontEngine *fontEngine;
  GBool fontEngine_owner;

  cairo_t *cairo;
  cairo_matrix_t orig_matrix;
  GBool needFontUpdate;                // set when the font needs to be updated
  GBool printing;
  GBool use_show_text_glyphs;
  GBool text_matrix_valid;
  cairo_surface_t *surface;
  cairo_glyph_t *glyphs;
  int glyphCount;
  cairo_text_cluster_t *clusters;
  int clusterCount;
  char *utf8;
  int utf8Count;
  int utf8Max;
  cairo_path_t *textClipPath;
  GBool inUncoloredPattern;     // inside a uncolored pattern (PaintType = 2)
  GBool inType3Char;		// inside a Type 3 CharProc
  double t3_glyph_wx, t3_glyph_wy;
  GBool t3_glyph_has_bbox;
  double t3_glyph_bbox[4];
  cairo_antialias_t antialias;
  GBool prescaleImages;

  TextPage *text;		// text for the current page
  ActualText *actualText;

  cairo_pattern_t *group;
  cairo_pattern_t *shape;
  cairo_pattern_t *mask;
  cairo_matrix_t mask_matrix;
  cairo_surface_t *cairo_shape_surface;
  cairo_t *cairo_shape;
  int knockoutCount;
  struct ColorSpaceStack {
    GBool knockout;
    GfxColorSpace *cs;
    cairo_matrix_t group_matrix;
    struct ColorSpaceStack *next;
  } * groupColorSpaceStack;

  struct MaskStack {
    cairo_pattern_t *mask;
    cairo_matrix_t mask_matrix;
    struct MaskStack *next;
  } *maskStack;

};

//------------------------------------------------------------------------
// CairoImageOutputDev
//------------------------------------------------------------------------

//XXX: this should ideally not inherit from CairoOutputDev but use it instead perhaps
class CairoImageOutputDev: public CairoOutputDev {
public:

  // Constructor.
  CairoImageOutputDev();

  // Destructor.
  virtual ~CairoImageOutputDev();

  //----- get info about output device

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  virtual GBool upsideDown() { return gTrue; }

  // Does this device use drawChar() or drawString()?
  virtual GBool useDrawChar() { return gFalse; }

  // Does this device use tilingPatternFill()?  If this returns false,
  // tiling pattern fills will be reduced to a series of other drawing
  // operations.
  virtual GBool useTilingPatternFill() { return gTrue; }

  // Does this device use functionShadedFill(), axialShadedFill(), and
  // radialShadedFill()?  If this returns false, these shaded fills
  // will be reduced to a series of other drawing operations.
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 11, 2)
  virtual GBool useShadedFills(int type) { return type <= 7; }
#else
  virtual GBool useShadedFills(int type) { return type < 4; }
#endif

  // Does this device use FillColorStop()?
  virtual GBool useFillColorStop() { return gFalse; }

  // Does this device use beginType3Char/endType3Char?  Otherwise,
  // text in Type 3 fonts will be drawn with drawChar/drawString.
  virtual GBool interpretType3Chars() { return gFalse; }

  // Does this device need non-text content?
  virtual GBool needNonText() { return gTrue; }

  //----- save/restore graphics state
  virtual void saveState(GfxState *state) { }
  virtual void restoreState(GfxState *state) { }

  //----- update graphics state
  virtual void updateAll(GfxState *state) { }
  virtual void setDefaultCTM(double *ctm) { }
  virtual void updateCTM(GfxState *state, double m11, double m12,
				 double m21, double m22, double m31, double m32) { }
  virtual void updateLineDash(GfxState *state) { }
  virtual void updateFlatness(GfxState *state) { }
  virtual void updateLineJoin(GfxState *state) { }
  virtual void updateLineCap(GfxState *state) { }
  virtual void updateMiterLimit(GfxState *state) { }
  virtual void updateLineWidth(GfxState *state) { }
  virtual void updateFillColor(GfxState *state) { }
  virtual void updateStrokeColor(GfxState *state) { }
  virtual void updateFillOpacity(GfxState *state) { }
  virtual void updateStrokeOpacity(GfxState *state) { }
  virtual void updateBlendMode(GfxState *state) { }

  //----- update text state
  virtual void updateFont(GfxState *state) { }

  //----- path painting
  virtual void stroke(GfxState *state) { }
  virtual void fill(GfxState *state) { }
  virtual void eoFill(GfxState *state) { }
  virtual void clipToStrokePath(GfxState *state) { }
  virtual GBool tilingPatternFill(GfxState *state, Gfx *gfx, Catalog *cat, Object *str,
				  double *pmat, int paintType, int tilingType, Dict *resDict,
				  double *mat, double *bbox,
				  int x0, int y0, int x1, int y1,
				  double xStep, double yStep) { return gTrue; }
  virtual GBool axialShadedFill(GfxState *state,
				GfxAxialShading *shading,
				double tMin, double tMax) { return gTrue; }
  virtual GBool radialShadedFill(GfxState *state,
				 GfxRadialShading *shading,
				 double sMin, double sMax) { return gTrue; }

  //----- path clipping
  virtual void clip(GfxState *state) { }
  virtual void eoClip(GfxState *state) { }

  //----- image drawing
  virtual void drawImageMask(GfxState *state, Object *ref, Stream *str,
			     int width, int height, GBool invert,
			     GBool interpolate, GBool inlineImg);
  virtual void drawImage(GfxState *state, Object *ref, Stream *str,
			 int width, int height, GfxImageColorMap *colorMap,
			 GBool interpolate, int *maskColors, GBool inlineImg);
  virtual void drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str,
				   int width, int height,
				   GfxImageColorMap *colorMap,
				   GBool interpolate,
				   Stream *maskStr,
				   int maskWidth, int maskHeight,
				   GfxImageColorMap *maskColorMap,
				   GBool maskInterpolate);
  virtual void drawMaskedImage(GfxState *state, Object *ref, Stream *str,
			       int width, int height,
			       GfxImageColorMap *colorMap,
			       GBool interpolate,
			       Stream *maskStr,
			       int maskWidth, int maskHeight,
			       GBool maskInvert, GBool maskInterpolate);
  virtual void setSoftMaskFromImageMask(GfxState *state, Object *ref, Stream *str,
                                        int width, int height, GBool invert,
                                        GBool inlineImg, double *baseMatrix);
  virtual void unsetSoftMaskFromImageMask(GfxState *state, double *baseMatrix) {}


  //----- transparency groups and soft masks
  virtual void beginTransparencyGroup(GfxState * /*state*/, double * /*bbox*/,
				      GfxColorSpace * /*blendingColorSpace*/,
				      GBool /*isolated*/, GBool /*knockout*/,
				      GBool /*forSoftMask*/) {}
  virtual void endTransparencyGroup(GfxState * /*state*/) {}
  virtual void paintTransparencyGroup(GfxState * /*state*/, double * /*bbox*/) {}
  virtual void setSoftMask(GfxState * /*state*/, double * /*bbox*/, GBool /*alpha*/,
			   Function * /*transferFunc*/, GfxColor * /*backdropColor*/) {}
  virtual void clearSoftMask(GfxState * /*state*/) {}

  //----- Image list
  // By default images are not rendred
  void setImageDrawDecideCbk(GBool (*cbk)(int img_id, void *data),
			     void *data) { imgDrawCbk = cbk; imgDrawCbkData = data; }
  // Iterate through list of images.
  int getNumImages() const { return numImages; }
  CairoImage *getImage(int i) const { return images[i]; }

private:
  void saveImage(CairoImage *image);
  void getBBox(GfxState *state, int width, int height,
               double *x1, double *y1, double *x2, double *y2);
  
  CairoImage **images;
  int numImages;
  int size;
  GBool (*imgDrawCbk)(int img_id, void *data);
  void *imgDrawCbkData;
};

#endif
