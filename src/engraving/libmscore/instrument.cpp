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

#include "infrastructure/htmlparser.h"
#include "types/typesconv.h"

#include "rw/xmlreader.h"
#include "rw/xmlwriter.h"
#include "rw/410/readcontext.h"
#include "rw/410/tread.h"
#include "rw/410/twrite.h"

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
const char* InstrChannel::PALM_MUTE_NAME = QT_TRANSLATE_NOOP("engraving/instruments", "palmmute");

Instrument InstrumentList::defaultInstrument;

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
    _musicXmlId   = i._musicXmlId;
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
    _musicXmlId   = i._musicXmlId;
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

String Instrument::recognizeMusicXmlId() const
{
    static const String defaultMusicXmlId(u"keyboard.piano");

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
        return defaultMusicXmlId;
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

    return defaultMusicXmlId;
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
    const bool groupHack = musicXmlId() == String(u"strings.group");
    const int idxref = channelIdx(arco);
    const int val32ref = (idxref < 0) ? -1 : channel(idxref)->bank();
    String fallback;
    int bestMatchStrength = 0;     // higher when fallback ID provides better match for instrument data

    for (InstrumentGroup* g : instrumentGroups) {
        for (InstrumentTemplate* it : g->instrumentTemplates) {
            if (it->musicXMLid != musicXmlId()) {
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
    InstrumentTemplate* tmplInstrumentId = mu::engraving::searchTemplateForMusicXmlId(_musicXmlId);

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
    applyProperty(p, _main, _bound);
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
            // Imagine ["staccato", "accent"] articulations
            // accent will override the gate time,
            // so we have to take the minimum value from all articulations
            *gateTime = std::min(*gateTime, a.gateTime);
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
//   musicXmlId
//---------------------------------------------------------

String Instrument::musicXmlId() const
{
    return _musicXmlId;
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

void Instrument::setLongNames(const StaffNameList& l)
{
    _longNames = l;
}

const StaffNameList& Instrument::longNames() const
{
    return _longNames;
}

void Instrument::setShortNames(const StaffNameList& l)
{
    _shortNames = l;
}

const StaffNameList& Instrument::shortNames() const
{
    return _shortNames;
}

void Instrument::appendLongName(const StaffName& n)
{
    _longNames.push_back(n);
}

void Instrument::appendShortName(const StaffName& n)
{
    _shortNames.push_back(n);
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
    instrument.setMusicXmlId(templ->musicXMLid);
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
    if (_musicXmlId.isEmpty()) {
        _musicXmlId = recognizeMusicXmlId();
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

std::list<String> StaffNameList::toStringList() const
{
    std::list<String> result;

    for (const StaffName& name : *this) {
        result.push_back(name.toString());
    }

    return result;
}
}
