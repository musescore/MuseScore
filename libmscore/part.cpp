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

#include "part.h"
#include "staff.h"
#include "xml.h"
#include "score.h"
#include "style.h"
#include "note.h"
#include "drumset.h"
#include "instrtemplate.h"
#include "text.h"
#include "measure.h"
#include "stringdata.h"
#include "stafftype.h"
#include "sym.h"
#include "chordrest.h"
#include "fret.h"

namespace Ms {

//---------------------------------------------------------
//   Part
//---------------------------------------------------------

Part::Part(Score* s)
   : ScoreElement(s)
      {
      _color   = DEFAULT_COLOR;
      _show    = true;
      _soloist = false;
      _instruments.setInstrument(new Instrument, -1);   // default instrument
      _preferSharpFlat = PreferSharpFlat::DEFAULT;
      }

//---------------------------------------------------------
//   initFromInstrTemplate
//---------------------------------------------------------

void Part::initFromInstrTemplate(const InstrumentTemplate* t)
      {
      _partName = t->trackName;
      setInstrument(Instrument::fromTemplate(t));
      }

//---------------------------------------------------------
//   staff
//---------------------------------------------------------

Staff* Part::staff(int idx) const
      {
      return _staves[idx];
      }

//---------------------------------------------------------
//   Part::masterPart
//---------------------------------------------------------

const Part* Part::masterPart() const
      {
      if (score()->isMaster())
            return this;
      if (_staves.empty())
            return this;

      Staff* st = _staves[0];
      LinkedElements* links = st->links();
      if (!links)
            return this;

      for (ScoreElement* le : *links) {
            if (le->isStaff() && toStaff(le)->score()->isMaster()) {
                  if (Part* p = toStaff(le)->part())
                        return p;
                  }
            }
      return this;
      }

//---------------------------------------------------------
//   Part::masterPart
//---------------------------------------------------------

Part* Part::masterPart()
      {
      return const_cast<Part*>(const_cast<const Part*>(this)->masterPart());
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Part::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());
      if (tag == "Staff") {
            Staff* staff = new Staff(score());
            staff->setPart(this);
            score()->staves().push_back(staff);
            _staves.push_back(staff);
            staff->read(e);
            }
      else if (tag == "Instrument") {
            Instrument* instr = new Instrument;
            instr->read(e, this);
            setInstrument(instr, Fraction(-1, 1));
            }
      else if (tag == "name")
            instrument()->setLongName(e.readElementText());
      else if (tag == "color")
            _color = e.readInt();
      else if (tag == "shortName")
            instrument()->setShortName(e.readElementText());
      else if (tag == "trackName")
            _partName = e.readElementText();
      else if (tag == "show")
            _show = e.readInt();
      else if (tag == "soloist")
            _soloist = e.readInt();
      else if (tag == "preferSharpFlat")
            _preferSharpFlat =
               e.readElementText() == "sharps" ? PreferSharpFlat::SHARPS : PreferSharpFlat::FLATS;
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Part::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (!readProperties(e))
                  e.unknown();
            }
      if (_partName.isEmpty())
            _partName = instrument()->trackName();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Part::write(XmlWriter& xml) const
      {
      xml.stag(this);
      for (const Staff* staff : _staves)
            staff->write(xml);
      if (!_show)
            xml.tag("show", _show);
      if (_soloist)
            xml.tag("soloist", _soloist);
      xml.tag("trackName", _partName);
      if (_color != DEFAULT_COLOR)
            xml.tag("color", _color);
      if (_preferSharpFlat != PreferSharpFlat::DEFAULT)
            xml.tag("preferSharpFlat",
               _preferSharpFlat == PreferSharpFlat::SHARPS ? "sharps" : "flats");
      instrument()->write(xml, this);
      xml.etag();
      }

//---------------------------------------------------------
//   setLongNames
//---------------------------------------------------------

void Part::setLongNames(QList<StaffName>& name, const Fraction& tick)
      {
      instrument(tick)->longNames() = name;
      }

void Part::setShortNames(QList<StaffName>& name, const Fraction& tick)
      {
      instrument(tick)->shortNames() = name;
      }

//---------------------------------------------------------
//   setStaves
//---------------------------------------------------------

