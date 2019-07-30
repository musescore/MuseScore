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
#include "libmscore/undo.h"

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
//   Chord::setPlayEventType
//---------------------------------------------------------

void Chord::setPlayEventType(Ms::PlayEventType v)
      {
      // Only create undo operation if the value has changed.
      if (v != chord()->playEventType())
            {
            chord()->score()->setPlaylistDirty();
            chord()->score()->undo(new ChangeChordPlayEventType(chord(), v));
            }
      }

//---------------------------------------------------------
//   Chord::add
//---------------------------------------------------------

void Chord::add(Ms::PluginAPI::Element* wrapped)
      {
      Ms::Element* s = wrapped->element();
      if (s)
            {
            // Ensure that the object has the expected ownership
            if (wrapped->ownership() == Ownership::SCORE) {
                  qWarning("Chord::add: Cannot add this element. The element is already part of the score.");
                  return;        // Don't allow operation.
                  }
            // Score now owns the object.
            wrapped->setOwnership(Ownership::SCORE);
            // Provide parentage for element.
            s->setParent(chord());
            // If a note, ensure the element has proper Tpc values. (Will crash otherwise)
            if (s->isNote()) {
                  s->setTrack(chord()->track());
                  toNote(s)->setTpcFromPitch();
                  }
            // Create undo op and add the element.
            chord()->score()->undoAddElement(s);
            }
      }

//---------------------------------------------------------
//   Chord::remove
//---------------------------------------------------------

void Chord::remove(Ms::PluginAPI::Element* wrapped)
      {
      Ms::Element* s = wrapped->element();
      if (s->parent() != chord())
            qWarning("PluginAPI::Chord::remove: The element is not a child of this chord. Use removeElement() instead.");
      else if (chord()->notes().size() <= 1 && s->type() == ElementType::NOTE)
            qWarning("PluginAPI::Chord::remove: Removal of final note is not allowed.");
      else if (s)
            chord()->score()->deleteItem(s); // Create undo op and remove the element.
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
