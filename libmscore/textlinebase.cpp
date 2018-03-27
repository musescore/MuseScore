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

TextLineBaseSegment::TextLineBaseSegment(Score* score)
   : LineSegment(score)
      {
      _text    = new Text(score);
      _endText = new Text(score);
      _text->setParent(this);
      _endText->setParent(this);
      _text->setFlag(ElementFlag::MOVABLE, false);
      _endText->setFlag(ElementFlag::MOVABLE, false);
      }

TextLineBaseSegment::TextLineBaseSegment(const TextLineBaseSegment& seg)
   : LineSegment(seg)
      {
      _text    = new Text(*seg._text);
      _endText = new Text(*seg._endText);
      _text->setParent(this);
      _endText->setParent(this);
      layout();    // set the right _text
      }

TextLineBaseSegment::~TextLineBaseSegment()
      {
      delete _text;
      delete _endText;
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void TextLineBaseSegment::setSelected(bool f)
      {
      SpannerSegment::setSelected(f);
      _text->setSelected(f);
      _endText->setSelected(f);
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
      if ((selected() && !(score() && score()->printing())) || !tl->visible() || !tl->lineVisible())
            color = curColor();
      else
            color = tl->lineColor();

      if (!_text->empty()) {
            painter->translate(_text->pos());
            _text->setVisible(tl->visible());
            _text->draw(painter);
            painter->translate(-_text->pos());
            }

      if (!_endText->empty()) {
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
      if (!_text->empty())
            shape.add(_text->bbox().translated(_text->pos()));
      if (!_endText->empty())
            shape.add(_endText->bbox().translated(_endText->pos()));
      qreal lw  = textLineBase()->lineWidth().val() * spatium();
      qreal lw2 = lw * .5;
      if (twoLines) {   // hairpins
            shape.add(QRectF(points[0].x(), points[0].y() - lw2,
               points[1].x() - points[0].x(), points[1].y() - points[0].y() + lw));
            shape.add(QRectF(points[2].x(), points[2].y() - lw2,
               points[3].x() - points[2].x(), points[3].y() - points[2].y() + lw));
            }
      else {
            for (int i = 0; i < npoints; ++i) {
                  shape.add(QRectF(points[i].x() - lw2, points[i].y() - lw2,
                     points[i+1].x() - points[i].x() + lw, points[i+1].y() - points[i].y() + lw));
                  }
            }
      return shape;
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
                  _text->setXmlText(tl->beginText());
                  _text->setFamily(tl->beginFontFamily());
                  _text->setSize(tl->beginFontSize());
                  _text->setOffset(tl->beginTextOffset());
                  _text->setAlign(tl->beginTextAlign());
                  _text->setBold(tl->beginFontBold());
                  _text->setItalic(tl->beginFontItalic());
                  _text->setUnderline(tl->beginFontUnderline());
                  break;
            case SpannerSegmentType::MIDDLE:
            case SpannerSegmentType::END:
                  _text->setXmlText(tl->continueText());
                  _text->setFamily(tl->continueFontFamily());
                  _text->setSize(tl->continueFontSize());
                  _text->setOffset(tl->continueTextOffset());
                  _text->setAlign(tl->continueTextAlign());
                  _text->setBold(tl->continueFontBold());
                  _text->setItalic(tl->continueFontItalic());
                  _text->setUnderline(tl->continueFontUnderline());
                  break;
            }
      _text->setTrack(track());
      _text->layout();

      if ((isSingleType() || isEndType())) {
            _endText->setXmlText(tl->endText());
            _endText->setFamily(tl->endFontFamily());
            _endText->setSize(tl->endFontSize());
            _endText->setOffset(tl->endTextOffset());
            _endText->setAlign(tl->endTextAlign());
            _endText->setBold(tl->endFontBold());
            _endText->setItalic(tl->endFontItalic());
            _endText->setUnderline(tl->endFontUnderline());
            _endText->setTrack(track());
            _endText->layout();
            }
      else {
            _endText->setXmlText("");
            }

      QPointF pp1;
      QPointF pp2(pos2());

      // diagonal line with no text - just use the basic rectangle for line (ignore hooks)
      if (_text->empty() && _endText->empty() && pp2.y() != 0) {
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
      if (!_text->empty()) {
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

      if (textLineBase()->endHookType() != HookType::NONE) {
            qreal h = pp2.y() + point(textLineBase()->endHookHeight());
            if (h > y2)
                  y2 = h;
            else if (h < y1)
                  y1 = h;
            }

      if (textLineBase()->beginHookType() != HookType::NONE) {
            qreal h = point(textLineBase()->beginHookHeight());
            if (h > y2)
                  y2 = h;
            else if (h < y1)
                  y1 = h;
            }
      bbox().setRect(x1, y1, x2 - x1, y2 - y1);
      if (!_text->empty())
            bbox() |= _text->bbox().translated(_text->pos());  // DEBUG
      // set end text position and extend bbox
      if (!_endText->empty()) {
            _endText->setPos(bbox().right(), 0);
            bbox() |= _endText->bbox().translated(_endText->pos());
            }

      if (!(tl->lineVisible() || score()->showInvisible()))
            return;

      if (tl->lineVisible() || !score()->printing()) {
            QPointF pp1(l, 0.0);

            qreal beginHookWidth;
            qreal endHookWidth;

            if (tl->beginHookType() == HookType::HOOK_45) {
                  beginHookWidth = fabs(tl->beginHookHeight().val() * _spatium * .4);
                  pp1.rx() += beginHookWidth;
                  }
            else
                  beginHookWidth = 0;

            if (tl->endHookType() == HookType::HOOK_45) {
                  endHookWidth = fabs(tl->endHookHeight().val() * _spatium * .4);
                  pp2.rx() -= endHookWidth;
                  }
            else
                  endHookWidth = 0;

            // don't draw backwards lines (or hooks) if text is longer than nominal line length
            bool backwards = !_text->empty() && pp1.x() > pp2.x() && !tl->diagonal();

            if ((tl->beginHookType() != HookType::NONE) && (isSingleType() || isBeginType())) {
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

                  if ((tl->endHookType() != HookType::NONE) && (isSingleType() || isEndType())) {
                        ++npoints;
                        qreal hh = tl->endHookHeight().val() * _spatium;
                        // painter->drawLine(QLineF(pp2.x(), pp2.y(), pp2.x() + endHookWidth, pp2.y() + hh));
                        points[npoints] = QPointF(pp2.x() + endHookWidth, pp2.y() + hh);
                        if (tl->endHookType() == HookType::HOOK_90T)
                              points[++npoints] = QPointF(pp2.x() + endHookWidth, pp2.y() - hh);
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
      _text->spatiumChanged(ov, nv);
      _endText->spatiumChanged(ov, nv);
      }

//---------------------------------------------------------
//   pids
//---------------------------------------------------------

static constexpr std::array<P_ID, 34> pids = { {
      P_ID::LINE_WIDTH,
      P_ID::LINE_VISIBLE,
      P_ID::BEGIN_HOOK_TYPE,
      P_ID::BEGIN_HOOK_HEIGHT,
      P_ID::END_HOOK_TYPE,
      P_ID::END_HOOK_HEIGHT,
      P_ID::BEGIN_TEXT,
      P_ID::BEGIN_TEXT_ALIGN,
      P_ID::BEGIN_TEXT_PLACE,
      P_ID::BEGIN_FONT_FACE,
      P_ID::BEGIN_FONT_SIZE,
      P_ID::BEGIN_FONT_BOLD,
      P_ID::BEGIN_FONT_ITALIC,
      P_ID::BEGIN_FONT_UNDERLINE,
      P_ID::BEGIN_TEXT_OFFSET,
      P_ID::CONTINUE_TEXT,
      P_ID::CONTINUE_TEXT_ALIGN,
      P_ID::CONTINUE_TEXT_PLACE,
      P_ID::CONTINUE_FONT_FACE,
      P_ID::CONTINUE_FONT_SIZE,
      P_ID::CONTINUE_FONT_BOLD,
      P_ID::CONTINUE_FONT_ITALIC,
      P_ID::CONTINUE_FONT_UNDERLINE,
      P_ID::CONTINUE_TEXT_OFFSET,
      P_ID::END_TEXT,
      P_ID::END_TEXT_ALIGN,
      P_ID::END_TEXT_PLACE,
      P_ID::END_FONT_FACE,
      P_ID::END_FONT_SIZE,
      P_ID::END_FONT_BOLD,
      P_ID::END_FONT_ITALIC,
      P_ID::END_FONT_UNDERLINE,
      P_ID::END_TEXT_OFFSET,
      } };

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant TextLineBaseSegment::getProperty(P_ID id) const
      {
      for (P_ID pid : pids) {
            if (pid == id)
                  return textLineBase()->getProperty(id);
            }
      return LineSegment::getProperty(id);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TextLineBaseSegment::setProperty(P_ID id, const QVariant& v)
      {
      for (P_ID pid : pids) {
            if (pid == id)
                  return textLineBase()->setProperty(id, v);
            }
      return LineSegment::setProperty(id, v);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant TextLineBaseSegment::propertyDefault(P_ID id) const
      {
      for (P_ID pid : pids) {
            if (pid == id)
                  return textLineBase()->propertyDefault(id);
            }
      return LineSegment::propertyDefault(id);
      }

//---------------------------------------------------------
//   TextLineBase
//---------------------------------------------------------

TextLineBase::TextLineBase(Score* s, ElementFlags f)
   : SLine(s, f)
      {
      setBeginHookHeight(Spatium(1.9));
      setEndHookHeight(Spatium(1.9));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextLineBase::write(XmlWriter& xml) const
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

void TextLineBase::spatiumChanged(qreal /*ov*/, qreal /*nv*/)
      {
      }

//---------------------------------------------------------
//   writeProperties
//    write properties different from prototype
//---------------------------------------------------------

void TextLineBase::writeProperties(XmlWriter& xml) const
      {
      for (P_ID pid : pids)
            writeProperty(xml, pid);
      SLine::writeProperties(xml);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool TextLineBase::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());
      for (P_ID i :pids) {
            if (readProperty(tag, e, i)) {
                  setPropertyFlags(i, PropertyFlags::UNSTYLED);
                  return true;
                  }
            }
      return SLine::readProperties(e);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant TextLineBase::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::BEGIN_TEXT:
                  return beginText();
            case P_ID::BEGIN_TEXT_ALIGN:
                  return QVariant::fromValue(beginTextAlign());
            case P_ID::CONTINUE_TEXT_ALIGN:
                  return QVariant::fromValue(continueTextAlign());
            case P_ID::END_TEXT_ALIGN:
                  return QVariant::fromValue(endTextAlign());
            case P_ID::BEGIN_TEXT_PLACE:
                  return int(_beginTextPlace);
            case P_ID::BEGIN_HOOK_TYPE:
                  return int(_beginHookType);
            case P_ID::BEGIN_HOOK_HEIGHT:
                  return _beginHookHeight;
            case P_ID::BEGIN_FONT_FACE:
                  return _beginFontFamily;
            case P_ID::BEGIN_FONT_SIZE:
                  return _beginFontSize;
            case P_ID::BEGIN_FONT_BOLD:
                  return _beginFontBold;
            case P_ID::BEGIN_FONT_ITALIC:
                  return _beginFontItalic;
            case P_ID::BEGIN_FONT_UNDERLINE:
                  return _beginFontUnderline;
            case P_ID::BEGIN_TEXT_OFFSET:
                  return _beginTextOffset;
            case P_ID::CONTINUE_TEXT:
                  return continueText();
            case P_ID::CONTINUE_TEXT_PLACE:
                  return int(_continueTextPlace);
            case P_ID::CONTINUE_FONT_FACE:
                  return _continueFontFamily;
            case P_ID::CONTINUE_FONT_SIZE:
                  return _continueFontSize;
            case P_ID::CONTINUE_FONT_BOLD:
                  return _continueFontBold;
            case P_ID::CONTINUE_FONT_ITALIC:
                  return _continueFontItalic;
            case P_ID::CONTINUE_FONT_UNDERLINE:
                  return _continueFontUnderline;
            case P_ID::CONTINUE_TEXT_OFFSET:
                  return _continueTextOffset;
            case P_ID::END_TEXT:
                  return endText();
            case P_ID::END_TEXT_PLACE:
                  return int(_endTextPlace);
            case P_ID::END_HOOK_TYPE:
                  return int(_endHookType);
            case P_ID::END_HOOK_HEIGHT:
                  return _endHookHeight;
            case P_ID::END_FONT_FACE:
                  return _endFontFamily;
            case P_ID::END_FONT_SIZE:
                  return _endFontSize;
            case P_ID::END_FONT_BOLD:
                  return _endFontBold;
            case P_ID::END_FONT_ITALIC:
                  return _endFontItalic;
            case P_ID::END_FONT_UNDERLINE:
                  return _endFontUnderline;
            case P_ID::END_TEXT_OFFSET:
                  return _endTextOffset;
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
            case P_ID::BEGIN_TEXT_ALIGN:
                  _beginTextAlign = v.value<Align>();
                  break;
            case P_ID::CONTINUE_TEXT_ALIGN:
                  _continueTextAlign = v.value<Align>();
                  break;
            case P_ID::END_TEXT_ALIGN:
                  _endTextAlign = v.value<Align>();
                  break;
            case P_ID::CONTINUE_TEXT_PLACE:
                  _continueTextPlace = PlaceText(v.toInt());
                  break;
            case P_ID::END_TEXT_PLACE:
                  _endTextPlace = PlaceText(v.toInt());
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
            case P_ID::BEGIN_TEXT_OFFSET:
                  setBeginTextOffset(v.toPointF());
                  break;
            case P_ID::CONTINUE_TEXT_OFFSET:
                  setContinueTextOffset(v.toPointF());
                  break;
            case P_ID::END_TEXT_OFFSET:
                  setEndTextOffset(v.toPointF());
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
            case P_ID::BEGIN_FONT_FACE:
                  setBeginFontFamily(v.toString());
                  break;
            case P_ID::BEGIN_FONT_SIZE:
                  if (v.toReal() <= 0)
                        qFatal("font size is %f", v.toReal());
                  setBeginFontSize(v.toReal());
                  break;
            case P_ID::BEGIN_FONT_BOLD:
                  setBeginFontBold(v.toBool());
                  break;
            case P_ID::BEGIN_FONT_ITALIC:
                  setBeginFontItalic(v.toBool());
                  break;
            case P_ID::BEGIN_FONT_UNDERLINE:
                  setBeginFontUnderline(v.toBool());
                  break;
            case P_ID::CONTINUE_FONT_FACE:
                  setContinueFontFamily(v.toString());
                  break;
            case P_ID::CONTINUE_FONT_SIZE:
                  setContinueFontSize(v.toReal());
                  break;
            case P_ID::CONTINUE_FONT_BOLD:
                  setContinueFontBold(v.toBool());
                  break;
            case P_ID::CONTINUE_FONT_ITALIC:
                  setContinueFontItalic(v.toBool());
                  break;
            case P_ID::CONTINUE_FONT_UNDERLINE:
                  setContinueFontUnderline(v.toBool());
                  break;
            case P_ID::END_FONT_FACE:
                  setEndFontFamily(v.toString());
                  break;
            case P_ID::END_FONT_SIZE:
                  setEndFontSize(v.toReal());
                  break;
            case P_ID::END_FONT_BOLD:
                  setEndFontBold(v.toBool());
                  break;
            case P_ID::END_FONT_ITALIC:
                  setEndFontItalic(v.toBool());
                  break;
            case P_ID::END_FONT_UNDERLINE:
                  setEndFontUnderline(v.toBool());
                  break;

            default:
                  return SLine::setProperty(id, v);
            }
      triggerLayout();
      return true;
      }

}

