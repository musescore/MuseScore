//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "elements.h"
#include "libmscore/property.h"

namespace Ms {
namespace PluginAPI {

//---------------------------------------------------------
//   Element::setOffsetX
//---------------------------------------------------------

void Element::setOffsetX(qreal offX)
      {
      offX *= element()->spatium();
      QPointF off(element()->offset());
      off.rx() += offX;
      set(Ms::Pid::OFFSET, off);
      }

//---------------------------------------------------------
//   Element::setOffsetY
//---------------------------------------------------------

void Element::setOffsetY(qreal offY)
      {
      offY *= element()->spatium();
      QPointF off(element()->offset());
      off.ry() += offY;
      set(Ms::Pid::OFFSET, off);
      }

//---------------------------------------------------------
//   Segment::elementAt
//---------------------------------------------------------

Element* Segment::elementAt(int track)
      {
      Ms::Element* el = segment()->elementAt(track);
      if (!el)
            return nullptr;
      return wrap(el, Ownership::SCORE);
      }

//---------------------------------------------------------
//   Note::setTpc
//---------------------------------------------------------

void Note::setTpc(int val)
      {
      if (!tpcIsValid(val)) {
            qWarning("PluginAPI::Note::setTpc: invalid tpc: %d", val);
            return;
            }

      if (note()->concertPitch())
            set(Pid::TPC1, val);
      else
            set(Pid::TPC2, val);
      }

//---------------------------------------------------------
//   wrap
///   \cond PLUGIN_API \private \endcond
///   Wraps Ms::Element choosing the correct wrapper type
///   at runtime based on the actual element type.
//---------------------------------------------------------

Element* wrap(Ms::Element* e, Ownership own)
      {
      using Ms::ElementType;
      switch(e->type()) {
            case ElementType::NOTE:
                  return wrap<Note>(toNote(e), own);
            case ElementType::CHORD:
                  return wrap<Chord>(toChord(e), own);
            case ElementType::SEGMENT:
                  return wrap<Segment>(toSegment(e), own);
            case ElementType::MEASURE:
                  return wrap<Measure>(toMeasure(e), own);
            default:
                  break;
            }
      return wrap<Element>(e, own);
      }
}
}
