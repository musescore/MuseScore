/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "drumset.h"
#include "instrtemplate.h"
#include "masterscore.h"
#include "stringdata.h"
#include "textbase.h"

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
    m_id = id;
    InstrChannel* a = new InstrChannel;
    a->setName(String::fromUtf8(InstrChannel::DEFAULT_NAME));
    m_channel.push_back(a);

    m_minPitchA   = 0;
    m_maxPitchA   = 127;
    m_minPitchP   = 0;
    m_maxPitchP   = 127;
    m_useDrumset  = false;
    m_drumset     = 0;
    m_singleNoteDynamics = true;
}

Instrument::Instrument(const Instrument& i)
{
    m_id           = i.m_id;
    m_soundId      = i.m_soundId;
    m_longNames    = i.m_longNames;
    m_shortNames   = i.m_shortNames;
    m_trackName    = i.m_trackName;
    m_minPitchA    = i.m_minPitchA;
    m_maxPitchA    = i.m_maxPitchA;
    m_minPitchP    = i.m_minPitchP;
    m_maxPitchP    = i.m_maxPitchP;
    m_transpose    = i.m_transpose;
    m_musicXmlId   = i.m_musicXmlId;
    m_stringData   = i.m_stringData;
    m_drumset      = 0;
    setDrumset(i.m_drumset);
    m_useDrumset   = i.m_useDrumset;
    m_stringData   = i.m_stringData;
    m_midiActions  = i.m_midiActions;
    m_articulation = i.m_articulation;
    m_singleNoteDynamics = i.m_singleNoteDynamics;
    for (InstrChannel* c : i.m_channel) {
        m_channel.push_back(new InstrChannel(*c));
    }
    m_clefType     = i.m_clefType;
    m_trait = i.m_trait;
}

void Instrument::operator=(const Instrument& i)
{
    muse::DeleteAll(m_channel);
    m_channel.clear();
    delete m_drumset;

    m_id           = i.m_id;
    m_soundId      = i.m_soundId;
    m_longNames    = i.m_longNames;
    m_shortNames   = i.m_shortNames;
    m_trackName    = i.m_trackName;
    m_minPitchA    = i.m_minPitchA;
    m_maxPitchA    = i.m_maxPitchA;
    m_minPitchP    = i.m_minPitchP;
    m_maxPitchP    = i.m_maxPitchP;
    m_transpose    = i.m_transpose;
    m_musicXmlId   = i.m_musicXmlId;
    m_stringData   = i.m_stringData;
    m_drumset      = 0;
    setDrumset(i.m_drumset);
    m_useDrumset   = i.m_useDrumset;
    m_stringData   = i.m_stringData;
    m_midiActions  = i.m_midiActions;
    m_articulation = i.m_articulation;
    m_singleNoteDynamics = i.m_singleNoteDynamics;
    for (InstrChannel* c : i.m_channel) {
        m_channel.push_back(new InstrChannel(*c));
    }
    m_clefType     = i.m_clefType;
    m_trait = i.m_trait;
}

//---------------------------------------------------------
//   ~Instrument
//---------------------------------------------------------

Instrument::~Instrument()
{
    muse::DeleteAll(m_channel);
    delete m_drumset;
    m_drumset = nullptr;
}

//---------------------------------------------------------
//   StaffName
//---------------------------------------------------------

StaffName::StaffName(const String& xmlText, int pos)
    : m_name(xmlText), m_pos(pos)
{
    TextBase::validateText(m_name); // enforce HTML encoding
}

String Instrument::recognizeMusicXmlId() const
{
    static const String defaultMusicXmlId(u"keyboard.piano");

    std::list<String> nameList;

    nameList.push_back(m_trackName);
    muse::join(nameList, m_longNames.toStringList());
    muse::join(nameList, m_shortNames.toStringList());

    const InstrumentTemplate* tmplByName = mu::engraving::searchTemplateForInstrNameList(nameList, m_useDrumset);

    if (tmplByName && !tmplByName->musicXmlId.isEmpty()) {
        return tmplByName->musicXmlId;
    }

    const InstrChannel* channel = this->channel(0);

    if (!channel) {
        return defaultMusicXmlId;
    }

    const InstrumentTemplate* tmplMidiProgram = mu::engraving::searchTemplateForMidiProgram(channel->bank(), channel->program(),
                                                                                            m_useDrumset);

    if (tmplMidiProgram && !tmplMidiProgram->musicXmlId.isEmpty()) {
        return tmplMidiProgram->musicXmlId;
    }

    if (m_useDrumset) {
        static const String drumsetId(u"drumset");
        return drumsetId;
    }

    return defaultMusicXmlId;
}

