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

namespace Ms {

//---------------------------------------------------------
//   Part
//---------------------------------------------------------

Part::Part(Score* s)
   : ScoreElement(s)
      {
      _show  = true;
      _instruments.setInstrument(new Instrument, -1);   // default instrument
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
            setInstrument(instr, -1);
            }
      else if (tag == "name")
            instrument()->setLongName(e.readElementText());
      else if (tag == "shortName")
            instrument()->setShortName(e.readElementText());
      else if (tag == "trackName")
            _partName = e.readElementText();
      else if (tag == "show")
            _show = e.readInt();
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
      xml.stag("Part");
      foreach(const Staff* staff, _staves)
            staff->write(xml);
      if (!_show)
            xml.tag("show", _show);
      xml.tag("trackName", _partName);
      instrument()->write(xml, const_cast<Part*>(this)); // Safe, we do not write anything to it
      xml.etag();
      }

//---------------------------------------------------------
//   setLongNames
//---------------------------------------------------------

void Part::setLongNames(QList<StaffName>& name, int tick)
      {
      instrument(tick)->longNames() = name;
      }

void Part::setShortNames(QList<StaffName>& name, int tick)
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
      c->program = program;
      c->bank    = bank;
      c->updateInitList();
//      instrument()->setChannel(0, c);
      }

//---------------------------------------------------------
//   volume
//---------------------------------------------------------

int Part::volume() const
      {
      return instrument()->channel(0)->volume;
      }

//---------------------------------------------------------
//   setVolume
//---------------------------------------------------------

void Part::setVolume(int volume)
      {
      instrument()->channel(0)->volume = volume;
      }

//---------------------------------------------------------
//   mute
//---------------------------------------------------------

bool Part::mute() const
      {
      return instrument()->channel(0)->mute;
      }

//---------------------------------------------------------
//   setMute
//---------------------------------------------------------

void Part::setMute(bool mute)
      {
      instrument()->channel(0)->mute = mute;
      }

//---------------------------------------------------------
//   reverb
//---------------------------------------------------------

int Part::reverb() const
      {
      return instrument()->channel(0)->reverb;
      }

//---------------------------------------------------------
//   setReverb
//---------------------------------------------------------

void Part::setReverb(int val)
      {
      instrument()->channel(0)->reverb = val;
      }

//---------------------------------------------------------
//   chorus
//---------------------------------------------------------

int Part::chorus() const
      {
      return instrument()->channel(0)->chorus;
      }

//---------------------------------------------------------
//   setChorus
//---------------------------------------------------------

void Part::setChorus(int val)
      {
      instrument()->channel(0)->chorus = val;
      }

//---------------------------------------------------------
//   pan
//---------------------------------------------------------

int Part::pan() const
      {
      return instrument()->channel(0)->pan;
      }

//---------------------------------------------------------
//   setPan
//---------------------------------------------------------

void Part::setPan(int pan)
      {
      instrument()->channel(0)->pan = pan;
      }

//---------------------------------------------------------
//   midiProgram
//---------------------------------------------------------

int Part::midiProgram() const
      {
      return instrument()->channel(0)->program;
      }

//---------------------------------------------------------
//   midiChannel
//---------------------------------------------------------

int Part::midiChannel() const
      {
      return masterScore()->midiChannel(instrument()->channel(0)->channel);
      }

//---------------------------------------------------------
//   midiPort
//---------------------------------------------------------

