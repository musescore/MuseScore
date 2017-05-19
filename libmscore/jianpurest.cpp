//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "jianpurest.h"
#include "jianpunote.h"
#include "score.h"
#include "xml.h"
#include "style.h"
#include "utils.h"
#include "tuplet.h"
#include "sym.h"
#include "stafftext.h"
#include "articulation.h"
#include "chord.h"
#include "note.h"
#include "measure.h"
#include "undo.h"
#include "staff.h"
#include "harmony.h"
#include "segment.h"
#include "stafftype.h"

namespace Ms {

JianpuRest::JianpuRest(Score* s)
  : Rest(s)
      {
      _durationDashCount = 0;
      _durationBeams.clear();
      }

JianpuRest::JianpuRest(Score* s, const TDuration& d)
  : Rest(s, d)
      {
      if (d.type() == TDuration::DurationType::V_WHOLE)
            _durationDashCount = 3;
      else if (d.type() == TDuration::DurationType::V_HALF)
            _durationDashCount = 1;
      else
            _durationDashCount = 0;
      }

JianpuRest::JianpuRest(const Rest& r, bool link)
   : Rest(r, link)
      {
      _durationDashCount = 0;
      _durationBeams.clear();
      }

JianpuRest::JianpuRest(const JianpuRest& r, bool link)
   : Rest(r, link)
      {
      _durationDashCount = r._durationDashCount;
      _durationBeams = r._durationBeams;
      }

JianpuRest::~JianpuRest()
      {
      for (auto ptr : _durationBeams)
            delete ptr;
      }

void JianpuRest::read(XmlReader& xml)
      {
      while (xml.readNextStartElement()) {
            const QStringRef& tag(xml.name());
            if (!ChordRest::readProperties(xml))
                  xml.unknown();
            }
      }

void JianpuRest::write(XmlWriter& xml) const
      {
      if (_gap)
            return;
      xml.stag(name());
      ChordRest::writeProperties(xml);
      el().write(xml);
      xml.etag();
      }

void JianpuRest::layout()
      {
      if (isGap())
            return;

      // Set the rest's position.
      // Jianpu bar-line span: -4 to +4
      qreal x = 0.0;
      qreal y = JianpuNote::NOTE_BASELINE * spatium() * 0.5;
      setPos(x, y);

      // Get rest's font metrics.
      StaffType* st = staff()->staffType(tick());
      QFontMetricsF fm(st->jianpuNoteFont(), MScore::paintDevice());
      QRectF rect = fm.tightBoundingRect("0");
      // Font bounding rectangle height is too large; make it smaller.
      rect.setRect(0, 0, rect.width(), rect.height() * JianpuNote::FONT_BBOX_HEIGHT_RATIO);

      // Update bounding box.
      setbbox(rect);

      // Update duration dash count.
      // TODO: Add checking for time signature, i.e., 4/4, 3/4, 2/4, etc.
      const TDuration duration = durationType();
      if (duration.type() == TDuration::DurationType::V_WHOLE)
            _durationDashCount = 3;
      else if (duration.type() == TDuration::DurationType::V_HALF)
            _durationDashCount = 1;
      else
            _durationDashCount = 0;

      // Lay out the duration beams, as necessary, underneath the rest number.
      int beamCount = durationType().hooks();
      if (beamCount > 0) {
            qreal beamDistance = JianpuNote::BEAM_HEIGHT + JianpuNote::BEAM_Y_SPACE;
            qreal x1 = pos().x();
            qreal x2 = x1 + bbox().width();
            // Add octave-dot box y-offset to align rest beams with note beams.
            y = pos().y() + bbox().height() +
                        JianpuNote::OCTAVE_DOTBOX_Y_OFFSET + JianpuNote::OCTAVE_DOTBOX_HEIGHT;
            for (int i = 0; i < beamCount; i++) {
                  _durationBeams.push_back(new QLineF(x1, y, x2, y));
                  y += beamDistance;
                  }
            }
      //qDebug("bbox x=%.0f y=%.0f w=%.0f h=%.0f", bbox().x(), bbox().y(), bbox().width(), bbox().height());
      //Q_ASSERT(bbox().x() < 20000 && bbox().y() < 20000);
      //Q_ASSERT(bbox().width() < 20000 && bbox().height() < 20000);
      }

void JianpuRest::draw(QPainter* painter) const
      {
      // Draw the rest number "0".
      StaffType* st = staff()->staffType(tick());
      QFont f(st->jianpuNoteFont());
      f.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
      painter->setFont(f);
      painter->setPen(QColor(curColor()));
      // We take bounding box y-position as top of the note number.
      // But function "drawText" takes y-position as baseline of the font, which is the near bottom of note number.
      // So adjust y-position for "drawText" to the bottom of the bounding box.
      painter->drawText(QPointF(pos().x() + bbox().x(),
                                pos().y() + bbox().y() + bbox().height()), "0");

      // Prepare painter/brush for duration dash/beam drawing.
      QBrush brush(curColor(), Qt::SolidPattern);
      painter->setBrush(brush);
      painter->setPen(Qt::NoPen);

      // Draw duration dashes for whole and half rests.
      qreal x = pos().x() + bbox().width() + JianpuNote::DURATION_DASH_X_SPACE;
      if (_durationDashCount > 0) {
            // TODO: calculate dash/space widths based on available space of the measure.
            qreal space = JianpuNote::DURATION_DASH_X_SPACE;
            qreal width = JianpuNote::DURATION_DASH_WIDTH;
            qreal height = JianpuNote::DURATION_DASH_HEIGHT;
            qreal y = JianpuNote::NOTE_BASELINE * spatium() * 0.5;
            y += (bbox().height() - height) / 2;
            QRectF dash(x, y, width, height);
            for (int i = 0; i < _durationDashCount; i++) {
                  painter->fillRect(dash, brush);
                  x += width + space ;
                  dash.moveLeft(x); // Move rect's left edge to next dash position.
                  }
            }

      // Draw duration dots from x-position where drawing of duration dashes left off.
      int dotCount = durationType().dots();
      if (dotCount > 0) {
            qreal space = JianpuNote::DURATION_DOT_X_SPACE;
            qreal width = JianpuNote::DURATION_DOT_WIDTH;
            qreal height = JianpuNote::DURATION_DOT_HEIGHT;
            x += JianpuNote::DURATION_DOT_X_OFFSET;
            qreal y = JianpuNote::NOTE_BASELINE * spatium() * 0.5;
            y += (bbox().height() - height) / 2;
            QRectF dot(x, y, width, height);
            for (int i = 0; i < dotCount; i++) {
                  painter->drawEllipse(dot);
                  x += width + space ;
                  dot.moveLeft(x); // Move rect's left edge to next dash position.
                  }
            }

      // Draw beam(s) underneath the rests with shorter durations.
      if (!_durationBeams.empty()) {
            qreal height = JianpuNote::BEAM_HEIGHT;
            for (const QLineF* line : _durationBeams) {
                  QRectF beam(line->x1(), line->y1(), line->length(), height);
                  painter->fillRect(beam, brush);
                  }
            }
      }

} // namespace Ms