String Instrument::recognizeId() const
{
    // When reading a score created with pre-3.6, MuseScore's instrument ID
    // isn't saved in the score file, so we must try to guess the ID based on
    // the MusicXML ID, which is saved. However, MusicXML IDs are not unique,
    // so we must also consider other data to find the best match.

    if (m_musicXmlId.startsWith(u"mdl.")) {
        // Use fixed mapping for MDL1 instruments to ensure we get the
        // marching versions (e.g. "marching-snare" and not "snare-drum").
        // See https://github.com/musescore/mdl/blob/master/resources/instruments/mdl_1_3_0.xml
        if (m_musicXmlId == u"mdl.drum.snare-drum") {
            return u"marching-snare";
        } else if (m_musicXmlId == u"mdl.drum.tenor-drum") {
            return u"marching-tenor-drums";
        } else if (m_musicXmlId == u"mdl.drum.bass-drum") {
            return u"marching-bass-drums";
        } else if (m_musicXmlId == u"mdl.metal.cymbal.crash") {
            return u"marching-cymbals";
        } else if (m_musicXmlId == u"mdl.drum.group.set") {
            return u"drumset";
        }
    }

    // Several instruments have MusicXML ID "strings.group". Let's use the
    // value of controller 32 of the "arco" channel to distinguish them.
    const String arco = String(u"arco");
    const bool groupHack = m_musicXmlId == String(u"strings.group");
    const int idxref = channelIdx(arco);
    const int val32ref = (idxref < 0) ? -1 : channel(idxref)->bank();

    // For other instruments, consider how closely the instrument data
    // matches each of our templates. Use the ID that gives the best match.
    String fallback; // ID that gave the best match so far
    int bestMatchStrength = 0; // higher when ID is a better match

    for (const InstrumentGroup* g : instrumentGroups) {
        for (const InstrumentTemplate* it : g->instrumentTemplates) {
            if (it->musicXmlId != m_musicXmlId) {
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
    const InstrumentTemplate* tmplInstrumentId = mu::engraving::searchTemplateForMusicXmlId(m_musicXmlId);

    if (tmplInstrumentId && !tmplInstrumentId->channel.empty() && tmplInstrumentId->channel[0].program() >= 0) {
        return tmplInstrumentId->channel[0].program();
    }

    std::list<String> nameList;

    nameList.push_back(m_trackName);
    muse::join(nameList, m_longNames.toStringList());
    muse::join(nameList, m_shortNames.toStringList());

    const InstrumentTemplate* tmplByName = mu::engraving::searchTemplateForInstrNameList(nameList);

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

    for (const NamedEventList& a : m_channel[channelIdx]->midiActions) {
        if (s == a.name) {
            return const_cast<NamedEventList*>(&a);
        }
    }

    for (const NamedEventList& a : m_midiActions) {
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
        m_init.push_back(MidiCoreEvent());
    }
    m_synti    = u"Fluid";       // default synthesizer
    m_channel  = -1;
    m_program  = -1;
    m_bank     = 0;
    m_volume   = defaultVolume;
    m_pan      = 64;   // actually 63.5 for center
    m_chorus   = 0;
    m_reverb   = 0;
    m_color = DEFAULT_COLOR;
}

//---------------------------------------------------------
//   initList
//---------------------------------------------------------

std::vector<MidiCoreEvent>& InstrChannel::initList() const
{
    if (m_mustUpdateInit) {
        updateInitList();
        m_mustUpdateInit = false;
    }
    return m_init;
}

//---------------------------------------------------------
//   setVolume
//---------------------------------------------------------

void InstrChannel::setVolume(char value)
{
    if (m_volume != value) {
        m_volume = value;
        firePropertyChanged(Prop::VOLUME);
    }
    m_mustUpdateInit = true;
}

//---------------------------------------------------------
//   setPan
//---------------------------------------------------------

void InstrChannel::setPan(char value)
{
    if (m_pan != value) {
        m_pan = value;
        firePropertyChanged(Prop::PAN);
    }
    m_mustUpdateInit = true;
}

//---------------------------------------------------------
//   setChorus
//---------------------------------------------------------

void InstrChannel::setChorus(char value)
{
    if (m_chorus != value) {
        m_chorus = value;
        firePropertyChanged(Prop::CHORUS);
    }
    m_mustUpdateInit = true;
}

//---------------------------------------------------------
//   setReverb
//---------------------------------------------------------

void InstrChannel::setReverb(char value)
{
    if (m_reverb != value) {
        m_reverb = value;
        firePropertyChanged(Prop::REVERB);
    }
    m_mustUpdateInit = true;
}

//---------------------------------------------------------
//   setName
//---------------------------------------------------------

void InstrChannel::setName(const String& value)
{
    if (m_name != value) {
        m_name = value;
        firePropertyChanged(Prop::NAME);
    }
}

//---------------------------------------------------------
//   setSynti
//---------------------------------------------------------

void InstrChannel::setSynti(const String& value)
{
    if (m_synti != value) {
        m_synti = value;
        firePropertyChanged(Prop::SYNTI);
    }
}

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void InstrChannel::setColor(int value)
{
    if (m_color != value) {
        m_color = value;
        firePropertyChanged(Prop::COLOR);
    }
}

//---------------------------------------------------------
//   setProgram
//---------------------------------------------------------

void InstrChannel::setProgram(int value)
{
    if (m_program != value) {
        m_program = value;
        firePropertyChanged(Prop::PROGRAM);
    }
    m_mustUpdateInit = true;
}

//---------------------------------------------------------
//   setBank
//---------------------------------------------------------

void InstrChannel::setBank(int value)
{
    if (m_bank != value) {
        m_bank = value;
        firePropertyChanged(Prop::BANK);
    }
    m_mustUpdateInit = true;
}

//---------------------------------------------------------
//   setChannel
//---------------------------------------------------------

void InstrChannel::setChannel(int value)
{
    if (m_channel != value) {
        m_channel = value;
        firePropertyChanged(Prop::CHANNEL);
    }
}

//---------------------------------------------------------
//   setUserBankController
//---------------------------------------------------------

void InstrChannel::setUserBankController(bool val)
{
    if (m_userBankController != val) {
        m_userBankController = val;
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
    if (m_program != -1) {
        e.setType(ME_CONTROLLER);
        e.setDataA(CTRL_PROGRAM);
        e.setDataB(m_program);
        m_init[int(A::PROGRAM)] = e;
    }

    e.setData(ME_CONTROLLER, CTRL_HBANK, (m_bank >> 7) & 0x7f);
    m_init[int(A::HBANK)] = e;

    e.setData(ME_CONTROLLER, CTRL_LBANK, m_bank & 0x7f);
    m_init[int(A::LBANK)] = e;

    e.setData(ME_CONTROLLER, CTRL_VOLUME, volume());
    m_init[int(A::VOLUME)] = e;

    e.setData(ME_CONTROLLER, CTRL_PANPOT, pan());
    m_init[int(A::PAN)] = e;

    e.setData(ME_CONTROLLER, CTRL_CHORUS_SEND, chorus());
    m_init[int(A::CHORUS)] = e;

    e.setData(ME_CONTROLLER, CTRL_REVERB_SEND, reverb());
    m_init[int(A::REVERB)] = e;
}

//---------------------------------------------------------
//   addListener
//---------------------------------------------------------

void InstrChannel::addListener(ChannelListener* l)
{
    m_notifier.addListener(l);
}

//---------------------------------------------------------
//   removeListener
//---------------------------------------------------------

void InstrChannel::removeListener(ChannelListener* l)
{
    m_notifier.removeListener(l);
}

//---------------------------------------------------------
//   PartChannelSettingsLink
//---------------------------------------------------------

PartChannelSettingsLink::PartChannelSettingsLink(InstrChannel* main, InstrChannel* bound, bool excerpt)
    : m_main(main), m_bound(bound), m_excerpt(excerpt)
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
    m_main(nullptr), m_bound(nullptr), m_excerpt(false)
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
    swap(l1.m_main, l2.m_main);
    swap(l1.m_bound, l2.m_bound);
    swap(l1.m_excerpt, l2.m_excerpt);
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
    applyProperty(p, m_main, m_bound);
}

//---------------------------------------------------------
//   channelIdx
//---------------------------------------------------------

int Instrument::channelIdx(const String& s) const
{
    int idx = 0;
    for (const InstrChannel* a : m_channel) {
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
//   getVelocityMultiplier
//---------------------------------------------------------

double Instrument::getVelocityMultiplier(const String& name) const
{
    for (const MidiArticulation& a : m_articulation) {
        if (a.name == name) {
            return double(a.velocity) / 100;
        }
    }
    return 1;
}

//---------------------------------------------------------
//   updateGateTime
//---------------------------------------------------------

void Instrument::updateGateTime(int* gateTime, const String& name) const
{
    for (const MidiArticulation& a : m_articulation) {
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
//   switchExpressive
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

bool Instrument::isVocalInstrument() const
{
    String instrumentFamily = family();
    return instrumentFamily == u"voices" || instrumentFamily == u"voice-groups";
}

bool Instrument::isNormallyMultiStaveInstrument() const
{
    String instrumentFamily = family();
    return instrumentFamily == u"keyboards"
           || instrumentFamily == u"organs"
           || instrumentFamily == u"keyboard-percussion"
           || instrumentFamily == u"harps"
           || instrumentFamily == u"accordions";
}

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool Instrument::operator==(const Instrument& i) const
{
    bool equal = i.m_longNames == m_longNames;
    equal &= i.m_shortNames == m_shortNames;

    if (i.m_channel.size() == m_channel.size()) {
        for (size_t cur = 0; cur < m_channel.size(); cur++) {
            if (*i.m_channel[cur] != *m_channel[cur]) {
                return false;
            }
        }
    } else {
        return false;
    }

    equal &= i.m_minPitchA == m_minPitchA;
    equal &= i.m_maxPitchA == m_maxPitchA;
    equal &= i.m_minPitchP == m_minPitchP;
    equal &= i.m_maxPitchP == m_maxPitchP;
    equal &= i.m_useDrumset == m_useDrumset;
    equal &= i.m_midiActions == m_midiActions;
    equal &= i.m_articulation == m_articulation;
    equal &= i.m_transpose.diatonic == m_transpose.diatonic;
    equal &= i.m_transpose.chromatic == m_transpose.chromatic;
    equal &= i.m_trackName == m_trackName;
    equal &= *i.stringData() == *stringData();
    equal &= i.m_singleNoteDynamics == m_singleNoteDynamics;

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
    auto search = searchTemplateIndexForId(m_id);

    if (!search.instrTemplate) {
        static String empty;
        return empty;
    }

    return search.instrTemplate->familyId();
}

String StaffName::toPlainText() const
{
    return TextBase::unEscape(m_name);
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
    return (i.m_pos == m_pos) && (i.m_name == m_name);
}

String StaffName::toString() const
{
    return m_name;
}

//---------------------------------------------------------
//   setUseDrumset
//---------------------------------------------------------

void Instrument::setUseDrumset(bool val)
{
    m_useDrumset = val;
    if (val && !m_drumset) {
        m_drumset = new Drumset(*smDrumset);
    }
}

//---------------------------------------------------------
//   setDrumset
//---------------------------------------------------------

void Instrument::setDrumset(const Drumset* ds)
{
    delete m_drumset;
    if (ds) {
        m_useDrumset = true;
        m_drumset = new Drumset(*ds);
    } else {
        m_useDrumset = false;
        m_drumset = 0;
    }
}

//---------------------------------------------------------
//   setLongName
//    f is in richtext format
//---------------------------------------------------------

void Instrument::setLongName(const String& f)
{
    m_longNames.clear();
    if (f.size() > 0) {
        m_longNames.push_back(StaffName(f, 0));
    }
}

//---------------------------------------------------------
//   setShortName
//    f is in richtext format
//---------------------------------------------------------

void Instrument::setShortName(const String& f)
{
    m_shortNames.clear();
    if (f.size() > 0) {
        m_shortNames.push_back(StaffName(f, 0));
    }
}

//---------------------------------------------------------
//   addLongName
//---------------------------------------------------------

void Instrument::addLongName(const StaffName& f)
{
    m_longNames.push_back(f);
}

//---------------------------------------------------------
//   addShortName
//---------------------------------------------------------

void Instrument::addShortName(const StaffName& f)
{
    m_shortNames.push_back(f);
}

size_t Instrument::cleffTypeCount() const
{
    return m_clefType.size();
}

//---------------------------------------------------------
//   clefType
//---------------------------------------------------------

ClefTypeList Instrument::clefType(size_t staffIdx) const
{
    if (staffIdx >= m_clefType.size()) {
        if (m_clefType.empty()) {
            return ClefTypeList();
        }
        return m_clefType[0];
    }
    return m_clefType[staffIdx];
}

//---------------------------------------------------------
//   setClefType
//---------------------------------------------------------

void Instrument::setClefType(size_t staffIdx, const ClefTypeList& c)
{
    while (m_clefType.size() <= staffIdx) {
        m_clefType.push_back(ClefTypeList());
    }
    m_clefType[staffIdx] = c;
}

//---------------------------------------------------------
//   minPitchP
//---------------------------------------------------------

int Instrument::minPitchP() const
{
    return m_minPitchP;
}

//---------------------------------------------------------
//   maxPitchP
//---------------------------------------------------------

int Instrument::maxPitchP() const
{
    return m_maxPitchP;
}

//---------------------------------------------------------
//   minPitchA
//---------------------------------------------------------

int Instrument::minPitchA() const
{
    return m_minPitchA;
}

//---------------------------------------------------------
//   maxPitchA
//---------------------------------------------------------

int Instrument::maxPitchA() const
{
    return m_maxPitchA;
}

//---------------------------------------------------------
//   musicXmlId
//---------------------------------------------------------

String Instrument::musicXmlId() const
{
    return m_musicXmlId;
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

bool InstrumentList::contains(const String& instrumentId) const
{
    for (const auto& pair : *this) {
        if (pair.second->id() == instrumentId) {
            return true;
        }
    }

    return false;
}

void Instrument::setLongNames(const StaffNameList& l)
{
    m_longNames = l;
}

const StaffNameList& Instrument::longNames() const
{
    return m_longNames;
}

void Instrument::setShortNames(const StaffNameList& l)
{
    m_shortNames = l;
}

const StaffNameList& Instrument::shortNames() const
{
    return m_shortNames;
}

void Instrument::appendLongName(const StaffName& n)
{
    m_longNames.push_back(n);
}

void Instrument::appendShortName(const StaffName& n)
{
    m_shortNames.push_back(n);
}

//---------------------------------------------------------
//   trackName
//---------------------------------------------------------

String Instrument::trackName() const
{
    return m_trackName;
}

void Instrument::setTrackName(const String& s)
{
    m_trackName = s;
}

String Instrument::nameAsXmlText() const
{
    return !m_longNames.empty() ? m_longNames.front().name() : String();
}

String Instrument::nameAsPlainText() const
{
    return !m_longNames.empty() ? m_longNames.front().toPlainText() : String();
}

String Instrument::abbreviatureAsXmlText() const
{
    return !m_shortNames.empty() ? m_shortNames.front().name() : String();
}

String Instrument::abbreviatureAsPlainText() const
{
    return !m_shortNames.empty() ? m_shortNames.front().toPlainText() : String();
}

Instrument Instrument::fromTemplate(const InstrumentTemplate* templ)
{
    Instrument instrument(templ->id);
    instrument.setSoundId(templ->soundId);
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
    instrument.setMusicXmlId(templ->musicXmlId);
    instrument.m_useDrumset = templ->useDrumset;

    if (templ->useDrumset) {
        instrument.setDrumset(templ->drumset ? templ->drumset : smDrumset);
    }

    for (staff_idx_t i = 0; i < templ->staffCount; ++i) {
        instrument.setClefType(i, templ->clefTypes[i]);
    }

    instrument.setMidiActions(templ->midiActions);
    instrument.setArticulation(templ->midiArticulations);
    instrument.m_channel.clear();

    for (const InstrChannel& c : templ->channel) {
        instrument.m_channel.push_back(new InstrChannel(c));
    }

    instrument.setStringData(templ->stringData);
    instrument.setSingleNoteDynamics(templ->singleNoteDynamics);
    instrument.setTrait(templ->trait);

    return instrument;
}

Trait Instrument::trait() const
{
    return m_trait;
}

void Instrument::setTrait(const Trait& trait)
{
    m_trait = trait;
}

bool Instrument::isPrimary() const
{
    return m_isPrimary;
}

void Instrument::setIsPrimary(bool isPrimary)
{
    m_isPrimary = isPrimary;
}

void Instrument::updateInstrumentId()
{
    if (m_musicXmlId.isEmpty()) {
        m_musicXmlId = recognizeMusicXmlId();
    }

    if (m_id.isEmpty()) {
        m_id = recognizeId();
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
    const InstrumentTemplate* tp = searchTemplate(templateName);
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
