//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "bend.h"
#include "score.h"
#include "undo.h"
#include "staff.h"
#include "chord.h"
#include "note.h"
#include "xml.h"

namespace Ms {

constexpr std::array<StyledProperty,6> Bend::_styledProperties;

//---------------------------------------------------------
//   label
//---------------------------------------------------------

static const char* label[] = {
      "", "1/4", "1/2", "3/4", "full",
      "1 1/4", "1 1/2", "1 3/4", "2",
      "2 1/4", "2 1/2", "2 3/4", "3"
      };

//---------------------------------------------------------
//   Bend
//---------------------------------------------------------

Bend::Bend(Score* s)
   : Element(s)
      {
      static constexpr std::array<P_ID,5> properties {
            P_ID::FONT_FACE,
            P_ID::FONT_SIZE,
            P_ID::FONT_BOLD,
            P_ID::FONT_ITALIC,
            P_ID::FONT_UNDERLINE,
            };

      for (P_ID id : properties)
            resetProperty(id);

      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE);
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont Bend::font(qreal sp) const
      {
      QFont f(_fontFace);
      f.setBold(_fontBold);
      f.setItalic(_fontItalic);
      f.setUnderline(_fontUnderline);
      qreal m = _fontSize;
      m *= sp / SPATIUM20;

      f.setPointSizeF(m);
      return f;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Bend::layout()
      {
      // during mtest, there may be no score. If so, exit.
      if (!score())
            return;

      qreal _spatium = spatium();

      if (staff() && !staff()->isTabStaff(tick())) {
            setbbox(QRectF());
            if (!parent()) {
                  noteWidth = -_spatium*2;
                  notePos   = QPointF(0.0, _spatium*3);
                  }
            }

      _lw        = _spatium * 0.15;
      Note* note = toNote(parent());
      if (note == 0) {
            noteWidth = 0.0;
            notePos = QPointF();
            }
      else {
            notePos   = note->pos();
            noteWidth = note->width();
            }
      QRectF bb;

      QFontMetricsF fm(font(_spatium));

      int n   = _points.size();
      qreal x = noteWidth;
      qreal y = -_spatium * .8;
      qreal x2, y2;

      qreal aw = _spatium * .5;
      QPolygonF arrowUp;
      arrowUp << QPointF(0, 0) << QPointF(aw*.5, aw) << QPointF(-aw*.5, aw);
      QPolygonF arrowDown;
      arrowDown << QPointF(0, 0) << QPointF(aw*.5, -aw) << QPointF(-aw*.5, -aw);

      for (int pt = 0; pt < n; ++pt) {
            if (pt == (n-1))
                  break;
            int pitch = _points[pt].pitch;
            if (pt == 0 && pitch) {
                  y2 = -notePos.y() -_spatium * 2;
                  x2 = x;
                  bb |= QRectF(x, y, x2-x, y2-y);

                  bb |= arrowUp.translated(x2, y2 + _spatium * .2).boundingRect();

                  int idx = (pitch + 12)/25;
                  const char* l = label[idx];
                  bb |= fm.boundingRect(QRectF(x2, y2, 0, 0),
                     Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, QString(l));
                  y = y2;
                  }
            if (pitch == _points[pt+1].pitch) {
                  if (pt == (n-2))
                        break;
                  x2 = x + _spatium;
                  y2 = y;
                  bb |= QRectF(x, y, x2-x, y2-y);
                  }
            else if (pitch < _points[pt+1].pitch) {
                  // up
                  x2 = x + _spatium*.5;
                  y2 = -notePos.y() -_spatium * 2;
                  qreal dx = x2 - x;
                  qreal dy = y2 - y;

                  QPainterPath path;
                  path.moveTo(x, y);
                  path.cubicTo(x+dx/2, y, x2, y+dy/4, x2, y2);
                  bb |= path.boundingRect();

                  bb |= arrowUp.translated(x2, y2 + _spatium * .2).boundingRect();

                  int idx = (_points[pt+1].pitch + 12)/25;
                  const char* l = label[idx];
                  QRectF r;
                  bb |= fm.boundingRect(QRectF(x2, y2, 0, 0),
                     Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, QString(l));
                  }
            else {
                  // down
                  x2 = x + _spatium*.5;
                  y2 = y + _spatium * 3;
                  qreal dx = x2 - x;
                  qreal dy = y2 - y;

                  QPainterPath path;
                  path.moveTo(x, y);
                  path.cubicTo(x+dx/2, y, x2, y+dy/4, x2, y2);
                  bb |= path.boundingRect();

                  bb |= arrowDown.translated(x2, y2 - _spatium * .2).boundingRect();
                  }
            x = x2;
            y = y2;
            }
      bb.adjust(-_lw, -_lw, _lw, _lw);
      setbbox(bb);
      setPos(0.0, 0.0);
      adjustReadPos();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Bend::draw(QPainter* painter) const
      {
      QPen pen(curColor(), _lw, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
      painter->setPen(pen);
      painter->setBrush(QBrush(curColor()));

      qreal _spatium = spatium();
      QFont f = font(_spatium * MScore::pixelRatio);
      painter->setFont(f);

      int n    = _points.size();
      qreal x  = noteWidth;
      qreal y  = -_spatium * .8;
      qreal x2, y2;

      qreal aw = _spatium * .5;
      QPolygonF arrowUp;
      arrowUp << QPointF(0, 0) << QPointF(aw*.5, aw) << QPointF(-aw*.5, aw);
      QPolygonF arrowDown;
      arrowDown << QPointF(0, 0) << QPointF(aw*.5, -aw) << QPointF(-aw*.5, -aw);
      QFontMetrics fm(f);
      for (int pt = 0; pt < n; ++pt) {
            if (pt == (n-1))
                  break;
            int pitch = _points[pt].pitch;
            if (pt == 0 && pitch) {
                  y2 = -notePos.y() -_spatium * 2;
                  x2 = x;
                  painter->drawLine(QLineF(x, y, x2, y2));

                  painter->setBrush(curColor());
                  painter->drawPolygon(arrowUp.translated(x2, y2));

                  int idx = (pitch + 12)/25;
                  const char* l = label[idx];
                  QString s(l);
                  qreal textWidth = fm.width(s);
                  qreal textHeight = fm.height();
                  painter->drawText(QRectF(x2 - textWidth / 2, y2 - textHeight / 2, .0, .0), Qt::AlignVCenter|Qt::TextDontClip, s);

                  y = y2;
                  }
            if (pitch == _points[pt+1].pitch) {
                  if (pt == (n-2))
                        break;
                  x2 = x + _spatium;
                  y2 = y;
                  painter->drawLine(QLineF(x, y, x2, y2));
                  }
            else if (pitch < _points[pt+1].pitch) {
                  // up
                  x2 = x + _spatium*.5;
                  y2 = -notePos.y() -_spatium * 2;
                  qreal dx = x2 - x;
                  qreal dy = y2 - y;

                  QPainterPath path;
                  path.moveTo(x, y);
                  path.cubicTo(x+dx/2, y, x2, y+dy/4, x2, y2);
                  painter->setBrush(Qt::NoBrush);
                  painter->drawPath(path);

                  painter->setBrush(curColor());
                  painter->drawPolygon(arrowUp.translated(x2, y2 ));

                  int idx = (_points[pt+1].pitch + 12)/25;
                  const char* l = label[idx];
                  qreal ty = y2; // - _spatium;
                  painter->drawText(QRectF(x2, ty, .0, .0),
                     Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, QString(l));
                  }
            else {
                  // down
                  x2 = x + _spatium*.5;
                  y2 = y + _spatium * 3;
                  qreal dx = x2 - x;
                  qreal dy = y2 - y;

                  QPainterPath path;
                  path.moveTo(x, y);
                  path.cubicTo(x+dx/2, y, x2, y+dy/4, x2, y2);
                  painter->setBrush(Qt::NoBrush);
                  painter->drawPath(path);

                  painter->setBrush(curColor());
                  painter->drawPolygon(arrowDown.translated(x2, y2));
                  }
            x = x2;
            y = y2;
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Bend::write(XmlWriter& xml) const
      {
      xml.stag("Bend");
      for (const PitchValue& v : _points) {
            xml.tagE(QString("point time=\"%1\" pitch=\"%2\" vibrato=\"%3\"")
               .arg(v.time).arg(v.pitch).arg(v.vibrato));
            }
      for (auto k : _styledProperties)
            writeProperty(xml, k.propertyIdx);
      writeProperty(xml, P_ID::PLAY);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Bend::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (readStyledProperty(e, tag))
                  ;
            else if (tag == "point") {
                  PitchValue pv;
                  pv.time    = e.intAttribute("time");
                  pv.pitch   = e.intAttribute("pitch");
                  pv.vibrato = e.intAttribute("vibrato");
                  _points.append(pv);
                  e.readNext();
                  }
            else if (tag == "play")
                  setPlayBend(e.readBool());
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Bend::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::FONT_FACE:
                  return _fontFace;
            case P_ID::FONT_SIZE:
                  return _fontSize;
            case P_ID::FONT_BOLD:
                  return _fontBold;
            case P_ID::FONT_ITALIC:
                  return _fontItalic;
            case P_ID::FONT_UNDERLINE:
                  return _fontUnderline;
            case P_ID::PLAY:
                  return bool(playBend());
            default:
                  return Element::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Bend::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_ID::FONT_FACE:
                  _fontFace = v.toString();
                  break;
            case P_ID::FONT_SIZE:
                  _fontSize = v.toReal();
                  break;
            case P_ID::FONT_BOLD:
                  _fontBold = v.toBool();
                  break;
            case P_ID::FONT_ITALIC:
                  _fontItalic = v.toBool();
                  break;
            case P_ID::FONT_UNDERLINE:
                  _fontUnderline = v.toBool();
                  break;
            case P_ID::PLAY:
                 setPlayBend(v.toBool());
                 break;
            default:
                  return Element::setProperty(id, v);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Bend::propertyDefault(P_ID id) const
      {
      for (auto k : _styledProperties) {
            if (k.propertyIdx == id)
                  return score()->styleV(k.styleIdx);
            }
      switch (id) {
            case P_ID::PLAY:
                  return true;
            default:
                  return Element::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Bend::reset()
      {
      for (auto k : _styledProperties)
            undoResetProperty(k.propertyIdx);
      Element::reset();
      }

}