int Part::midiPort() const
      {
      return masterScore()->midiPort(instrument()->channel(0)->channel);
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

void Part::setMidiChannel(int ch, int port, int tick)
      {
      Channel* channel = instrument(tick)->channel(0);
      if (channel->channel == -1) {
            // Add new mapping
            MidiMapping mm;
            mm.part = this;
            mm.articulation = channel;
            mm.channel = -1;
            mm.port = -1;
            if (ch != -1)
                  mm.channel = ch;
            if (port != -1)
                  mm.port = port;
            channel->channel = masterScore()->midiMapping()->size();
            masterScore()->midiMapping()->append(mm);
            }
      else {
            // Update existing mapping
            if (channel->channel >= masterScore()->midiMapping()->size()) {
                  qDebug()<<"Can't' set midi channel: midiMapping is empty!";
                  return;
                  }

            if (ch != -1)
                  masterScore()->midiMapping(channel->channel)->channel = ch;
            if (port != -1)
                  masterScore()->midiMapping(channel->channel)->port = port;
            masterScore()->midiMapping(channel->channel)->part = this;
            }
      }

//---------------------------------------------------------
//   setInstrument
//---------------------------------------------------------

void Part::setInstrument(Instrument* i, int tick)
      {
      _instruments.setInstrument(i, tick);
      }

void Part::setInstrument(const Instrument&& i, int tick)
      {
      _instruments.setInstrument(new Instrument(i), tick);
      }
void Part::setInstrument(const Instrument& i, int tick)
      {
      _instruments.setInstrument(new Instrument(i), tick);
      }

//---------------------------------------------------------
//   removeInstrument
//---------------------------------------------------------

void Part::removeInstrument(int tick)
      {
      auto i = _instruments.find(tick);
      if (i == _instruments.end()) {
            qDebug("Part::removeInstrument: not found at tick %d", tick);
            return;
            }
      _instruments.erase(i);
      }

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

Instrument* Part::instrument(int tick)
      {
      return _instruments.instrument(tick);
      }

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

const Instrument* Part::instrument(int tick) const
      {
      return _instruments.instrument(tick);
      }

//---------------------------------------------------------
//   instrumentId
//---------------------------------------------------------

QString Part::instrumentId(int tick) const
      {
      return instrument(tick)->instrumentId();
      }

//---------------------------------------------------------
//   longName
//---------------------------------------------------------

QString Part::longName(int tick) const
      {
      const QList<StaffName>& nl = longNames(tick);
      return nl.empty() ? "" : nl[0].name();
      }

//---------------------------------------------------------
//   instrumentName
//---------------------------------------------------------

QString Part::instrumentName(int tick) const
      {
      return instrument(tick)->trackName();
      }

//---------------------------------------------------------
//   shortName
//---------------------------------------------------------

QString Part::shortName(int tick) const
      {
      const QList<StaffName>& nl = shortNames(tick);
      return nl.empty() ? "" : nl[0].name();
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

QVariant Part::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::VISIBLE:
                  return QVariant(_show);
            case P_ID::USE_DRUMSET:
                  return instrument()->useDrumset();
            case P_ID::PART_VOLUME:
                  return volume();
            case P_ID::PART_MUTE:
                  return mute();
            case P_ID::PART_PAN:
                  return pan();
            case P_ID::PART_REVERB:
                  return reverb();
            case P_ID::PART_CHORUS:
                  return chorus();
            default:
                  return QVariant();
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Part::setProperty(P_ID id, const QVariant& property)
      {
      switch (id) {
            case P_ID::VISIBLE:
                  setShow(property.toBool());
                  break;
            case P_ID::USE_DRUMSET:
                  instrument()->setUseDrumset(property.toBool());
                  break;
            case P_ID::PART_VOLUME:
                  setVolume(property.toInt());
                  break;
            case P_ID::PART_MUTE:
                  setMute(property.toBool());
                  break;
            case P_ID::PART_PAN:
                  setPan(property.toInt());
                  break;
            case P_ID::PART_REVERB:
                  setReverb(property.toInt());
                  break;
            case P_ID::PART_CHORUS:
                  setChorus(property.toInt());
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
      return _staves.front()->idx() * VOICES;
      }

//---------------------------------------------------------
//   endTrack
//---------------------------------------------------------

int Part::endTrack() const
      {
      return _staves.back()->idx() * VOICES + VOICES;
      }

//---------------------------------------------------------
//   insertTime
//---------------------------------------------------------

void Part::insertTime(int tick, int len)
      {
      if (len == 0)
            return;

      // move all instruments

      if (len < 0) {
            // remove instruments between tickpos >= tick and tickpos < (tick+len)
            // ownership goes back to class InstrumentChange()

            auto si = _instruments.lower_bound(tick);
            auto ei = _instruments.lower_bound(tick-len);
            _instruments.erase(si, ei);
            }

      InstrumentList il;
      for (auto i = _instruments.lower_bound(tick); i != _instruments.end();) {
            Instrument* instrument = i->second;
            int tick = i->first;
            _instruments.erase(i++);
            _instruments[tick + len] = instrument;
            }
      _instruments.insert(il.begin(), il.end());
      }

//---------------------------------------------------------
//   lyricCount
//---------------------------------------------------------

int Part::lyricCount()
      {
      if (!score())
            return 0;
      int count = 0;
      SegmentType st = SegmentType::ChordRest;
      for (Segment* seg = score()->firstMeasure()->first(st); seg; seg = seg->next1(st)) {
            for (int i = startTrack(); i < endTrack() ; ++i) {
                  ChordRest* cr = toChordRest(seg->element(i));
                  if (cr)
                        count += cr->lyrics().size();
                  }
            }
      return count;
      }

//---------------------------------------------------------
//   harmonyCount
//---------------------------------------------------------

int Part::harmonyCount()
      {
      if (!score())
            return 0;
      int count = 0;
      SegmentType st = SegmentType::ChordRest;
      for (Segment* seg = score()->firstMeasure()->first(st); seg; seg = seg->next1(st)) {
            for (Element* e : seg->annotations()) {
                  if (e->type() == ElementType::HARMONY && e->track() >= startTrack() && e->track() < endTrack())
                        count++;
                  }
            }
      return count;
      }

//---------------------------------------------------------
//   hasPitchedStaff
//---------------------------------------------------------

bool Part::hasPitchedStaff()
      {
      if (!staves())
            return false;
      for (Staff* s : *staves()) {
            if (s && s->isPitchedStaff(0))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   hasTabStaff
//---------------------------------------------------------

bool Part::hasTabStaff()
      {
      if (!staves())
            return false;
      for (Staff* s : *staves()) {
            if (s && s->isTabStaff(0))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   hasDrumStaff
//---------------------------------------------------------

bool Part::hasDrumStaff()
      {
      if (!staves())
            return false;
      for (Staff* s : *staves()) {
            if (s && s->isDrumStaff(0))
                  return true;
            }
      return false;
      }
}

