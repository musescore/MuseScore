//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "score.h"
#include "measurenumber.h"
// #include "xml.h"
#include "measure.h"
#include "staff.h"

namespace Ms {

//---------------------------------------------------------
//   measureNumberStyle
//---------------------------------------------------------

static const ElementStyle measureNumberStyle {
      { Sid::measureNumberVPlacement, Pid::PLACEMENT },
      { Sid::measureNumberHPlacement, Pid::HPLACEMENT },
      };

//---------------------------------------------------------
//   MeasureNumber
//---------------------------------------------------------

MeasureNumber::MeasureNumber(Score* s, Tid tid, ElementFlags flags)
      : TextBase(s, tid, flags)
      {
      setFlag(ElementFlag::ON_STAFF, true);
      initElementStyle(&measureNumberStyle);

      setHPlacement(score()->styleV(Sid::measureNumberHPlacement).value<HPlacement>());
      }

//---------------------------------------------------------
//   MeasureNumber
//     Copy constructor
//---------------------------------------------------------

MeasureNumber::MeasureNumber(const MeasureNumber& other): TextBase(other)
      {
      setFlag(ElementFlag::ON_STAFF, true);
      initElementStyle(&measureNumberStyle);

      setHPlacement(other.hPlacement());
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant MeasureNumber::getProperty(Pid id) const
      {
      switch (id) {
            case Pid::HPLACEMENT:
                  return int(hPlacement());
            default:
                  return TextBase::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool MeasureNumber::setProperty(Pid id, const QVariant& val)
      {
      switch (id) {
            case Pid::HPLACEMENT:
                  setHPlacement(HPlacement(val.toInt()));
                  setLayoutInvalid();
                  triggerLayout();
                  return true;
            default:
                  return TextBase::setProperty(id, val);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant MeasureNumber::propertyDefault(Pid id) const
      {
      switch(id) {
            case Pid::SUB_STYLE:
                  return int(Tid::MEASURE_NUMBER);
            case Pid::PLACEMENT:
                  return score()->styleV(Sid::measureNumberVPlacement);
            case Pid::HPLACEMENT:
                  return score()->styleV(Sid::measureNumberHPlacement);
            default:
                  return TextBase::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool MeasureNumber::readProperties(XmlReader& xml)
      {
      if (readProperty(xml.name(), xml, Pid::HPLACEMENT))
            return true;
      else
            return TextBase::readProperties(xml);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void MeasureNumber::layout()
      {
      setPos(QPointF());
      if (!parent())
            setOffset(0.0, 0.0);

      // TextBase::layout1() needs to be called even if there's no measure attached to it.
      // This happens for example in the palettes.
      TextBase::layout1();
      // this could be if (!measure()) but it is the same as current and slower
      // See implementation of MeasureNumber::measure().
      if (!parent())
            return;

      if (placeBelow()) {
            qreal yoff = bbox().height();

            // If there is only one line, the barline spans outside the staff lines, so the default position is not correct.
            if (staff()->constStaffType(measure()->tick())->lines() == 1)
                  yoff += 2.0 * spatium();
            else
                  yoff += staff()->height();

            rypos() = yoff;
            }
      else {
            qreal yoff = 0.0;

            // If there is only one line, the barline spans outside the staff lines, so the default position is not correct.
            if (staff()->constStaffType(measure()->tick())->lines() == 1)
                  yoff -= 2.0 * spatium();

            rypos() = yoff;
            }

      if (hPlacement() == HPlacement::CENTER) {
            // measure numbers should be centered over where there can be notes.
            // This means that header and trailing segments should be ignored,
            // which includes all timesigs, clefs, keysigs, etc.
            // This is how it should be centered:
            // |bb 4/4 notes-chords #| other measure |
            // |      ------18------ | other measure |

            //    x1 - left measure position of free space
            //    x2 - right measure position of free space

            const Measure* mea = measure();

            // find first chordrest
            Segment* chordRest = mea->first(SegmentType::ChordRest);

            Segment* s1 = chordRest->prevActive();
            // unfortunately, using !s1->header() does not work
            while (s1 && (s1->isChordRestType()
                          || s1->isBreathType()
                          || s1->isClefType()
                          || s1->isBarLineType()
                          || !s1->element(staffIdx() * VOICES)))
                  s1 = s1->prevActive();

            Segment* s2 = chordRest->next();
            // unfortunately, using !s1->trailer() does not work
            while (s2 && (s2->isChordRestType()
                          || s2->isBreathType()
                          || s2->isClefType()
                          || s2->isBarLineType()
                          || !s2->element(staffIdx() * VOICES)))
                  s2 = s2->nextActive();

            // if s1/s2 does not exist, it means there is no header/trailer segment. Align with start/end of measure.
            qreal x1 = s1 ? s1->x() + s1->minRight() : 0;
            qreal x2 = s2 ? s2->x() - s2->minLeft() : mea->width();

            rxpos() = (x1 + x2) * 0.5;
            }
      else if (hPlacement() == HPlacement::RIGHT)
            rxpos() = measure()->width();
      }

} // namespace MS
