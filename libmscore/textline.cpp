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
      layout();    // set right _text
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void TextLineSegment::setSelected(bool f)
      {
      Element::setSelected(f);
      if (_text) {
            if (spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_BEGIN) {
                  if (textLine()->beginText())
                        _text->setSelected(f);
                  }
            else if (textLine()->continueText())
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
      qreal mag = magS();
      qreal l = 0.0;
      int sym = (spannerSegmentType() == SEGMENT_MIDDLE || spannerSegmentType() == SEGMENT_END)
         ? tl->continueSymbol() : tl->beginSymbol();
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
      else if (sym != noSym) {
            const QRectF& bb = symbols[score()->symIdx()][sym].bbox(mag);
            qreal h = bb.height() * .5;
            QPointF o = tl->beginSymbolOffset() * _spatium;
            painter->setPen(color);
            symbols[score()->symIdx()][sym].draw(painter, mag, QPointF(o.x(), h + o.y()));
            l = bb.width() + textlineTextDistance;
            }

      QPen pen(normalColor ? tl->lineColor() : color, textlineLineWidth, tl->lineStyle());
      painter->setPen(pen);
      if (spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_END) {
            if (tl->endSymbol() != noSym) {
                  int sym = tl->endSymbol();
                  const QRectF& bb = symbols[score()->symIdx()][sym].bbox(mag);
                  qreal h = bb.height() * .5;
                  QPointF o = tl->endSymbolOffset() * _spatium;
                  pp2.setX(pp2.x() - bb.width() + textlineTextDistance);
                  symbols[score()->symIdx()][sym].draw(painter, mag, QPointF(pp2.x() + textlineTextDistance + o.x(), h + o.y()));
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
                  if (tl->beginText()) {
                        if (_text == 0) {
                              _text = new Text(*tl->beginText());
                              _text->setFlag(ELEMENT_MOVABLE, false);
                              _text->setParent(this);
                              }
                        }
                  else {
                        delete _text;
                        _text = 0;
                        }
                  break;
            case SEGMENT_MIDDLE:
            case SEGMENT_END:
                  if (tl->continueText()) {
                        if (_text == 0) {
                              _text = new Text(*tl->continueText());
                              _text->setFlag(ELEMENT_MOVABLE, false);
                              _text->setParent(this);
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

      int sym = textLine()->beginSymbol();
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
      else if (sym != noSym) {
            qreal hh = symbols[score()->symIdx()][sym].height(magS()) * .5;
            y1 = -hh;
            y2 = hh;
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
            case P_BEGIN_HOOK:
            case P_END_HOOK:
            case P_BEGIN_HOOK_HEIGHT:
            case P_END_HOOK_HEIGHT:
            case P_BEGIN_HOOK_TYPE:
            case P_END_HOOK_TYPE:
            case P_BEGIN_SYMBOL:
            case P_CONTINUE_SYMBOL:
            case P_END_SYMBOL:
            case P_BEGIN_SYMBOL_OFFSET:
            case P_CONTINUE_SYMBOL_OFFSET:
            case P_END_SYMBOL_OFFSET:
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
            case P_BEGIN_HOOK:
            case P_END_HOOK:
            case P_BEGIN_HOOK_HEIGHT:
            case P_END_HOOK_HEIGHT:
            case P_BEGIN_HOOK_TYPE:
            case P_END_HOOK_TYPE:
            case P_BEGIN_SYMBOL:
            case P_CONTINUE_SYMBOL:
            case P_END_SYMBOL:
            case P_BEGIN_SYMBOL_OFFSET:
            case P_CONTINUE_SYMBOL_OFFSET:
            case P_END_SYMBOL_OFFSET:
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
            case P_BEGIN_HOOK:
            case P_END_HOOK:
            case P_BEGIN_HOOK_HEIGHT:
            case P_END_HOOK_HEIGHT:
            case P_BEGIN_HOOK_TYPE:
            case P_END_HOOK_TYPE:
            case P_BEGIN_SYMBOL:
            case P_CONTINUE_SYMBOL:
            case P_END_SYMBOL:
            case P_BEGIN_SYMBOL_OFFSET:
            case P_CONTINUE_SYMBOL_OFFSET:
            case P_END_SYMBOL_OFFSET:
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

      _beginHookHeight   = Spatium(1.5);
      _endHookHeight     = Spatium(1.5);
      _beginHook         = false;
      _endHook           = false;
      _beginHookType     = HOOK_90;
      _endHookType       = HOOK_90;

      _beginTextPlace    = PLACE_LEFT;
      _continueTextPlace = PLACE_LEFT;
      _beginSymbol       = noSym;
      _continueSymbol    = noSym;
      _endSymbol         = noSym;
      }

TextLine::TextLine(const TextLine& e)
   : SLine(e)
      {
      _beginTextPlace       = e._beginTextPlace;
      _continueTextPlace    = e._continueTextPlace;

      _beginHook            = e._beginHook;
      _endHook              = e._endHook;
      _beginHookType        = e._beginHookType;
      _endHookType          = e._endHookType;
      _beginHookHeight      = e._beginHookHeight;
      _endHookHeight        = e._endHookHeight;

      _beginSymbol          = e._beginSymbol;
      _continueSymbol       = e._continueSymbol;
      _endSymbol            = e._endSymbol;
      _beginSymbolOffset    = e._beginSymbolOffset;
      _continueSymbolOffset = e._continueSymbolOffset;
      _endSymbolOffset      = e._endSymbolOffset;
      _beginText            = 0;
      _continueText         = 0;
      if (e._beginText) {
            _beginText = e._beginText->clone(); // deep copy
            _beginText->setParent(this);
            }
      if (e._continueText) {
            _continueText = e._continueText->clone();
            _continueText->setParent(this);
            }
      }

//---------------------------------------------------------
//   setBeginText
//---------------------------------------------------------

void TextLine::setBeginText(const QString& s, const TextStyle& textStyle)
      {
      if (!_beginText) {
            _beginText = new Text(score());
            _beginText->setParent(this);
            }
      _beginText->setTextStyle(textStyle);
      _beginText->setText(s);
      }

//---------------------------------------------------------
//   setContinueText
//---------------------------------------------------------

void TextLine::setContinueText(const QString& s, const TextStyle& textStyle)
      {
      if (!_continueText) {
            _continueText = new Text(score());
            _continueText->setParent(this);
            }
      _continueText->setTextStyle(textStyle);
      _continueText->setText(s);
      }

//---------------------------------------------------------
//   setBeginText
//---------------------------------------------------------

void TextLine::setBeginText(Text* v)
      {
      delete _beginText;
      _beginText = v;
      }

//---------------------------------------------------------
//   setContinueText
//---------------------------------------------------------

void TextLine::setContinueText(Text* v)
      {
      delete _continueText;
      _continueText = v;
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

      SLine::writeProperties(xml);
      if (_beginText) {
            xml.stag("beginText");
            _beginText->writeProperties(xml);
            xml.etag();
            }
      if (_continueText) {
            xml.stag("continueText");
            _continueText->writeProperties(xml);
            xml.etag();
            }
      if (QVariant(_beginSymbol) != propertyDefault(P_BEGIN_SYMBOL)) {
            writeProperty(xml, P_BEGIN_SYMBOL);
            writeProperty(xml, P_BEGIN_SYMBOL_OFFSET);
            }
      if (QVariant(_continueSymbol) != propertyDefault(P_CONTINUE_SYMBOL)) {
            writeProperty(xml, P_CONTINUE_SYMBOL);
            writeProperty(xml, P_CONTINUE_SYMBOL_OFFSET);
            }
      if (QVariant(_endSymbol) != propertyDefault(P_END_SYMBOL)) {
            writeProperty(xml, P_END_SYMBOL);
            writeProperty(xml, P_END_SYMBOL_OFFSET);
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
      else if (tag == "hookUp")           // obsolete
            _endHookHeight *= qreal(-1.0);
      else if (tag == "beginSymbol" || tag == "symbol") {     // "symbol" is obsolete
            QString text(e.readElementText());
            _beginSymbol = text[0].isNumber() ? SymId(text.toInt()) : Sym::name2id(text);
            }
      else if (tag == "continueSymbol") {
            QString text(e.readElementText());
            _continueSymbol = text[0].isNumber() ? SymId(text.toInt()) : Sym::name2id(text);
            }
      else if (tag == "endSymbol") {
            QString text(e.readElementText());
            _endSymbol = text[0].isNumber() ? SymId(text.toInt()) : Sym::name2id(text);
            }
      else if (tag == "beginSymbolOffset")
            _beginSymbolOffset = e.readPoint();
      else if (tag == "continueSymbolOffset")
            _continueSymbolOffset = e.readPoint();
      else if (tag == "endSymbolOffset")
            _endSymbolOffset = e.readPoint();
      else if (tag == "beginTextPlace")
            _beginTextPlace = readPlacement(e);
      else if (tag == "continueTextPlace")
            _continueTextPlace = readPlacement(e);
      else if (tag == "beginText") {
            _beginText = new Text(score());
            _beginText->setParent(this);
            _beginText->read(e);
            }
      else if (tag == "continueText") {
            _continueText = new Text(score());
            _continueText->setParent(this);
            _continueText->read(e);
            }
      else if (!SLine::readProperties(e)) {
            qDebug(" ==readSLineProps: failed");
            return false;
            }
      return true;
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
      if (_beginText)
            _beginText->layout();
      if (_continueText)
            _continueText->layout();
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
//   getProperty
//---------------------------------------------------------

QVariant TextLine::getProperty(P_ID id) const
      {
      switch (id) {
            case P_BEGIN_TEXT_PLACE:
                  return _beginTextPlace;
            case P_CONTINUE_TEXT_PLACE:
                  return _continueTextPlace;
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
            case P_BEGIN_SYMBOL:
                  return _beginSymbol;
            case P_CONTINUE_SYMBOL:
                  return _continueSymbol;
            case P_END_SYMBOL:
                  return _endSymbol;
            case P_BEGIN_SYMBOL_OFFSET:
                  return _beginSymbolOffset;
            case P_CONTINUE_SYMBOL_OFFSET:
                  return _continueSymbolOffset;
            case P_END_SYMBOL_OFFSET:
                  return _endSymbolOffset;
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
            case P_BEGIN_SYMBOL:
                  _beginSymbol = SymId(v.toInt());
                  break;
            case P_CONTINUE_SYMBOL:
                  _continueSymbol = SymId(v.toInt());
                  break;
            case P_END_SYMBOL:
                  _endSymbol = SymId(v.toInt());
                  break;
            case P_BEGIN_SYMBOL_OFFSET:
                  _beginSymbolOffset = v.toPointF();
                  break;
            case P_CONTINUE_SYMBOL_OFFSET:
                  _continueSymbolOffset = v.toPointF();
                  break;
            case P_END_SYMBOL_OFFSET:
                  _endSymbolOffset = v.toPointF();
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
            case P_BEGIN_SYMBOL:
            case P_CONTINUE_SYMBOL:
            case P_END_SYMBOL:
                  return noSym;
            case P_BEGIN_SYMBOL_OFFSET:
            case P_CONTINUE_SYMBOL_OFFSET:
            case P_END_SYMBOL_OFFSET:
                  return QPointF();
            default:
                  return SLine::propertyDefault(id);
            }
      }

}

