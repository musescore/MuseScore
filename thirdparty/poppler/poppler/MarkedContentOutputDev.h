//========================================================================
//
// MarkedContentOutputDev.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2013 Igalia S.L.
//
//========================================================================

#ifndef MARKEDCONTENTOUTPUTDEV_H
#define MARKEDCONTENTOUTPUTDEV_H

#include "goo/gtypes.h"
#include "goo/gmem.h"
#include "OutputDev.h"
#include "GfxState.h"
#include "GfxFont.h"
#include <vector>

class Dict;
class UnicodeMap;


class TextSpan {
public:
  TextSpan(const TextSpan& other): data(other.data) {
    data->refcount++;
  }

  TextSpan& operator=(const TextSpan& other) {
    if (this != &other) {
      data = other.data;
      data->refcount++;
    }
    return *this;
  }

  ~TextSpan() {
    if (data && --data->refcount == 0)
      delete data;
  }

  GfxFont* getFont() const { return data->font; }
  GooString* getText() const { return data->text; }
  GfxRGB& getColor() const { return data->color; }

private:
  // Note: Takes ownership of strings, increases refcount for font.
  TextSpan(GooString *text,
           GfxFont *font,
           const GfxRGB& color)
      : data(new Data) {
    data->text = text;
    data->font = font;
    data->color = color;
    if (data->font)
      data->font->incRefCnt();
  }

  struct Data {
    GfxFont   *font;
    GooString *text;
    GfxRGB     color;
    unsigned refcount;

    Data(): refcount(1) {}

    ~Data() {
      assert(refcount == 0);
      if (font)
        font->decRefCnt();
      delete text;
    }
  };

  Data *data;

  friend class MarkedContentOutputDev;
};


typedef std::vector<TextSpan> TextSpanArray;


class MarkedContentOutputDev: public OutputDev {
public:
  MarkedContentOutputDev(int mcidA);
  virtual ~MarkedContentOutputDev();

  virtual GBool isOk() { return gTrue; }
  virtual GBool upsideDown() { return gTrue; }
  virtual GBool useDrawChar() { return gTrue; }
  virtual GBool interpretType3Chars() { return gFalse; }
  virtual GBool needNonText() { return gFalse; }
  virtual GBool needCharCount() { return gFalse; }

  virtual void startPage(int pageNum, GfxState *state, XRef *xref);
  virtual void endPage();

  virtual void drawChar(GfxState *state,
                        double xx, double yy,
                        double dx, double dy,
                        double ox, double oy,
                        CharCode c, int nBytes,
                        Unicode *u, int uLen);

  virtual void beginMarkedContent(char *name, Dict *properties);
  virtual void endMarkedContent(GfxState *state);

  const TextSpanArray& getTextSpans() const;

private:

  void endSpan();
  bool inMarkedContent() const { return mcidStack.size() > 0; }
  bool needFontChange(GfxFont* font) const;

  GfxFont         *currentFont;
  GooString       *currentText;
  GfxRGB           currentColor;
  TextSpanArray    textSpans;
  int              mcid;
  std::vector<int> mcidStack;
  double           pageWidth;
  double           pageHeight;
  UnicodeMap      *unicodeMap;
};

#endif /* !MARKEDCONTENTOUTPUTDEV_H */
