//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "instrchange.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "part.h"
#include "undo.h"
#include "mscore.h"
#include "xml.h"
#include "measure.h"
#include "system.h"
#include "chord.h"
#include "keysig.h"
#include "stafftypechange.h"
#include <vector>

namespace Ms {

//---------------------------------------------------------
//   instrumentChangeStyle
//---------------------------------------------------------

static const ElementStyle instrumentChangeStyle {
      { Sid::instrumentChangePlacement,          Pid::PLACEMENT              },
      { Sid::instrumentChangeMinDistance,        Pid::MIN_DISTANCE           },
      };

//---------------------------------------------------------
//   InstrumentChange
//---------------------------------------------------------

InstrumentChange::InstrumentChange(Score* s)
   : TextBase(s, Tid::INSTRUMENT_CHANGE, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
      {
      initElementStyle(&instrumentChangeStyle);
      _instrument = new Instrument();
      }

InstrumentChange::InstrumentChange(const Instrument& i, Score* s)
   : TextBase(s, Tid::INSTRUMENT_CHANGE, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
      {
      initElementStyle(&instrumentChangeStyle);
      _instrument = new Instrument(i);
      }

InstrumentChange::InstrumentChange(const InstrumentChange& is)
   : TextBase(is)
      {
      _instrument = new Instrument(*is._instrument);
      _init = is._init;
      _lines = is._lines;
      _staffGroup = is._staffGroup;
      }

InstrumentChange::~InstrumentChange()
      {
      delete _instrument;
      }

void InstrumentChange::setInstrument(const Instrument& i)
      {
      *_instrument = i;
      //delete _instrument;
      //_instrument = new Instrument(i);
      }

void InstrumentChange::setupInstrument(const Instrument* instrument)
      {
      if (_init) {
            Fraction tickStart = segment()->tick();
            Part* part = staff()->part();
            Interval oldV = part->instrument(tickStart)->transpose();
            //Instrument* oi = ic->instrument();  //part->instrument(tickStart);
            //Instrument* instrument = new Instrument(Instrument::fromTemplate(it));

            // change the clef for each stave and add staff type change if necessary
            // must be part of the same loop, as it will first add the staff type change, then add the clef, then set the offset for the clef
            for (int i = 0; i < part->nstaves(); i++) {
                  Spatium clefOffset;
                  Staff* staff = part->staff(i);
                  if (part->staff(i)->staffType(tickStart)->lines() != _lines || _staffGroup != StaffGroup::STANDARD) {
                        clefOffset = setupStaffType(staff);
                        }
                  setupClefs(instrument, i, staff, clefOffset);
                  InstrumentChange* nextIc = score()->nextInstrumentChange(segment(), staff, true);
                  if (nextIc) {
                        Spatium nextClefOffset = nextIc->setupStaffType(staff);
                        nextIc->setupClefs(nextIc->instrument(), i, staff, nextClefOffset);
                        }
                  }
            
            // Change key signature if necessary
            if (instrument->transpose() != oldV) {
                  for (int i = 0; i < part->nstaves(); i++) {
                        if (!part->staff(i)->keySigEvent(tickStart).isAtonal()) {
                              KeySigEvent ks;
                              ks.setForInstrumentChange(true);
                              Key key = part->staff(i)->key(tickStart);
                              if (!score()->styleB(Sid::concertPitch))
                                    key = transposeKey(key, oldV);
                              ks.setKey(key);
                              score()->undoChangeKeySig(part->staff(i), tickStart, ks);
                              }
                        }
                  }

            // change instrument in all linked scores
            for (ScoreElement* se : linkList()) {
                  InstrumentChange* lic = static_cast<InstrumentChange*>(se);
                  Instrument* newInstrument = new Instrument(*instrument);
                  lic->score()->undo(new ChangeInstrument(lic, newInstrument));
                  }

            // transpose for current score only
            // this automatically propagates to linked scores
            if (part->instrument(tickStart)->transpose() != oldV) {
                  auto i = part->instruments()->upper_bound(tickStart.ticks());    // find(), ++i
                  Fraction tickEnd;
                  if (i == part->instruments()->end())
                        tickEnd = Fraction(-1, 1);
                  else
                        tickEnd = Fraction::fromTicks(i->first);
                  score()->transpositionChanged(part, oldV, tickStart, tickEnd);
                  }

            InstrumentChangeWarning* w = score()->nextICWarning(part, segment());
            if (w)
                  w->setPlainText(instrument->trackName());
            else {
                  Chord* nextChord = score()->nextChord(segment(), part, true);
                  if (nextChord)
                        setNextChord(nextChord);
                  }
            setPlainText(tr("To %1").arg(instrument->trackName()));
            }
      }

//---------------------------------------------------------
//   setupStaffType
//    sets the staff type for the new instrument.
//    returns the clef offset.
//---------------------------------------------------------

Spatium InstrumentChange::setupStaffType(Staff* staff)
      {
      StaffType* st = staff->staffType(segment()->measure()->tick());
      Spatium oldOffset = st->yoffset();
      Spatium newOffset = Spatium(2.5 - 0.5 * (float)_lines);
      Spatium clefOffset = newOffset - oldOffset;
      StaffType nst = *st;
      nst.setGroup(_staffGroup);
      nst.setLines(_lines);
      nst.setYoffset(newOffset);
      score()->undo(new ChangeStaffType(staff, nst, segment()->measure()->tick()));
      //StaffType* nst = staff->setStaffType(segment()->measure()->tick(), *st);
      staff->staffTypeListChanged(segment()->measure()->tick());
      return clefOffset;
      }

//---------------------------------------------------------
//   setupClefs
//    setup the new clefs for the instrument
//---------------------------------------------------------

void InstrumentChange::setupClefs(const Instrument* instrument, int i, Staff* staff, Spatium clefOffset)
      {
      bool concert = score()->styleB(Sid::concertPitch);
      if (staff->clefList().size() == 0 || staff->clefType(tick()) != instrument->clefType(i)) {
            ClefType clefType = concert ? instrument->clefType(i)._concertClef : instrument->clefType(i)._transposingClef;
            // If instrument change is at the start of a measure, use the measure as the element, as this will place the instrument change before the barline.
            Element* element = rtick().isZero() ? toElement(findMeasure()) : toElement(this);
            score()->undoChangeClef(staff, element, clefType, true, QPointF(0, Spatium::toDouble(clefOffset) * SPATIUM20));
      }
      else if (i < clefs().size())
            score()->deleteItem(clefs().at(i));
      }

//---------------------------------------------------------
//   setNextChord
//    places the warning on the specified chord
//---------------------------------------------------------

void InstrumentChange::setNextChord(ChordRest* chord)
      {
      InstrumentChangeWarning* nextWarning = score()->nextICWarning(chord->part(), chord->segment());
      if (nextWarning)
            score()->undoRemoveElement(nextWarning);
      if (_init && _showWarning && chord->tick() != tick()) {
            InstrumentChangeWarning* instrumentChangeWarning = new InstrumentChangeWarning(score());
            instrumentChangeWarning->setPlainText(instrument()->trackName());
            chord->undoAddAnnotation(instrumentChangeWarning, true);
            }
      }

//---------------------------------------------------------
//   keySigs
//---------------------------------------------------------

std::vector<KeySig*> InstrumentChange::keySigs() const
      {
      std::vector<KeySig*> keysigs;
      Segment* seg = segment()->prev1(SegmentType::KeySig);
      if (seg) {
            int startVoice = part()->staff(0)->idx() * VOICES;
            int endVoice = part()->staff(part()->nstaves() - 1)->idx() * VOICES;
            Fraction t = tick();
            for (int i = startVoice; i <= endVoice; i += VOICES) {
                  KeySig* ks = toKeySig(seg->element(i));
                  if (ks && ks->forInstrumentChange() && ks->tick() == t)
                        keysigs.push_back(ks);
                  }
            }
      return keysigs;
      }

//---------------------------------------------------------
//   clefs
//---------------------------------------------------------

std::vector<Clef*> InstrumentChange::clefs() const
      {
      std::vector<Clef*> clefs;
      Segment* seg = segment()->prev1(SegmentType::Clef);
      int minTrack = part()->staff(0)->idx() * VOICES;
      int maxTrack = part()->staff(part()->nstaves() - 1)->idx() * VOICES;
      if (seg) {
            int startVoice = part()->staff(0)->idx() * VOICES;
            int endVoice = part()->staff(part()->nstaves() - 1)->idx() * VOICES;
            Fraction t = tick();
            for (int i = startVoice; i <= endVoice; i += VOICES) {
                  Clef* clef = toClef(seg->element(i));
                  if (clef && clef->forInstrumentChange() && clef->tick() == t && clef->track() >= minTrack && clef->track() <= maxTrack)
                        clefs.push_back(clef);
                  }
            }
      return clefs;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void InstrumentChange::write(XmlWriter& xml) const
      {
      xml.stag(this);
      _instrument->write(xml, part());
      if (_init)
            xml.tag("init", _init);
      switch (_staffGroup) {
            case StaffGroup::PERCUSSION: xml.tag("staffGroup", "percussion"); break;
            case StaffGroup::TAB: xml.tag("staffGroup", "tab"); break;
            }
      xml.tag("lines", _lines);
      TextBase::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void InstrumentChange::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Instrument")
                  _instrument->read(e, part());
            else if (tag == "init")
                  _init = e.readBool();
            else if (tag == "staffGroup") {
                  QString text = e.readElementText();
                  if (text == "percussion")
                        _staffGroup = StaffGroup::PERCUSSION;
                  else if (text == "tab")
                        _staffGroup = StaffGroup::TAB;
                  else
                        _staffGroup = StaffGroup::STANDARD;
            }
            else if (tag == "lines")
                  _lines = e.readInt();
            else if (!TextBase::readProperties(e))
                  e.unknown();
            }
      if (score()->mscVersion() < 206) {
            // previous versions did not honor transposition of instrument change
            // except in ways that it should not have
            // notes entered before the instrument change was added would not be altered,
            // so original transposition remained in effect
            // notes added afterwards would be transposed by both intervals, resulting in tpc corruption
            // here we set the instrument change to inherit the staff transposition to emulate previous versions
            // in Note::read(), we attempt to fix the tpc corruption
            // There is also code in read206 to try to deal with this, but it is out of date and therefore disabled
            // What this means is, scores created in 2.1 or later should be fine, scores created in 2.0 maybe not so much

            Interval v = staff() ? staff()->part()->instrument()->transpose() : 0;
            _instrument->setTranspose(v);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant InstrumentChange::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::SUB_STYLE:
                  return int(Tid::INSTRUMENT_CHANGE);
            default:
                  return TextBase::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void InstrumentChange::layout()
      {
      TextBase::layout();
      autoplaceSegmentElement();
      }

}

