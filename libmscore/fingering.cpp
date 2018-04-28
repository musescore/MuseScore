//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "fingering.h"
#include "score.h"
#include "staff.h"
#include "undo.h"
#include "xml.h"
#include "chord.h"
#include "part.h"
#include "measure.h"
#include "stem.h"

namespace Ms {


//---------------------------------------------------------
//   Fingering
//      Element(Score* = 0, ElementFlags = ElementFlag::NOTHING);
//---------------------------------------------------------

Fingering::Fingering(SubStyleId ssid, Score* s, ElementFlag ef)
   : TextBase(s, ef)
      {
      initSubStyle(ssid);
      }

Fingering::Fingering(Score* s)
  : Fingering(SubStyleId::FINGERING, s, ElementFlag::HAS_TAG)
      {
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Fingering::layout()
      {
      TextBase::layout();

      if (autoplace() && note()) {
            Chord* chord = note()->chord();
            Staff* staff = chord->staff();
            Part* part   = staff->part();
            int n        = part->nstaves();
            bool voices  = chord->measure()->hasVoices(staff->idx());
            bool below   = voices ? !chord->up() : (n > 1) && (staff->rstaff() == n-1);
            bool tight   = voices && !chord->beam();

            qreal x = 0.0;
            qreal y = 0.0;
            qreal headWidth = note()->bboxRightPos();
            qreal headHeight = note()->headHeight();
            qreal fh = headHeight;        // TODO: fingering number height

            if (chord->notes().size() == 1) {
                  x = headWidth * .5;
                  if (below) {
                        // place fingering below note
                        y = fh + spatium() * .4;
                        if (tight) {
                              y += 0.5 * spatium();
                              if (chord->stem())
                                    x += 0.5 * spatium();
                              }
                        else if (chord->stem() && !chord->up()) {
                              // on stem side
                              y += chord->stem()->height();
                              x -= spatium() * .4;
                              }
                        }
                  else {
                        // place fingering above note
                        y = -headHeight - spatium() * .4;
                        if (tight) {
                              y -= 0.5 * spatium();
                              if (chord->stem())
                                    x -= 0.5 * spatium();
                              }
                        else if (chord->stem() && chord->up()) {
                              // on stem side
                              y -= chord->stem()->height();
                              x += spatium() * .4;
                              }
                        }
                  }
            else {
                  x -= spatium();
                  }
            setUserOff(QPointF(x, y));
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Fingering::draw(QPainter* painter) const
      {
      TextBase::draw(painter);
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Fingering::accessibleInfo() const
      {
      QString rez = Element::accessibleInfo();
      if (subStyleId() == SubStyleId::STRING_NUMBER)
            rez += " " + QObject::tr("String number");
      return QString("%1: %2").arg(rez).arg(plainText());
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Fingering::propertyDefault(Pid id) const
      {
      switch (id) {
            case Pid::SUB_STYLE:
                  return int(SubStyleId::FINGERING);
            default:
                  return TextBase::propertyDefault(id);
            }
      }

}