void Part::setStaves(int n)
      {
      int ns = _staves.size();
      if (n < ns) {
            qDebug("Part::setStaves(): remove staves not implemented!");
            return;
            }
      int staffIdx = score()->staffIdx(this) + ns;
      for (int i = ns; i < n; ++i) {
            Staff* staff = new Staff(score());
            staff->setPart(this);
            _staves.push_back(staff);
            score()->staves().insert(staffIdx, staff);
            for (Measure* m = score()->firstMeasure(); m; m = m->nextMeasure()) {
                  m->insertStaff(staff, staffIdx);
                  if (m->hasMMRest())
                        m->mmRest()->insertStaff(staff, staffIdx);
                  }
            ++staffIdx;
            }
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Part::insertStaff(Staff* staff, int idx)
      {
      if (idx < 0 || idx > _staves.size())
            idx = _staves.size();
      _staves.insert(idx, staff);
      staff->setPart(this);
      }

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Part::removeStaff(Staff* staff)
      {
      if (!_staves.removeOne(staff)) {
            qDebug("Part::removeStaff: not found %p", staff);
            return;
            }
      }

//---------------------------------------------------------
//   setMidiProgram
//    TODO
//---------------------------------------------------------

void Part::setMidiProgram(int program, int bank)
      {
      Channel* c = instrument()->channel(0);
      c->setProgram(program);
      c->setBank(bank);
//      instrument()->setChannel(0, c);
      }

//---------------------------------------------------------
//   midiProgram
//---------------------------------------------------------

int Part::midiProgram() const
      {
      return instrument()->playbackChannel(0, masterScore())->program();
      }

//---------------------------------------------------------
//   midiChannel
//---------------------------------------------------------

int Part::midiChannel() const
      {
      return masterScore()->midiChannel(instrument()->channel(0)->channel());
      }

//---------------------------------------------------------
//   midiPort
//---------------------------------------------------------

int Part::midiPort() const
      {
      return masterScore()->midiPort(instrument()->channel(0)->channel());
      }

//---------------------------------------------------------
//   setMidiChannel
//   Called from importmusicxml, importMidi and importGtp*.
//   Specify tick to set MIDI channel to an InstrumentChange element.
//   Usage:
//   setMidiChannel(channel)       to set channel
//   setMidiChannel(-1, port)      to set port
//   setMidiChannel(channel, port) to set both
//---------------------------------------------------------

void Part::setMidiChannel(int ch, int port, const Fraction& tick)
      {
      Channel* channel = instrument(tick)->channel(0);
      if (channel->channel() == -1)
            masterScore()->addMidiMapping(channel, this, port, ch);
      else
            masterScore()->updateMidiMapping(channel, this, port, ch);
      }

//---------------------------------------------------------
//   setInstrument
//---------------------------------------------------------

void Part::setInstrument(Instrument* i, Fraction tick)
      {
      _instruments.setInstrument(i, tick.ticks());
      }

void Part::setInstrument(const Instrument&& i, Fraction tick)
      {
      _instruments.setInstrument(new Instrument(i), tick.ticks());
      }

void Part::setInstrument(const Instrument& i, Fraction tick)
      {
      _instruments.setInstrument(new Instrument(i), tick.ticks());
      }

//---------------------------------------------------------
//   removeInstrument
//---------------------------------------------------------

void Part::removeInstrument(const Fraction& tick)
      {
      auto i = _instruments.find(tick.ticks());
      if (i == _instruments.end()) {
            qDebug("Part::removeInstrument: not found at tick %d", tick.ticks());
            return;
            }
      _instruments.erase(i);
      }

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

Instrument* Part::instrument(Fraction tick)
      {
      return _instruments.instrument(tick.ticks());
      }

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

const Instrument* Part::instrument(Fraction tick) const
      {
      return _instruments.instrument(tick.ticks());
      }

//---------------------------------------------------------
//   instruments
//---------------------------------------------------------

const InstrumentList* Part::instruments() const
      {
      return &_instruments;
      }

//---------------------------------------------------------
//   instrumentId
//---------------------------------------------------------

QString Part::instrumentId(const Fraction& tick) const
      {
      return instrument(tick)->instrumentId();
      }

//---------------------------------------------------------
//   longName
//---------------------------------------------------------

QString Part::longName(const Fraction& tick) const
      {
      const QList<StaffName>& nl = longNames(tick);
      return nl.empty() ? "" : nl[0].name();
      }

//---------------------------------------------------------
//   instrumentName
//---------------------------------------------------------

QString Part::instrumentName(const Fraction& tick) const
      {
      return instrument(tick)->trackName();
      }

//---------------------------------------------------------
//   shortName
//---------------------------------------------------------

QString Part::shortName(const Fraction& tick) const
      {
      const QList<StaffName>& nl = shortNames(tick);
      return nl.empty() ? "" : nl[0].name();
      }

//---------------------------------------------------------
//   setLongNameAll
//---------------------------------------------------------

void Part::setLongNameAll(const QString& s)
      {
      for (auto instrument : _instruments)
            instrument.second->setLongName(s);
      }

//---------------------------------------------------------
//   setShortNameAll
//---------------------------------------------------------

void Part::setShortNameAll(const QString& s)
      {
      for (auto instrument : _instruments)
            instrument.second->setShortName(s);
      }

//---------------------------------------------------------
//   setLongName
//---------------------------------------------------------

void Part::setLongName(const QString& s)
      {
      instrument()->setLongName(s);
      }

//---------------------------------------------------------
//   setShortName
//---------------------------------------------------------

void Part::setShortName(const QString& s)
      {
      instrument()->setShortName(s);
      }

//---------------------------------------------------------
//   setPlainLongNameAll
//---------------------------------------------------------

void Part::setPlainLongNameAll(const QString& s)
      {
      setLongNameAll(XmlWriter::xmlString(s));
      }

//---------------------------------------------------------
//   setPlainShortNameAll
//---------------------------------------------------------

void Part::setPlainShortNameAll(const QString& s)
      {
      setShortNameAll(XmlWriter::xmlString(s));
      }

//---------------------------------------------------------
//   setPlainLongName
//---------------------------------------------------------

void Part::setPlainLongName(const QString& s)
      {
      setLongName(XmlWriter::xmlString(s));
      }

//---------------------------------------------------------
//   setPlainShortName
//---------------------------------------------------------

void Part::setPlainShortName(const QString& s)
      {
      setShortName(XmlWriter::xmlString(s));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Part::getProperty(Pid id) const
      {
      switch (id) {
            case Pid::VISIBLE:
                  return QVariant(_show);
            case Pid::USE_DRUMSET:
                  return instrument()->useDrumset();
            case Pid::PREFER_SHARP_FLAT:
                  return int(preferSharpFlat());
            default:
                  return QVariant();
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Part::setProperty(Pid id, const QVariant& property)
      {
      switch (id) {
            case Pid::VISIBLE:
                  setShow(property.toBool());
                  break;
            case Pid::USE_DRUMSET:
                  instrument()->setUseDrumset(property.toBool());
                  break;
            case Pid::PREFER_SHARP_FLAT:
                  setPreferSharpFlat(PreferSharpFlat(property.toInt()));
                  break;
            default:
                  qDebug("Part::setProperty: unknown id %d", int(id));
                  break;
            }
      score()->setLayoutAll();
      return true;
      }

//---------------------------------------------------------
//   startTrack
//---------------------------------------------------------

int Part::startTrack() const
      {
      if (_staves.empty())
            return -1;
      return _staves.front()->idx() * VOICES;
      }

//---------------------------------------------------------
//   endTrack
//---------------------------------------------------------

int Part::endTrack() const
      {
      if (_staves.empty())
            return -1;
      return _staves.back()->idx() * VOICES + VOICES;
      }

//---------------------------------------------------------
//   insertTime
//---------------------------------------------------------

void Part::insertTime(const Fraction& tick, const Fraction& len)
      {
      if (len.isZero())
            return;

      // move all instruments

      if (len < Fraction(0,1)) {
            // remove instruments between tickpos >= tick and tickpos < (tick+len)
            // ownership goes back to class InstrumentChange()

            auto si = _instruments.lower_bound(tick.ticks());
            auto ei = _instruments.lower_bound((tick-len).ticks());
            _instruments.erase(si, ei);
            }

      InstrumentList il;
      for (auto i = _instruments.lower_bound(tick.ticks()); i != _instruments.end();) {
            Instrument* instrument = i->second;
            int t = i->first;
            _instruments.erase(i++);
            _instruments[t + len.ticks()] = instrument;
            }
      _instruments.insert(il.begin(), il.end());
      }

//---------------------------------------------------------
//   lyricCount
//---------------------------------------------------------

int Part::lyricCount() const
      {
      if (!score())
            return 0;

      if (!score()->firstMeasure())
            return 0;

      size_t count = 0;
      SegmentType st = SegmentType::ChordRest;
      for (Segment* seg = score()->firstMeasure()->first(st); seg; seg = seg->next1(st)) {
            for (int i = startTrack(); i < endTrack() ; ++i) {
                  ChordRest* cr = toChordRest(seg->element(i));
                  if (cr)
                        count += cr->lyrics().size();
                  }
            }
      return int(count);
      }

//---------------------------------------------------------
//   harmonyCount
//---------------------------------------------------------

int Part::harmonyCount() const
      {
      if (!score())
            return 0;

      Measure* firstM = score()->firstMeasure();
      if (!firstM)
            return 0;

      SegmentType st = SegmentType::ChordRest;
      int count = 0;
      for (const Segment* seg = firstM->first(st); seg; seg = seg->next1(st)) {
            for (const Element* e : seg->annotations()) {
                  if ((e->isHarmony() || (e->isFretDiagram() && toFretDiagram(e)->harmony())) && e->track() >= startTrack() && e->track() < endTrack())
                        count++;
                  }
            }
      return count;
      }

//---------------------------------------------------------
//   updateHarmonyChannels
///   update the harmony channel by creating a new channel
///   when appropriate or using the existing one
///
///   checkRemoval can be set to true to check to see if we
///   can remove the harmony channel
//---------------------------------------------------------
void Part::updateHarmonyChannels(bool isDoOnInstrumentChanged, bool checkRemoval)
      {

      auto onInstrumentChanged = [this]() {
            masterScore()->rebuildMidiMapping();
            masterScore()->updateChannel();
            score()->setInstrumentsChanged(true);
            score()->setLayoutAll(); //do we need this?
            };


      // usage of harmony count is okay even if expensive since checking harmony channel will shortcircuit if existent
      // harmonyCount will only be called on loading of a score (where it will need to be scanned for harmony anyway)
      // or when the first harmony of a score is just added
      if (checkRemoval) {
            //may be a bit expensive since it gets called after every single delete or undo, but it should be okay for now
            //~OPTIM~
            if (harmonyCount() == 0) {
                  Instrument* instr = instrument();
                  int hChIdx = instr->channelIdx(Channel::HARMONY_NAME);
                  if (hChIdx != -1) {
                        Channel* hChan = instr->channel(hChIdx);
                        instr->removeChannel(hChan);
                        delete hChan;
                        if (isDoOnInstrumentChanged)
                              onInstrumentChanged();
                        return;
                        }
                  }
            }

      if (!harmonyChannel() && harmonyCount() > 0) {
            Instrument* instr = instrument();
            Channel* c = new Channel(*instr->channel(0));
            // default to program 0, which is piano in General MIDI
            c->setProgram(0);
            if (c->bank() == 128) // drumset?
                  c->setBank(0);
            c->setName(Channel::HARMONY_NAME);
            instr->appendChannel(c);
            onInstrumentChanged();
            }
      }

//---------------------------------------------------------
//   harmonyChannel
//---------------------------------------------------------

const Channel* Part::harmonyChannel() const
      {
            const Instrument* instr = instrument();
            if (!instr)
                  return nullptr;

            int chanIdx = instr->channelIdx(Channel::HARMONY_NAME);
            if (chanIdx == -1)
                  return nullptr;

            const Channel* chan = instr->channel(chanIdx);
            Q_ASSERT(chan);
            return chan;
      }

//---------------------------------------------------------
//   hasPitchedStaff
//---------------------------------------------------------

bool Part::hasPitchedStaff() const
      {
      if (!staves())
            return false;
      for (Staff* s : *staves()) {
            if (s && s->isPitchedStaff(Fraction(0,1)))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   hasTabStaff
//---------------------------------------------------------

bool Part::hasTabStaff() const
      {
      if (!staves())
            return false;
      for (Staff* s : *staves()) {
            if (s && s->isTabStaff(Fraction(0,1)))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   hasDrumStaff
//---------------------------------------------------------

bool Part::hasDrumStaff() const
      {
      if (!staves())
            return false;
      for (Staff* s : *staves()) {
            if (s && s->isDrumStaff(Fraction(0,1)))
                  return true;
            }
      return false;
      }
}

