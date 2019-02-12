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
#include "skyline.h"
#include "system.h"

namespace Ms {

//---------------------------------------------------------
//   fingeringStyle
//---------------------------------------------------------

static const ElementStyle fingeringStyle {
      };

//---------------------------------------------------------
//   Fingering
//      Element(Score* = 0, ElementFlags = ElementFlag::NOTHING);
//---------------------------------------------------------

Fingering::Fingering(Score* s, Tid tid, ElementFlags ef)
   : TextBase(s, tid, ef)
      {
      setPlacement(Placement::ABOVE);
      initElementStyle(&fingeringStyle);
      }

Fingering::Fingering(Score* s, ElementFlags ef)
  : Fingering(s, Tid::FINGERING, ef)
      {
      }

//---------------------------------------------------------
//   layoutType
//---------------------------------------------------------

ElementType Fingering::layoutType()
      {
      switch (tid()) {
            case Tid::FINGERING:
            case Tid::RH_GUITAR_FINGERING:
            case Tid::STRING_NUMBER:
                  return ElementType::CHORD;
            default:
                  return ElementType::NOTE;
            }
      }

//---------------------------------------------------------
//   calculatePlacement
//---------------------------------------------------------

void Fingering::calculatePlacement()
      {
      Note* n = note();
      if (!n)
            return;
      Chord* chord = n->chord();
      Staff* staff = chord->staff();
      Part* part   = staff->part();
      int nstaves  = part->nstaves();
      bool voices  = chord->measure()->hasVoices(staff->idx());
      bool below   = voices ? !chord->up() : (nstaves > 1) && (staff->rstaff() == nstaves - 1);
      setPlacement(below ? Placement::BELOW : Placement::ABOVE);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Fingering::layout()
      {
      if (parent()) {
            const int tick = parent()->tick();
            const Staff* st = staff();
            if (st && st->isTabStaff(tick)
               && !st->staffType(tick)->showTabFingering()) {
                  setbbox(QRectF());
                  return;
                  }
            }

      TextBase::layout();
      rypos() = 0.0;    // handle placement below

      if (autoplace() && note()) {
            Note* n      = note();
            Chord* chord = n->chord();
            bool voices  = chord->measure()->hasVoices(chord->staffIdx());
            bool tight   = voices && chord->notes().size() == 1 && !chord->beam() && tid() != Tid::STRING_NUMBER;

            qreal headWidth = n->bboxRightPos();

            // temporarily exclude self from chord shape
            setAutoplace(false);

            if (layoutType() == ElementType::CHORD) {
                  Stem* stem = chord->stem();
                  Segment* s = chord->segment();
                  Measure* m = s->measure();
                  SysStaff* ss = m->system()->staff(chord->vStaffIdx());
                  Staff* vStaff = chord->staff();     // TODO: use current height at tick
                  if (n->mirror())
                        rxpos() -= n->ipos().x();
                  rxpos() += headWidth * .5;
                  if (placeAbove()) {
                        if (tight) {
                              if (chord->stem())
                                    rxpos() -= 0.8 * spatium();
                              rypos() -= spatium() * 1.5;
                              }
                        else {
                              QRectF r = bbox().translated(m->pos() + s->pos() + chord->pos() + n->pos() + pos());
                              SkylineLine sk(false);
                              sk.add(r.x(), r.bottom(), r.width());
                              qreal d = sk.minDistance(ss->skyline().north());
                              if (d > 0.0)
                                    rypos() -= d + height() * .25;
                              // force extra space above staff & chord (but not other fingerings)
                              qreal top;
                              if (chord->up() && chord->beam() && stem) {
                                    top = stem->y() + stem->bbox().top();
                                    }
                              else {
                                    Note* un = chord->upNote();
                                    top = qMin(0.0, un->y() + un->bbox().top());
                                    }
                              top -= spatium() * 0.5;
                              qreal diff = (bbox().bottom() + ipos().y() + n->y()) - top;
                              if (diff > 0.0)
                                    rypos() -= diff;
                              }
                        }
                  else {
                        if (tight) {
                              if (chord->stem())
                                    rxpos() += 0.8 * spatium();
                              rypos() += spatium() * 1.5;
                              }
                        else {
                              QRectF r = bbox().translated(m->pos() + s->pos() + chord->pos() + n->pos() + pos());
                              SkylineLine sk(true);
                              sk.add(r.x(), r.top(), r.width());
                              qreal d = ss->skyline().south().minDistance(sk);
                              if (d > 0.0)
                                    rypos() += d + height() * .25;
                              // force extra space below staff & chord (but not other fingerings)
                              qreal bottom;
                              if (!chord->up() && chord->beam() && stem) {
                                    bottom = stem->y() + stem->bbox().bottom();
                                    }
                              else {
                                    Note* dn = chord->downNote();
                                    bottom = qMax(vStaff->height(), dn->y() + dn->bbox().bottom());
                                    }
                              bottom += spatium() * 0.5;
                              qreal diff = bottom - (bbox().top() + ipos().y() + n->y());
                              if (diff > 0.0)
                                    rypos() += diff;
                              }
                        }
                  }
            else if (tid() == Tid::LH_GUITAR_FINGERING) {
                  // place to left of note
                  qreal left = n->shape().left();
                  if (left - n->x() > 0.0)
                        rxpos() -= left;
                  else
                        rxpos() -= n->x();
                  }
            // for other fingering styles, do not autoplace

            // restore autoplace
            setAutoplace(true);
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
      if (tid() == Tid::STRING_NUMBER)
            rez += " " + QObject::tr("String number");
      return QString("%1: %2").arg(rez).arg(plainText());
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Fingering::propertyDefault(Pid id) const
      {
      switch (id) {
            case Pid::PLACEMENT:
                  return int(Placement::ABOVE);
            case Pid::SUB_STYLE:
                  return int(Tid::FINGERING);
            default:
                  return TextBase::propertyDefault(id);
            }
      }

}

