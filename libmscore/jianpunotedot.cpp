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

#include "jianpunotedot.h"
#include "jianpunote.h"
#include "score.h"
#include "staff.h"
#include "sym.h"
#include "xml.h"
#include "chord.h"

namespace Ms {

JianpuNoteDot::JianpuNoteDot(Score* score)
   : NoteDot(score)
      {
      }

JianpuNoteDot::JianpuNoteDot(const NoteDot& noteDot, bool /* link */)
   : NoteDot(noteDot)
      {
      }

JianpuNoteDot::JianpuNoteDot(const JianpuNoteDot& noteDot, bool /* link */)
   : NoteDot(noteDot)
      {
      }

JianpuNoteDot::~JianpuNoteDot()
      {
      }

void JianpuNoteDot::layout()
      {
      QRectF dotBox(0, 0, JianpuNote::DURATION_DOT_WIDTH, JianpuNote::DURATION_DOT_HEIGHT);
      setbbox(dotBox);
      }

void JianpuNoteDot::draw(QPainter* painter) const
      {
      QBrush brush(curColor(), Qt::SolidPattern);
      painter->setBrush(brush);
      painter->setPen(Qt::NoPen);
      JianpuNote* jn = dynamic_cast<JianpuNote*>(note());   // parent note
      Q_ASSERT(jn != nullptr);
      const QRectF& noteBox = jn->noteNumberBox();
      const QRectF& dashBox = jn->durationDashBox();
      QRectF dot(noteBox.x() + noteBox.width() + dashBox.width() + JianpuNote::DURATION_DOT_X_OFFSET,
                 noteBox.y() + (noteBox.height() - bbox().height()) / 2,
                 bbox().width(), bbox().height());
      painter->drawEllipse(dot);
      qDebug("noteBox x=%.0f y=%.0f w=%.0f h=%.0f", noteBox.x(), noteBox.y(), noteBox.width(), noteBox.height());
      qDebug("dashBox x=%.0f y=%.0f w=%.0f h=%.0f", dashBox.x(), dashBox.y(), dashBox.width(), dashBox.height());
      qDebug("dot x=%.0f y=%.0f w=%.0f h=%.0f", dot.x(), dot.y(), dot.width(), dot.height());
      }

} // namespace Ms

