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

TextLineSegment::TextLineSegment(const TextLineSegment& seg)
   : LineSegment(seg)
      {
      layout1();    // set the right _text
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
            if (spannerSegmentType() == SpannerSegmentType::SINGLE || spannerSegmentType() == SpannerSegmentType::BEGIN) {
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
      else if (!tl->visible())
            color = Qt::gray;
      else {
            color = tl->curColor();
            normalColor = true;
            }
      qreal l = 0.0;
      if (_text) {
            SpannerSegmentType st = spannerSegmentType();
            if (
               ((st == SpannerSegmentType::SINGLE || st == SpannerSegmentType::BEGIN) && (tl->beginTextPlace() == PlaceText::LEFT))
               || ((st == SpannerSegmentType::MIDDLE || st == SpannerSegmentType::END) && (tl->continueTextPlace() == PlaceText::LEFT))
               ) {
                  QRectF bb(_text->bbox());
                  l = _text->pos().x() + bb.width() + textlineTextDistance;
                  }
            painter->translate(_text->pos());
            _text->setVisible(tl->visible());
            _text->draw(painter);
            painter->translate(-_text->pos());
            }

      if (spannerSegmentType() == SpannerSegmentType::SINGLE || spannerSegmentType() == SpannerSegmentType::END) {
            if (_endText) {
                  painter->translate(_endText->pos());
                  _endText->setVisible(tl->visible());
                  _endText->draw(painter);
                  painter->translate(-_endText->pos());
                  }
            }
      QPen pen(normalColor ? tl->lineColor() : color, textlineLineWidth, tl->lineStyle());
      painter->setPen(pen);

      QPointF pp1(l, 0.0);

      qreal beginHookWidth;
      qreal endHookWidth;

      if (tl->beginHook() && tl->beginHookType() == HookType::HOOK_45) {
            beginHookWidth = fabs(tl->beginHookHeight().val() * _spatium * .4);
            pp1.rx() += beginHookWidth;
            }
      else
            beginHookWidth = 0;

      if (tl->endHook() && tl->endHookType() == HookType::HOOK_45) {
            endHookWidth = fabs(tl->endHookHeight().val() * _spatium * .4);
            pp2.rx() -= endHookWidth;
            }
      else
            endHookWidth = 0;

      // don't draw backwards lines (or hooks) if text is longer than nominal line length
      if (pp1.x() < pp2.x())
            painter->drawLine(QLineF(pp1.x(), pp1.y(), pp2.x(), pp2.y()));

      if (tl->beginHook()
         && (spannerSegmentType() == SpannerSegmentType::SINGLE
             || spannerSegmentType() == SpannerSegmentType::BEGIN)
         ) {
            qreal hh = tl->beginHookHeight().val() * _spatium;
            painter->drawLine(QLineF(pp1.x(), pp1.y(), pp1.x() - beginHookWidth, pp1.y() + hh));
            }

      if (tl->endHook() && pp1.x() < pp2.x()
         && (spannerSegmentType() == SpannerSegmentType::SINGLE
             || spannerSegmentType() == SpannerSegmentType::END)
         ) {
            qreal hh = tl->endHookHeight().val() * _spatium;
            painter->drawLine(QLineF(pp2.x(), pp2.y(), pp2.x() + endHookWidth, pp2.y() + hh));
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
//   setText
//---------------------------------------------------------

void TextLineSegment::setText(Text* t)
      {
      if (t) {
            if (_text == 0) {
                  _text = new Text(*t);
                  _text->setFlag(ElementFlag::MOVABLE, false);
                  _text->setParent(this);
                  }
            else {
                  _text->setTextStyleType(t->textStyleType());
                  _text->setTextStyle(t->textStyle());
                  _text->setText(t->text());
                  }
            }
      else {
            delete _text;
            _text = 0;
            }
      }

//---------------------------------------------------------
//   layout1
//---------------------------------------------------------

void TextLineSegment::layout1()
      {
      TextLine* tl = textLine();
      if (parent() && tl && tl->type() != Element::Type::OTTAVA
                  && tl->type() != Element::Type::PEDAL
                  && tl->type() != Element::Type::VOLTA)
            rypos() += -5.0 * spatium();

      if (!tl->diagonal())
            _userOff2.setY(0);
      switch (spannerSegmentType()) {
            case SpannerSegmentType::SINGLE:
            case SpannerSegmentType::BEGIN:
                  setText(tl->_beginText);
                  break;
            case SpannerSegmentType::MIDDLE:
            case SpannerSegmentType::END:
                  setText(tl->_continueText);
                  break;
            }
      if (tl->_endText) {
            if (_endText == 0) {
                  _endText = new Text(*tl->_endText);
                  _endText->setFlag(ElementFlag::MOVABLE, false);
                  _endText->setParent(this);
                  }
            else {
                  _endText->setTextStyleType(tl->_endText->textStyleType());
                  _endText->setTextStyle(tl->_endText->textStyle());
                  _endText->setText(tl->_endText->text());
                  }
            }
      else {
            delete _endText;
            _endText = 0;
            }

      if (_text)
            _text->layout();
      if (_endText)
            _endText->layout();

      QPointF pp1;
      QPointF pp2(pos2());

      if (!_text && !_endText && pp2.y() != 0) {
            setbbox(QRectF(pp1, pp2).normalized());
            return;
            }
      qreal y1 = point(-textLine()->lineWidth());
      qreal y2 = -y1;

      if (_text) {
            qreal h = _text->height();
            if (textLine()->beginTextPlace() == PlaceText::ABOVE)
                  y1 = -h;
            else if (textLine()->beginTextPlace() == PlaceText::BELOW)
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
      if (_endText) {
            _endText->setPos(bbox().right(), 0);
            bbox() |= _endText->bbox().translated(_endText->pos());
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant TextLineSegment::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::BEGIN_TEXT_PLACE:
            case P_ID::CONTINUE_TEXT_PLACE:
            case P_ID::END_TEXT_PLACE:
            case P_ID::BEGIN_HOOK:
            case P_ID::END_HOOK:
            case P_ID::BEGIN_HOOK_HEIGHT:
            case P_ID::END_HOOK_HEIGHT:
            case P_ID::BEGIN_HOOK_TYPE:
            case P_ID::END_HOOK_TYPE:
            case P_ID::BEGIN_TEXT:
            case P_ID::CONTINUE_TEXT:
            case P_ID::END_TEXT:
            case P_ID::LINE_COLOR:
            case P_ID::LINE_WIDTH:
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
            case P_ID::BEGIN_TEXT_PLACE:
            case P_ID::CONTINUE_TEXT_PLACE:
            case P_ID::END_TEXT_PLACE:
            case P_ID::BEGIN_HOOK:
            case P_ID::END_HOOK:
            case P_ID::BEGIN_HOOK_HEIGHT:
            case P_ID::END_HOOK_HEIGHT:
            case P_ID::BEGIN_HOOK_TYPE:
            case P_ID::END_HOOK_TYPE:
            case P_ID::BEGIN_TEXT:
            case P_ID::CONTINUE_TEXT:
            case P_ID::END_TEXT:
            case P_ID::LINE_COLOR:
            case P_ID::LINE_WIDTH:
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
            case P_ID::BEGIN_TEXT_PLACE:
            case P_ID::CONTINUE_TEXT_PLACE:
            case P_ID::END_TEXT_PLACE:
            case P_ID::BEGIN_HOOK:
            case P_ID::END_HOOK:
            case P_ID::BEGIN_HOOK_HEIGHT:
            case P_ID::END_HOOK_HEIGHT:
            case P_ID::BEGIN_HOOK_TYPE:
            case P_ID::END_HOOK_TYPE:
            case P_ID::BEGIN_TEXT:
            case P_ID::CONTINUE_TEXT:
            case P_ID::END_TEXT:
            case P_ID::LINE_COLOR:
            case P_ID::LINE_WIDTH:
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
      _beginHookType     = HookType::HOOK_90;
      _endHookType       = HookType::HOOK_90;

      _beginTextPlace    = PlaceText::LEFT;
      _continueTextPlace = PlaceText::LEFT;
      _endTextPlace      = PlaceText::LEFT;
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
//   createBeginTextElement
//---------------------------------------------------------

void TextLine::createBeginTextElement()
      {
      if (!_beginText) {
            _beginText = new Text(score());
            _beginText->setParent(this);
            _beginText->setTextStyleType(TextStyleType::TEXTLINE);
            }
      }

//---------------------------------------------------------
//   createContinueTextElement
//---------------------------------------------------------

void TextLine::createContinueTextElement()
      {
      if (!_continueText) {
            _continueText = new Text(score());
            _continueText->setParent(this);
            _continueText->setTextStyleType(TextStyleType::TEXTLINE);
            }
      }

//---------------------------------------------------------
//   createEndTextElement
//---------------------------------------------------------

void TextLine::createEndTextElement()
      {
      if (!_endText) {
            _endText = new Text(score());
            _endText->setParent(this);
            _endText->setTextStyleType(TextStyleType::TEXTLINE);
            }
      }

//---------------------------------------------------------
//   setBeginText
//---------------------------------------------------------

void TextLine::setBeginText(const QString& s, TextStyleType textStyle)
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
      if (s.isEmpty()) {
            delete _beginText;
            _beginText = 0;
            return;
            }
      createBeginTextElement();
      _beginText->setText(s);
      }

//---------------------------------------------------------
//   setContinueText
//---------------------------------------------------------

void TextLine::setContinueText(const QString& s, TextStyleType textStyle)
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
      if (s.isEmpty()) {
            delete _continueText;
            _continueText = 0;
            return;
            }
      createContinueTextElement();
      _continueText->setText(s);
      }

//---------------------------------------------------------
//   setEndText
//---------------------------------------------------------

void TextLine::setEndText(const QString& s, TextStyleType textStyle)
      {
      if (s.isEmpty()) {
            delete _endText;
            _endText = 0;
            return;
            }
      if (!_endText) {
            _endText = new Text(score());
            _endText->setParent(this);
            }
      _endText->setTextStyleType(textStyle);
      _endText->setText(s);
      }

void TextLine::setEndText(const QString& s)
      {
      if (s.isEmpty()) {
            delete _endText;
            _endText = 0;
            return;
            }
      createEndTextElement();
      _endText->setText(s);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextLine::write(Xml& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));
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
      e.addSpanner(e.intAttribute("id", -1), this);

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
      writeProperty(xml, P_ID::BEGIN_HOOK);
      writeProperty(xml, P_ID::BEGIN_HOOK_HEIGHT);
      writeProperty(xml, P_ID::BEGIN_HOOK_TYPE);
      writeProperty(xml, P_ID::END_HOOK);
      writeProperty(xml, P_ID::END_HOOK_HEIGHT);
      writeProperty(xml, P_ID::END_HOOK_TYPE);
      writeProperty(xml, P_ID::BEGIN_TEXT_PLACE);
      writeProperty(xml, P_ID::CONTINUE_TEXT_PLACE);
      writeProperty(xml, P_ID::END_TEXT_PLACE);

      SLine::writeProperties(xml);

      if (_beginText) {
            bool textDiff  = _beginText->text() != propertyDefault(P_ID::BEGIN_TEXT).toString();
            bool styleDiff = _beginText->textStyle() != propertyDefault(P_ID::BEGIN_TEXT_STYLE).value<TextStyle>();
            if (styleDiff)
                  textDiff = true;
            if (textDiff || styleDiff) {
                  xml.stag("beginText");
                  _beginText->writeProperties(xml, textDiff, styleDiff);
                  xml.etag();
                  }
            }
      if (_continueText) {
            bool textDiff  = _continueText->text() != propertyDefault(P_ID::CONTINUE_TEXT).toString();
            bool styleDiff = _continueText->textStyle() != propertyDefault(P_ID::CONTINUE_TEXT_STYLE).value<TextStyle>();
            if (styleDiff)
                  textDiff = true;
            if (textDiff || styleDiff) {
                  xml.stag("continueText");
                  _continueText->writeProperties(xml, textDiff, styleDiff);
                  xml.etag();
                  }
            }
      if (_endText) {
            bool textDiff  = _endText->text() != propertyDefault(P_ID::END_TEXT).toString();
            bool styleDiff = _endText->textStyle() != propertyDefault(P_ID::END_TEXT_STYLE).value<TextStyle>();
            if (styleDiff)
                  textDiff = true;
            if (textDiff || styleDiff) {
                  xml.stag("endText");
                  _endText->writeProperties(xml, textDiff, styleDiff);
                  xml.etag();
                  }
            }
      }

//---------------------------------------------------------
//   resolveSymCompatibility
//---------------------------------------------------------

static QString resolveSymCompatibility(SymId i, QString programVersion)
      {
      if (!programVersion.isEmpty() && programVersion < "1.1")
            i = SymId(int(i) + 5);
      switch(int(i)) {
            case 197:   return "keyboardPedalPed";
            case 191:   return "keyboardPedalUp";
            case 193:   return "noSym"; //SymId(pedaldotSym);
            case 192:   return "noSym"; //SymId(pedaldashSym);
            case 139:   return "ornamentTrill";
            default:    return "noSym";
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool TextLine::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "beginHook")
            _beginHook = e.readBool();
      else if (tag == "beginHookHeight") {
            _beginHookHeight = Spatium(e.readDouble());
            _beginHook = true;
            }
      else if (tag == "beginHookType")
            _beginHookType = HookType(e.readInt());
      else if (tag == "endHook")
            _endHook = e.readBool();
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
            setBeginText(QString("<sym>%1</sym>").arg(text[0].isNumber() ? resolveSymCompatibility(SymId(text.toInt()), score()->mscoreVersion()) : text));
            }
      else if (tag == "continueSymbol") {
            QString text(e.readElementText());
            setContinueText(QString("<sym>%1</sym>").arg(text[0].isNumber() ? resolveSymCompatibility(SymId(text.toInt()), score()->mscoreVersion()) : text));
            }
      else if (tag == "endSymbol") {
            QString text(e.readElementText());
            setEndText(QString("<sym>%1</sym>").arg(text[0].isNumber() ? resolveSymCompatibility(SymId(text.toInt()), score()->mscoreVersion()) : text));
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
            else
                  _beginText->setText("");
            _beginText->read(e);
            }
      else if (tag == "continueText") {
            if (!_continueText) {
                  _continueText = new Text(score());
                  _continueText->setParent(this);
                  }
            else
                  _continueText->setText("");
            _continueText->read(e);
            }
      else if (tag == "endText") {
            if (!_endText) {
                  _endText = new Text(score());
                  _endText->setParent(this);
                  }
            else
                  _endText->setText("");
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
            case P_ID::BEGIN_TEXT_PLACE:
                  return int(_beginTextPlace);
            case P_ID::CONTINUE_TEXT_PLACE:
                  return int(_continueTextPlace);
            case P_ID::END_TEXT_PLACE:
                  return int(_endTextPlace);
            case P_ID::BEGIN_HOOK:
                  return _beginHook;
            case P_ID::END_HOOK:
                  return _endHook;
            case P_ID::BEGIN_HOOK_HEIGHT:
                  return _beginHookHeight.val();
            case P_ID::END_HOOK_HEIGHT:
                  return _endHookHeight.val();
            case P_ID::BEGIN_HOOK_TYPE:
                  return int(_beginHookType);
            case P_ID::END_HOOK_TYPE:
                  return int(_endHookType);
            case P_ID::BEGIN_TEXT:
                  return beginText();
            case P_ID::CONTINUE_TEXT:
                  return continueText();
            case P_ID::END_TEXT:
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
            case P_ID::BEGIN_TEXT_PLACE:
                  _beginTextPlace = PlaceText(v.toInt());
                  break;
            case P_ID::CONTINUE_TEXT_PLACE:
                  _continueTextPlace = PlaceText(v.toInt());
                  break;
            case P_ID::END_TEXT_PLACE:
                  _endTextPlace = PlaceText(v.toInt());
                  break;
            case P_ID::BEGIN_HOOK:
                  _beginHook = v.toBool();
                  break;
            case P_ID::END_HOOK:
                  _endHook = v.toBool();
                  break;
            case P_ID::BEGIN_HOOK_HEIGHT:
                  _beginHookHeight = Spatium(v.toDouble());
                  break;
            case P_ID::END_HOOK_HEIGHT:
                  _endHookHeight = Spatium(v.toDouble());
                  break;
            case P_ID::BEGIN_HOOK_TYPE:
                  _beginHookType = HookType(v.toInt());
                  break;
            case P_ID::END_HOOK_TYPE:
                  _endHookType = HookType(v.toInt());
                  break;
            case P_ID::BEGIN_TEXT:
                  setBeginText(v.toString());
                  break;
            case P_ID::CONTINUE_TEXT:
                  setContinueText(v.toString());
                  break;
            case P_ID::END_TEXT:
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
            case P_ID::BEGIN_TEXT_PLACE:
            case P_ID::CONTINUE_TEXT_PLACE:
            case P_ID::END_TEXT_PLACE:
                  return int(PlaceText::LEFT);
            case P_ID::BEGIN_HOOK:
            case P_ID::END_HOOK:
                  return false;
            case P_ID::BEGIN_HOOK_HEIGHT:
            case P_ID::END_HOOK_HEIGHT:
                  return 1.5;
            case P_ID::BEGIN_HOOK_TYPE:
            case P_ID::END_HOOK_TYPE:
                  return int(HookType::HOOK_90);
            case P_ID::BEGIN_TEXT:
            case P_ID::CONTINUE_TEXT:
            case P_ID::END_TEXT:
                  return QString("");

            default:
                  return SLine::propertyDefault(id);
            }
      }

}

