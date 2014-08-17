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

#include "instrument_p.h"
#include "xml.h"
#include "drumset.h"
#include "articulation.h"
#include "utils.h"
#include "stringdata.h"
#include "instrtemplate.h"
#include "mscore.h"

namespace Ms {

Instrument InstrumentList::defaultInstrument;

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void NamedEventList::write(Xml& xml, const QString& n) const
      {
      xml.stag(QString("%1 name=\"%2\"").arg(n).arg(name));
      if (!descr.isEmpty())
            xml.tag("descr", descr);
      foreach(const MidiCoreEvent& e, events)
            e.write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void NamedEventList::read(XmlReader& e)
      {
      name = e.attribute("name");
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "program") {
                  MidiCoreEvent ev(ME_CONTROLLER, 0, CTRL_PROGRAM, e.intAttribute("value", 0));
                  events.push_back(ev);
                  e.skipCurrentElement();
                  }
            else if (tag == "controller") {
                  MidiCoreEvent ev;
                  ev.setType(ME_CONTROLLER);
                  ev.setDataA(e.intAttribute("ctrl", 0));
                  ev.setDataB(e.intAttribute("value", 0));
                  events.push_back(ev);
                  e.skipCurrentElement();
                  }
            else if (tag == "descr")
                  descr = e.readElementText();
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   operator
//---------------------------------------------------------

bool MidiArticulation::operator==(const MidiArticulation& i) const
      {
      return (i.name == name) && (i.velocity == velocity) && (i.gateTime == gateTime);
      }

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

InstrumentData::InstrumentData()
      {
      Channel a;
      a.name  = "normal";
      _channel.append(a);

      _minPitchA   = 0;
      _maxPitchA   = 127;
      _minPitchP   = 0;
      _maxPitchP   = 127;
      _useDrumset  = DrumsetKind::NONE;
      _drumset     = 0;
      }

InstrumentData::InstrumentData(const InstrumentData& i)
   : QSharedData(i)
      {
      _longNames    = i._longNames;
      _shortNames   = i._shortNames;
      _trackName    = i._trackName;
      _minPitchA    = i._minPitchA;
      _maxPitchA    = i._maxPitchA;
      _minPitchP    = i._minPitchP;
      _maxPitchP    = i._maxPitchP;
      _transpose    = i._transpose;
      _useDrumset   = i._useDrumset;
      _stringData   = i._stringData;
      _drumset      = 0;
      setDrumset(i._drumset);
      _stringData   = i._stringData;
      _midiActions  = i._midiActions;
      _articulation = i._articulation;
      _channel      = i._channel;
      _clefType     = i._clefType;
      }

//---------------------------------------------------------
//   ~InstrumentData
//---------------------------------------------------------

InstrumentData::~InstrumentData()
      {
      delete _drumset;
      }

//---------------------------------------------------------
//   StaffName::write
//---------------------------------------------------------

void StaffName::write(Xml& xml, const char* tag) const
      {
      if (!name.isEmpty())
            xml.writeXml(QString("%1 pos=\"%2\"").arg(tag).arg(pos), name);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffName::read(XmlReader& e)
      {
      pos  = e.intAttribute("pos", 0);
      name = e.readXml();
      if (name.startsWith("<html>")) {
            // compatibility to old html implementation:
            name = QTextDocumentFragment::fromHtml(name).toPlainText();
            }
      }

//---------------------------------------------------------
//   InstrumentData::write
//---------------------------------------------------------

void InstrumentData::write(Xml& xml) const
      {
      xml.stag("Instrument");
      foreach(const StaffName& doc, _longNames)
            doc.write(xml, "longName");
      foreach(const StaffName& doc, _shortNames)
            doc.write(xml, "shortName");
//      if (!_trackName.isEmpty())
            xml.tag("trackName", _trackName);
      if (_minPitchP > 0)
            xml.tag("minPitchP", _minPitchP);
      if (_maxPitchP < 127)
            xml.tag("maxPitchP", _maxPitchP);
      if (_minPitchA > 0)
            xml.tag("minPitchA", _minPitchA);
      if (_maxPitchA < 127)
            xml.tag("maxPitchA", _maxPitchA);
      if (_transpose.diatonic)
            xml.tag("transposeDiatonic", _transpose.diatonic);
      if (_transpose.chromatic)
            xml.tag("transposeChromatic", _transpose.chromatic);
      if (_useDrumset != DrumsetKind::NONE) {
            xml.tag("useDrumset", int(_useDrumset));
            _drumset->save(xml);
            }
      for (int i = 0; i < _clefType.size(); ++i) {
            ClefTypeList ct = _clefType[i];
            if (ct._concertClef == ct._transposingClef) {
                  if (ct._concertClef != ClefType::G) {
                        QString tag = ClefInfo::tag(ct._concertClef);
                        if (i)
                              xml.tag(QString("clef staff=\"%1\"").arg(i+1), tag);
                        else
                              xml.tag("clef", tag);
                        }
                  }
            else {
                  QString tag1 = ClefInfo::tag(ct._concertClef);
                  QString tag2 = ClefInfo::tag(ct._transposingClef);
                  if (i) {
                        xml.tag(QString("concertClef staff=\"%1\"").arg(i+1), tag1);
                        xml.tag(QString("transposingClef staff=\"%1\"").arg(i+1), tag2);
                        }
                  else {
                        xml.tag("concertClef", tag1);
                        xml.tag("transposingClef", tag2);
                        }
                  }
            }

      if (!(_stringData == StringData()))
            _stringData.write(xml);
      foreach(const NamedEventList& a, _midiActions)
            a.write(xml, "MidiAction");
      foreach(const MidiArticulation& a, _articulation)
            a.write(xml);
      foreach(const Channel& a, _channel)
            a.write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   InstrumentData::read
//---------------------------------------------------------

void InstrumentData::read(XmlReader& e)
      {
      int program = -1;
      int bank    = 0;
      int chorus  = 30;
      int reverb  = 30;
      int volume  = 100;
      int pan     = 60;
      bool customDrumset = false;

      _channel.clear();
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "longName") {
                  StaffName name;
                  name.read(e);
                  _longNames.append(name);
                  }
            else if (tag == "shortName") {
                  StaffName name;
                  name.read(e);
                  _shortNames.append(name);
                  }
            else if (tag == "trackName")
                  _trackName = e.readElementText();
            else if (tag == "minPitch") {      // obsolete
                  _minPitchP = _minPitchA = e.readInt();
                  }
            else if (tag == "maxPitch") {       // obsolete
                  _maxPitchP = _maxPitchA = e.readInt();
                  }
            else if (tag == "minPitchA")
                  _minPitchA = e.readInt();
            else if (tag == "minPitchP")
                  _minPitchP = e.readInt();
            else if (tag == "maxPitchA")
                  _maxPitchA = e.readInt();
            else if (tag == "maxPitchP")
                  _maxPitchP = e.readInt();
            else if (tag == "transposition") {    // obsolete
                  _transpose.chromatic = e.readInt();
                  _transpose.diatonic = chromatic2diatonic(_transpose.chromatic);
                  }
            else if (tag == "transposeChromatic")
                  _transpose.chromatic = e.readInt();
            else if (tag == "transposeDiatonic")
                  _transpose.diatonic = e.readInt();
            else if (tag == "useDrumset") {
                  int drumset = e.readInt();
                  if (!drumset)
                        _useDrumset = DrumsetKind::NONE;
                  else  {
                        _useDrumset = DrumsetKind::DEFAULT_DRUMS;
                         _drumset = new Drumset(*smDrumset);
                        }
                  }
            else if (tag == "Drum") {
                  // if we see on of this tags, a custom drumset will
                  // be created
                  if (_drumset == 0)
                        _drumset = new Drumset(*smDrumset);
                  if (!customDrumset) {
                        _drumset->clear();
                        customDrumset = true;
                        }
                  _drumset->load(e);
                  }
            // support tag "Tablature" for a while for compatibility with existent 2.0 scores
            else if (tag == "Tablature" || tag == "StringData")
                  _stringData.read(e);
            else if (tag == "MidiAction") {
                  NamedEventList a;
                  a.read(e);
                  _midiActions.append(a);
                  }
            else if (tag == "Articulation") {
                  MidiArticulation a;
                  a.read(e);
                  _articulation.append(a);
                  }
            else if (tag == "Channel" || tag == "channel") {
                  Channel a;
                  a.read(e);
                  _channel.append(a);
                  }
            else if (tag == "clef") {           // sets both transposing and concert clef
                  int idx = e.intAttribute("staff", 1) - 1;
                  QString val(e.readElementText());
                  ClefType ct = Clef::clefType(val);
                  setClefType(idx, ClefTypeList(ct, ct));
                  }
            else if (tag == "concertClef") {
                  int idx = e.intAttribute("staff", 1) - 1;
                  QString val(e.readElementText());
                  setClefType(idx, ClefTypeList(Clef::clefType(val), clefType(idx)._transposingClef));
                  }
            else if (tag == "transposingClef") {
                  int idx = e.intAttribute("staff", 1) - 1;
                  QString val(e.readElementText());
                  setClefType(idx, ClefTypeList(clefType(idx)._concertClef, Clef::clefType(val)));
                  }

            else if (tag == "chorus")           // obsolete
                  chorus = e.readInt();
            else if (tag == "reverb")           // obsolete
                  reverb = e.readInt();
            else if (tag == "midiProgram")      // obsolete
                  program = e.readInt();
            else if (tag == "volume")           // obsolete
                  volume = e.readInt();
            else if (tag == "pan")              // obsolete
                  pan = e.readInt();
            else if (tag == "midiChannel")      // obsolete
                  e.skipCurrentElement();
            else
                  e.unknown();
            }
      if (_channel.isEmpty()) {      // for backward compatibility
            Channel a;
            a.chorus  = chorus;
            a.reverb  = reverb;
            a.name    = "normal";
            a.program = program;
            a.bank    = bank;
            a.volume  = volume;
            a.pan     = pan;
            _channel.append(a);
            }
      if (_useDrumset != DrumsetKind::NONE) {
            if (_channel[0].bank == 0)
                  _channel[0].bank = 128;
            _channel[0].updateInitList();
            }
      }

//---------------------------------------------------------
//   action
//---------------------------------------------------------

NamedEventList* InstrumentData::midiAction(const QString& s, int channelIdx) const
      {
      // first look in channel list

      foreach(const NamedEventList& a, _channel[channelIdx].midiActions) {
            if (s == a.name)
                  return const_cast<NamedEventList*>(&a);
            }

      foreach(const NamedEventList& a, _midiActions) {
            if (s == a.name)
                  return const_cast<NamedEventList*>(&a);
            }
      return 0;
      }

//---------------------------------------------------------
//   Channel
//---------------------------------------------------------

Channel::Channel()
      {
      for(int i = 0; i < int(A::INIT_COUNT); ++i)
            init.push_back(MidiCoreEvent());
      synti    = "Fluid";     // default synthesizer
      channel  = -1;
      program  = -1;
      bank     = 0;
      volume   = 100;
      pan      = 64;
      chorus   = 0;
      reverb   = 0;

      mute     = false;
      solo     = false;
      soloMute = false;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Channel::write(Xml& xml) const
      {
      if (name.isEmpty() || name == "normal")
            xml.stag("Channel");
      else
            xml.stag(QString("Channel name=\"%1\"").arg(name));
      if (!descr.isEmpty())
            xml.tag("descr", descr);
      updateInitList();
      foreach(const MidiCoreEvent& e, init) {
            if (e.type() == ME_INVALID)
                  continue;
            if (e.type() == ME_CONTROLLER) {
                  if (e.dataA() == CTRL_HBANK && e.dataB() == 0)
                        continue;
                  if (e.dataA() == CTRL_LBANK && e.dataB() == 0)
                        continue;
                  if (e.dataA() == CTRL_VOLUME && e.dataB() == 100)
                        continue;
                  if (e.dataA() == CTRL_PANPOT && e.dataB() == 64)
                        continue;
                  if (e.dataA() == CTRL_REVERB_SEND && e.dataB() == 0)
                        continue;
                  if (e.dataA() == CTRL_CHORUS_SEND && e.dataB() == 0)
                        continue;
                  }

            e.write(xml);
            }
      if (!MScore::testMode)
            // xml.tag("synti", ::synti->name(synti));
            xml.tag("synti", synti);
      if (mute)
            xml.tag("mute", mute);
      if (solo)
            xml.tag("solo", solo);
      foreach(const NamedEventList& a, midiActions)
            a.write(xml, "MidiAction");
      foreach(const MidiArticulation& a, articulation)
            a.write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Channel::read(XmlReader& e)
      {
      // synti = 0;
      name = e.attribute("name");
      if (name == "")
            name = "normal";
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "program") {
                  program = e.intAttribute("value", -1);
                  if (program == -1)
                        program = e.readInt();
                  else
                        e.readNext();
                  }
            else if (tag == "controller") {
                  int value = e.intAttribute("value", 0);
                  int ctrl  = e.intAttribute("ctrl", 0);
                  switch (ctrl) {
                        case CTRL_HBANK:
                              bank = (value << 7) + (bank & 0x7f);
                              break;
                        case CTRL_LBANK:
                              bank = (bank & ~0x7f) + (value & 0x7f);
                              break;
                        case CTRL_VOLUME:
                              volume = value;
                              break;
                        case CTRL_PANPOT:
                              pan = value;
                              break;
                        case CTRL_CHORUS_SEND:
                              chorus = value;
                              break;
                        case CTRL_REVERB_SEND:
                              reverb = value;
                              break;
                        default:
                              {
                              Event e(ME_CONTROLLER);
                              e.setOntime(-1);
                              e.setChannel(0);
                              e.setDataA(ctrl);
                              e.setDataB(value);
                              init.push_back(e);
                              }
                              break;
                        }
                  e.readNext();
                  }
            else if (tag == "Articulation") {
                  MidiArticulation a;
                  a.read(e);
                  articulation.append(a);
                  }
            else if (tag == "MidiAction") {
                  NamedEventList a;
                  a.read(e);
                  midiActions.append(a);
                  }
            else if (tag == "synti")
                  synti = e.readElementText();
            else if (tag == "descr")
                  descr = e.readElementText();
            else if (tag == "mute")
                  mute = e.readInt();
            else if (tag == "solo")
                  solo = e.readInt();
            else
                  e.unknown();
            }
      updateInitList();
      }

//---------------------------------------------------------
//   updateInitList
//---------------------------------------------------------

void Channel::updateInitList() const
      {
      MidiCoreEvent e;
      if (program != -1) {
            e.setType(ME_CONTROLLER);
            e.setDataA(CTRL_PROGRAM);
            e.setDataB(program);
            init[int(A::PROGRAM)] = e;
            }

      e.setData(ME_CONTROLLER, CTRL_HBANK, (bank >> 7) & 0x7f);
      init[int(A::HBANK)] = e;

      e.setData(ME_CONTROLLER, CTRL_LBANK, bank & 0x7f);
      init[int(A::LBANK)] = e;

      e.setData(ME_CONTROLLER, CTRL_VOLUME, volume);
      init[int(A::VOLUME)] = e;

      e.setData(ME_CONTROLLER, CTRL_PANPOT, pan);
      init[int(A::PAN)] = e;

      e.setData(ME_CONTROLLER, CTRL_CHORUS_SEND, chorus);
      init[int(A::CHORUS)] = e;

      e.setData(ME_CONTROLLER, CTRL_REVERB_SEND, reverb);
      init[int(A::REVERB)] = e;
      }

//---------------------------------------------------------
//   channelIdx
//---------------------------------------------------------

int InstrumentData::channelIdx(const QString& s) const
      {
      int idx = 0;
      foreach(const Channel& a, _channel) {
            if (a.name.isEmpty() && s == "normal")
                  return idx;
            if (s == a.name)
                  return idx;
            ++idx;
            }
      return -1;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void MidiArticulation::write(Xml& xml) const
      {
      if (name.isEmpty())
            xml.stag("Articulation");
      else
            xml.stag(QString("Articulation name=\"%1\"").arg(name));
      if (!descr.isEmpty())
            xml.tag("descr", descr);
      xml.tag("velocity", velocity);
      xml.tag("gateTime", gateTime);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void MidiArticulation::read(XmlReader& e)
      {
      name = e.attribute("name");
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "velocity") {
                  QString text(e.readElementText());
                  if (text.endsWith("%"))
                        text = text.left(text.size()-1);
                  velocity = text.toInt();
                  }
            else if (tag == "gateTime") {
                  QString text(e.readElementText());
                  if (text.endsWith("%"))
                        text = text.left(text.size()-1);
                  gateTime = text.toInt();
                  }
            else if (tag == "descr")
                  descr = e.readElementText();
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   updateVelocity
//---------------------------------------------------------

void InstrumentData::updateVelocity(int* velocity, int /*channelIdx*/, const QString& name)
      {
      foreach(const MidiArticulation& a, _articulation) {
            if (a.name == name) {
                  *velocity = *velocity * a.velocity / 100;
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   updateGateTime
//---------------------------------------------------------

void InstrumentData::updateGateTime(int* gateTime, int /*channelIdx*/, const QString& name)
      {
      foreach(const MidiArticulation& a, _articulation) {
            if (a.name == name) {
                  *gateTime = a.gateTime;
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool InstrumentData::operator==(const InstrumentData& i) const
      {
      int n = _longNames.size();
      if (i._longNames.size() != n)
            return false;
      for (int k = 0; k < n; ++k) {
            if (!(i._longNames[k] == _longNames[k]))
                  return false;
            }
      n = _shortNames.size();
      if (i._shortNames.size() != n)
            return false;
      for (int k = 0; k < n; ++k) {
            if (!(i._shortNames[k] == _shortNames[k].name))
                  return false;
            }
      return i._minPitchA == _minPitchA
         &&  i._maxPitchA == _maxPitchA
         &&  i._minPitchP == _minPitchP
         &&  i._maxPitchP == _maxPitchP
         &&  i._useDrumset == _useDrumset
         &&  i._midiActions == _midiActions
         &&  i._channel == _channel
         &&  i._articulation == _articulation
         &&  i._transpose.diatonic == _transpose.diatonic
         &&  i._transpose.chromatic == _transpose.chromatic
         &&  i._trackName == _trackName
         &&  *i.stringData() == *stringData();
         ;
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool StaffName::operator==(const StaffName& i) const
      {
      return (i.pos == pos) && (i.name == name);
      }

//---------------------------------------------------------
//   setUseDrumset
//---------------------------------------------------------

void InstrumentData::setUseDrumset(DrumsetKind val)
      {
      _useDrumset = val;
      if (val != DrumsetKind::NONE && _drumset == 0) {
            switch (val) {
                  case DrumsetKind::DEFAULT_DRUMS:
                        _drumset = new Drumset(*smDrumset);
                        break;
                  case DrumsetKind::GUITAR_PRO:
                        _drumset = new Drumset(*gpDrumset);
                        break;
                  default:
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   setDrumset
//---------------------------------------------------------

void InstrumentData::setDrumset(Drumset* ds)
      {
      delete _drumset;
      if (ds)
            _drumset = new Drumset(*ds);
      else
            _drumset = 0;
      }

//---------------------------------------------------------
//   setLongName
//---------------------------------------------------------

void InstrumentData::setLongName(const QString& f)
      {
      _longNames.clear();
      if (f.length() > 0)
            _longNames.append(StaffName(f, 0));
      }

//---------------------------------------------------------
//   setShortName
//---------------------------------------------------------

void InstrumentData::setShortName(const QString& f)
      {
      _shortNames.clear();
      if (f.length() > 0)
            _shortNames.append(StaffName(f, 0));
      }

//---------------------------------------------------------
//   addLongName
//---------------------------------------------------------

void InstrumentData::addLongName(const StaffName& f)
      {
      _longNames.append(f);
      }

//---------------------------------------------------------
//   addShortName
//---------------------------------------------------------

void InstrumentData::addShortName(const StaffName& f)
      {
      _shortNames.append(f);
      }

//---------------------------------------------------------
//   clefType
//---------------------------------------------------------

ClefTypeList InstrumentData::clefType(int staffIdx) const
      {
      if (staffIdx >= _clefType.size()) {
            if (_clefType.isEmpty())
                  return ClefTypeList(staffIdx == 1 ? ClefType::F : ClefType::G);
            return _clefType[0];
            }
      return _clefType[staffIdx];
      }

//---------------------------------------------------------
//   setClefType
//---------------------------------------------------------

void InstrumentData::setClefType(int staffIdx, const ClefTypeList& c)
      {
      while (_clefType.size() <= staffIdx)
            _clefType.append(ClefTypeList());
      _clefType[staffIdx] = c;
      }

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

Instrument::Instrument()
      {
      d = new InstrumentData;
      }

Instrument::Instrument(const Instrument& s)
   : d(s.d)
      {
      }

Instrument::~Instrument()
      {
      }

//---------------------------------------------------------
//   operator=
//---------------------------------------------------------

Instrument& Instrument::operator=(const Instrument& s)
      {
      d = s.d;
      return *this;
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool Instrument::operator==(const Instrument& s) const
      {
      return d->operator==(*s.d);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Instrument::read(XmlReader& e)
      {
      d->read(e);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Instrument::write(Xml& xml) const
      {
      d->write(xml);
      }

//---------------------------------------------------------
//   midiAction
//---------------------------------------------------------

NamedEventList* Instrument::midiAction(const QString& s, int channel) const
      {
      return d->midiAction(s, channel);
      }

//---------------------------------------------------------
//   channelIdx
//---------------------------------------------------------

int Instrument::channelIdx(const QString& s) const
      {
      return d->channelIdx(s);
      }

//---------------------------------------------------------
//   updateVelocity
//---------------------------------------------------------

void Instrument::updateVelocity(int* velocity, int channel, const QString& name)
      {
      d->updateVelocity(velocity, channel, name);
      }

//---------------------------------------------------------
//   updateGateTime
//---------------------------------------------------------

void Instrument::updateGateTime(int* gateTime, int channel, const QString& name)
      {
      d->updateGateTime(gateTime, channel, name);
      }

//---------------------------------------------------------
//   minPitchP
//---------------------------------------------------------

int Instrument::minPitchP() const
      {
      return d->_minPitchP;
      }

//---------------------------------------------------------
//   maxPitchP
//---------------------------------------------------------

int Instrument::maxPitchP() const
      {
      return d->_maxPitchP;
      }

//---------------------------------------------------------
//   minPitchA
//---------------------------------------------------------

int Instrument::minPitchA() const
      {
      return d->_minPitchA;
      }

//---------------------------------------------------------
//   maxPitchA
//---------------------------------------------------------

int Instrument::maxPitchA() const
      {
      return d->_maxPitchA;
      }

//---------------------------------------------------------
//   setMinPitchP
//---------------------------------------------------------

void Instrument::setMinPitchP(int v)
      {
      d->setMinPitchP(v);
      }

//---------------------------------------------------------
//   setMaxPitchP
//---------------------------------------------------------

void Instrument::setMaxPitchP(int v)
      {
      d->setMaxPitchP(v);
      }

//---------------------------------------------------------
//   setMinPitchA
//---------------------------------------------------------

void Instrument::setMinPitchA(int v)
      {
      d->setMinPitchA(v);
      }

//---------------------------------------------------------
//   setMaxPitchA
//---------------------------------------------------------

void Instrument::setMaxPitchA(int v)
      {
      d->setMaxPitchA(v);
      }

//---------------------------------------------------------
//   transpose
//---------------------------------------------------------

Interval Instrument::transpose() const
      {
      return d->transpose();
      }

//---------------------------------------------------------
//   setTranspose
//---------------------------------------------------------

void Instrument::setTranspose(const Interval& v)
      {
      d->setTranspose(v);
      }

//---------------------------------------------------------
//   setDrumset
//---------------------------------------------------------

void Instrument::setDrumset(Drumset* ds)
      {
      d->setDrumset(ds);
      }

//---------------------------------------------------------
//   drumset
//---------------------------------------------------------

Drumset* Instrument::drumset() const
      {
      return d->drumset();
      }

//---------------------------------------------------------
//   useDrumset
//---------------------------------------------------------

DrumsetKind Instrument::useDrumset() const
      {
      return d->useDrumset();
      }

//---------------------------------------------------------
//   setUseDrumset
//---------------------------------------------------------

void Instrument::setUseDrumset(DrumsetKind val)
      {
      d->setUseDrumset(val);
      }

//---------------------------------------------------------
//   setAmateurPitchRange
//---------------------------------------------------------

void Instrument::setAmateurPitchRange(int a, int b)
      {
      d->setAmateurPitchRange(a, b);
      }

//---------------------------------------------------------
//   setProfessionalPitchRange
//---------------------------------------------------------

void Instrument::setProfessionalPitchRange(int a, int b)
      {
      d->setProfessionalPitchRange(a, b);
      }

//---------------------------------------------------------
//   channel
//---------------------------------------------------------

Channel& Instrument::channel(int idx)
      {
      return d->channel(idx);
      }

//---------------------------------------------------------
//   channel
//---------------------------------------------------------

const Channel& Instrument::channel(int idx) const
      {
      return d->channel(idx);
      }

//---------------------------------------------------------
//   midiActions
//---------------------------------------------------------

const QList<NamedEventList>& Instrument::midiActions() const
      {
      return d->midiActions();
      }

//---------------------------------------------------------
//   articulation
//---------------------------------------------------------

const QList<MidiArticulation>& Instrument::articulation() const
      {
      return d->articulation();
      }

//---------------------------------------------------------
//   channel
//---------------------------------------------------------

const QList<Channel>& Instrument::channel() const
      {
      return d->channel();
      }

//---------------------------------------------------------
//   setMidiActions
//---------------------------------------------------------

void Instrument::setMidiActions(const QList<NamedEventList>& l)
      {
      d->setMidiActions(l);
      }

//---------------------------------------------------------
//   setArticulation
//---------------------------------------------------------

void Instrument::setArticulation(const QList<MidiArticulation>& l)
      {
      d->setArticulation(l);
      }

//---------------------------------------------------------
//   setChannel
//---------------------------------------------------------

void Instrument::setChannel(const QList<Channel>& l)
      {
      d->setChannel(l);
      }

//---------------------------------------------------------
//   setChannel
//---------------------------------------------------------

void Instrument::setChannel(int i, const Channel& c)
      {
      d->setChannel(i, c);
      }

//---------------------------------------------------------
//   clefType
//---------------------------------------------------------

ClefTypeList Instrument::clefType(int staffIdx) const
      {
      return d->clefType(staffIdx);
      }

//---------------------------------------------------------
//   setClefType
//---------------------------------------------------------

void Instrument::setClefType(int staffIdx, const ClefTypeList& cl)
      {
      d->setClefType(staffIdx, cl);
      }

//---------------------------------------------------------
//   tablature
//---------------------------------------------------------

const StringData* Instrument::stringData() const
      {
      return d->stringData();
      }

//---------------------------------------------------------
//   setTablature
//---------------------------------------------------------

void Instrument::setStringData(const StringData& t)
      {
      d->setStringData(t);
      }

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

const Instrument& InstrumentList::instrument(int tick) const
      {
      if (empty())
            return defaultInstrument;
      auto i = upper_bound(tick);
      if (i == begin())
            return defaultInstrument;
      --i;
      return i->second;
      }

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

Instrument& InstrumentList::instrument(int tick)
      {
      if (empty())
            return defaultInstrument;
      auto i = upper_bound(tick);
      if (i == begin())
            return defaultInstrument;
      --i;
      return i->second;
      }

//---------------------------------------------------------
//   setInstrument
//---------------------------------------------------------

void InstrumentList::setInstrument(const Instrument& instr, int tick)
      {
      if (!insert({tick, instr}).second)
            (*this)[tick] = instr;
      }

//---------------------------------------------------------
//   longName
//---------------------------------------------------------

const QList<StaffName>& Instrument::longNames() const
      {
      return d->_longNames;
      }

//---------------------------------------------------------
//   shortName
//---------------------------------------------------------

const QList<StaffName>& Instrument::shortNames() const
      {
      return d->_shortNames;
      }

//---------------------------------------------------------
//   longName
//---------------------------------------------------------

QList<StaffName>& Instrument::longNames()
      {
      return d->_longNames;
      }

//---------------------------------------------------------
//   setLongName
//---------------------------------------------------------

void Instrument::setLongName(const QString& f)
      {
      d->setLongName(f);
      }

//---------------------------------------------------------
//   addLongName
//---------------------------------------------------------

void Instrument::addLongName(const StaffName& f)
      {
      d->addLongName(f);
      }

//---------------------------------------------------------
//   addShortName
//---------------------------------------------------------

void Instrument::addShortName(const StaffName& f)
      {
      d->addShortName(f);
      }

//---------------------------------------------------------
//   setShortName
//---------------------------------------------------------

void Instrument::setShortName(const QString& f)
      {
      d->setShortName(f);
      }

//---------------------------------------------------------
//   shortName
//---------------------------------------------------------

QList<StaffName>& Instrument::shortNames()
      {
      return d->_shortNames;
      }

//---------------------------------------------------------
//   trackName
//---------------------------------------------------------

QString Instrument::trackName() const
      {
      return d->_trackName;
      }

void Instrument::setTrackName(const QString& s)
      {
      d->_trackName = s;
      }

//---------------------------------------------------------
//   fromTemplate
//---------------------------------------------------------

Instrument Instrument::fromTemplate(const InstrumentTemplate* t)
      {
      Instrument instr;
      instr.setAmateurPitchRange(t->minPitchA, t->maxPitchA);
      instr.setProfessionalPitchRange(t->minPitchP, t->maxPitchP);
      foreach(StaffName sn, t->longNames)
            instr.addLongName(StaffName(sn.name, sn.pos));
      foreach(StaffName sn, t->shortNames)
            instr.addShortName(StaffName(sn.name, sn.pos));
      instr.setTrackName(t->trackName);
      instr.setTranspose(t->transpose);
      if (t->useDrumset != DrumsetKind::NONE) {
            instr.setUseDrumset(DrumsetKind::DEFAULT_DRUMS);
            instr.setDrumset(new Drumset(*((t->drumset) ? t->drumset : smDrumset)));
            }
      for (int i = 0; i < t->nstaves(); ++i)
            instr.setClefType(i, t->clefTypes[i]);
      instr.setMidiActions(t->midiActions);
      instr.setArticulation(t->articulation);
      instr.setChannel(t->channel);
      instr.setStringData(t->stringData);
      return instr;
      }
}

