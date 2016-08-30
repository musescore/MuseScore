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

#include "textlinebase.h"
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
//   TextLineBaseSegment
//---------------------------------------------------------

TextLineBaseSegment::TextLineBaseSegment(const TextLineBaseSegment& seg)
   : LineSegment(seg)
      {
      layout();    // set the right _text
      }

TextLineBaseSegment::~TextLineBaseSegment()
      {
      delete _text;
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void TextLineBaseSegment::setSelected(bool f)
      {
      SpannerSegment::setSelected(f);
      if (_text) {
            if (spannerSegmentType() == SpannerSegmentType::SINGLE || spannerSegmentType() == SpannerSegmentType::BEGIN) {
                  if (textLineBase()->_beginText)
                        _text->setSelected(f);
                  }
            else if (textLineBase()->_continueText)
                  _text->setSelected(f);
            }
      if (_endText) {
            if (spannerSegmentType() == SpannerSegmentType::SINGLE || spannerSegmentType() == SpannerSegmentType::END) {
                  if (textLineBase()->_endText)
                        _endText->setSelected(f);
                  }
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TextLineBaseSegment::draw(QPainter* painter) const
      {
      TextLineBase* tl   = textLineBase();
      qreal _spatium = spatium();

      // color for line (text color comes from the text properties)
      QColor color;
      if (selected() && !(score() && score()->printing()))
            color = (track() > -1) ? MScore::selectColor[voice()] : MScore::selectColor[0];
      else if (!tl->visible() || !tl->lineVisible())
            color = Qt::gray;
      else
            color = tl->lineColor();

      if (_text) {
            painter->translate(_text->pos());
            _text->setVisible(tl->visible());
            _text->draw(painter);
            painter->translate(-_text->pos());
            }

      if (_endText) {
            painter->translate(_endText->pos());
            _endText->setVisible(tl->visible());
            _endText->draw(painter);
            painter->translate(-_endText->pos());
            }

      if (npoints == 0)
            return;
      qreal textlineLineWidth    = tl->lineWidth().val() * _spatium;
      QPen pen(color, textlineLineWidth, tl->lineStyle());
      if (tl->lineStyle() == Qt::CustomDashLine) {
            QVector<qreal> dashes { tl->dashLineLen(), tl->dashGapLen() };
            pen.setDashPattern(dashes);
            }
      painter->setPen(pen);

      if (twoLines) {   // hairpins
            painter->drawLines(&points[0], 1);
            painter->drawLines(&points[2], 1);
            }
      else {
            for (int i = 0; i < npoints; ++i)
                  painter->drawLines(&points[i], 1);
            }
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape TextLineBaseSegment::shape() const
      {
      Shape shape;
      if (_text)
            shape.add(_text->bbox().translated(_text->pos()));
      if (_endText)
            shape.add(_endText->bbox().translated(_endText->pos()));
      qreal lw = textLineBase()->lineWidth().val() * spatium();
      if (twoLines) {   // hairpins
            shape.add(QRectF(points[0].x(), points[0].y() - lw * .5,
               points[1].x() - points[0].x(), points[1].y() - points[0].y() + lw));
            shape.add(QRectF(points[2].x(), points[2].y() - lw * .5,
               points[3].x() - points[2].x(), points[3].y() - points[2].y() + lw));
            }
      else {
            for (int i = 0; i < npoints; ++i) {
                  shape.add(QRectF(points[i].x() - lw * .5, points[i].y() - lw * .5,
                     points[i+1].x() - points[i].x() + lw, points[i+1].y() - points[i].y() + lw));
                  }
            }
      return shape;
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void TextLineBaseSegment::setText(Text* t)
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
                  _text->setXmlText(t->xmlText());
                  }
            _text->setTrack(track());
            _text->layout();
            }
      else {
            delete _text;
            _text = 0;
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextLineBaseSegment::layout()
      {
      npoints      = 0;
      TextLineBase* tl = textLineBase();
      qreal _spatium = spatium();

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
      if ((isSingleType() || isEndType()) && tl->_endText) {
            if (_endText == 0) {
                  _endText = new Text(*tl->_endText);
                  _endText->setFlag(ElementFlag::MOVABLE, false);
                  _endText->setParent(this);
                  }
            else {
                  _endText->setTextStyleType(tl->_endText->textStyleType());
                  _endText->setTextStyle(tl->_endText->textStyle());
                  _endText->setXmlText(tl->_endText->xmlText());
                  }
            _endText->setTrack(track());
            _endText->layout();
            }
      else {
            delete _endText;
            _endText = 0;
            }

      QPointF pp1;
      QPointF pp2(pos2());

      // diagonal line with no text - just use the basic rectangle for line (ignore hooks)
      if (!_text && !_endText && pp2.y() != 0) {
            npoints = 2;
            points[0] = pp1;
            points[1] = pp2;
            setbbox(QRectF(pp1, pp2).normalized());
            return;
            }

      // line has text or is not diagonal - calculate reasonable bbox

      qreal x1 = qMin(0.0, pp2.x());
      qreal x2 = qMax(0.0, pp2.x());
      qreal y0 = point(-textLineBase()->lineWidth());
      qreal y1 = qMin(0.0, pp2.y()) + y0;
      qreal y2 = qMax(0.0, pp2.y()) - y0;

      qreal l = 0.0;
      if (_text) {
            qreal textlineTextDistance = _spatium * .5;
            if (((isSingleType() || isBeginType()) && (tl->beginTextPlace() == PlaceText::LEFT)) || ((isMiddleType() || isEndType()) && (tl->continueTextPlace() == PlaceText::LEFT)))
                  l = _text->pos().x() + _text->bbox().width() + textlineTextDistance;
            qreal h = _text->height();
            if (textLineBase()->beginTextPlace() == PlaceText::ABOVE)
                  y1 = qMin(y1, -h);
            else if (textLineBase()->beginTextPlace() == PlaceText::BELOW)
                  y2 = qMax(y2, h);
            else {
                  y1 = qMin(y1, -h * .5);
                  y2 = qMax(y2, h * .5);
                  }
            x2 = qMax(x2, _text->width());
            }

      if (textLineBase()->endHook()) {
            qreal h = pp2.y() + point(textLineBase()->endHookHeight());
            if (h > y2)
                  y2 = h;
            else if (h < y1)
                  y1 = h;
            }

      if (textLineBase()->beginHook()) {
            qreal h = point(textLineBase()->beginHookHeight());
            if (h > y2)
                  y2 = h;
            else if (h < y1)
                  y1 = h;
            }
      bbox().setRect(x1, y1, x2 - x1, y2 - y1);
      if (_text)
            bbox() |= _text->bbox().translated(_text->pos());  // DEBUG
      // set end text position and extend bbox
      if (_endText) {
            _endText->setPos(bbox().right(), 0);
            bbox() |= _endText->bbox().translated(_endText->pos());
            }

      if (!(tl->lineVisible() || score()->showInvisible()))
            return;

      if (tl->lineVisible() || !score()->printing()) {
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
            bool backwards = _text && pp1.x() > pp2.x() && !tl->diagonal();

            if (tl->beginHook() && (isSingleType() || isBeginType())) {
                  qreal hh = tl->beginHookHeight().val() * _spatium;
                  points[npoints] = QPointF(pp1.x() - beginHookWidth, pp1.y() + hh);
                  ++npoints;
                  points[npoints] = pp1;
                  }
            if (!backwards) {
                  points[npoints] = pp1;
                  ++npoints;
                  points[npoints] = pp2;
                  // painter->drawLine(QLineF(pp1.x(), pp1.y(), pp2.x(), pp2.y()));

                  if (tl->endHook() && (isSingleType() || isEndType())) {
                        ++npoints;
                        qreal hh = tl->endHookHeight().val() * _spatium;
                        // painter->drawLine(QLineF(pp2.x(), pp2.y(), pp2.x() + endHookWidth, pp2.y() + hh));
                        points[npoints] = QPointF(pp2.x() + endHookWidth, pp2.y() + hh);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void TextLineBaseSegment::spatiumChanged(qreal ov, qreal nv)
      {
      textLineBase()->spatiumChanged(ov, nv);
      if (_text)
            _text->spatiumChanged(ov, nv);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant TextLineBaseSegment::getProperty(P_ID id) const
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
            case P_ID::LINE_VISIBLE:
            case P_ID::LINE_COLOR:
            case P_ID::LINE_WIDTH:
                  return textLineBase()->getProperty(id);
            default:
                  return LineSegment::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TextLineBaseSegment::setProperty(P_ID id, const QVariant& v)
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
            case P_ID::LINE_VISIBLE:
            case P_ID::LINE_COLOR:
            case P_ID::LINE_WIDTH:
                  return textLineBase()->setProperty(id, v);
            default:
                  return LineSegment::setProperty(id, v);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant TextLineBaseSegment::propertyDefault(P_ID id) const
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
            case P_ID::LINE_VISIBLE:
            case P_ID::LINE_COLOR:
            case P_ID::LINE_WIDTH:
                  return textLineBase()->propertyDefault(id);
            default:
                  return LineSegment::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   TextLineBase
//---------------------------------------------------------

TextLineBase::TextLineBase(Score* s)
   : SLine(s)
      {
      _beginText         = 0;
      _continueText      = 0;
      _endText           = 0;

      _beginHookHeight   = Spatium(1.5);
      _endHookHeight     = Spatium(1.5);
      _lineVisible       = true;
      _beginHook         = false;
      _endHook           = false;
      _beginHookType     = HookType::HOOK_90;
      _endHookType       = HookType::HOOK_90;

      _beginTextPlace    = PlaceText::LEFT;
      _continueTextPlace = PlaceText::LEFT;
      _endTextPlace      = PlaceText::LEFT;
      }

TextLineBase::TextLineBase(const TextLineBase& e)
   : SLine(e)
      {
      _beginTextPlace       = e._beginTextPlace;
      _continueTextPlace    = e._continueTextPlace;
      _endTextPlace         = e._endTextPlace;

      _lineVisible          = e._lineVisible;
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
//   TextLineBase
//---------------------------------------------------------

TextLineBase::~TextLineBase()
      {
      delete _beginText;
      delete _continueText;
      delete _endText;
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void TextLineBase::setScore(Score* s)
      {
      Spanner::setScore(s);
      if (_beginText)
            _beginText->setScore(s);
      if (_continueText)
            _continueText->setScore(s);
      if (_endText)
            _endText->setScore(s);
      }

//---------------------------------------------------------
//   createBeginTextElement
//---------------------------------------------------------

void TextLineBase::createBeginTextElement()
      {
      if (!_beginText) {
            _beginText = new Text(score());
            _beginText->setParent(this);
            _beginText->setTextStyleType(static_cast<TextStyleType>(propertyDefault(P_ID::TEXT_STYLE_TYPE).toInt()));
            }
      }

//---------------------------------------------------------
//   createContinueTextElement
//---------------------------------------------------------

void TextLineBase::createContinueTextElement()
      {
      if (!_continueText) {
            _continueText = new Text(score());
            _continueText->setParent(this);
            _continueText->setTextStyleType(static_cast<TextStyleType>(propertyDefault(P_ID::TEXT_STYLE_TYPE).toInt()));
            }
      }

//---------------------------------------------------------
//   createEndTextElement
//---------------------------------------------------------

void TextLineBase::createEndTextElement()
      {
      if (!_endText) {
            _endText = new Text(score());
            _endText->setParent(this);
            _endText->setTextStyleType(static_cast<TextStyleType>(propertyDefault(P_ID::TEXT_STYLE_TYPE).toInt()));
            }
      }

//---------------------------------------------------------
//   setBeginText
//---------------------------------------------------------

void TextLineBase::setBeginText(const QString& s, TextStyleType textStyle)
      {
      if (!_beginText) {
            _beginText = new Text(score());
            _beginText->setParent(this);
            }
      _beginText->setTextStyleType(textStyle);
      _beginText->setXmlText(s);
      }

void TextLineBase::setBeginText(const QString& s)
      {
      if (s.isEmpty()) {
            delete _beginText;
            _beginText = 0;
            return;
            }
      createBeginTextElement();
      _beginText->setXmlText(s);
      }

//---------------------------------------------------------
//   setContinueText
//---------------------------------------------------------

void TextLineBase::setContinueText(const QString& s, TextStyleType textStyle)
      {
      if (!_continueText) {
            _continueText = new Text(score());
            _continueText->setParent(this);
            }
      _continueText->setTextStyleType(textStyle);
      _continueText->setXmlText(s);
      }

void TextLineBase::setContinueText(const QString& s)
      {
      if (s.isEmpty()) {
            delete _continueText;
            _continueText = 0;
            return;
            }
      createContinueTextElement();
      _continueText->setXmlText(s);
      }

//---------------------------------------------------------
//   setEndText
//---------------------------------------------------------

void TextLineBase::setEndText(const QString& s, TextStyleType textStyle)
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
      _endText->setXmlText(s);
      }

void TextLineBase::setEndText(const QString& s)
      {
      if (s.isEmpty()) {
            delete _endText;
            _endText = 0;
            return;
            }
      createEndTextElement();
      _endText->setXmlText(s);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextLineBase::write(Xml& xml) const
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

void TextLineBase::read(XmlReader& e)
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
//   spatiumChanged
//---------------------------------------------------------

void TextLineBase::spatiumChanged(qreal ov, qreal nv)
      {
      if (_beginText)
            _beginText->spatiumChanged(ov, nv);
      if (_continueText)
            _continueText->spatiumChanged(ov, nv);
      }

//---------------------------------------------------------
//   beginText
//---------------------------------------------------------

QString TextLineBase::beginText() const
      {
      return _beginText ? _beginText->xmlText() : "";
      }

//---------------------------------------------------------
//   continueText
//---------------------------------------------------------

QString TextLineBase::continueText() const
      {
      return _continueText ? _continueText->xmlText() : "";
      }

//---------------------------------------------------------
//   endText
//---------------------------------------------------------

QString TextLineBase::endText() const
      {
      return _endText ? _endText->xmlText() : "";
      }

//---------------------------------------------------------
//   writeProperties
//    write properties different from prototype
//---------------------------------------------------------

void TextLineBase::writeProperties(Xml& xml) const
      {
      writeProperty(xml, P_ID::LINE_VISIBLE);
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
            bool textDiff  = _beginText->xmlText() != propertyDefault(P_ID::BEGIN_TEXT).toString();
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
            bool textDiff  = _continueText->xmlText() != propertyDefault(P_ID::CONTINUE_TEXT).toString();
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
            bool textDiff  = _endText->xmlText() != propertyDefault(P_ID::END_TEXT).toString();
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

bool TextLineBase::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "lineVisible")
            _lineVisible = e.readBool();
      else if (tag == "beginHook")
            _beginHook = e.readBool();
      else if (tag == "beginHookHeight") {
            _beginHookHeight = Spatium(e.readDouble());
            if (score()->mscVersion() <= 114)
                  _beginHook = true;
            }
      else if (tag == "beginHookType")
            _beginHookType = HookType(e.readInt());
      else if (tag == "endHook")
            _endHook = e.readBool();
      else if (tag == "endHookHeight" || tag == "hookHeight") { // hookHeight is obsolete
            _endHookHeight = Spatium(e.readDouble());
            if (score()->mscVersion() <= 114)
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
                  _beginText->setTextStyleType(static_cast<TextStyleType>(propertyDefault(P_ID::TEXT_STYLE_TYPE).toInt()));
                  }
            else
                  _beginText->setXmlText("");
            _beginText->read(e);
            }
      else if (tag == "continueText") {
            if (!_continueText) {
                  _continueText = new Text(score());
                  _continueText->setParent(this);
                  _continueText->setTextStyleType(static_cast<TextStyleType>(propertyDefault(P_ID::TEXT_STYLE_TYPE).toInt()));
                  }
            else
                  _continueText->setXmlText("");
            _continueText->read(e);
            }
      else if (tag == "endText") {
            if (!_endText) {
                  _endText = new Text(score());
                  _endText->setParent(this);
                  _endText->setTextStyleType(static_cast<TextStyleType>(propertyDefault(P_ID::TEXT_STYLE_TYPE).toInt()));
                  }
            else
                  _endText->setXmlText("");
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

QVariant TextLineBase::getProperty(P_ID id) const
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
                  return _beginHookHeight;
            case P_ID::END_HOOK_HEIGHT:
                  return _endHookHeight;
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
            case P_ID::LINE_VISIBLE:
                  return lineVisible();
            default:
                  return SLine::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TextLineBase::setProperty(P_ID id, const QVariant& v)
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
                  _beginHookHeight = v.value<Spatium>();
                  break;
            case P_ID::END_HOOK_HEIGHT:
                  _endHookHeight = v.value<Spatium>();
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
            case P_ID::LINE_VISIBLE:
                  setLineVisible(v.toBool());
                  break;
            default:
                  return SLine::setProperty(id, v);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant TextLineBase::propertyDefault(P_ID id) const
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
                  return Spatium(1.5);
            case P_ID::BEGIN_HOOK_TYPE:
            case P_ID::END_HOOK_TYPE:
                  return int(HookType::HOOK_90);
            case P_ID::BEGIN_TEXT:
            case P_ID::CONTINUE_TEXT:
            case P_ID::END_TEXT:
                  return QString("");
            case P_ID::LINE_VISIBLE:
                  return true;
            case P_ID::BEGIN_TEXT_STYLE:
            case P_ID::CONTINUE_TEXT_STYLE:
            case P_ID::END_TEXT_STYLE:
                  return QVariant::fromValue(score()->textStyle(static_cast<TextStyleType>(propertyDefault(P_ID::TEXT_STYLE_TYPE).toInt())));
            case P_ID::TEXT_STYLE_TYPE:
                  return int(TextStyleType::TEXTLINE);

            default:
                  return SLine::propertyDefault(id);
            }
      }

}

