//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "textline.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "xml.h"
#include "utils.h"
#include "score.h"
#include "sym.h"
#include "text.h"
#include "mscore.h"

namespace Ms {

//---------------------------------------------------------
//   TextLineSegment
//---------------------------------------------------------

TextLineSegment::TextLineSegment(Score* s)
   : LineSegment(s)
      {
      _text = 0;
      }

TextLineSegment::TextLineSegment(const TextLineSegment& seg)
   : LineSegment(seg)
      {
      _text = 0;
      layout();    // set the right _text
      }

TextLineSegment::~TextLineSegment()
      {
      delete _text;
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void TextLineSegment::setSelected(bool f)
      {
      Element::setSelected(f);
      if (_text) {
            if (spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_BEGIN) {
                  if (textLine()->_beginText)
                        _text->setSelected(f);
                  }
            else if (textLine()->_continueText)
                  _text->setSelected(f);
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TextLineSegment::draw(QPainter* painter) const
      {
      TextLine* tl   = textLine();
      qreal _spatium = spatium();

      qreal textlineLineWidth    = tl->lineWidth().val() * _spatium;
      qreal textlineTextDistance = _spatium * .5;

      QPointF pp2(pos2());

      QColor color;
      bool normalColor = false;
      if (selected() && !(score() && score()->printing()))
            color = MScore::selectColor[0];
      else if (!visible())
            color = Qt::gray;
      else {
            color = textLine()->curColor();
            normalColor = true;
            }
      qreal l = 0.0;
      if (_text) {
            SpannerSegmentType st = spannerSegmentType();
            if (
               ((st == SEGMENT_SINGLE || st == SEGMENT_BEGIN) && (tl->beginTextPlace() == PLACE_LEFT))
               || ((st == SEGMENT_MIDDLE || st == SEGMENT_END) && (tl->continueTextPlace() == PLACE_LEFT))
               ) {
                  QRectF bb(_text->bbox());
                  l = _text->pos().x() + bb.width() + textlineTextDistance;
                  }
            painter->translate(_text->pos());
            painter->setPen(normalColor ? _text->curColor() : color);
            _text->draw(painter);
            painter->translate(-_text->pos());
            }

      QPen pen(normalColor ? tl->lineColor() : color, textlineLineWidth, tl->lineStyle());
      painter->setPen(pen);
      if (spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_END) {
            if (tl->_endText) {
#if 0 // TODO
                  SymId sym = tl->endSymbol();
                  const QRectF& bb = symBbox(sym);
                  qreal h = bb.height() * .5;
                  QPointF o = tl->endSymbolOffset() * _spatium;
                  pp2.setX(pp2.x() - bb.width() + textlineTextDistance);
                  drawSymbol(sym, painter, QPointF(pp2.x() + textlineTextDistance + o.x(), h + o.y()));
#endif
                  }
            }

      QPointF pp1(l, 0.0);

      if (tl->beginHook() && tl->beginHookType() == HOOK_45)
            pp1.rx() += fabs(tl->beginHookHeight().val() * _spatium * .4);
      if (tl->endHook() && tl->endHookType() == HOOK_45)
            pp2.rx() -= fabs(tl->endHookHeight().val() * _spatium * .4);

      painter->drawLine(QLineF(pp1.x(), pp1.y(), pp2.x(), pp2.y()));

      if (tl->beginHook()) {
            qreal hh = tl->beginHookHeight().val() * _spatium;
            if (spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_BEGIN) {
                  if (tl->beginHookType() == HOOK_45)
                        painter->drawLine(QLineF(pp1.x(), pp1.y(), pp1.x() - fabs(hh * .4), pp1.y() + hh));
                  else
                        painter->drawLine(QLineF(pp1.x(), pp1.y(), pp1.x(), pp1.y() + hh));
                  }
            }
      if (tl->endHook()) {
            qreal hh = tl->endHookHeight().val() * _spatium;
            if (spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_END) {
                  if (tl->endHookType() == HOOK_45)
                        painter->drawLine(QLineF(pp2.x(), pp2.y(), pp2.x() + fabs(hh * .4), pp2.y() + hh));
                  else
                        painter->drawLine(QLineF(pp2.x(), pp2.y(), pp2.x(), pp2.y() + hh));
                  }
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextLineSegment::layout()
      {
      layout1();
      adjustReadPos();
      }

//---------------------------------------------------------
//   layout1
//---------------------------------------------------------

void TextLineSegment::layout1()
      {
      TextLine* tl = textLine();
      if (!tl->diagonal())
            _userOff2.setY(0);
      switch (spannerSegmentType()) {
            case SEGMENT_SINGLE:
            case SEGMENT_BEGIN:
                  if (tl->_beginText) {
                        if (_text == 0) {
                              _text = new Text(*tl->_beginText);
                              _text->setFlag(ELEMENT_MOVABLE, false);
                              _text->setParent(this);
                              }
                        else {
                              _text->setTextStyleType(tl->_beginText->textStyleType());
                              _text->setText(tl->_beginText->text());
                              }
                        }
                  else {
                        delete _text;
                        _text = 0;
                        }
                  break;
            case SEGMENT_MIDDLE:
            case SEGMENT_END:
                  if (tl->_continueText) {
                        if (_text == 0) {
                              _text = new Text(*tl->_continueText);
                              _text->setFlag(ELEMENT_MOVABLE, false);
                              _text->setParent(this);
                              }
                        else {
                              _text->setTextStyleType(tl->_beginText->textStyleType());
                              _text->setText(tl->_continueText->text());
                              }
                        }
                  else {
                        delete _text;
                        _text = 0;
                        }
                  break;
            }
      if (_text)
            _text->layout();

      QPointF pp1;
      QPointF pp2(pos2());

      if (!_text && pp2.y() != 0) {
            setbbox(QRectF(pp1, pp2).normalized());
            return;
            }
      qreal y1 = point(-textLine()->lineWidth());
      qreal y2 = -y1;

      if (_text) {
            qreal h = _text->height();
            if (textLine()->beginTextPlace() == PLACE_ABOVE)
                  y1 = -h;
            else if (textLine()->beginTextPlace() == PLACE_BELOW)
                  y2 = h;
            else {
                  y1 = -h * .5;
                  y2 = h * .5;
                  }
            }
      if (textLine()->endHook()) {
            qreal h = point(textLine()->endHookHeight());
            if (h > y2)
                  y2 = h;
            else if (h < y1)
                  y1 = h;
            }
      if (textLine()->beginHook()) {
            qreal h = point(textLine()->beginHookHeight());
            if (h > y2)
                  y2 = h;
            else if (h < y1)
                  y1 = h;
            }
      bbox().setRect(.0, y1, pp2.x(), y2 - y1);
      }

//---------------------------------------------------------
//   clearText
//---------------------------------------------------------

void TextLineSegment::clearText()
      {
      delete _text;
      _text = 0;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant TextLineSegment::getProperty(P_ID id) const
      {
      switch (id) {
            case P_BEGIN_TEXT_PLACE:
            case P_CONTINUE_TEXT_PLACE:
            case P_END_TEXT_PLACE:
            case P_BEGIN_HOOK:
            case P_END_HOOK:
            case P_BEGIN_HOOK_HEIGHT:
            case P_END_HOOK_HEIGHT:
            case P_BEGIN_HOOK_TYPE:
            case P_END_HOOK_TYPE:
            case P_BEGIN_TEXT:
            case P_CONTINUE_TEXT:
            case P_END_TEXT:
            case P_LINE_COLOR:
            case P_LINE_WIDTH:
                  return textLine()->getProperty(id);
            default:
                  return LineSegment::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TextLineSegment::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_BEGIN_TEXT_PLACE:
            case P_CONTINUE_TEXT_PLACE:
            case P_END_TEXT_PLACE:
            case P_BEGIN_HOOK:
            case P_END_HOOK:
            case P_BEGIN_HOOK_HEIGHT:
            case P_END_HOOK_HEIGHT:
            case P_BEGIN_HOOK_TYPE:
            case P_END_HOOK_TYPE:
            case P_BEGIN_TEXT:
            case P_CONTINUE_TEXT:
            case P_END_TEXT:
            case P_LINE_COLOR:
            case P_LINE_WIDTH:
                  return textLine()->setProperty(id, v);
            default:
                  return LineSegment::setProperty(id, v);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant TextLineSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_BEGIN_TEXT_PLACE:
            case P_CONTINUE_TEXT_PLACE:
            case P_END_TEXT_PLACE:
            case P_BEGIN_HOOK:
            case P_END_HOOK:
            case P_BEGIN_HOOK_HEIGHT:
            case P_END_HOOK_HEIGHT:
            case P_BEGIN_HOOK_TYPE:
            case P_END_HOOK_TYPE:
            case P_BEGIN_TEXT:
            case P_CONTINUE_TEXT:
            case P_END_TEXT:
            case P_LINE_COLOR:
            case P_LINE_WIDTH:
                  return textLine()->propertyDefault(id);
            default:
                  return LineSegment::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   TextLine
//---------------------------------------------------------

TextLine::TextLine(Score* s)
   : SLine(s)
      {
      _beginText         = 0;
      _continueText      = 0;
      _endText           = 0;

      _beginHookHeight   = Spatium(1.5);
      _endHookHeight     = Spatium(1.5);
      _beginHook         = false;
      _endHook           = false;
      _beginHookType     = HOOK_90;
      _endHookType       = HOOK_90;

      _beginTextPlace    = PLACE_LEFT;
      _continueTextPlace = PLACE_LEFT;
      _endTextPlace      = PLACE_LEFT;
      }

TextLine::TextLine(const TextLine& e)
   : SLine(e)
      {
      _beginTextPlace       = e._beginTextPlace;
      _continueTextPlace    = e._continueTextPlace;
      _endTextPlace         = e._endTextPlace;

      _beginHook            = e._beginHook;
      _endHook              = e._endHook;
      _beginHookType        = e._beginHookType;
      _endHookType          = e._endHookType;
      _beginHookHeight      = e._beginHookHeight;
      _endHookHeight        = e._endHookHeight;

      _beginText            = 0;
      _continueText         = 0;
      _endText              = 0;

      if (e._beginText) {
            _beginText = e._beginText->clone(); // deep copy
            _beginText->setParent(this);
            }
      if (e._continueText) {
            _continueText = e._continueText->clone();
            _continueText->setParent(this);
            }
      if (e._endText) {
            _endText = e._endText->clone();
            _endText->setParent(this);
            }
      }

//---------------------------------------------------------
//   TextLine
//---------------------------------------------------------

TextLine::~TextLine()
      {
      delete _beginText;
      delete _continueText;
      delete _endText;
      }

//---------------------------------------------------------
//   setBeginText
//---------------------------------------------------------

void TextLine::setBeginText(const QString& s, int textStyle)
      {
      if (!_beginText) {
            _beginText = new Text(score());
            _beginText->setParent(this);
            }
      _beginText->setTextStyleType(textStyle);
      _beginText->setText(s);
      }

void TextLine::setBeginText(const QString& s)
      {
      if (!_beginText) {
            _beginText = new Text(score());
            _beginText->setParent(this);
            _beginText->setTextStyleType(TEXT_STYLE_TEXTLINE);
            }
      _beginText->setText(s);
      }

//---------------------------------------------------------
//   setContinueText
//---------------------------------------------------------

void TextLine::setContinueText(const QString& s, int textStyle)
      {
      if (!_continueText) {
            _continueText = new Text(score());
            _continueText->setParent(this);
            }
      _continueText->setTextStyleType(textStyle);
      _continueText->setText(s);
      }

void TextLine::setContinueText(const QString& s)
      {
      if (!_continueText) {
            _continueText = new Text(score());
            _continueText->setParent(this);
            _continueText->setTextStyleType(TEXT_STYLE_TEXTLINE);
            }
      _continueText->setText(s);
      }

//---------------------------------------------------------
//   setEndText
//---------------------------------------------------------

void TextLine::setEndText(const QString& s, int textStyle)
      {
      if (!_endText) {
            _endText = new Text(score());
            _endText->setParent(this);
            }
      _endText->setTextStyleType(textStyle);
      _endText->setText(s);
      }

void TextLine::setEndText(const QString& s)
      {
      if (!_endText) {
            _endText = new Text(score());
            _endText->setParent(this);
            _endText->setTextStyleType(TEXT_STYLE_TEXTLINE);
            }
      _endText->setText(s);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextLine::write(Xml& xml) const
      {
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id()));
      writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextLine::read(XmlReader& e)
      {
      qDeleteAll(spannerSegments());
      spannerSegments().clear();
      setId(e.intAttribute("id", -1));

      while (e.readNextStartElement()) {
            if (!readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* TextLine::createLineSegment()
      {
      return new TextLineSegment(score());
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick() to _tick2
//---------------------------------------------------------

void TextLine::layout()
      {
      SLine::layout();
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void TextLineSegment::spatiumChanged(qreal ov, qreal nv)
      {
      textLine()->spatiumChanged(ov, nv);
      if (_text)
            _text->spatiumChanged(ov, nv);
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void TextLine::spatiumChanged(qreal ov, qreal nv)
      {
      if (_beginText)
            _beginText->spatiumChanged(ov, nv);
      if (_continueText)
            _continueText->spatiumChanged(ov, nv);
      }

//---------------------------------------------------------
//   beginText
//---------------------------------------------------------

QString TextLine::beginText() const
      {
      return _beginText ? _beginText->text() : "";
      }

//---------------------------------------------------------
//   continueText
//---------------------------------------------------------

QString TextLine::continueText() const
      {
      return _continueText ? _continueText->text() : "";
      }

//---------------------------------------------------------
//   endText
//---------------------------------------------------------

QString TextLine::endText() const
      {
      return _endText ? _endText->text() : "";
      }

//---------------------------------------------------------
//   writeProperties
//    write properties different from prototype
//---------------------------------------------------------

void TextLine::writeProperties(Xml& xml) const
      {
      if (_beginHook) {
            writeProperty(xml, P_BEGIN_HOOK_HEIGHT);
            writeProperty(xml, P_BEGIN_HOOK_TYPE);
            }
      if (_endHook) {
            writeProperty(xml, P_END_HOOK_HEIGHT);
            writeProperty(xml, P_END_HOOK_TYPE);
            }

      writeProperty(xml, P_BEGIN_TEXT_PLACE);
      writeProperty(xml, P_CONTINUE_TEXT_PLACE);
      writeProperty(xml, P_END_TEXT_PLACE);

      SLine::writeProperties(xml);

      if (_beginText) {
            bool textDiff  = _beginText->text() != propertyDefault(P_BEGIN_TEXT).toString();
            bool styleDiff = _beginText->textStyle() != propertyDefault(P_BEGIN_TEXT_STYLE).value<TextStyle>();
            if (textDiff || styleDiff) {
                  xml.stag("beginText");
                  _beginText->writeProperties(xml, textDiff, styleDiff);
                  xml.etag();
                  }
            }
      if (_continueText) {
            bool textDiff  = _continueText->text() != propertyDefault(P_CONTINUE_TEXT).toString();
            bool styleDiff = _continueText->textStyle() != propertyDefault(P_CONTINUE_TEXT_STYLE).value<TextStyle>();
            if (textDiff || styleDiff) {
                  xml.stag("continueText");
                  _continueText->writeProperties(xml, textDiff, styleDiff);
                  xml.etag();
                  }
            }
      if (_endText) {
            bool textDiff  = _endText->text() != propertyDefault(P_END_TEXT).toString();
            bool styleDiff = _endText->textStyle() != propertyDefault(P_END_TEXT_STYLE).value<TextStyle>();
            if (textDiff || styleDiff) {
                  xml.stag("endText");
                  _endText->writeProperties(xml, textDiff, styleDiff);
                  xml.etag();
                  }
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool TextLine::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "beginHookHeight") {
            _beginHookHeight = Spatium(e.readDouble());
            _beginHook = true;
            }
      else if (tag == "beginHookType")
            _beginHookType = HookType(e.readInt());
      else if (tag == "endHookHeight" || tag == "hookHeight") { // hookHeight is obsolete
            _endHookHeight = Spatium(e.readDouble());
            _endHook = true;
            }
      else if (tag == "endHookType")
            _endHookType = HookType(e.readInt());
      else if (tag == "hookUp")                       // obsolete
            _endHookHeight *= qreal(-1.0);
      else if (tag == "beginSymbol" || tag == "symbol") {     // "symbol" is obsolete
            QString text(e.readElementText());
            setBeginText(QString("<sym>%1</sym>").arg(text[0].isNumber() ? Sym::id2name(SymId(text.toInt())) : text));
            }
      else if (tag == "continueSymbol") {
            QString text(e.readElementText());
            setContinueText(QString("<sym>%1</sym>").arg(text[0].isNumber() ? Sym::id2name(SymId(text.toInt())) : text));
            }
      else if (tag == "endSymbol") {
            QString text(e.readElementText());
            setEndText(QString("<sym>%1</sym>").arg(text[0].isNumber() ? Sym::id2name(SymId(text.toInt())) : text));
            }
      else if (tag == "beginSymbolOffset")            // obsolete
            e.readPoint();
      else if (tag == "continueSymbolOffset")         // obsolete
            e.readPoint();
      else if (tag == "endSymbolOffset")              // obsolete
            e.readPoint();
      else if (tag == "beginTextPlace")
            _beginTextPlace = readPlacement(e);
      else if (tag == "continueTextPlace")
            _continueTextPlace = readPlacement(e);
      else if (tag == "endTextPlace")
            _endTextPlace = readPlacement(e);
      else if (tag == "beginText") {
            if (!_beginText) {
                  _beginText = new Text(score());
                  _beginText->setParent(this);
                  }
            _beginText->read(e);
            }
      else if (tag == "continueText") {
            if (!_continueText) {
                  _continueText = new Text(score());
                  _continueText->setParent(this);
                  }
            _continueText->read(e);
            }
      else if (tag == "endText") {
            if (!_endText) {
                  _endText = new Text(score());
                  _endText->setParent(this);
                  }
            _endText->read(e);
            }
      else if (!SLine::readProperties(e)) {
            qDebug(" ==readSLineProps: failed");
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant TextLine::getProperty(P_ID id) const
      {
      switch (id) {
            case P_BEGIN_TEXT_PLACE:
                  return _beginTextPlace;
            case P_CONTINUE_TEXT_PLACE:
                  return _continueTextPlace;
            case P_END_TEXT_PLACE:
                  return _endTextPlace;
            case P_BEGIN_HOOK:
                  return _beginHook;
            case P_END_HOOK:
                  return _endHook;
            case P_BEGIN_HOOK_HEIGHT:
                  return _beginHookHeight.val();
            case P_END_HOOK_HEIGHT:
                  return _endHookHeight.val();
            case P_BEGIN_HOOK_TYPE:
                  return _beginHookType;
            case P_END_HOOK_TYPE:
                  return _endHookType;
            case P_BEGIN_TEXT:
                  return beginText();
            case P_CONTINUE_TEXT:
                  return continueText();
            case P_END_TEXT:
                  return endText();
            default:
                  return SLine::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TextLine::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_BEGIN_TEXT_PLACE:
                  _beginTextPlace = PlaceText(v.toInt());
                  break;
            case P_CONTINUE_TEXT_PLACE:
                  _continueTextPlace = PlaceText(v.toInt());
                  break;
            case P_END_TEXT_PLACE:
                  _endTextPlace = PlaceText(v.toInt());
                  break;
            case P_BEGIN_HOOK:
                  _beginHook = v.toBool();
                  break;
            case P_END_HOOK:
                  _endHook = v.toBool();
                  break;
            case P_BEGIN_HOOK_HEIGHT:
                  _beginHookHeight = Spatium(v.toDouble());
                  break;
            case P_END_HOOK_HEIGHT:
                  _endHookHeight = Spatium(v.toDouble());
                  break;
            case P_BEGIN_HOOK_TYPE:
                  _beginHookType = HookType(v.toInt());
                  break;
            case P_END_HOOK_TYPE:
                  _endHookType = HookType(v.toInt());
                  break;
            case P_BEGIN_TEXT:
                  setBeginText(v.toString());
                  break;
            case P_CONTINUE_TEXT:
                  setContinueText(v.toString());
                  break;
            case P_END_TEXT:
                  setEndText(v.toString());
                  break;
            default:
                  return SLine::setProperty(id, v);
            }
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant TextLine::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_BEGIN_TEXT_PLACE:
            case P_CONTINUE_TEXT_PLACE:
            case P_END_TEXT_PLACE:
                  return PLACE_LEFT;
            case P_BEGIN_HOOK:
            case P_END_HOOK:
                  return false;
            case P_BEGIN_HOOK_HEIGHT:
            case P_END_HOOK_HEIGHT:
                  return 1.5;
            case P_BEGIN_HOOK_TYPE:
            case P_END_HOOK_TYPE:
                  return HOOK_90;
            case P_BEGIN_TEXT:
            case P_CONTINUE_TEXT:
            case P_END_TEXT:
                  return QString("");

            default:
                  return SLine::propertyDefault(id);
            }
      }

}

