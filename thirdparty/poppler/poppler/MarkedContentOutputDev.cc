//========================================================================
//
// MarkedContentOutputDev.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2013 Igalia S.L.
//
//========================================================================

#include "MarkedContentOutputDev.h"
#include "GlobalParams.h"
#include "UnicodeMap.h"
#include "GfxState.h"
#include "GfxFont.h"
#include "Annot.h"
#include <vector>


MarkedContentOutputDev::MarkedContentOutputDev(int mcidA):
  currentFont(NULL),
  currentText(NULL),
  mcid(mcidA),
  pageWidth(0.0),
  pageHeight(0.0),
  unicodeMap(NULL)
{
  currentColor.r = currentColor.g = currentColor.b = 0;
}


MarkedContentOutputDev::~MarkedContentOutputDev()
{
  if (unicodeMap)
    unicodeMap->decRefCnt();
  if (currentFont)
    currentFont->decRefCnt();
  delete currentText;
}


void MarkedContentOutputDev::endSpan()
{
  if (currentText && currentText->getLength()) {
    // The TextSpan takes ownership of currentText and
    // increases the reference count for currentFont.
    textSpans.push_back(TextSpan(currentText,
                                 currentFont,
                                 currentColor));
  }
  currentText = NULL;
}


void MarkedContentOutputDev::startPage(int pageNum, GfxState *state, XRef *xref)
{
  if (state) {
    pageWidth  = state->getPageWidth();
    pageHeight = state->getPageHeight();
  } else {
    pageWidth = pageHeight = 0.0;
  }
}


void MarkedContentOutputDev::endPage()
{
  pageWidth = pageHeight = 0.0;
}


void MarkedContentOutputDev::beginMarkedContent(char *name, Dict *properties)
{
  int id = -1;
  if (properties)
    properties->lookupInt("MCID", NULL, &id);

  if (id == -1)
    return;

  // The stack keep track of MCIDs of nested marked content.
  if (inMarkedContent() || id == mcid)
    mcidStack.push_back(id);
}


void MarkedContentOutputDev::endMarkedContent(GfxState *state)
{
  if (inMarkedContent()) {
      mcidStack.pop_back();
      // The outer marked content sequence MCID was popped, ensure
      // that the last piece of text collected ends up in a TextSpan.
      if (!inMarkedContent())
        endSpan();
  }
}


bool MarkedContentOutputDev::needFontChange(GfxFont* font) const
{
  if (currentFont == font)
    return gFalse;

  if (!currentFont)
    return font != NULL && font->isOk();

  if (font == NULL)
    return gTrue;

  // Two non-null valid fonts are the same if they point to the same Ref
  if (currentFont->getID()->num == font->getID()->num &&
      currentFont->getID()->gen == font->getID()->gen)
    return gFalse;

  return gTrue;
}


void MarkedContentOutputDev::drawChar(GfxState *state,
                                      double xx, double yy,
                                      double dx, double dy,
                                      double ox, double oy,
                                      CharCode c, int nBytes,
                                      Unicode *u, int uLen)
{
  if (!inMarkedContent() || !uLen)
    return;


  // Color changes are tracked here so the color can be chosen depending on
  // the render mode (for mode 1 stroke color is used), so there is no need
  // to implement both updateFillColor() and updateStrokeColor().
  GBool colorChange = gFalse;
  GfxRGB color;
  if ((state->getRender() & 3) == 1)
    state->getStrokeRGB(&color);
  else
    state->getFillRGB(&color);

  colorChange = (color.r != currentColor.r ||
                 color.g != currentColor.g ||
                 color.b != currentColor.b);

  // Check also for font changes.
  GBool fontChange = needFontChange(state->getFont());

  // Save a span with the current changes.
  if (colorChange || fontChange) {
    endSpan();
  }

  // Perform the color/font changes.
  if (colorChange)
    currentColor = color;

  if (fontChange) {
    if (currentFont != NULL) {
      currentFont->decRefCnt();
      currentFont = NULL;
    }
    if (state->getFont() != NULL) {
      currentFont = state->getFont();
      currentFont->incRefCnt();
    }
  }


  double sp, dx2, dy2, w1, h1, x1, y1;

  // Subtract char and word spacing from the (dx,dy) values
  sp = state->getCharSpace();
  if (c == (CharCode) 0x20)
    sp += state->getWordSpace();
  state->textTransformDelta(sp * state->getHorizScaling(), 0, &dx2, &dy2);
  dx -= dx2;
  dy -= dy2;
  state->transformDelta(dx, dy, &w1, &h1);
  state->transform(xx, yy, &x1, &y1);

  // Throw away characters that are not inside the page boundaries.
  if (x1 + w1 < 0 || x1 > pageWidth || y1 + h1 < 0 || y1 > pageHeight)
    return;

  // Make a sanity check on character size. Note: (x != x) <-> isnan(x)
  if (x1 != x1 || y1 != y1 || w1 != w1 || h1 != h1)
    return;

  for (int i = 0; i < uLen; i++) {
    // Soft hyphen markers are skipped, as they are invisible unless
    // rendering is done to an actual device and the hyphenation hint
    // used. MarkedContentOutputDev extracts the *visible* text content.
    if (u[i] != 0x00AD) {
      // Add the UTF-8 sequence to the current text span.
      if (!unicodeMap)
        unicodeMap = globalParams->getTextEncoding();

      char buf[8];
      int n = unicodeMap->mapUnicode(u[i], buf, sizeof(buf));
      if (n > 0) {
        if (currentText == NULL)
          currentText = new GooString();
        currentText->append(buf, n);
      }
    }
  }
}


const TextSpanArray& MarkedContentOutputDev::getTextSpans() const
{
  return textSpans;
}
