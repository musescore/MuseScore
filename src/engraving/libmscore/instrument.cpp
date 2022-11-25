/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "instrument.h"

#include "rw/xml.h"
#include "infrastructure/htmlparser.h"
#include "types/typesconv.h"

#include "compat/midi/event.h"
//#include "compat/midi/midipatch.h"

#include "drumset.h"
#include "instrtemplate.h"
#include "masterscore.h"
#include "mscore.h"
#include "part.h"
#include "score.h"
#include "stringdata.h"
#include "textbase.h"
#include "utils.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//: Channel name for otherwise unnamed channels
const char* InstrChannel::DEFAULT_NAME = QT_TRANSLATE_NOOP("engraving/instruments", "normal");
//: Channel name for the chord symbols playback channel, best keep translation shorter than 11 letters
const char* InstrChannel::HARMONY_NAME = QT_TRANSLATE_NOOP("engraving/instruments", "harmony");

Instrument InstrumentList::defaultInstrument;
const std::initializer_list<InstrChannel::Prop> PartChannelSettingsLink::excerptProperties {
    InstrChannel::Prop::SOLOMUTE,
    InstrChannel::Prop::SOLO,
    InstrChannel::Prop::MUTE,
};

static void midi_event_write(const MidiCoreEvent& e, XmlWriter& xml)
{
    switch (e.type()) {
    case ME_NOTEON:
        xml.tag("note-on", { { "channel", e.channel() }, { "pitch", e.pitch() }, { "velo", e.velo() } });
        break;

    case ME_NOTEOFF:
        xml.tag("note-off", { { "channel", e.channel() }, { "pitch", e.pitch() }, { "velo", e.velo() } });
        break;

    case ME_CONTROLLER:
        if (e.controller() == CTRL_PROGRAM) {
            if (e.channel() == 0) {
                xml.tag("program", { { "value", e.value() } });
            } else {
                xml.tag("program", { { "channel", e.channel() }, { "value", e.value() } });
            }
        } else {
            if (e.channel() == 0) {
                xml.tag("controller", { { "ctrl", e.controller() }, { "value", e.value() } });
            } else {
                xml.tag("controller", { { "channel", e.channel() }, { "ctrl", e.controller() }, { "value", e.value() } });
            }
        }
        break;
    default:
        LOGD("MidiCoreEvent::write: unknown type");
        break;
    }
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void NamedEventList::write(XmlWriter& xml, const AsciiStringView& n) const
{
    xml.startElement(n, { { "name", name } });
    if (!descr.empty()) {
        xml.tag("descr", descr);
    }
    for (const MidiCoreEvent& e : events) {
        midi_event_write(e, xml);
    }
    xml.endElement();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void NamedEventList::read(XmlReader& e)
{
    name = e.attribute("name");
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "program") {
            MidiCoreEvent ev(ME_CONTROLLER, 0, CTRL_PROGRAM, e.intAttribute("value", 0));
            events.push_back(ev);
            e.skipCurrentElement();
        } else if (tag == "controller") {
            MidiCoreEvent ev;
            ev.setType(ME_CONTROLLER);
            ev.setDataA(e.intAttribute("ctrl", 0));
            ev.setDataB(e.intAttribute("value", 0));
            events.push_back(ev);
            e.skipCurrentElement();
        } else if (tag == "descr") {
            descr = e.readText();
        } else {
            e.unknown();
        }
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

Instrument::Instrument(String id)
{
    _id = id;
    InstrChannel* a = new InstrChannel;
    a->setName(String::fromUtf8(InstrChannel::DEFAULT_NAME));
    _channel.push_back(a);

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
    for (InstrChannel* c : i._channel) {
        _channel.push_back(new InstrChannel(*c));
    }
    _clefType     = i._clefType;
    _trait = i._trait;
}

void Instrument::operator=(const Instrument& i)
{
    DeleteAll(_channel);
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
    for (InstrChannel* c : i._channel) {
        _channel.push_back(new InstrChannel(*c));
    }
    _clefType     = i._clefType;
    _trait = i._trait;
}

//---------------------------------------------------------
//   ~Instrument
//---------------------------------------------------------

Instrument::~Instrument()
{
    DeleteAll(_channel);
    delete _drumset;
    _drumset = nullptr;
}

//---------------------------------------------------------
//   StaffName
//---------------------------------------------------------

StaffName::StaffName(const String& xmlText, int pos)
    : _name(xmlText), _pos(pos)
{
    TextBase::validateText(_name); // enforce HTML encoding
}

//---------------------------------------------------------
//   StaffName::write
//---------------------------------------------------------

void StaffName::write(XmlWriter& xml, const char* tag) const
{
    if (!name().isEmpty()) {
        if (pos() == 0) {
            xml.writeXml(String::fromUtf8(tag), name());
        } else {
            xml.writeXml(String(u"%1 pos=\"%2\"").arg(String::fromUtf8(tag)).arg(pos()), name());
        }
    }
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffName::read(XmlReader& e)
{
    _pos  = e.intAttribute("pos", 0);
    _name = e.readXml();
    if (_name.startsWith(u"<html>")) {
        // compatibility to old html implementation:
        _name = HtmlParser::parse(_name);
    }
}

//---------------------------------------------------------
//   Instrument::write
//---------------------------------------------------------

void Instrument::write(XmlWriter& xml, const Part* part) const
{
    if (_id.isEmpty()) {
        xml.startElement("Instrument");
    } else {
        xml.startElement("Instrument", { { "id", _id } });
    }
    _longNames.write(xml, "longName");
    _shortNames.write(xml, "shortName");
//      if (!_trackName.empty())
    xml.tag("trackName", _trackName);
    if (_minPitchP > 0) {
        xml.tag("minPitchP", _minPitchP);
    }
    if (_maxPitchP < 127) {
        xml.tag("maxPitchP", _maxPitchP);
    }
    if (_minPitchA > 0) {
        xml.tag("minPitchA", _minPitchA);
    }
    if (_maxPitchA < 127) {
        xml.tag("maxPitchA", _maxPitchA);
    }
    if (_transpose.diatonic) {
        xml.tag("transposeDiatonic", _transpose.diatonic);
    }
    if (_transpose.chromatic) {
        xml.tag("transposeChromatic", _transpose.chromatic);
    }
    if (!_instrumentId.isEmpty()) {
        xml.tag("instrumentId", _instrumentId);
    }
    if (_useDrumset) {
        xml.tag("useDrumset", _useDrumset);
        _drumset->save(xml);
    }
    for (size_t i = 0; i < _clefType.size(); ++i) {
        ClefTypeList ct = _clefType[i];
        if (ct._concertClef == ct._transposingClef) {
            if (ct._concertClef != ClefType::G) {
                if (i) {
                    xml.tag("clef", { { "staff", i + 1 } }, TConv::toXml(ct._concertClef));
                } else {
                    xml.tag("clef", TConv::toXml(ct._concertClef));
                }
            }
        } else {
            if (i) {
                xml.tag("concertClef", { { "staff", i + 1 } }, TConv::toXml(ct._concertClef));
                xml.tag("transposingClef", { { "staff", i + 1 } }, TConv::toXml(ct._transposingClef));
            } else {
                xml.tag("concertClef", TConv::toXml(ct._concertClef));
                xml.tag("transposingClef", TConv::toXml(ct._transposingClef));
            }
        }
    }

    if (_singleNoteDynamics != getSingleNoteDynamicsFromTemplate()) {
        xml.tag("singleNoteDynamics", _singleNoteDynamics);
    }

    if (!(_stringData == StringData())) {
        _stringData.write(xml);
    }
    for (const NamedEventList& a : _midiActions) {
        a.write(xml, "MidiAction");
    }
    for (const MidiArticulation& a : _articulation) {
        a.write(xml);
    }
    for (const InstrChannel* a : _channel) {
        a->write(xml, part);
    }
    xml.endElement();
}

String Instrument::recognizeInstrumentId() const
{
    static const String defaultInstrumentId(u"keyboard.piano");

    std::list<String> nameList;

    nameList.push_back(_trackName);
    mu::join(nameList, _longNames.toStringList());
    mu::join(nameList, _shortNames.toStringList());

    const InstrumentTemplate* tmplByName = mu::engraving::searchTemplateForInstrNameList(nameList, _useDrumset);

    if (tmplByName && !tmplByName->musicXMLid.isEmpty()) {
        return tmplByName->musicXMLid;
    }

    const InstrChannel* channel = this->channel(0);

    if (!channel) {
        return defaultInstrumentId;
    }

    const InstrumentTemplate* tmplMidiProgram = mu::engraving::searchTemplateForMidiProgram(channel->bank(), channel->program(),
                                                                                            _useDrumset);

    if (tmplMidiProgram && !tmplMidiProgram->musicXMLid.isEmpty()) {
        return tmplMidiProgram->musicXMLid;
    }

    if (_useDrumset) {
        static const String drumsetId(u"drumset");
        return drumsetId;
    }

    return defaultInstrumentId;
}

String Instrument::recognizeId() const
{
    // When reading a score create with pre-3.6, instruments doesn't
    // have an id define in the instrument. So try to find the instrumentId
    // based on MusicXMLid.
    // This requires a hack for instruments using MusicXMLid "strings.group"
    // because there are multiple instrument using this same id.
    // For these instruments, use the value of controller 32 of the "arco"
    // channel to find the correct instrument.
    // There are some duplicate MusicXML IDs among other instruments too. In
    // that case we check the pitch range and use the shortest ID that matches.
    const String arco = String(u"arco");
    const bool groupHack = instrumentId() == String(u"strings.group");
    const int idxref = channelIdx(arco);
    const int val32ref = (idxref < 0) ? -1 : channel(idxref)->bank();
    String fallback;
    int bestMatchStrength = 0;     // higher when fallback ID provides better match for instrument data

    for (InstrumentGroup* g : instrumentGroups) {
        for (InstrumentTemplate* it : g->instrumentTemplates) {
            if (it->musicXMLid != instrumentId()) {
                continue;
            }
            if (groupHack) {
                if (fallback.isEmpty()) {
                    // Instrument "Strings" doesn't have bank defined so
                    // if no "strings.group" instrument with requested bank
                    // is found, assume "Strings".
                    fallback = it->id;
                }
                for (const InstrChannel& chan : it->channel) {
                    if ((chan.name() == arco) && (chan.bank() == val32ref)) {
                        return it->id;
                    }
                }
            } else {
                int matchStrength = 0
                                    + ((minPitchP() == it->minPitchP) ? 1 : 0)
                                    + ((minPitchA() == it->minPitchA) ? 1 : 0)
                                    + ((maxPitchA() == it->maxPitchA) ? 1 : 0)
                                    + ((maxPitchP() == it->maxPitchP) ? 1 : 0);
                const int perfectMatchStrength = 4;
                assert(matchStrength <= perfectMatchStrength);
                if (fallback.isEmpty() || matchStrength > bestMatchStrength) {
                    // Set a fallback ID or update it because we've found a better one.
                    fallback = it->id;
                    bestMatchStrength = matchStrength;
                    if (bestMatchStrength == perfectMatchStrength) {
                        break;     // stop looking for matches
                    }
                } else if ((matchStrength == bestMatchStrength) && (it->id.size() < fallback.size())) {
                    // Update fallback ID because we've found a shorter one that is equally good.
                    // Shorter IDs tend to correspond to more generic instruments (e.g. "piano"
                    // vs. "grand-piano") so it's better to use a shorter one if unsure.
                    fallback = it->id;
                }
            }
        }
    }

    return fallback.isEmpty() ? String(u"piano") : fallback;
}

int Instrument::recognizeMidiProgram() const
{
    InstrumentTemplate* tmplInstrumentId = mu::engraving::searchTemplateForMusicXmlId(_instrumentId);

    if (tmplInstrumentId && !tmplInstrumentId->channel.empty() && tmplInstrumentId->channel[0].program() >= 0) {
        return tmplInstrumentId->channel[0].program();
    }

    std::list<String> nameList;

    nameList.push_back(_trackName);
    mu::join(nameList, _longNames.toStringList());
    mu::join(nameList, _shortNames.toStringList());

    InstrumentTemplate* tmplByName = mu::engraving::searchTemplateForInstrNameList(nameList);

    if (tmplByName && !tmplByName->channel.empty() && tmplByName->channel[0].program() >= 0) {
        return tmplByName->channel[0].program();
    }

    return 0;
}

//---------------------------------------------------------
//   Instrument::read
//---------------------------------------------------------

void Instrument::read(XmlReader& e, Part* part)
{
    bool customDrumset = false;
    bool readSingleNoteDynamics = false;

    _channel.clear();         // remove default channel
    _id = e.attribute("id");
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "singleNoteDynamics") {
            _singleNoteDynamics = e.readBool();
            readSingleNoteDynamics = true;
        } else if (!readProperties(e, part, &customDrumset)) {
            e.unknown();
        }
    }

    if (_instrumentId.isEmpty()) {
        _instrumentId = recognizeInstrumentId();
    }

    if (_id.isEmpty()) {
        _id = recognizeId();
    }

    if (channel(0) && channel(0)->program() == -1) {
        channel(0)->setProgram(recognizeMidiProgram());
    }

    if (!readSingleNoteDynamics) {
        setSingleNoteDynamicsFromTemplate();
    }
}

//---------------------------------------------------------
//   Instrument::readProperties
//---------------------------------------------------------

bool Instrument::readProperties(XmlReader& e, Part* part, bool* customDrumset)
{
    const AsciiStringView tag(e.name());
    if (tag == "longName") {
        StaffName name;
        name.read(e);
        _longNames.push_back(name);
    } else if (tag == "shortName") {
        StaffName name;
        name.read(e);
        _shortNames.push_back(name);
    } else if (tag == "trackName") {
        _trackName = e.readText();
    } else if (tag == "minPitch") {      // obsolete
        _minPitchP = _minPitchA = e.readInt();
    } else if (tag == "maxPitch") {       // obsolete
        _maxPitchP = _maxPitchA = e.readInt();
    } else if (tag == "minPitchA") {
        _minPitchA = e.readInt();
    } else if (tag == "minPitchP") {
        _minPitchP = e.readInt();
    } else if (tag == "maxPitchA") {
        _maxPitchA = e.readInt();
    } else if (tag == "maxPitchP") {
        _maxPitchP = e.readInt();
    } else if (tag == "transposition") {    // obsolete
        _transpose.chromatic = e.readInt();
        _transpose.diatonic = chromatic2diatonic(_transpose.chromatic);
    } else if (tag == "transposeChromatic") {
        _transpose.chromatic = e.readInt();
    } else if (tag == "transposeDiatonic") {
        _transpose.diatonic = e.readInt();
    } else if (tag == "instrumentId") {
        _instrumentId = e.readText();
    } else if (tag == "useDrumset") {
        _useDrumset = e.readInt();
        if (_useDrumset) {
            delete _drumset;
            _drumset = new Drumset(*smDrumset);
        }
    } else if (tag == "Drum") {
        // if we see on of this tags, a custom drumset will
        // be created
        if (!_drumset) {
            _drumset = new Drumset(*smDrumset);
        }
        if (!(*customDrumset)) {
            const_cast<Drumset*>(_drumset)->clear();
            *customDrumset = true;
        }
        const_cast<Drumset*>(_drumset)->load(e);
    }
    // support tag "Tablature" for a while for compatibility with existent 2.0 scores
    else if (tag == "Tablature" || tag == "StringData") {
        _stringData.read(e);
    } else if (tag == "MidiAction") {
        NamedEventList a;
        a.read(e);
        _midiActions.push_back(a);
    } else if (tag == "Articulation") {
        MidiArticulation a;
        a.read(e);
        _articulation.push_back(a);
    } else if (tag == "Channel" || tag == "channel") {
        InstrChannel* a = new InstrChannel;
        a->read(e, part);
        _channel.push_back(a);
    } else if (tag == "clef") {           // sets both transposing and concert clef
        int idx = e.intAttribute("staff", 1) - 1;
        ClefType ct = TConv::fromXml(e.readAsciiText(), ClefType::G);
        setClefType(idx, ClefTypeList(ct, ct));
    } else if (tag == "concertClef") {
        int idx = e.intAttribute("staff", 1) - 1;
        setClefType(idx, ClefTypeList(TConv::fromXml(e.readAsciiText(), ClefType::G), clefType(idx)._transposingClef));
    } else if (tag == "transposingClef") {
        int idx = e.intAttribute("staff", 1) - 1;
        setClefType(idx, ClefTypeList(clefType(idx)._concertClef, TConv::fromXml(e.readAsciiText(), ClefType::G)));
    } else {
        return false;
    }

    return true;
}

//---------------------------------------------------------
//   action
//---------------------------------------------------------

NamedEventList* Instrument::midiAction(const String& s, int channelIdx) const
{
    // first look in channel list

    for (const NamedEventList& a : _channel[channelIdx]->midiActions) {
        if (s == a.name) {
            return const_cast<NamedEventList*>(&a);
        }
    }

    for (const NamedEventList& a : _midiActions) {
        if (s == a.name) {
            return const_cast<NamedEventList*>(&a);
        }
    }
    return 0;
}

//---------------------------------------------------------
//   Channel
//---------------------------------------------------------

InstrChannel::InstrChannel()
{
    for (int i = 0; i < int(A::INIT_COUNT); ++i) {
        _init.push_back(MidiCoreEvent());
    }
    _synti    = u"Fluid";       // default synthesizer
    _channel  = -1;
    _program  = -1;
    _bank     = 0;
    _volume   = defaultVolume;
    _pan      = 64;   // actually 63.5 for center
    _chorus   = 0;
    _reverb   = 0;
    _color = DEFAULT_COLOR;

    _mute     = false;
    _solo     = false;
    _soloMute = false;

//      LOGD("construct Channel ");
}

//---------------------------------------------------------
//   initList
//---------------------------------------------------------

std::vector<MidiCoreEvent>& InstrChannel::initList() const
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

void InstrChannel::setVolume(char value)
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

void InstrChannel::setPan(char value)
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

void InstrChannel::setChorus(char value)
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

void InstrChannel::setReverb(char value)
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

void InstrChannel::setName(const String& value)
{
    if (_name != value) {
        _name = value;
        firePropertyChanged(Prop::NAME);
    }
}

//---------------------------------------------------------
//   setSynti
//---------------------------------------------------------

void InstrChannel::setSynti(const String& value)
{
    if (_synti != value) {
        _synti = value;
        firePropertyChanged(Prop::SYNTI);
    }
}

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void InstrChannel::setColor(int value)
{
    if (_color != value) {
        _color = value;
        firePropertyChanged(Prop::COLOR);
    }
}

//---------------------------------------------------------
//   setProgram
//---------------------------------------------------------

void InstrChannel::setProgram(int value)
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

void InstrChannel::setBank(int value)
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

void InstrChannel::setChannel(int value)
{
    if (_channel != value) {
        _channel = value;
        firePropertyChanged(Prop::CHANNEL);
    }
}

//---------------------------------------------------------
//   setSoloMute
//---------------------------------------------------------

void InstrChannel::setSoloMute(bool value)
{
    if (_soloMute != value) {
        _soloMute = value;
        firePropertyChanged(Prop::SOLOMUTE);
    }
}

//---------------------------------------------------------
//   setMute
//---------------------------------------------------------

void InstrChannel::setMute(bool value)
{
    if (_mute != value) {
        _mute = value;
        firePropertyChanged(Prop::MUTE);
    }
}

//---------------------------------------------------------
//   setSolo
//---------------------------------------------------------

void InstrChannel::setSolo(bool value)
{
    if (_solo != value) {
        _solo = value;
        firePropertyChanged(Prop::SOLO);
    }
}

//---------------------------------------------------------
//   setUserBankController
//---------------------------------------------------------

void InstrChannel::setUserBankController(bool val)
{
    if (_userBankController != val) {
        _userBankController = val;
        firePropertyChanged(Prop::USER_BANK_CONTROL);
    }
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void InstrChannel::write(XmlWriter& xml, const Part* part) const
{
    if (_name.isEmpty() || _name == DEFAULT_NAME) {
        xml.startElement("Channel");
    } else {
        xml.startElement("Channel", { { "name", _name } });
    }
    if (_color != DEFAULT_COLOR) {
        xml.tag("color", _color);
    }

    for (const MidiCoreEvent& e : initList()) {
        if (e.type() == ME_INVALID) {
            continue;
        }
        if (e.type() == ME_CONTROLLER) {
            // Don't write bank if automatically switched, but always write if switched by the user
            if (e.dataA() == CTRL_HBANK && e.dataB() == 0 && !_userBankController) {
                continue;
            }
            if (e.dataA() == CTRL_LBANK && e.dataB() == 0 && !_userBankController) {
                continue;
            }
            if (e.dataA() == CTRL_VOLUME && e.dataB() == 100) {
                continue;
            }
            if (e.dataA() == CTRL_PANPOT && e.dataB() == 64) {
                continue;
            }
            if (e.dataA() == CTRL_REVERB_SEND && e.dataB() == 0) {
                continue;
            }
            if (e.dataA() == CTRL_CHORUS_SEND && e.dataB() == 0) {
                continue;
            }
        }

        midi_event_write(e, xml);
    }
    if (!MScore::testMode) {
        // xml.tag("synti", ::synti->name(synti));
        xml.tag("synti", _synti);
    }
    if (_mute) {
        xml.tag("mute", _mute);
    }
    if (_solo) {
        xml.tag("solo", _solo);
    }

    if (part && part->masterScore()->exportMidiMapping() && part->score() == part->masterScore()) {
        xml.tag("midiPort",    part->masterScore()->midiMapping(_channel)->port());
        xml.tag("midiChannel", part->masterScore()->midiMapping(_channel)->channel());
    }
    for (const NamedEventList& a : midiActions) {
        a.write(xml, "MidiAction");
    }
    for (const MidiArticulation& a : articulation) {
        a.write(xml);
    }
    xml.endElement();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void InstrChannel::read(XmlReader& e, Part* part)
{
    // synti = 0;
    _name = e.attribute("name");
    if (_name == "") {
        _name = String::fromUtf8(DEFAULT_NAME);
    }

    int midiPort = -1;
    int midiChannel = -1;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "program") {
            _program = e.intAttribute("value", -1);
            if (_program == -1) {
                _program = e.readInt();
            } else {
                e.readNext();
            }
        } else if (tag == "controller") {
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
        } else if (tag == "Articulation") {
            MidiArticulation a;
            a.read(e);
            articulation.push_back(a);
        } else if (tag == "MidiAction") {
            NamedEventList a;
            a.read(e);
            midiActions.push_back(a);
        } else if (tag == "synti") {
            _synti = e.readText();
        } else if (tag == "color") {
            _color = e.readInt();
        } else if (tag == "mute") {
            _mute = e.readInt();
        } else if (tag == "solo") {
            _solo = e.readInt();
        } else if (tag == "midiPort") {
            midiPort = e.readInt();
        } else if (tag == "midiChannel") {
            midiChannel = e.readInt();
        } else {
            e.unknown();
        }
    }

    _mustUpdateInit = true;

    if ((midiPort != -1 || midiChannel != -1) && part && part->score()->isMaster()) {
        part->masterScore()->addMidiMapping(this, part, midiPort, midiChannel);
    }
}

//---------------------------------------------------------
//   switchExpressive
//    Switches channel from non-expressive to expressive patch or vice versa
//    This works only with MuseScore General soundfont
//---------------------------------------------------------

void InstrChannel::switchExpressive(Synthesizer* synth, bool expressive, bool force /* = false */)
{
    UNUSED(synth);
    UNUSED(expressive);
    UNUSED(force);
    //! TODO Needs poting to MU4
    NOT_IMPLEMENTED;

//    if ((_userBankController && !force) || !synth) {
//        return;
//    }

//    // Don't try to switch if we already have done so
//    if (expressive == _switchedToExpressive) {
//        return;
//    }

//    // Check that we're actually changing the MuseScore General soundfont
//    const auto fontsInfo = synth->soundFontsInfo();
//    if (fontsInfo.empty()) {
//        return;
//    }
//    const auto& info = fontsInfo.front();
//    if (!info.fontName.contains("MS Basic", Qt::CaseInsensitive)) {
//        LOGD().nospace() << "Soundfont '" << info.fontName << "' is not MuseScore General, cannot update expressive";
//        return;
//    }

//    // Work out where the new expressive patch will be
//    // All expressive instruments are +1 bank higher than the
//    // normal counterparts, except on bank 0, where they are placed on bank 17
//    // and on bank 8, which uses bank 18 instead.
//    int searchBankNum;
//    int newBankNum;
//    if (expressive) {
//        int relativeBank = bank() % 129;
//        if (relativeBank == 0) {
//            newBankNum = 17;
//        } else if (relativeBank == 8) {
//            newBankNum = 18;
//        } else {
//            newBankNum = relativeBank + 1;
//        }
//        _switchedToExpressive = true;
//    } else {
//        int relativeBank = bank() % 129;
//        if (relativeBank == 17) {
//            newBankNum = 0;
//        } else if (relativeBank == 18) {
//            newBankNum = 8;
//        } else {
//            newBankNum = relativeBank - 1;
//        }
//        _switchedToExpressive = false;
//    }

//    // Floor bank num to multiple of 129 and add new num to get bank num of new patch
//    searchBankNum = (bank() / 129) * 129 + newBankNum;
//    const auto& pl = synth->getPatchInfo();
//    for (const MidiPatch* p : pl) {
//        if (p->synti == "Fluid") {
//            if (searchBankNum == p->bank && program() == p->prog) {
//                setBank(p->bank);
//                return;
//            }
//        }
//    }
}

//---------------------------------------------------------
//   updateInitList
//---------------------------------------------------------

void InstrChannel::updateInitList() const
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

void InstrChannel::addListener(ChannelListener* l)
{
    _notifier.addListener(l);
}

//---------------------------------------------------------
//   removeListener
//---------------------------------------------------------

void InstrChannel::removeListener(ChannelListener* l)
{
    _notifier.removeListener(l);
}

//---------------------------------------------------------
//   PartChannelSettingsLink
//---------------------------------------------------------

PartChannelSettingsLink::PartChannelSettingsLink(InstrChannel* main, InstrChannel* bound, bool excerpt)
    : _main(main), _bound(bound), _excerpt(excerpt)
{
    if (excerpt) {
        for (InstrChannel::Prop p : excerptProperties) {
            applyProperty(p, /* from */ bound, /* to */ main);
        }
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
    if (this != &other) {
        swap(*this, other);
    }
    return *this;
}

//---------------------------------------------------------
//   swap
//---------------------------------------------------------

void swap(PartChannelSettingsLink& l1, PartChannelSettingsLink& l2)
{
    mu::engraving::swap(static_cast<ChannelListener&>(l1), static_cast<ChannelListener&>(l2));
    using std::swap;
    swap(l1._main, l2._main);
    swap(l1._bound, l2._bound);
    swap(l1._excerpt, l2._excerpt);
}

//---------------------------------------------------------
//   PartChannelSettingsLink::applyProperty
//---------------------------------------------------------

void PartChannelSettingsLink::applyProperty(InstrChannel::Prop p, const InstrChannel* from, InstrChannel* to)
{
    switch (p) {
    case InstrChannel::Prop::VOLUME:
        to->setVolume(from->volume());
        break;
    case InstrChannel::Prop::PAN:
        to->setPan(from->pan());
        break;
    case InstrChannel::Prop::CHORUS:
        to->setChorus(from->chorus());
        break;
    case InstrChannel::Prop::REVERB:
        to->setReverb(from->reverb());
        break;
    case InstrChannel::Prop::NAME:
        to->setName(from->name());
        break;
    case InstrChannel::Prop::PROGRAM:
        to->setProgram(from->program());
        break;
    case InstrChannel::Prop::BANK:
        to->setBank(from->bank());
        break;
    case InstrChannel::Prop::COLOR:
        to->setColor(from->color());
        break;
    case InstrChannel::Prop::SOLOMUTE:
        to->setSoloMute(from->soloMute());
        break;
    case InstrChannel::Prop::SOLO:
        to->setSolo(from->solo());
        break;
    case InstrChannel::Prop::MUTE:
        to->setMute(from->mute());
        break;
    case InstrChannel::Prop::SYNTI:
        to->setSynti(from->synti());
        break;
    case InstrChannel::Prop::CHANNEL:
        to->setChannel(from->channel());
        break;
    case InstrChannel::Prop::USER_BANK_CONTROL:
        to->setUserBankController(from->userBankController());
        break;
    }
}

//---------------------------------------------------------
//   PartChannelSettingsLink::propertyChanged
//---------------------------------------------------------

void PartChannelSettingsLink::propertyChanged(InstrChannel::Prop p)
{
    if (isExcerptProperty(p) == _excerpt) {
        applyProperty(p, _main, _bound);
    }
}

//---------------------------------------------------------
//   channelIdx
//---------------------------------------------------------

int Instrument::channelIdx(const String& s) const
{
    int idx = 0;
    for (const InstrChannel* a : _channel) {
        if (a->name().isEmpty() && s == InstrChannel::DEFAULT_NAME) {
            return idx;
        }
        if (s == a->name()) {
            return idx;
        }
        ++idx;
    }
    return -1;
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void MidiArticulation::write(XmlWriter& xml) const
{
    if (name.isEmpty()) {
        xml.startElement("Articulation");
    } else {
        xml.startElement("Articulation", { { "name", name } });
    }
    if (!descr.isEmpty()) {
        xml.tag("descr", descr);
    }
    xml.tag("velocity", velocity);
    xml.tag("gateTime", gateTime);
    xml.endElement();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void MidiArticulation::read(XmlReader& e)
{
    name = e.attribute("name");
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "velocity") {
            String text(e.readText());
            if (text.endsWith(u"%")) {
                text = text.left(text.size() - 1);
            }
            velocity = text.toInt();
        } else if (tag == "gateTime") {
            String text(e.readText());
            if (text.endsWith(u"%")) {
                text = text.left(text.size() - 1);
            }
            gateTime = text.toInt();
        } else if (tag == "descr") {
            descr = e.readText();
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   updateVelocity
//---------------------------------------------------------

void Instrument::updateVelocity(int* velocity, int /*channelIdx*/, const String& name)
{
    *velocity *= getVelocityMultiplier(name);
}

//---------------------------------------------------------
//   updateVelocity
//---------------------------------------------------------

double Instrument::getVelocityMultiplier(const String& name)
{
    for (const MidiArticulation& a : _articulation) {
        if (a.name == name) {
            return double(a.velocity) / 100;
        }
    }
    return 1;
}

//---------------------------------------------------------
//   updateGateTime
//---------------------------------------------------------

void Instrument::updateGateTime(int* gateTime, int /*channelIdx*/, const String& name)
{
    for (const MidiArticulation& a : _articulation) {
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
    if (!synth || (expressive && !singleNoteDynamics())) {
        return;
    }

    for (InstrChannel* c : channel()) {
        c->switchExpressive(synth, expressive, force);
        if (score->playbackChannel(c)) {
            score->playbackChannel(c)->switchExpressive(synth, expressive, force);
        }
    }
}

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool Instrument::operator==(const Instrument& i) const
{
    bool equal = i._longNames == _longNames;
    equal &= i._shortNames == _shortNames;

    if (i._channel.size() == _channel.size()) {
        for (size_t cur = 0; cur < _channel.size(); cur++) {
            if (*i._channel[cur] != *_channel[cur]) {
                return false;
            }
        }
    } else {
        return false;
    }

    equal &= i._minPitchA == _minPitchA;
    equal &= i._maxPitchA == _maxPitchA;
    equal &= i._minPitchP == _minPitchP;
    equal &= i._maxPitchP == _maxPitchP;
    equal &= i._useDrumset == _useDrumset;
    equal &= i._midiActions == _midiActions;
    equal &= i._articulation == _articulation;
    equal &= i._transpose.diatonic == _transpose.diatonic;
    equal &= i._transpose.chromatic == _transpose.chromatic;
    equal &= i._trackName == _trackName;
    equal &= *i.stringData() == *stringData();
    equal &= i._singleNoteDynamics == _singleNoteDynamics;

    return equal;
}

bool Instrument::operator!=(const Instrument& i) const
{
    return !(*this == i);
}

//---------------------------------------------------------
//   isDifferentInstrument
///   Checks if the passed instrument is a different instrument.
///   Does not compare channels.
//---------------------------------------------------------

bool Instrument::isDifferentInstrument(const Instrument& i) const
{
    return this->operator!=(i);
}

String Instrument::family() const
{
    auto search = searchTemplateIndexForId(_id);

    if (!search.instrTemplate) {
        static String empty;
        return empty;
    }

    return search.instrTemplate->familyId();
}

String StaffName::toPlainText() const
{
    return TextBase::unEscape(_name);
}

StaffName StaffName::fromPlainText(const String& plainText, int pos)
{
    return { TextBase::plainToXmlText(plainText), pos };
}

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool StaffName::operator==(const StaffName& i) const
{
    return (i._pos == _pos) && (i._name == _name);
}

String StaffName::toString() const
{
    return _name;
}

//---------------------------------------------------------
//   setUseDrumset
//---------------------------------------------------------

void Instrument::setUseDrumset(bool val)
{
    _useDrumset = val;
    if (val && !_drumset) {
        _drumset = new Drumset(*smDrumset);
    }
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
    } else {
        _useDrumset = false;
        _drumset = 0;
    }
}

//---------------------------------------------------------
//   setLongName
//    f is in richtext format
//---------------------------------------------------------

void Instrument::setLongName(const String& f)
{
    _longNames.clear();
    if (f.size() > 0) {
        _longNames.push_back(StaffName(f, 0));
    }
}

//---------------------------------------------------------
//   setShortName
//    f is in richtext format
//---------------------------------------------------------

void Instrument::setShortName(const String& f)
{
    _shortNames.clear();
    if (f.size() > 0) {
        _shortNames.push_back(StaffName(f, 0));
    }
}

//---------------------------------------------------------
//   addLongName
//---------------------------------------------------------

void Instrument::addLongName(const StaffName& f)
{
    _longNames.push_back(f);
}

//---------------------------------------------------------
//   addShortName
//---------------------------------------------------------

void Instrument::addShortName(const StaffName& f)
{
    _shortNames.push_back(f);
}

size_t Instrument::cleffTypeCount() const
{
    return _clefType.size();
}

//---------------------------------------------------------
//   clefType
//---------------------------------------------------------

ClefTypeList Instrument::clefType(size_t staffIdx) const
{
    if (staffIdx >= _clefType.size()) {
        if (_clefType.empty()) {
            return ClefTypeList();
        }
        return _clefType[0];
    }
    return _clefType[staffIdx];
}

//---------------------------------------------------------
//   setClefType
//---------------------------------------------------------

void Instrument::setClefType(size_t staffIdx, const ClefTypeList& c)
{
    while (_clefType.size() <= staffIdx) {
        _clefType.push_back(ClefTypeList());
    }
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

String Instrument::instrumentId() const
{
    return _instrumentId;
}

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

const Instrument* InstrumentList::instrument(int tick) const
{
    if (empty()) {
        return &defaultInstrument;
    }
    auto i = upper_bound(tick);
    if (i == begin()) {
        return &defaultInstrument;
    }
    --i;
    return i->second;
}

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

Instrument* InstrumentList::instrument(int tick)
{
    if (empty()) {
        return &defaultInstrument;
    }
    auto i = upper_bound(tick);
    if (i == begin()) {
        return &defaultInstrument;
    }
    --i;
    return i->second;
}

//---------------------------------------------------------
//   setInstrument
//---------------------------------------------------------

void InstrumentList::setInstrument(Instrument* instr, int tick)
{
    if (!insert({ tick, instr }).second) {
        (*this)[tick] = instr;
    }
}

bool InstrumentList::contains(const std::string& instrumentId) const
{
    for (const auto& pair : *this) {
        if (pair.second->id().toStdString() == instrumentId) {
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------
//   longName
//---------------------------------------------------------

const std::list<StaffName>& Instrument::longNames() const
{
    return _longNames;
}

//---------------------------------------------------------
//   shortName
//---------------------------------------------------------

const std::list<StaffName>& Instrument::shortNames() const
{
    return _shortNames;
}

//---------------------------------------------------------
//   longName
//---------------------------------------------------------

std::list<StaffName>& Instrument::longNames()
{
    return _longNames;
}

//---------------------------------------------------------
//   shortName
//---------------------------------------------------------

std::list<StaffName>& Instrument::shortNames()
{
    return _shortNames;
}

//---------------------------------------------------------
//   trackName
//---------------------------------------------------------

String Instrument::trackName() const
{
    return _trackName;
}

void Instrument::setTrackName(const String& s)
{
    _trackName = s;
}

String Instrument::nameAsXmlText() const
{
    return !_longNames.empty() ? _longNames.front().name() : String();
}

String Instrument::nameAsPlainText() const
{
    return !_longNames.empty() ? _longNames.front().toPlainText() : String();
}

String Instrument::abbreviatureAsXmlText() const
{
    return !_shortNames.empty() ? _shortNames.front().name() : String();
}

String Instrument::abbreviatureAsPlainText() const
{
    return !_shortNames.empty() ? _shortNames.front().toPlainText() : String();
}

//---------------------------------------------------------
//   fromTemplate
//---------------------------------------------------------

Instrument Instrument::fromTemplate(const InstrumentTemplate* templ)
{
    Instrument instrument(templ->id);
    instrument.setAmateurPitchRange(templ->minPitchA, templ->maxPitchA);
    instrument.setProfessionalPitchRange(templ->minPitchP, templ->maxPitchP);

    for (const StaffName& sn : templ->longNames) {
        instrument.addLongName(StaffName(sn.name(), sn.pos()));
    }

    for (const StaffName& sn : templ->shortNames) {
        instrument.addShortName(StaffName(sn.name(), sn.pos()));
    }

    instrument.setTrackName(templ->trackName);
    instrument.setTranspose(templ->transpose);
    instrument.setInstrumentId(templ->musicXMLid);
    instrument._useDrumset = templ->useDrumset;

    if (templ->useDrumset) {
        instrument.setDrumset(templ->drumset ? templ->drumset : smDrumset);
    }

    for (staff_idx_t i = 0; i < templ->staffCount; ++i) {
        instrument.setClefType(i, templ->clefTypes[i]);
    }

    instrument.setMidiActions(templ->midiActions);
    instrument.setArticulation(templ->midiArticulations);
    instrument._channel.clear();

    for (const InstrChannel& c : templ->channel) {
        instrument._channel.push_back(new InstrChannel(c));
    }

    instrument.setStringData(templ->stringData);
    instrument.setSingleNoteDynamics(templ->singleNoteDynamics);
    instrument.setTrait(templ->trait);

    return instrument;
}

Trait Instrument::trait() const
{
    return _trait;
}

void Instrument::setTrait(const Trait& trait)
{
    _trait = trait;
}

bool Instrument::isPrimary() const
{
    return _isPrimary;
}

void Instrument::setIsPrimary(bool isPrimary)
{
    _isPrimary = isPrimary;
}

void Instrument::updateInstrumentId()
{
    if (_instrumentId.isEmpty()) {
        _instrumentId = recognizeInstrumentId();
    }

    if (_id.isEmpty()) {
        _id = recognizeId();
    }
}

//---------------------------------------------------------
//   Instrument::playbackChannel
//---------------------------------------------------------

const InstrChannel* Instrument::playbackChannel(int idx, const MasterScore* score) const
{
    return score->playbackChannel(channel(idx));
}

//---------------------------------------------------------
//   Instrument::playbackChannel
//---------------------------------------------------------

InstrChannel* Instrument::playbackChannel(int idx, MasterScore* score)
{
    return score->playbackChannel(channel(idx));
}

//---------------------------------------------------------
//   getSingleNoteDynamicsFromTemplate
//---------------------------------------------------------

bool Instrument::getSingleNoteDynamicsFromTemplate() const
{
    String templateName = trackName().toLower().replace(u" ", u"-").replace(u"♭", u"b");
    InstrumentTemplate* tp = searchTemplate(templateName);
    if (tp) {
        return tp->singleNoteDynamics;
    }
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
    for (const StaffName& sn : *this) {
        sn.write(xml, name);
    }
}

std::list<String> StaffNameList::toStringList() const
{
    std::list<String> result;

    for (const StaffName& name : *this) {
        result.push_back(name.toString());
    }

    return result;
}
}
