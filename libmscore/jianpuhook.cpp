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

#include "jianpuhook.h"
#include "jianpunote.h"
#include "chord.h"
#include "note.h"


namespace Ms {

JianpuHook::JianpuHook(Score* s)
  : Hook(s)
      {
      }

JianpuHook::JianpuHook(const Hook& h)
  : Hook(h)
      {
      }

JianpuHook::~JianpuHook()
      {
      for (auto ptr : _durationBeams)
            delete ptr;
      }

void JianpuHook::layout()
      {
      // Calculate and set hook's position.
      // Jianpu bar-line span: -4 to +4
      qreal x = 0.0;
      qreal y = JianpuNote::NOTE_BASELINE * spatium() * 0.5;
      setPos(x, y);
      setbbox(QRectF());

      // Lay out the duration beams underneath the note number.
      int beamCount = chord()->durationType().hooks();
      if (beamCount > 0) {
            qreal beamDistance = JianpuNote::BEAM_HEIGHT + JianpuNote::BEAM_Y_SPACE;
            const Note* note = chord()->notes().at(0);
            qreal x1 = note->pos().x();
            qreal x2 = x1 + note->bbox().width();
            y = note->pos().y() + note->bbox().height();
            const JianpuNote* jn = dynamic_cast<const JianpuNote*>(note);
            if (jn && jn->noteOctave() >= 0) {
                  // Note's bounding box does not include space for lower-octave dots.
                  // Add octave-dot box y-offset to align with beams of other notes.
                  y += JianpuNote::OCTAVE_DOTBOX_Y_OFFSET + JianpuNote::OCTAVE_DOTBOX_HEIGHT;
                  }
            for (int i = 0; i < beamCount; i++) {
                  _durationBeams.push_back(new QLineF(x1, y, x2, y));
                  addbbox(QRectF(x1, y, x2 - x1, beamDistance));
                  y += beamDistance;
                  }
            }
      //qDebug("bbox x=%.0f y=%.0f w=%.0f h=%.0f", bbox().x(), bbox().y(), bbox().width(), bbox().height());
      //Q_ASSERT(bbox().x() < 20000 && bbox().y() < 20000);
      //Q_ASSERT(bbox().width() < 20000 && bbox().height() < 20000);
      }

void JianpuHook::draw(QPainter* painter) const
      {
      // Draw beam(s) underneath the note number.
      if (!_durationBeams.empty()) {
            QBrush brush(curColor(), Qt::SolidPattern);
            painter->setBrush(brush);
            painter->setPen(Qt::NoPen);
            qreal height = JianpuNote::BEAM_HEIGHT;
            for (const QLineF* line : _durationBeams) {
                  QRectF beam(line->x1(), line->y1(), line->length(), height);
                  painter->fillRect(beam, brush);
                  }
            }
      }

} // namespace Ms
