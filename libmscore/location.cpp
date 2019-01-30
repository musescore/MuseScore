//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "location.h"

#include "chord.h"
#include "element.h"
#include "measure.h"
#include "mscore.h"
#include "xml.h"

namespace Ms {

static constexpr Location absDefaults = Location::absolute();
static constexpr Location relDefaults = Location::relative();

//---------------------------------------------------------
//   Location::track
//---------------------------------------------------------

int Location::track() const
      {
      if ((_staff == absDefaults._staff) || (_voice == absDefaults._voice))
            return INT_MIN;
      return VOICES * _staff + _voice;
      }

//---------------------------------------------------------
//   Location::setTrack
//---------------------------------------------------------

void Location::setTrack(int track)
      {
      _staff = track / VOICES;
      _voice = track % VOICES;
      }

//---------------------------------------------------------
//   Location::write
//    Only relative locations should be written
//---------------------------------------------------------

void Location::write(XmlWriter& xml) const
      {
      Q_ASSERT(isRelative());
      xml.stag("location");
      xml.tag("staves", _staff, relDefaults._staff);
      xml.tag("voices", _voice, relDefaults._voice);
      xml.tag("measures", _measure, relDefaults._measure);
      xml.tag("fractions", _frac.reduced(), relDefaults._frac);
      xml.tag("grace", _graceIndex, relDefaults._graceIndex);
      xml.tag("notes", _note, relDefaults._note);
      xml.etag();
      }

//---------------------------------------------------------
//   Location::read
//---------------------------------------------------------

void Location::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "staves")
                  _staff = e.readInt();
            else if (tag == "voices")
                  _voice = e.readInt();
            else if (tag == "measures")
                  _measure = e.readInt();
            else if (tag == "fractions")
                  _frac = e.readFraction();
            else if (tag == "grace")
                  _graceIndex = e.readInt();
            else if (tag == "notes")
                  _note = e.readInt();
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   Location::toAbsolute
//---------------------------------------------------------

void Location::toAbsolute(const Location& ref)
      {
      if (isAbsolute())
            return;
      _staff += ref._staff;
      _voice += ref._voice;
      _measure += ref._measure;
      _frac += ref._frac;
      _note += ref._note;
      _rel = false;
      }

//---------------------------------------------------------
//   Location::toRelative
//---------------------------------------------------------

void Location::toRelative(const Location& ref)
      {
      if (isRelative())
            return;
      _staff -= ref._staff;
      _voice -= ref._voice;
      _measure -= ref._measure;
      _frac -= ref._frac;
      _note -= ref._note;
      _rel = true;
      }

//---------------------------------------------------------
//   Location::fillPositionForElement
//    Fills default fields of Location by values relevant
//    for the given Element. This function fills only
//    position values, not dealing with parameters specific
//    for Chords and Notes, like grace index.
//---------------------------------------------------------

void Location::fillPositionForElement(const Element* e, bool absfrac)
      {
      Q_ASSERT(isAbsolute());
      if (!e) {
            qWarning("Location::fillPositionForElement: element is nullptr");
            return;
            }
      if (track() == absDefaults.track())
            setTrack(track(e));
      if (frac() == absDefaults.frac())
            setFrac(absfrac ? e->tick() : e->rtick());
      if (measure() == absDefaults.measure())
            setMeasure(absfrac ? 0 : measure(e));
      }

//---------------------------------------------------------
//   Location::fillForElement
//    Fills default fields of Location by values relevant
//    for the given Element, including parameters specific
//    for Chords and Notes.
//---------------------------------------------------------

void Location::fillForElement(const Element* e, bool absfrac)
      {
      Q_ASSERT(isAbsolute());
      if (!e) {
            qWarning("Location::fillForElement: element is nullptr");
            return;
            }

      fillPositionForElement(e, absfrac);
      setGraceIndex(graceIndex(e));
      setNote(note(e));
      }

//---------------------------------------------------------
//   Location::forElement
//---------------------------------------------------------

Location Location::forElement(const Element* e, bool absfrac)
      {
      Location i = Location::absolute();
      i.fillForElement(e, absfrac);
      return i;
      }

//---------------------------------------------------------
//   Location::positionForElement
//---------------------------------------------------------

Location Location::positionForElement(const Element* e, bool absfrac)
      {
      Location i = Location::absolute();
      i.fillPositionForElement(e, absfrac);
      return i;
      }

//---------------------------------------------------------
//   Location::track
//---------------------------------------------------------

int Location::track(const Element* e)
      {
      int track = e->track();
      if (track < 0) {
            const MeasureBase* mb = e->findMeasureBase();
            if (mb && !mb->isMeasure()) {
                  // Such elements are written in the first staff,
                  // see writeMeasure() in scorefile.cpp
                  track = 0;
                  }
            }
      return track;
      }

//---------------------------------------------------------
//   Location::measure
//---------------------------------------------------------

int Location::measure(const Element* e)
      {
      const Measure* m = toMeasure(e->findMeasure());
      if (m)
            return m->measureIndex();
      qWarning("Location::measure: cannot find element's measure (%s)", e->name());
      return 0;
      }

//---------------------------------------------------------
//   Location::graceIndex
//---------------------------------------------------------

int Location::graceIndex(const Element* e)
      {
      if (e->isChord() || (e->parent() && e->parent()->isChord())) {
            const Chord* ch = e->isChord() ? toChord(e) : toChord(e->parent());
            if (ch->isGrace())
                  return ch->graceIndex();
            }
      return absDefaults.graceIndex();
      }

//---------------------------------------------------------
//   Location::note
//---------------------------------------------------------

int Location::note(const Element* e)
      {
      if (e->isNote()) {
            const Note* n = toNote(e);
            const std::vector<Note*>& notes = n->chord()->notes();
            if (notes.size() == 1)
                  return 0;
            int noteIdx;
            for (noteIdx = 0; noteIdx < int(notes.size()); ++noteIdx) {
                  if (n == notes.at(noteIdx))
                        break;
                  }
            return noteIdx;
            }
      return absDefaults.note();
      }

//---------------------------------------------------------
//   Location::getLocationProperty
//---------------------------------------------------------

QVariant Location::getLocationProperty(Pid pid, const Element* start, const Element* end)
      {
      switch(pid) {
            case Pid::LOCATION_STAVES:
                  return (track(start) / VOICES) - (track(end) / VOICES);
            case Pid::LOCATION_VOICES:
                  return (track(start) % VOICES) - (track(end) / VOICES);
            case Pid::LOCATION_MEASURES:
                  return measure(end) - measure(start);
            case Pid::LOCATION_FRACTIONS:
                  return end->rtick() - start->rtick();
            case Pid::LOCATION_GRACE:
                  return graceIndex(end) - graceIndex(end);
            case Pid::LOCATION_NOTE:
                  return note(start) - note(end);
            default:
                  return QVariant();
            }
      }

//---------------------------------------------------------
//   Location::operator==
//---------------------------------------------------------

bool Location::operator==(const Location& pi2) const {
      const Location& pi1 = *this;
      return ((pi1._frac == pi2._frac)
             && (pi1._measure == pi2._measure)
             && (pi1._voice == pi2._voice)
             && (pi1._staff == pi2._staff)
             && (pi1._graceIndex == pi2._graceIndex)
             && (pi1._note == pi2._note)
             );
      }
}

