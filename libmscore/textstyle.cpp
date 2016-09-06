//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "mscore.h"
#include "textstyle.h"
#include "textstyle_p.h"
#include "xml.h"
#include "score.h"
#include "articulation.h"
#include "harmony.h"
#include "chordlist.h"
#include "page.h"
#include "mscore.h"
#include "clef.h"

namespace Ms {

//---------------------------------------------------------
//   TextStyle
//---------------------------------------------------------

TextStyle::TextStyle()
      {
      d = new TextStyleData;
      _hidden = TextStyleHidden::NEVER;
      }

TextStyle::TextStyle(QString _name, QString _family, qreal _size,
   bool _bold, bool _italic, bool _underline,
   Align _align,
   const QPointF& _off, OffsetType _ot,
   bool sd,
   bool hasFrame,
   bool square,
   Spatium fw, Spatium pw, int fr, QColor co, bool _circle, bool _systemFlag,
   QColor fg, QColor bg, TextStyleHidden hidden)
      {
      d = new TextStyleData(_name, _family, _size,
         _bold, _italic, _underline, _align, _off, _ot,
         sd, hasFrame, square, fw, pw, fr, co, _circle, _systemFlag, fg, bg);
      _hidden = hidden;
      }

TextStyle::TextStyle(const TextStyle& s)
   : d(s.d)
      {
      _hidden = s._hidden;
      }
TextStyle::~TextStyle()
      {
      }

//---------------------------------------------------------
//   TextStyle::operator=
//---------------------------------------------------------

TextStyle& TextStyle::operator=(const TextStyle& s)
      {
      d = s.d;
      return *this;
      }

//---------------------------------------------------------
//   TextStyleData
//---------------------------------------------------------

TextStyleData::TextStyleData()
      {
      family                 = "FreeSerif";
      size                   = 10.0;
      bold                   = false;
      italic                 = false;
      underline              = false;
      hasFrame               = false;
      _square                = false;
      sizeIsSpatiumDependent = false;
      frameWidth             = Spatium(0);
      paddingWidth           = Spatium(0);
      frameWidthMM           = 0.0;
      paddingWidthMM         = 0.0;
      frameRound             = 25;
      frameColor             = MScore::defaultColor;
      circle                 = false;
      systemFlag             = false;
      foregroundColor        = Qt::black;
      backgroundColor        = QColor(255, 255, 255, 0);
      }

TextStyleData::TextStyleData(
   QString _name, QString _family, qreal _size,
   bool _bold, bool _italic, bool _underline,
   Align _align,
   const QPointF& _off, OffsetType _ot,
   bool sd,
   bool _hasFrame,
   bool square,
   Spatium fw, Spatium pw, int fr, QColor co, bool _circle, bool _systemFlag,
   QColor fg, QColor bg)
   :
   ElementLayout(_align, _off, _ot),
   name(_name), size(_size), bold(_bold),
   italic(_italic), underline(_underline),
   sizeIsSpatiumDependent(sd), hasFrame(_hasFrame), _square(square), frameWidth(fw), paddingWidth(pw),
   frameRound(fr), frameColor(co), circle(_circle), systemFlag(_systemFlag),
   foregroundColor(fg), backgroundColor(bg)
      {
      //hasFrame       = (fw.val() != 0.0) || (bg.alpha() != 0);
      family         = _family;
      frameWidthMM   = 0.0;
      paddingWidthMM = 0.0;
      }

//---------------------------------------------------------
//   operator!=
//---------------------------------------------------------

bool TextStyleData::operator!=(const TextStyleData& s) const
      {
      return s.name                   != name
          || s.family                 != family
          || s.size                   != size
          || s.bold                   != bold
          || s.italic                 != italic
          || s.underline              != underline
          || s.hasFrame               != hasFrame
          || s._square                != _square
          || s.sizeIsSpatiumDependent != sizeIsSpatiumDependent
          || s.frameWidth             != frameWidth
          || s.paddingWidth           != paddingWidth
          || s.frameRound             != frameRound
          || s.frameColor             != frameColor
          || s.circle                 != circle
          || s.systemFlag             != systemFlag
          || s.foregroundColor        != foregroundColor
          || s.backgroundColor        != backgroundColor
          || s.align()                != align()
          || s.offset()               != offset()
          || s.offsetType()           != offsetType()
          ;
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont TextStyleData::font(qreal _spatium) const
      {
      qreal m = size;
      QFont f(family);
      f.setBold(bold);
      f.setItalic(italic);
      f.setUnderline(underline);

      if (sizeIsSpatiumDependent)
            m *= _spatium / SPATIUM20;
      f.setPointSizeF(m);

      return f;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextStyleData::write(Xml& xml) const
      {
      xml.stag("TextStyle");
      writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void TextStyleData::writeProperties(Xml& xml) const
      {
      ElementLayout::writeProperties(xml);
      if (!name.isEmpty())
            xml.tag("name", name);
      xml.tag("family", family);
      xml.tag("size", size);
      if (bold)
            xml.tag("bold", bold);
      if (italic)
            xml.tag("italic", italic);
      if (underline)
            xml.tag("underline", underline);
      if (sizeIsSpatiumDependent)
            xml.tag("sizeIsSpatiumDependent", sizeIsSpatiumDependent);
      if (foregroundColor != Qt::black)
            xml.tag("foregroundColor", foregroundColor);
      if (backgroundColor != QColor(255, 255, 255, 0))
            xml.tag("backgroundColor", backgroundColor);

      if (hasFrame) {
            xml.tag("frameWidthS",   frameWidth.val());
            xml.tag("paddingWidthS", paddingWidth.val());
            xml.tag("frameRound",   frameRound);
            xml.tag("frameColor",   frameColor);
            if (circle)
                  xml.tag("circle", circle);
            if (_square)
                  xml.tag("square", _square);
            }
      if (systemFlag)
            xml.tag("systemFlag", systemFlag);
      }

//---------------------------------------------------------
//   writeProperties
//    write only changes to the reference r
//---------------------------------------------------------

void TextStyleData::writeProperties(Xml& xml, const TextStyleData& r) const
      {
      ElementLayout::writeProperties(xml, r);
      if (!name.isEmpty() && name != r.name)
            xml.tag("name", name);
      if (family != r.family)
            xml.tag("family", family);
      if (size != r.size)
            xml.tag("size", size);
      if (bold != r.bold)
            xml.tag("bold", bold);
      if (italic != r.italic)
            xml.tag("italic", italic);
      if (underline != r.underline)
            xml.tag("underline", underline);
      if (sizeIsSpatiumDependent != r.sizeIsSpatiumDependent)
            xml.tag("sizeIsSpatiumDependent", sizeIsSpatiumDependent);
      if (foregroundColor != r.foregroundColor)
            xml.tag("foregroundColor", foregroundColor);
      if (backgroundColor != r.backgroundColor)
            xml.tag("backgroundColor", backgroundColor);
      if (hasFrame != r.hasFrame)
            xml.tag("frame", hasFrame);
      if (hasFrame) {
            if (frameWidth.val() != r.frameWidth.val())
                  xml.tag("frameWidthS",   frameWidth.val());
            if (paddingWidth.val() != r.paddingWidth.val())
                  xml.tag("paddingWidthS", paddingWidth.val());
            if (frameRound != r.frameRound)
                  xml.tag("frameRound",   frameRound);
            if (frameColor != r.frameColor)
                  xml.tag("frameColor",   frameColor);
            if (circle != r.circle)
                  xml.tag("circle", circle);
            if (_square != r._square)
                  xml.tag("square", _square);
            }
      if (systemFlag != r.systemFlag)
            xml.tag("systemFlag", systemFlag);
      }

//---------------------------------------------------------
//   restyle
//---------------------------------------------------------

void TextStyleData::restyle(const TextStyleData& os, const TextStyleData& ns)
      {
      ElementLayout::restyle(os, ns);
      if (name == os.name)
            name = ns.name;
      if (family == os.family)
            family = ns.family;
      if (size == os.size)
            size = ns.size;
      if (bold == os.bold)
            bold = ns.bold;
      if (italic == os.italic)
            italic = ns.italic;
      if (underline == os.underline)
            underline = ns.underline;
      if (sizeIsSpatiumDependent == os.sizeIsSpatiumDependent)
            sizeIsSpatiumDependent = ns.sizeIsSpatiumDependent;
      if (foregroundColor == os.foregroundColor)
            foregroundColor = ns.foregroundColor;
      if (backgroundColor == os.backgroundColor)
            backgroundColor = ns.backgroundColor;
      if (hasFrame == os.hasFrame)
            hasFrame = ns.hasFrame;
      if (_square == os._square)
            _square = ns._square;
      if (frameWidth.val() == os.frameWidth.val())
            frameWidth = ns.frameWidth;
      if (paddingWidth.val() == os.paddingWidth.val())
            paddingWidth = ns.paddingWidth;
      if (frameRound == os.frameRound)
            frameRound = ns.frameRound;
      if (frameColor == os.frameColor)
            frameColor = ns.frameColor;
      if (circle == os.circle)
            circle = ns.circle;
      if (systemFlag == os.systemFlag)
            systemFlag = ns.systemFlag;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextStyleData::read(XmlReader& e)
      {
      frameWidth = Spatium(0.0);
      name = e.attribute("name");

      while (e.readNextStartElement()) {
            if (!readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool TextStyleData::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "name")
            name = e.readElementText();
      else if (tag == "family")
            family = e.readElementText();
      else if (tag == "size")
            size = e.readDouble();
      else if (tag == "bold")
            bold = e.readInt();
      else if (tag == "italic")
            italic = e.readInt();
      else if (tag == "underline")
            underline = e.readInt();
      else if (tag == "align")
            setAlign(Align(e.readInt()));
      else if (tag == "anchor")     // obsolete
            e.skipCurrentElement();
      else if (ElementLayout::readProperties(e))
            ;
      else if (tag == "sizeIsSpatiumDependent" || tag == "spatiumSizeDependent")
            sizeIsSpatiumDependent = e.readInt();
      else if (tag == "frameWidth") { // obsolete
            hasFrame = true;
            frameWidthMM = e.readDouble();
            }
      else if (tag == "frameWidthS") {
            hasFrame = true;
            frameWidth = Spatium(e.readDouble());
            }
      else if (tag == "frame")
            hasFrame = e.readInt();
      else if (tag == "square")
            _square = e.readInt();
      else if (tag == "paddingWidth")          // obsolete
            paddingWidthMM = e.readDouble();
      else if (tag == "paddingWidthS")
            paddingWidth = Spatium(e.readDouble());
      else if (tag == "frameRound")
            frameRound = e.readInt();
      else if (tag == "frameColor")
            frameColor = e.readColor();
      else if (tag == "foregroundColor")
            foregroundColor = e.readColor();
      else if (tag == "backgroundColor")
            backgroundColor = e.readColor();
      else if (tag == "circle")
            circle = e.readInt();
      else if (tag == "systemFlag")
            systemFlag = e.readInt();
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   TextStyle method wrappers
//---------------------------------------------------------

QString TextStyle::name() const                          {  return d->name;    }
QString TextStyle::family() const                        {  return d->family;  }
qreal TextStyle::size() const                            {  return d->size;    }
bool TextStyle::bold() const                             {  return d->bold;    }
bool TextStyle::italic() const                           { return d->italic; }
bool TextStyle::underline() const                        { return d->underline; }
bool TextStyle::hasFrame() const                         { return d->hasFrame; }
bool TextStyle::square() const                           { return d->_square; }
Align TextStyle::align() const                           { return d->align(); }
const QPointF& TextStyle::offset() const                 { return d->offset(); }
QPointF TextStyle::offset(qreal spatium) const           { return d->offset(spatium); }
OffsetType TextStyle::offsetType() const                 { return d->offsetType(); }
bool TextStyle::sizeIsSpatiumDependent() const           { return d->sizeIsSpatiumDependent; }

Spatium TextStyle::frameWidth()  const                   { return d->frameWidth; }
Spatium TextStyle::paddingWidth() const                  { return d->paddingWidth; }
qreal TextStyle::frameWidthMM()  const                   { return d->frameWidthMM; }
qreal TextStyle::paddingWidthMM() const                  { return d->paddingWidthMM; }
void TextStyle::setFrameWidth(Spatium v)                 { d->frameWidth = v; }
void TextStyle::setPaddingWidth(Spatium v)               { d->paddingWidth = v; }
void TextStyle::setSquare(bool val)                      { d->_square = val; }

int TextStyle::frameRound() const                        { return d->frameRound; }
QColor TextStyle::frameColor() const                     { return d->frameColor; }
bool TextStyle::circle() const                           { return d->circle;     }
bool TextStyle::systemFlag() const                       { return d->systemFlag; }
QColor TextStyle::foregroundColor() const                { return d->foregroundColor; }
QColor TextStyle::backgroundColor() const                { return d->backgroundColor; }
void TextStyle::setName(const QString& s)                { d->name = s; }
void TextStyle::setFamily(const QString& s)              { d->family = s; }
void TextStyle::setSize(qreal v)                         { d->size = v; }
void TextStyle::setBold(bool v)                          { d->bold = v; }
void TextStyle::setItalic(bool v)                        { d->italic = v; }
void TextStyle::setUnderline(bool v)                     { d->underline = v; }
void TextStyle::setHasFrame(bool v)                      { d->hasFrame = v; }
void TextStyle::setAlign(Align v)                        { d->setAlign(v); }
void TextStyle::setXoff(qreal v)                         { d->setXoff(v); }
void TextStyle::setYoff(qreal v)                         { d->setYoff(v); }
void TextStyle::setOffsetType(OffsetType v)              { d->setOffsetType(v); }
void TextStyle::setSizeIsSpatiumDependent(bool v)        { d->sizeIsSpatiumDependent = v; }
void TextStyle::setFrameRound(int v)                     { d->frameRound = v; }
void TextStyle::setFrameColor(const QColor& v)           { d->frameColor = v; }
void TextStyle::setCircle(bool v)                        { d->circle = v;     }
void TextStyle::setSystemFlag(bool v)                    { d->systemFlag = v; }
void TextStyle::setForegroundColor(const QColor& v)      { d->foregroundColor = v; }
void TextStyle::setBackgroundColor(const QColor& v)      { d->backgroundColor = v; }
void TextStyle::write(Xml& xml) const                    { d->write(xml); }
void TextStyle::read(XmlReader& v)               { d->read(v); }
QFont TextStyle::font(qreal space) const                 { return d->font(space); }
QRectF TextStyle::bbox(qreal sp, const QString& s) const { return d->bbox(sp, s); }
QFontMetricsF TextStyle::fontMetrics(qreal space) const  { return d->fontMetrics(space); }
bool TextStyle::operator!=(const TextStyle& s) const     { return d->operator!=(*s.d); }
void TextStyle::layout(Element* e) const                 { d->layout(e); }
void TextStyle::writeProperties(Xml& xml) const          { d->writeProperties(xml); }
void TextStyle::writeProperties(Xml& xml, const TextStyle& r) const { d->writeProperties(xml, *r.d); }
void TextStyle::restyle(const TextStyle& os, const TextStyle& ns) { d->restyle(*os.d, *ns.d); }
bool TextStyle::readProperties(XmlReader& v)     { return d->readProperties(v); }

}
