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
      { Sid::fingeringPlacement, Pid::PLACEMENT  },
      { Sid::fingeringMinDistance, Pid::MIN_DISTANCE },
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

Placement Fingering::calculatePlacement() const
      {
      Note* n = note();
      if (!n)
            return Placement::ABOVE;
      Chord* chord = n->chord();
      Staff* staff = chord->staff();
      Part* part   = staff->part();
      int nstaves  = part->nstaves();
      bool voices  = chord->measure()->hasVoices(staff->idx(), chord->tick(), chord->actualTicks());
      bool below   = voices ? !chord->up() : (nstaves > 1) && (staff->rstaff() == nstaves - 1);
      return below ? Placement::BELOW : Placement::ABOVE;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Fingering::layout()
      {
      if (parent()) {
            Fraction tick = parent()->tick();
            const Staff* st = staff();
            if (st && st->isTabStaff(tick) && !st->staffType(tick)->showTabFingering()) {
                  setbbox(QRectF());
                  return;
                  }
            }

      TextBase::layout();
      rypos() = 0.0;    // handle placement below

      if (autoplace() && note()) {
            Note* n      = note();
            Chord* chord = n->chord();
            bool voices  = chord->measure()->hasVoices(chord->staffIdx(), chord->tick(), chord->actualTicks());
            bool tight   = voices && chord->notes().size() == 1 && !chord->beam() && tid() != Tid::STRING_NUMBER;

            qreal headWidth = n->bboxRightPos();

            // update offset after drag
            qreal rebase = 0.0;
            if (offsetChanged() != OffsetChange::NONE && !tight)
                  rebase = rebaseOffset();

            // temporarily exclude self from chord shape
            setAutoplace(false);

            if (layoutType() == ElementType::CHORD) {
                  bool above = placeAbove();
                  Stem* stem = chord->stem();
                  Segment* s = chord->segment();
                  Measure* m = s->measure();
                  qreal sp = spatium();
                  qreal md = minDistance().val() * sp;
                  SysStaff* ss = m->system()->staff(chord->vStaffIdx());
                  Staff* vStaff = chord->staff();     // TODO: use current height at tick

                  if (n->mirror())
                        rxpos() -= n->ipos().x();
                  rxpos() += headWidth * .5;
                  if (above) {
                        if (tight) {
                              if (chord->stem())
                                    rxpos() -= 0.8 * sp;
                              rypos() -= 1.5 * sp;
                              }
                        else {
                              QRectF r = bbox().translated(m->pos() + s->pos() + chord->pos() + n->pos() + pos());
                              SkylineLine sk(false);
                              sk.add(r.x(), r.bottom(), r.width());
                              qreal d = sk.minDistance(ss->skyline().north());
                              qreal yd = 0.0;
                              if (d > 0.0 && isStyled(Pid::MIN_DISTANCE))
                                    yd -= d + height() * .25;
                              // force extra space above staff & chord (but not other fingerings)
                              qreal top;
                              if (chord->up() && chord->beam() && stem) {
                                    top = stem->y() + stem->bbox().top();
                                    }
                              else {
                                    Note* un = chord->upNote();
                                    top = qMin(0.0, un->y() + un->bbox().top());
                                    }
                              top -= md;
                              qreal diff = (bbox().bottom() + ipos().y() + yd + n->y()) - top;
                              if (diff > 0.0)
                                    yd -= diff;
                              if (offsetChanged() != OffsetChange::NONE) {
                                    // user moved element within the skyline
                                    // we may need to adjust minDistance, yd, and/or offset
                                    bool inStaff = above ? r.bottom() + rebase > 0.0 : r.top() + rebase < staff()->height();
                                    rebaseMinDistance(md, yd, sp, rebase, above, inStaff);
                                    }
                              rypos() += yd;
                              }
                        }
                  else {
                        if (tight) {
                              if (chord->stem())
                                    rxpos() += 0.8 * sp;
                              rypos() += 1.5 * sp;
                              }
                        else {
                              QRectF r = bbox().translated(m->pos() + s->pos() + chord->pos() + n->pos() + pos());
                              SkylineLine sk(true);
                              sk.add(r.x(), r.top(), r.width());
                              qreal d = ss->skyline().south().minDistance(sk);
                              qreal yd = 0.0;
                              if (d > 0.0 && isStyled(Pid::MIN_DISTANCE))
                                    yd += d + height() * .25;
                              // force extra space below staff & chord (but not other fingerings)
                              qreal bottom;
                              if (!chord->up() && chord->beam() && stem) {
                                    bottom = stem->y() + stem->bbox().bottom();
                                    }
                              else {
                                    Note* dn = chord->downNote();
                                    bottom = qMax(vStaff->height(), dn->y() + dn->bbox().bottom());
                                    }
                              bottom += md;
                              qreal diff = bottom - (bbox().top() + ipos().y() + yd + n->y());
                              if (diff > 0.0)
                                    yd += diff;
                              if (offsetChanged() != OffsetChange::NONE) {
                                    // user moved element within the skyline
                                    // we may need to adjust minDistance, yd, and/or offset
                                    bool inStaff = above ? r.bottom() + rebase > 0.0 : r.top() + rebase < staff()->height();
                                    rebaseMinDistance(md, yd, sp, rebase, above, inStaff);
                                    }
                              rypos() += yd;
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
      else if (offsetChanged() != OffsetChange::NONE) {
            // rebase horizontally too, as autoplace may have adjusted it
            rebaseOffset(false);
            }
      setOffsetChanged(false);
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
      return QString("%1: %2").arg(rez, plainText());
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Fingering::propertyDefault(Pid id) const
      {
      switch (id) {
            case Pid::PLACEMENT:
                  return int(calculatePlacement());
            case Pid::SUB_STYLE:
                  return int(Tid::FINGERING);
            default:
                  return TextBase::propertyDefault(id);
            }
      }

}

