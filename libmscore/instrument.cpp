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

#include "instrument.h"
#include "xml.h"
#include "drumset.h"
#include "articulation.h"
#include "utils.h"
#include "stringdata.h"
#include "instrtemplate.h"
#include "mscore.h"
#include "part.h"
#include "score.h"

#include "audio/midi/synthesizer.h"
#include "audio/midi/midipatch.h"

namespace Ms {

//: Channel name for otherwise unamed channels
const char* Channel::DEFAULT_NAME = QT_TRANSLATE_NOOP("InstrumentsXML", "normal");
//: Channel name for the chord symbols playback channel, best keep translation shorter than 11 letters
const char* Channel::HARMONY_NAME = QT_TRANSLATE_NOOP("InstrumentsXML", "harmony");

Instrument InstrumentList::defaultInstrument;
const std::initializer_list<Channel::Prop> PartChannelSettingsLink::excerptProperties {
      Channel::Prop::SOLOMUTE,
      Channel::Prop::SOLO,
      Channel::Prop::MUTE,
      };

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void NamedEventList::write(XmlWriter& xml, const QString& n) const
      {
      xml.stag(QString("%1 name=\"%2\"").arg(n, name));
      if (!descr.isEmpty())
            xml.tag("descr", descr);
      for (const MidiCoreEvent& e : events)
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

Instrument::Instrument(QString id)
      {
      _id = id;
      Channel* a = new Channel;
      a->setName(Channel::DEFAULT_NAME);
      _channel.append(a);

      _minPitchA   = 0;
      _maxPitchA   = 127;
      _minPitchP   = 0;
      _maxPitchP   = 127;
      _useDrumset  = false;
      _drumset     = 0;
      _singleNoteDynamics = true;
      }

Instrument::Instrument(const Instrument& i)
      {
      _id           = i._id;
      _longNames    = i._longNames;
      _shortNames   = i._shortNames;
      _trackName    = i._trackName;
      _minPitchA    = i._minPitchA;
      _maxPitchA    = i._maxPitchA;
      _minPitchP    = i._minPitchP;
      _maxPitchP    = i._maxPitchP;
      _transpose    = i._transpose;
      _instrumentId = i._instrumentId;
      _stringData   = i._stringData;
      _drumset      = 0;
      setDrumset(i._drumset);
      _useDrumset   = i._useDrumset;
      _stringData   = i._stringData;
      _midiActions  = i._midiActions;
      _articulation = i._articulation;
      _singleNoteDynamics = i._singleNoteDynamics;
      for (Channel* c : i._channel)
            _channel.append(new Channel(*c));
      _clefType     = i._clefType;
      }

void Instrument::operator=(const Instrument& i)
      {
      qDeleteAll(_channel);
      _channel.clear();
      delete _drumset;

      _id           = i._id;
      _longNames    = i._longNames;
      _shortNames   = i._shortNames;
      _trackName    = i._trackName;
      _minPitchA    = i._minPitchA;
      _maxPitchA    = i._maxPitchA;
      _minPitchP    = i._minPitchP;
      _maxPitchP    = i._maxPitchP;
      _transpose    = i._transpose;
      _instrumentId = i._instrumentId;
      _stringData   = i._stringData;
      _drumset      = 0;
      setDrumset(i._drumset);
      _useDrumset   = i._useDrumset;
      _stringData   = i._stringData;
      _midiActions  = i._midiActions;
      _articulation = i._articulation;
      _singleNoteDynamics = i._singleNoteDynamics;
      for (Channel* c : i._channel)
            _channel.append(new Channel(*c));
      _clefType     = i._clefType;
      }

//---------------------------------------------------------
//   ~Instrument
//---------------------------------------------------------

Instrument::~Instrument()
      {
      qDeleteAll(_channel);
      delete _drumset;
      }

//---------------------------------------------------------
//   StaffName
//---------------------------------------------------------

StaffName::StaffName(const QString& s, int p) : _name(s), _pos(p)
      {
      Text::validateText(_name); // enforce HTML encoding
      }

//---------------------------------------------------------
//   StaffName::write
//---------------------------------------------------------

void StaffName::write(XmlWriter& xml, const char* tag) const
      {
      if (!name().isEmpty()) {
            if (pos() == 0)
                  xml.writeXml(QString("%1").arg(tag), name());
            else
                  xml.writeXml(QString("%1 pos=\"%2\"").arg(tag).arg(pos()), name());
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffName::read(XmlReader& e)
      {
      _pos  = e.intAttribute("pos", 0);
      _name = e.readXml();
      if (_name.startsWith("<html>")) {
            // compatibility to old html implementation:
            _name = QTextDocumentFragment::fromHtml(_name).toPlainText();
            }
      }

//---------------------------------------------------------
//   Instrument::write
//---------------------------------------------------------

void Instrument::write(XmlWriter& xml, const Part* part) const
      {
      if (_id.isEmpty())
            xml.stag("Instrument");
      else
            xml.stag(QString("Instrument id=\"%1\"").arg(_id));
      _longNames.write(xml, "longName");
      _shortNames.write(xml, "shortName");
//      if (!_trackName.empty())
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
      if (!_instrumentId.isEmpty())
            xml.tag("instrumentId", _instrumentId);
      if (_useDrumset) {
            xml.tag("useDrumset", _useDrumset);
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

      if (_singleNoteDynamics != getSingleNoteDynamicsFromTemplate())
            xml.tag("singleNoteDynamics", _singleNoteDynamics);

      if (!(_stringData == StringData()))
            _stringData.write(xml);
      for (const NamedEventList& a : _midiActions)
            a.write(xml, "MidiAction");
      for (const MidiArticulation& a : _articulation)
            a.write(xml);
      for (const Channel* a : _channel)
            a->write(xml, part);
      xml.etag();
      }

QString Instrument::recognizeInstrumentId() const
      {
      static QString defaultInstrumentId("keyboard.piano");

      QList<QString> nameList;

      nameList << _trackName;
      nameList << _longNames.toStringList();
      nameList << _shortNames.toStringList();

      InstrumentTemplate* tmplByName = Ms::searchTemplateForInstrNameList(nameList);

      if (tmplByName && !tmplByName->musicXMLid.isEmpty())
            return tmplByName->musicXMLid;

      if (!channel(0))
            return defaultInstrumentId;

      InstrumentTemplate* tmplMidiProgram = Ms::searchTemplateForMidiProgram(channel(0)->program(), useDrumset());

      if (tmplMidiProgram && !tmplMidiProgram->musicXMLid.isEmpty())
            return tmplMidiProgram->musicXMLid;

      InstrumentTemplate* guessedTmpl = Ms::guessTemplateByNameData(nameList);

      if (guessedTmpl && !guessedTmpl->musicXMLid.isEmpty())
            return guessedTmpl->musicXMLid;

      return defaultInstrumentId;
      }

int Instrument::recognizeMidiProgram() const
      {
      InstrumentTemplate* tmplInstrumentId = Ms::searchTemplateForMusicXmlId(_instrumentId);

      if (tmplInstrumentId && !tmplInstrumentId->channel.isEmpty() && tmplInstrumentId->channel[0].program() >= 0)
            return tmplInstrumentId->channel[0].program();

      QList<QString> nameList;

      nameList << _trackName;
      nameList << _longNames.toStringList();
      nameList << _shortNames.toStringList();

      InstrumentTemplate* tmplByName = Ms::searchTemplateForInstrNameList(nameList);

      if (tmplByName && !tmplByName->channel.isEmpty() && tmplByName->channel[0].program() >= 0)
            return tmplByName->channel[0].program();

      InstrumentTemplate* guessedTmpl = Ms::guessTemplateByNameData(nameList);

      if (guessedTmpl && !guessedTmpl->channel.isEmpty() && guessedTmpl->channel[0].program() >= 0)
            return guessedTmpl->channel[0].program();

      return 0;
      }

//---------------------------------------------------------
//   Instrument::read
//---------------------------------------------------------

void Instrument::read(XmlReader& e, Part* part)
      {
      bool customDrumset = false;
      bool readSingleNoteDynamics = false;

      _channel.clear();       // remove default channel
      _id = e.attribute("id");
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "singleNoteDynamics") {
                  _singleNoteDynamics = e.readBool();
                  readSingleNoteDynamics = true;
                  }
            else if (!readProperties(e, part, &customDrumset))
                  e.unknown();
            }

      if (_instrumentId.isEmpty())
            _instrumentId = recognizeInstrumentId();

      if (channel(0) && channel(0)->program() == -1) {
          channel(0)->setProgram(recognizeMidiProgram());
      }

      if (!readSingleNoteDynamics)
            setSingleNoteDynamicsFromTemplate();

      if (_useDrumset) {
            if (_channel[0]->bank() == 0 && _channel[0]->synti().toLower() != "zerberus")
                  _channel[0]->setBank(128);
            }
      }

//---------------------------------------------------------
//   Instrument::readProperties
//---------------------------------------------------------

bool Instrument::readProperties(XmlReader& e, Part* part, bool* customDrumset)
      {
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
      else if (tag == "instrumentId")
            _instrumentId = e.readElementText();
      else if (tag == "useDrumset") {
            _useDrumset = e.readInt();
            if (_useDrumset) {
                  delete _drumset;
                  _drumset = new Drumset(*smDrumset);
                  }
            }
      else if (tag == "Drum") {
            // if we see on of this tags, a custom drumset will
            // be created
            if (!_drumset)
                  _drumset = new Drumset(*smDrumset);
            if (!(*customDrumset)) {
                  const_cast<Drumset*>(_drumset)->clear();
                  *customDrumset = true;
                  }
            const_cast<Drumset*>(_drumset)->load(e);
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
            Channel* a = new Channel;
            a->read(e, part);
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
      else
            return false;

      return true;
      }

//---------------------------------------------------------
//   action
//---------------------------------------------------------

NamedEventList* Instrument::midiAction(const QString& s, int channelIdx) const
      {
      // first look in channel list

      foreach(const NamedEventList& a, _channel[channelIdx]->midiActions) {
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
            _init.push_back(MidiCoreEvent());
      _synti    = "Fluid";     // default synthesizer
      _channel  = -1;
      _program  = -1;
      _bank     = 0;
      _volume   = defaultVolume;
      _pan      = 64; // actually 63.5 for center
      _chorus   = 0;
      _reverb   = 0;
      _color = DEFAULT_COLOR;

      _mute     = false;
      _solo     = false;
      _soloMute = false;

//      qDebug("construct Channel ");
      }

//---------------------------------------------------------
//   initList
//---------------------------------------------------------

std::vector<MidiCoreEvent>& Channel::initList() const
      {
      if (_mustUpdateInit) {
            updateInitList();
            _mustUpdateInit = false;
            }
      return _init;
      }

//---------------------------------------------------------
//   setVolume
//---------------------------------------------------------

void Channel::setVolume(char value)
      {
      if (_volume != value) {
            _volume = value;
            firePropertyChanged(Prop::VOLUME);
            }
      _mustUpdateInit = true;
      }

//---------------------------------------------------------
//   setPan
//---------------------------------------------------------

void Channel::setPan(char value)
      {
      if (_pan != value) {
            _pan = value;
            firePropertyChanged(Prop::PAN);
            }
      _mustUpdateInit = true;
      }

//---------------------------------------------------------
//   setChorus
//---------------------------------------------------------

void Channel::setChorus(char value)
      {
      if (_chorus != value) {
            _chorus = value;
            firePropertyChanged(Prop::CHORUS);
            }
      _mustUpdateInit = true;
      }

//---------------------------------------------------------
//   setReverb
//---------------------------------------------------------

void Channel::setReverb(char value)
      {
      if (_reverb != value) {
            _reverb = value;
            firePropertyChanged(Prop::REVERB);
            }
      _mustUpdateInit = true;
      }

//---------------------------------------------------------
//   setName
//---------------------------------------------------------

void Channel::setName(const QString& value)
      {
      if (_name != value) {
            _name = value;
            firePropertyChanged(Prop::NAME);
            }
      }

//---------------------------------------------------------
//   setDescr
//---------------------------------------------------------

void Channel::setDescr(const QString& value)
      {
      if (_descr != value) {
            _descr = value;
            firePropertyChanged(Prop::DESCR);
            }
      }

//---------------------------------------------------------
//   setSynti
//---------------------------------------------------------

void Channel::setSynti(const QString& value)
      {
      if (_synti != value) {
            _synti = value;
            firePropertyChanged(Prop::SYNTI);
            }
      }

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void Channel::setColor(int value)
      {
      if (_color != value) {
            _color = value;
            firePropertyChanged(Prop::COLOR);
            }
      }

//---------------------------------------------------------
//   setProgram
//---------------------------------------------------------

void Channel::setProgram(int value)
      {
      if (_program != value) {
            _program = value;
            firePropertyChanged(Prop::PROGRAM);
            }
      _mustUpdateInit = true;
      }

//---------------------------------------------------------
//   setBank
//---------------------------------------------------------

void Channel::setBank(int value)
      {
      if (_bank != value) {
            _bank = value;
            firePropertyChanged(Prop::BANK);
            }
      _mustUpdateInit = true;
      }

//---------------------------------------------------------
//   setChannel
//---------------------------------------------------------

void Channel::setChannel(int value)
      {
      if (_channel != value) {
            _channel = value;
            firePropertyChanged(Prop::CHANNEL);
            }
      }

//---------------------------------------------------------
//   setSoloMute
//---------------------------------------------------------

void Channel::setSoloMute(bool value)
      {
      if (_soloMute != value) {
            _soloMute = value;
            firePropertyChanged(Prop::SOLOMUTE);
            }
      }

//---------------------------------------------------------
//   setMute
//---------------------------------------------------------

void Channel::setMute(bool value)
      {
      if (_mute != value) {
            _mute = value;
            firePropertyChanged(Prop::MUTE);
            }
      }

//---------------------------------------------------------
//   setSolo
//---------------------------------------------------------

void Channel::setSolo(bool value)
      {
      if (_solo != value) {
            _solo = value;
            firePropertyChanged(Prop::SOLO);
            }
      }

//---------------------------------------------------------
//   setUserBankController
//---------------------------------------------------------

void Channel::setUserBankController(bool val)
      {
      if (_userBankController != val) {
            _userBankController = val;
            firePropertyChanged(Prop::USER_BANK_CONTROL);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Channel::write(XmlWriter& xml, const Part* part) const
      {
      if (_name.isEmpty() || _name == DEFAULT_NAME)
            xml.stag("Channel");
      else
            xml.stag(QString("Channel name=\"%1\"").arg(_name));
      if (!_descr.isEmpty())
            xml.tag("descr", _descr);
      if (_color != DEFAULT_COLOR)
            xml.tag("color", _color);

      for (const MidiCoreEvent& e : initList()) {
            if (e.type() == ME_INVALID)
                  continue;
            if (e.type() == ME_CONTROLLER) {
                  // Don't write bank if automatically switched, but always write if switched by the user
                  if (e.dataA() == CTRL_HBANK && e.dataB() == 0 && !_userBankController)
                        continue;
                  if (e.dataA() == CTRL_LBANK && e.dataB() == 0 && !_userBankController)
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
            xml.tag("synti", _synti);
      if (_mute)
            xml.tag("mute", _mute);
      if (_solo)
            xml.tag("solo", _solo);

      if (part && part->masterScore()->exportMidiMapping() && part->score() == part->masterScore()) {
            xml.tag("midiPort",    part->masterScore()->midiMapping(_channel)->port());
            xml.tag("midiChannel", part->masterScore()->midiMapping(_channel)->channel());
            }
      for (const NamedEventList& a : midiActions)
            a.write(xml, "MidiAction");
      for (const MidiArticulation& a : articulation)
            a.write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Channel::read(XmlReader& e, Part* part)
      {
      // synti = 0;
      _name = e.attribute("name");
      if (_name == "")
            _name = DEFAULT_NAME;

      int midiPort = -1;
      int midiChannel = -1;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "program") {
                  _program = e.intAttribute("value", -1);
                  if (_program == -1)
                        _program = e.readInt();
                  else
                        e.readNext();
                  }
            else if (tag == "controller") {
                  int value = e.intAttribute("value", 0);
                  int ctrl  = e.intAttribute("ctrl", 0);
                  switch (ctrl) {
                        case CTRL_HBANK:
                              _bank = (value << 7) + (_bank & 0x7f);
                              _userBankController = true;
                              break;
                        case CTRL_LBANK:
                              _bank = (_bank & ~0x7f) + (value & 0x7f);
                              _userBankController = true;
                              break;
                        case CTRL_VOLUME:
                              _volume = value;
                              break;
                        case CTRL_PANPOT:
                              _pan = value;
                              break;
                        case CTRL_CHORUS_SEND:
                              _chorus = value;
                              break;
                        case CTRL_REVERB_SEND:
                              _reverb = value;
                              break;
                        default:
                              {
                              Event ev(ME_CONTROLLER);
                              ev.setOntime(-1);
                              ev.setChannel(0);
                              ev.setDataA(ctrl);
                              ev.setDataB(value);
                              _init.push_back(ev);
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
                  _synti = e.readElementText();
            else if (tag == "descr")
                  _descr = e.readElementText();
            else if (tag == "color")
                  _color = e.readInt();
            else if (tag == "mute")
                  _mute = e.readInt();
            else if (tag == "solo")
                  _solo = e.readInt();
            else if (tag == "midiPort") {
                  midiPort = e.readInt();
                  }
            else if (tag == "midiChannel") {
                  midiChannel = e.readInt();
                  }
            else
                  e.unknown();
            }
      if (128 == _bank && "zerberus" == _synti.toLower())
            _bank = 0;

      _mustUpdateInit = true;

      if ((midiPort != -1 || midiChannel != -1) && part && part->score()->isMaster())
            part->masterScore()->addMidiMapping(this, part, midiPort, midiChannel);
      }

//---------------------------------------------------------
//   switchExpressive
//    Switches channel from non-expressive to expressive patch or vice versa
//    This works only with MuseScore General soundfont
//---------------------------------------------------------

void Channel::switchExpressive(Synthesizer* synth, bool expressive, bool force /* = false */)
      {
      if ((_userBankController && !force) || !synth)
            return;

      // Don't try to switch if we already have done so
      if (expressive == _switchedToExpressive)
            return;

      // Check that we're actually changing the MuseScore General soundfont
      const auto fontsInfo = synth->soundFontsInfo();
      if (fontsInfo.empty())
            return;
      const auto& info = fontsInfo.front();
      if (!info.fontName.contains("MuseScore_General", Qt::CaseInsensitive)) {
            qDebug().nospace() << "Soundfont '" << info.fontName << "' is not MuseScore General, cannot update expressive";
            return;
            }

      // Work out where the new expressive patch will be
      // All expressive instruments are +1 bank higher than the
      // normal counterparts, except on bank 0, where they are placed on bank 17
      // and on bank 8, which uses bank 18 instead.
      int searchBankNum;
      int newBankNum;
      if (expressive) {
            int relativeBank = bank() % 129;
            if (relativeBank == 0)
                  newBankNum = 17;
            else if (relativeBank == 8)
                  newBankNum = 18;
            else
                  newBankNum = relativeBank + 1;
            _switchedToExpressive = true;
            }
      else {
            int relativeBank = bank() % 129;
            if (relativeBank == 17)
                  newBankNum = 0;
            else if (relativeBank == 18)
                  newBankNum = 8;
            else
                  newBankNum = relativeBank - 1;
            _switchedToExpressive = false;
            }

      // Floor bank num to multiple of 129 and add new num to get bank num of new patch
      searchBankNum = (bank() / 129) * 129 + newBankNum;
      const auto& pl = synth->getPatchInfo();
      for (const MidiPatch* p : pl) {
            if (p->synti == "Fluid") {
                  if (searchBankNum == p->bank && program() == p->prog) {
                        setBank(p->bank);
                        return;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   updateInitList
//---------------------------------------------------------

void Channel::updateInitList() const
      {
      MidiCoreEvent e;
      if (_program != -1) {
            e.setType(ME_CONTROLLER);
            e.setDataA(CTRL_PROGRAM);
            e.setDataB(_program);
            _init[int(A::PROGRAM)] = e;
            }

      e.setData(ME_CONTROLLER, CTRL_HBANK, (_bank >> 7) & 0x7f);
      _init[int(A::HBANK)] = e;

      e.setData(ME_CONTROLLER, CTRL_LBANK, _bank & 0x7f);
      _init[int(A::LBANK)] = e;

      e.setData(ME_CONTROLLER, CTRL_VOLUME, volume());
      _init[int(A::VOLUME)] = e;

      e.setData(ME_CONTROLLER, CTRL_PANPOT, pan());
      _init[int(A::PAN)] = e;

      e.setData(ME_CONTROLLER, CTRL_CHORUS_SEND, chorus());
      _init[int(A::CHORUS)] = e;

      e.setData(ME_CONTROLLER, CTRL_REVERB_SEND, reverb());
      _init[int(A::REVERB)] = e;

      }

//---------------------------------------------------------
//   addListener
//---------------------------------------------------------

void Channel::addListener(ChannelListener* l)
      {
      _notifier.addListener(l);
      }

//---------------------------------------------------------
//   removeListener
//---------------------------------------------------------

void Channel::removeListener(ChannelListener* l)
      {
      _notifier.removeListener(l);
      }

//---------------------------------------------------------
//   PartChannelSettingsLink
//---------------------------------------------------------

PartChannelSettingsLink::PartChannelSettingsLink(Channel* main, Channel* bound, bool excerpt)
   : _main(main), _bound(bound), _excerpt(excerpt)
      {
      if (excerpt) {
            for (Channel::Prop p : excerptProperties)
                  applyProperty(p, /* from */ bound, /* to */ main);
            }
      // Maybe it would be good to assign common properties if the link
      // is constructed in non-excerpt mode. But it is not currently
      // necessary as playback channels are currently recreated on each
      // MIDI remapping.

      main->addListener(this);
      }

//---------------------------------------------------------
//   PartChannelSettingsLink
//---------------------------------------------------------

PartChannelSettingsLink::PartChannelSettingsLink(PartChannelSettingsLink&& other)
   : ChannelListener(), // swap() will set the notifier instead
   _main(nullptr), _bound(nullptr), _excerpt(false)
      {
      swap(*this, other);
      }

//---------------------------------------------------------
//   PartChannelSettingsLink::operator=
//---------------------------------------------------------

PartChannelSettingsLink& PartChannelSettingsLink::operator=(PartChannelSettingsLink&& other)
      {
      if (this != &other)
            swap(*this, other);
      return *this;
      }

//---------------------------------------------------------
//   swap
//---------------------------------------------------------

void swap(PartChannelSettingsLink& l1, PartChannelSettingsLink& l2)
      {
      Ms::swap(static_cast<ChannelListener&>(l1), static_cast<ChannelListener&>(l2));
      using std::swap;
      swap(l1._main, l2._main);
      swap(l1._bound, l2._bound);
      swap(l1._excerpt, l2._excerpt);
      }

//---------------------------------------------------------
//   PartChannelSettingsLink::applyProperty
//---------------------------------------------------------

void PartChannelSettingsLink::applyProperty(Channel::Prop p, const Channel* from, Channel* to)
      {
      switch (p) {
            case Channel::Prop::VOLUME:
                  to->setVolume(from->volume());
                  break;
            case Channel::Prop::PAN:
                  to->setPan(from->pan());
                  break;
            case Channel::Prop::CHORUS:
                  to->setChorus(from->chorus());
                  break;
            case Channel::Prop::REVERB:
                  to->setReverb(from->reverb());
                  break;
            case Channel::Prop::NAME:
                  to->setName(from->name());
                  break;
            case Channel::Prop::DESCR:
                  to->setDescr(from->descr());
                  break;
            case Channel::Prop::PROGRAM:
                  to->setProgram(from->program());
                  break;
            case Channel::Prop::BANK:
                  to->setBank(from->bank());
                  break;
            case Channel::Prop::COLOR:
                  to->setColor(from->color());
                  break;
            case Channel::Prop::SOLOMUTE:
                  to->setSoloMute(from->soloMute());
                  break;
            case Channel::Prop::SOLO:
                  to->setSolo(from->solo());
                  break;
            case Channel::Prop::MUTE:
                  to->setMute(from->mute());
                  break;
            case Channel::Prop::SYNTI:
                  to->setSynti(from->synti());
                  break;
            case Channel::Prop::CHANNEL:
                  to->setChannel(from->channel());
                  break;
            case Channel::Prop::USER_BANK_CONTROL:
                  to->setUserBankController(from->userBankController());
                  break;
            };
      }

//---------------------------------------------------------
//   PartChannelSettingsLink::propertyChanged
//---------------------------------------------------------

void PartChannelSettingsLink::propertyChanged(Channel::Prop p)
      {
      if (isExcerptProperty(p) == _excerpt)
            applyProperty(p, _main, _bound);
      }

//---------------------------------------------------------
//   channelIdx
//---------------------------------------------------------

int Instrument::channelIdx(const QString& s) const
      {
      int idx = 0;
      for (const Channel* a : _channel) {
            if (a->name().isEmpty() && s == Channel::DEFAULT_NAME)
                  return idx;
            if (s == a->name())
                  return idx;
            ++idx;
            }
      return -1;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void MidiArticulation::write(XmlWriter& xml) const
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

void Instrument::updateVelocity(int* velocity, int /*channelIdx*/, const QString& name)
      {
      *velocity *= getVelocityMultiplier(name);
      }

//---------------------------------------------------------
//   updateVelocity
//---------------------------------------------------------

qreal Instrument::getVelocityMultiplier(const QString& name)
      {
      for (const MidiArticulation& a : qAsConst(_articulation)) {
            if (a.name == name) {
                  return qreal(a.velocity) / 100;
                  }
            }
      return 1;
      }

//---------------------------------------------------------
//   updateGateTime
//---------------------------------------------------------

void Instrument::updateGateTime(int* gateTime, int /*channelIdx*/, const QString& name)
      {
      for (const MidiArticulation& a : qAsConst(_articulation)) {
            if (a.name == name) {
                  *gateTime = a.gateTime;
                  break;
                  }
            }
      }


//---------------------------------------------------------
//   updateGateTime
//---------------------------------------------------------

void Instrument::switchExpressive(MasterScore* score, Synthesizer* synth, bool expressive, bool force /* = false */)
      {
      // Only switch to expressive where necessary
      if (!synth || (expressive && !singleNoteDynamics()))
            return;

      for (Channel* c : channel()) {
            c->switchExpressive(synth, expressive, force);
            if (score->playbackChannel(c))
                  score->playbackChannel(c)->switchExpressive(synth, expressive, force);
            }
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool Instrument::operator==(const Instrument& i) const
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
            if (!(i._shortNames[k] == _shortNames[k].name()))
                  return false;
            }
      n = _channel.size();
      if (i._channel.size() != n)
            return false;
      for (int k = 0; k < n; ++k) {
            if (!(*i._channel[k] == *_channel[k]))
                  return false;
            }

      return i._minPitchA == _minPitchA
         &&  i._maxPitchA == _maxPitchA
         &&  i._minPitchP == _minPitchP
         &&  i._maxPitchP == _maxPitchP
         &&  i._useDrumset == _useDrumset
         &&  i._midiActions == _midiActions
         &&  i._articulation == _articulation
         &&  i._transpose.diatonic == _transpose.diatonic
         &&  i._transpose.chromatic == _transpose.chromatic
         &&  i._trackName == _trackName
         &&  *i.stringData() == *stringData()
         &&  i._singleNoteDynamics == _singleNoteDynamics;
      }

//---------------------------------------------------------
//   isDifferentInstrument
///   Checks if the passed instrument is a different instrument.
///   Does not compare channels.
//---------------------------------------------------------

bool Instrument::isDifferentInstrument(const Instrument& i) const
      {
      int n = _longNames.size();
      if (i._longNames.size() != n)
            return true;
      for (int k = 0; k < n; ++k) {
            if (!(i._longNames[k] == _longNames[k]))
                  return true;
            }
      n = _shortNames.size();
      if (i._shortNames.size() != n)
            return true;
      for (int k = 0; k < n; ++k) {
            if (!(i._shortNames[k] == _shortNames[k].name()))
                  return true;
            }

      return i._minPitchA != _minPitchA
            || i._maxPitchA != _maxPitchA
            || i._minPitchP != _minPitchP
            || i._maxPitchP != _maxPitchP
            || i._useDrumset != _useDrumset
            || i._midiActions != _midiActions
            || i._articulation != _articulation
            || i._transpose.diatonic != _transpose.diatonic
            || i._transpose.chromatic != _transpose.chromatic
            || i._trackName != _trackName
            || !(*i.stringData() == *stringData())
            || i._singleNoteDynamics != _singleNoteDynamics;
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool StaffName::operator==(const StaffName& i) const
      {
      return (i._pos == _pos) && (i._name == _name);
      }

QString StaffName::toString() const
      {
      return _name;
      }

//---------------------------------------------------------
//   setUseDrumset
//---------------------------------------------------------

void Instrument::setUseDrumset(bool val)
      {
      _useDrumset = val;
      if (val && !_drumset)
            _drumset = new Drumset(*smDrumset);
      }

//---------------------------------------------------------
//   setDrumset
//---------------------------------------------------------

void Instrument::setDrumset(const Drumset* ds)
      {
      delete _drumset;
      if (ds) {
            _useDrumset = true;
            _drumset = new Drumset(*ds);
            }
      else {
            _useDrumset = false;
            _drumset = 0;
            }
      }

//---------------------------------------------------------
//   setLongName
//    f is in richtext format
//---------------------------------------------------------

void Instrument::setLongName(const QString& f)
      {
      _longNames.clear();
      if (f.length() > 0)
            _longNames.append(StaffName(f, 0));
      }

//---------------------------------------------------------
//   setShortName
//    f is in richtext format
//---------------------------------------------------------

void Instrument::setShortName(const QString& f)
      {
      _shortNames.clear();
      if (f.length() > 0)
            _shortNames.append(StaffName(f, 0));
      }

//---------------------------------------------------------
//   addLongName
//---------------------------------------------------------

void Instrument::addLongName(const StaffName& f)
      {
      _longNames.append(f);
      }

//---------------------------------------------------------
//   addShortName
//---------------------------------------------------------

void Instrument::addShortName(const StaffName& f)
      {
      _shortNames.append(f);
      }

//---------------------------------------------------------
//   clefType
//---------------------------------------------------------

ClefTypeList Instrument::clefType(int staffIdx) const
      {
      if (staffIdx >= _clefType.size()) {
            if (_clefType.empty())
                  return ClefTypeList(staffIdx == 1 ? ClefType::F : ClefType::G);
            return _clefType[0];
            }
      return _clefType[staffIdx];
      }

//---------------------------------------------------------
//   setClefType
//---------------------------------------------------------

void Instrument::setClefType(int staffIdx, const ClefTypeList& c)
      {
      while (_clefType.size() <= staffIdx)
            _clefType.append(ClefTypeList());
      _clefType[staffIdx] = c;
      }

//---------------------------------------------------------
//   minPitchP
//---------------------------------------------------------

int Instrument::minPitchP() const
      {
      return _minPitchP;
      }

//---------------------------------------------------------
//   maxPitchP
//---------------------------------------------------------

int Instrument::maxPitchP() const
      {
      return _maxPitchP;
      }

//---------------------------------------------------------
//   minPitchA
//---------------------------------------------------------

int Instrument::minPitchA() const
      {
      return _minPitchA;
      }

//---------------------------------------------------------
//   maxPitchA
//---------------------------------------------------------

int Instrument::maxPitchA() const
      {
      return _maxPitchA;
      }

//---------------------------------------------------------
//   instrumentId
//---------------------------------------------------------

QString Instrument::instrumentId() const
      {
      return _instrumentId;
      }

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

const Instrument* InstrumentList::instrument(int tick) const
      {
      if (empty())
            return &defaultInstrument;
      auto i = upper_bound(tick);
      if (i == begin())
            return &defaultInstrument;
      --i;
      return i->second;
      }

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

Instrument* InstrumentList::instrument(int tick)
      {
      if (empty())
            return &defaultInstrument;
      auto i = upper_bound(tick);
      if (i == begin())
            return &defaultInstrument;
      --i;
      return i->second;
      }

//---------------------------------------------------------
//   setInstrument
//---------------------------------------------------------

void InstrumentList::setInstrument(Instrument* instr, int tick)
      {
      if (!insert({tick, instr}).second)
            (*this)[tick] = instr;
      }

//---------------------------------------------------------
//   longName
//---------------------------------------------------------

const QList<StaffName>& Instrument::longNames() const
      {
      return _longNames;
      }

//---------------------------------------------------------
//   shortName
//---------------------------------------------------------

const QList<StaffName>& Instrument::shortNames() const
      {
      return _shortNames;
      }

//---------------------------------------------------------
//   longName
//---------------------------------------------------------

QList<StaffName>& Instrument::longNames()
      {
      return _longNames;
      }

//---------------------------------------------------------
//   shortName
//---------------------------------------------------------

QList<StaffName>& Instrument::shortNames()
      {
      return _shortNames;
      }

//---------------------------------------------------------
//   trackName
//---------------------------------------------------------

QString Instrument::trackName() const
      {
      return _trackName;
      }

void Instrument::setTrackName(const QString& s)
      {
      _trackName = s;
      }

//---------------------------------------------------------
//   fromTemplate
//---------------------------------------------------------

Instrument Instrument::fromTemplate(const InstrumentTemplate* t)
      {
      Instrument instr(t->id);
      instr.setAmateurPitchRange(t->minPitchA, t->maxPitchA);
      instr.setProfessionalPitchRange(t->minPitchP, t->maxPitchP);
      for (const StaffName &sn : t->longNames)
            instr.addLongName(StaffName(sn.name(), sn.pos()));
      for (const StaffName &sn : t->shortNames)
            instr.addShortName(StaffName(sn.name(), sn.pos()));
      instr.setTrackName(t->trackName);
      instr.setTranspose(t->transpose);
      instr.setInstrumentId(t->musicXMLid);
      if (t->useDrumset)
            instr.setDrumset(t->drumset ? t->drumset : smDrumset);
      for (int i = 0; i < t->nstaves(); ++i)
            instr.setClefType(i, t->clefTypes[i]);
      instr.setMidiActions(t->midiActions);
      instr.setArticulation(t->articulation);
      instr._channel.clear();
      for (const Channel& c : t->channel)
            instr._channel.append(new Channel(c));
      instr.setStringData(t->stringData);
      instr.setSingleNoteDynamics(t->singleNoteDynamics);
      return instr;
      }

//---------------------------------------------------------
//  updateInstrumentId
//---------------------------------------------------------

void Instrument::updateInstrumentId()
      {
      if (!_id.isEmpty() || _instrumentId.isEmpty())
            return;

      // When reading a score create with pre-3.6, instruments doesn't
      // have an id define in the instrument. So try to find the instrumentId
      // based on MusicXMLid.
      // This requires a hack for instruments using MusicXMLid "strings.group"
      // because there are multiple instrument using this same id.
      // For these instruments, use the value of controller 32 of the "arco"
      // channel to find the correct instrument.
      const QString arco = QString("arco");
      const bool groupHack = instrumentId() == QString("strings.group");
      const int idxref = channelIdx(arco);
      const int val32ref = (idxref < 0) ? -1 : channel(idxref)->bank();
      QString fallback;

      for (InstrumentGroup* g : instrumentGroups) {
            for (InstrumentTemplate* it : g->instrumentTemplates) {
                  if (it->musicXMLid == instrumentId()) {
                        if (groupHack) {
                              if (fallback.isEmpty())
                                    // Instrument "Strings" doesn't have bank defined so
                                    // if no "strings.group" instrument with requested bank
                                    // is found, assume "Strings".
                                    fallback = it->id;
                              for (const Channel& chan : it->channel) {
                                    if ((chan.name() == arco) && (chan.bank() == val32ref)) {
                                          _id = it->id;
                                          return;
                                          }
                                    }
                              }
                        else {
                              _id = it->id;
                              return;
                              }
                        }
                  }
            }

      _id = fallback.isEmpty() ? QString("piano") : fallback;
      }

//---------------------------------------------------------
//   Instrument::playbackChannel
//---------------------------------------------------------

const Channel* Instrument::playbackChannel(int idx, const MasterScore* score) const
      {
      return score->playbackChannel(channel(idx));
      }


//---------------------------------------------------------
//   Instrument::playbackChannel
//---------------------------------------------------------

Channel* Instrument::playbackChannel(int idx, MasterScore* score)
      {
      return score->playbackChannel(channel(idx));
      }

//---------------------------------------------------------
//   getSingleNoteDynamicsFromTemplate
//---------------------------------------------------------

bool Instrument::getSingleNoteDynamicsFromTemplate() const
      {
      QString templateName = trackName().toLower().replace(" ", "-").replace("♭", "b");
      InstrumentTemplate* tp = searchTemplate(templateName);
      if (tp)
            return tp->singleNoteDynamics;
      return true;
      }

//---------------------------------------------------------
//   setSingleNoteDynamicsFromTemplate
//---------------------------------------------------------

void Instrument::setSingleNoteDynamicsFromTemplate()
      {
      setSingleNoteDynamics(getSingleNoteDynamicsFromTemplate());
      }

//---------------------------------------------------------
//   StaffNameList::write
//---------------------------------------------------------

void StaffNameList::write(XmlWriter& xml, const char* name) const
      {
      for (const StaffName& sn : *this)
            sn.write(xml, name);
      }

QStringList StaffNameList::toStringList() const
      {
      QStringList result;

      for (const StaffName& name : *this)
            result << name.toString();

      return result;
      }
}

