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
#include "fraction.h"
#include "part.h"
#include "libmscore/property.h"
#include "libmscore/undo.h"

namespace Ms {
namespace PluginAPI {

//---------------------------------------------------------
//   Element::setOffsetX
//---------------------------------------------------------

void Element::setOffsetX(qreal offX)
      {
      const qreal offY = element()->offset().y() / element()->spatium();
      set(Ms::Pid::OFFSET, QPointF(offX, offY));
      }

//---------------------------------------------------------
//   Element::setOffsetY
//---------------------------------------------------------

void Element::setOffsetY(qreal offY)
      {
      const qreal offX = element()->offset().x() / element()->spatium();
      set(Ms::Pid::OFFSET, QPointF(offX, offY));
      }

//---------------------------------------------------------
//   Element::bbox
//   return the element bbox in spatium units, rather than in raster units as stored internally
//---------------------------------------------------------

QRectF Element::bbox() const
      {
      QRectF bbox       = element()->bbox();
      qreal  spatium    = element()->spatium();
      return QRectF(bbox.x() / spatium, bbox.y() / spatium, bbox.width() / spatium, bbox.height() / spatium);
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
//   Note::isChildAllowed
///   Check if element type can be a child of note.
///   \since MuseScore 3.3.3
//---------------------------------------------------------

bool Note::isChildAllowed(Ms::ElementType elementType)
      {
      switch(elementType) {
            case ElementType::NOTEHEAD:
            case ElementType::NOTEDOT:
            case ElementType::FINGERING:
            case ElementType::SYMBOL:
            case ElementType::IMAGE:
            case ElementType::TEXT:
            case ElementType::BEND:
            case ElementType::TIE:
            case ElementType::ACCIDENTAL:
            case ElementType::TEXTLINE:
            case ElementType::GLISSANDO:
                  return true;
            default:
                  return false;
            }
      }


//---------------------------------------------------------
//   Note::add
///   \since MuseScore 3.3.3
//---------------------------------------------------------

void Note::add(Ms::PluginAPI::Element* wrapped)
      {
      Ms::Element* s = wrapped ? wrapped->element() : nullptr;
      if (s)
            {
            // Ensure that the object has the expected ownership
            if (wrapped->ownership() == Ownership::SCORE) {
                  qWarning("Note::add: Cannot add this element. The element is already part of the score.");
                  return;        // Don't allow operation.
                  }
            // Score now owns the object.
            wrapped->setOwnership(Ownership::SCORE);

            addInternal(note(), s);
            }
      }

//---------------------------------------------------------
//   Note::addInternal
///   \since MuseScore 3.3.3
//---------------------------------------------------------

void Note::addInternal(Ms::Note* note, Ms::Element* s)
      {
      // Provide parentage for element.
      s->setScore(note->score());
      s->setParent(note);
      s->setTrack(note->track());

      if (s && isChildAllowed(s->type())) {
            // Create undo op and add the element.
            toScore(note->score())->undoAddElement(s);
            }
      else if (s) {
            qDebug("Note::add() not impl. %s", s->name());
            }
      }

//---------------------------------------------------------
//   Note::remove
///   \since MuseScore 3.3.3
//---------------------------------------------------------

void Note::remove(Ms::PluginAPI::Element* wrapped)
      {
      Ms::Element* s = wrapped->element();
      if (!s)
            qWarning("PluginAPI::Note::remove: Unable to retrieve element. %s", qPrintable(wrapped->name()));
      else if (s->parent() != note())
            qWarning("PluginAPI::Note::remove: The element is not a child of this note. Use removeElement() instead.");
      else if (isChildAllowed(s->type()))
            note()->score()->deleteItem(s); // Create undo op and remove the element.
      else
            qDebug("Note::remove() not impl. %s", s->name());
      }

//---------------------------------------------------------
//   DurationElement::globalDuration
//---------------------------------------------------------

FractionWrapper* DurationElement::globalDuration() const
      {
      return wrap(durationElement()->globalTicks());
      }

//---------------------------------------------------------
//   DurationElement::actualDuration
//---------------------------------------------------------

FractionWrapper* DurationElement::actualDuration() const
      {
      return wrap(durationElement()->actualTicks());
      }

//---------------------------------------------------------
//   DurationElement::parentTuplet
//---------------------------------------------------------

Tuplet* DurationElement::parentTuplet()
      {
      return wrap<Tuplet>(durationElement()->tuplet());
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
      Ms::Element* s = wrapped ? wrapped->element() : nullptr;
      if (s)
            {
            // Ensure that the object has the expected ownership
            if (wrapped->ownership() == Ownership::SCORE) {
                  qWarning("Chord::add: Cannot add this element. The element is already part of the score.");
                  return;        // Don't allow operation.
                  }
            // Score now owns the object.
            wrapped->setOwnership(Ownership::SCORE);

            addInternal(chord(), s);
            }
      }

//---------------------------------------------------------
//   Chord::addInternal
//---------------------------------------------------------

void Chord::addInternal(Ms::Chord* chord, Ms::Element* s)
      {
      // Provide parentage for element.
      s->setScore(chord->score());
      s->setParent(chord);
      // If a note, ensure the element has proper Tpc values. (Will crash otherwise)
      if (s->isNote()) {
            s->setTrack(chord->track());
            toNote(s)->setTpcFromPitch();
            }
      // Create undo op and add the element.
      chord->score()->undoAddElement(s);
      }

//---------------------------------------------------------
//   Page::pagenumber
//---------------------------------------------------------

int Page::pagenumber() const
      {
      return page()->no();
      }

//---------------------------------------------------------
//   Chord::remove
//---------------------------------------------------------

void Chord::remove(Ms::PluginAPI::Element* wrapped)
      {
      Ms::Element* s = wrapped->element();
      if (!s)
            qWarning("PluginAPI::Chord::remove: Unable to retrieve element. %s", qPrintable(wrapped->name()));
      else if (s->parent() != chord())
            qWarning("PluginAPI::Chord::remove: The element is not a child of this chord. Use removeElement() instead.");
      else if (chord()->notes().size() <= 1 && s->type() == ElementType::NOTE)
            qWarning("PluginAPI::Chord::remove: Removal of final note is not allowed.");
      else
            chord()->score()->deleteItem(s); // Create undo op and remove the element.
      }

//---------------------------------------------------------
//   Staff::part
//---------------------------------------------------------

Part* Staff::part()
      {
      return wrap<Part>(staff()->part());
      }

//---------------------------------------------------------
//   wrap
///   \cond PLUGIN_API \private \endcond
///   Wraps Ms::Element choosing the correct wrapper type
///   at runtime based on the actual element type.
//---------------------------------------------------------

Element* wrap(Ms::Element* e, Ownership own)
      {
      if (!e)
            return nullptr;

      using Ms::ElementType;
      switch(e->type()) {
            case ElementType::NOTE:
                  return wrap<Note>(toNote(e), own);
            case ElementType::CHORD:
                  return wrap<Chord>(toChord(e), own);
            case ElementType::TUPLET:
                  return wrap<Tuplet>(toTuplet(e), own);
            case ElementType::SEGMENT:
                  return wrap<Segment>(toSegment(e), own);
            case ElementType::MEASURE:
                  return wrap<Measure>(toMeasure(e), own);
            case ElementType::PAGE:
                  return wrap<Page>(toPage(e), own);
            default:
                  if (e->isDurationElement()) {
                        if (e->isChordRest())
                              return wrap<ChordRest>(toChordRest(e), own);
                        return wrap<DurationElement>(toDurationElement(e), own);
                        }
                  break;
            }
      return wrap<Element>(e, own);
      }
}
}
