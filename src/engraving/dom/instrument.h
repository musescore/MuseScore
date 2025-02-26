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

#ifndef MU_ENGRAVING_INSTRUMENT_H
#define MU_ENGRAVING_INSTRUMENT_H

#include <list>

#include "global/containers.h"
#include "global/types/string.h"

#include "clef.h"
#include "interval.h"
#include "notifier.h"
#include "stringdata.h"

#include "../compat/midi/midicoreevent.h"

namespace mu::engraving {
class ChannelListener;
class Drumset;
class InstrumentTemplate;
class MasterScore;
class Part;
class StringData;
class Synthesizer;

//---------------------------------------------------------
//   StaffName
//---------------------------------------------------------

class StaffName
{
public:
    StaffName() = default;
    StaffName(const String& xmlText, int pos = 0);

    String toPlainText() const;
    static StaffName fromPlainText(const String& plainText, int pos = 0);

    bool operator==(const StaffName&) const;
    String toString() const;
    int pos() const { return m_pos; }
    void setPos(int p) { m_pos = p; }
    String name() const { return m_name; }
    void setName(const String& n) { m_name = n; }

private:
    String m_name;       // html string
    int m_pos = 0;       // even number -> between staves
};

//---------------------------------------------------------
//   StaffNameList
//---------------------------------------------------------

class StaffNameList : public std::list<StaffName>
{
    OBJECT_ALLOCATOR(engraving, StaffNameList)
public:
    StaffNameList() = default;
    StaffNameList(const std::list<StaffName>& l)
        : std::list<StaffName>(l) {}

    std::list<String> toStringList() const;
};

//---------------------------------------------------------
//   NamedEventList
//---------------------------------------------------------

struct NamedEventList {
    String name;
    String descr;
    std::vector<MidiCoreEvent> events;

    bool operator==(const NamedEventList& i) const { return i.name == name && i.events == events; }
};

//---------------------------------------------------------
//   MidiArticulation
//---------------------------------------------------------

struct MidiArticulation {
    String name;
    String descr;
    int velocity = 0;         // velocity change: -100% - +100%
    int gateTime = 0;         // gate time change: -100% - +100%

    MidiArticulation() {}
    MidiArticulation(const String& n, const String& d, int v, int g)
        : name(n), descr(d), velocity(v), gateTime(g) {}

    bool operator==(const MidiArticulation& i) const;
};

//---------------------------------------------------------
//   Channel
//---------------------------------------------------------

class InstrChannel
{
public:
    static const char* DEFAULT_NAME;
    static const char* HARMONY_NAME;
    static const char* PALM_MUTE_NAME;
    static const int DEFAULT_COLOR = 0x3399ff;
    static constexpr char defaultVolume = 100;

    enum class A : char {
        HBANK, LBANK, PROGRAM, VOLUME, PAN, CHORUS, REVERB,
        INIT_COUNT
    };

    enum class Prop : char {
        VOLUME, PAN, CHORUS, REVERB, NAME, PROGRAM, BANK, COLOR,
        SYNTI, CHANNEL, USER_BANK_CONTROL
    };

    std::vector<MidiCoreEvent>& initList() const;

    String name() const { return m_name; }
    void setName(const String& value);
    String synti() const { return m_synti; }
    void setSynti(const String& value);
    int color() const { return m_color; }
    void setColor(int value);

    char volume() const { return m_volume; }
    void setVolume(char value);
    char pan() const { return m_pan; }
    void setPan(char value);
    char chorus() const { return m_chorus; }
    void setChorus(char value);
    char reverb() const { return m_reverb; }
    void setReverb(char value);

    void addToInit(const MidiCoreEvent& e) { m_init.push_back(e); }
    void setMustUpdateInit(bool arg) { m_mustUpdateInit = arg; }

    int program() const { return m_program; }
    void setProgram(int value);
    int bank() const { return m_bank; }
    void setBank(int value);
    int channel() const { return m_channel; }
    void setChannel(int value);

    // If the bank controller is set by the user or not
    bool userBankController() const { return m_userBankController; }
    void setUserBankController(bool val);

    bool isHarmonyChannel() const { return m_name == String::fromUtf8(InstrChannel::HARMONY_NAME); }

    std::list<NamedEventList> midiActions;
    std::vector<MidiArticulation> articulation;

    InstrChannel();

    void updateInitList() const;
    bool operator==(const InstrChannel& c) const { return (m_name == c.m_name) && (m_channel == c.m_channel); }
    bool operator!=(const InstrChannel& c) const { return !(*this == c); }

    void addListener(ChannelListener* l);
    void removeListener(ChannelListener* l);

    void switchExpressive(Synthesizer* synth, bool expressive, bool force = false);

    void setNotifyAboutChangedEnabled(bool arg) { m_notifyAboutChangedEnabled = arg; }

private:
    void firePropertyChanged(InstrChannel::Prop prop)
    {
        if (m_notifyAboutChangedEnabled) {
            m_notifier.notify(prop);
        }
    }

    Notifier<InstrChannel::Prop> m_notifier;
    bool m_notifyAboutChangedEnabled = true;

    // this are the indexes of controllers which are always present in
    // Channel init EventList (maybe zero)
    String m_name;

    int m_color = 0;    //rgb

    String m_synti;

    char m_volume = 0;
    char m_pan = 0;

    char m_chorus = 0;
    char m_reverb = 0;

    int m_program = 0;       // current values as shown in mixer
    int m_bank = 0;          // initialized from "init"
    int m_channel = 0;       // mscore channel number, mapped to midi port/channel

    // MuseScore General-specific SND flags:
    //! TODO Needs porting to MU4
    bool m_userBankController = false;     // if the user has changed the bank controller as opposed to switchExpressive
    //bool _switchedToExpressive = false;   // if the patch has been automatically switched to an expr variant

    mutable std::vector<MidiCoreEvent> m_init;
    mutable bool m_mustUpdateInit = true;
};

//---------------------------------------------------------
//   ChannelListener
//---------------------------------------------------------

class ChannelListener : public Listener<InstrChannel::Prop>
{
    OBJECT_ALLOCATOR(engraving, ChannelListener)
public:
    virtual void propertyChanged(InstrChannel::Prop property) = 0;
    void setNotifier(InstrChannel* ch)
    {
        Listener::setNotifier(nullptr);
        if (ch) {
            ch->addListener(this);
        }
    }

private:
    void receive(InstrChannel::Prop prop) override { propertyChanged(prop); }
};

//---------------------------------------------------------
//   PartChannelSettingsLink
//---------------------------------------------------------

class PartChannelSettingsLink final : private ChannelListener
{
public:
    PartChannelSettingsLink() = default;
    PartChannelSettingsLink(InstrChannel* main, InstrChannel* bound, bool excerpt);
    PartChannelSettingsLink(const PartChannelSettingsLink&) = delete;
    PartChannelSettingsLink(PartChannelSettingsLink&&);
    PartChannelSettingsLink& operator=(const PartChannelSettingsLink&) = delete;
    PartChannelSettingsLink& operator=(PartChannelSettingsLink&&);
    ~PartChannelSettingsLink() {}

    friend void swap(PartChannelSettingsLink&, PartChannelSettingsLink&);

private:

    static void applyProperty(InstrChannel::Prop p, const InstrChannel* from, InstrChannel* to);
    void propertyChanged(InstrChannel::Prop p) override;

    InstrChannel* m_main = nullptr;
    InstrChannel* m_bound = nullptr;
    bool m_excerpt = false;
};

//---------------------------------------------------------
//   Trait
//---------------------------------------------------------

enum class TraitType : unsigned char
{
    Unknown,
    Tuning,
    Transposition,
    Course
};

struct Trait
{
    String name;

    TraitType type = TraitType::Unknown;

    bool isDefault = false;
    bool isHiddenOnScore = false;

    bool isValid() const { return !name.isEmpty(); }
};

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

class Instrument
{
public:
    Instrument(String id = String());
    Instrument(const Instrument&);
    ~Instrument();

    NamedEventList* midiAction(const String& s, int channel) const;
    int channelIdx(const String& s) const;
    double getVelocityMultiplier(const String& name) const;
    void updateGateTime(int* gateTime, const String& name) const;

    String recognizeMusicXmlId() const;
    String recognizeId() const;
    int recognizeMidiProgram() const;

    void operator=(const Instrument&);
    bool operator==(const Instrument&) const;
    bool operator!=(const Instrument&) const;

    bool isDifferentInstrument(const Instrument& i) const;

    String id() const { return m_id; }
    const String& soundId() const { return m_soundId; }
    void setSoundId(const String& id) { m_soundId = id; }
    String family() const;
    void setId(const String& id) { m_id = id; }
    void setMinPitchP(int v) { m_minPitchP = v; }
    void setMaxPitchP(int v) { m_maxPitchP = v; }
    void setMinPitchA(int v) { m_minPitchA = v; }
    void setMaxPitchA(int v) { m_maxPitchA = v; }
    Interval transpose() const { return m_transpose; }
    void setTranspose(const Interval& v) { m_transpose = v; }
    void setMusicXmlId(const String& musicXmlId) { m_musicXmlId = musicXmlId; }

    void setDrumset(const Drumset* ds);
    const Drumset* drumset() const { return m_drumset; }
    Drumset* drumset() { return m_drumset; }
    bool useDrumset() const { return m_useDrumset; }
    void setUseDrumset(bool val);
    void setAmateurPitchRange(int a, int b) { m_minPitchA = a; m_maxPitchA = b; }
    void setProfessionalPitchRange(int a, int b) { m_minPitchP = a; m_maxPitchP = b; }
    InstrChannel* channel(int idx) { return muse::value(m_channel, idx); }
    const InstrChannel* channel(int idx) const { return muse::value(m_channel, idx); }
    InstrChannel* playbackChannel(int idx, MasterScore*);
    const InstrChannel* playbackChannel(int idx, const MasterScore*) const;
    size_t cleffTypeCount() const;
    ClefTypeList clefType(size_t staffIdx) const;
    void setClefType(size_t staffIdx, const ClefTypeList& c);

    const std::list<NamedEventList>& midiActions() const { return m_midiActions; }
    void addMidiAction(const NamedEventList& l) { m_midiActions.push_back(l); }

    const std::vector<MidiArticulation>& articulation() const { return m_articulation; }
    void addMidiArticulation(const MidiArticulation& a) { m_articulation.push_back(a); }

    const std::vector<InstrChannel*>& channel() const { return m_channel; }
    void appendChannel(InstrChannel* c) { m_channel.push_back(c); }
    void removeChannel(InstrChannel* c) { muse::remove(m_channel, c); }
    void clearChannels() { m_channel.clear(); }

    void setMidiActions(const std::list<NamedEventList>& l) { m_midiActions = l; }
    void setArticulation(const std::vector<MidiArticulation>& l) { m_articulation = l; }
    const StringData* stringData() const { return &m_stringData; }
    void setStringData(const StringData& d) { m_stringData.set(d); }
    bool hasStrings() const { return m_stringData.strings() > 0; }

    void setLongName(const String& f);
    void setShortName(const String& f);

    void addLongName(const StaffName& f);
    void addShortName(const StaffName& f);

    int minPitchP() const;
    int maxPitchP() const;
    int minPitchA() const;
    int maxPitchA() const;
    String musicXmlId() const;

    const StaffNameList& longNames() const;
    const StaffNameList& shortNames() const;
    void setLongNames(const StaffNameList& l);
    void setShortNames(const StaffNameList& l);
    void appendLongName(const StaffName& n);
    void appendShortName(const StaffName& n);

    String trackName() const;
    void setTrackName(const String& s);
    String nameAsXmlText() const;
    String nameAsPlainText() const;
    String abbreviatureAsXmlText() const;
    String abbreviatureAsPlainText() const;
    static Instrument fromTemplate(const InstrumentTemplate* t);

    Trait trait() const;
    void setTrait(const Trait& trait);

    bool isPrimary() const;
    void setIsPrimary(bool isPrimary);

    void updateInstrumentId();

    bool singleNoteDynamics() const { return m_singleNoteDynamics; }
    void setSingleNoteDynamics(bool val) { m_singleNoteDynamics = val; }
    void setSingleNoteDynamicsFromTemplate();
    bool getSingleNoteDynamicsFromTemplate() const;
    void switchExpressive(MasterScore* score, Synthesizer* synth, bool expressive, bool force = false);

    bool isVocalInstrument() const;
    bool isNormallyMultiStaveInstrument() const;

private:

    StaffNameList m_longNames;
    StaffNameList m_shortNames;
    String m_trackName;
    String m_id;
    String m_soundId;

    int m_minPitchA = 0;
    int m_maxPitchA = 0;
    int m_minPitchP = 0;
    int m_maxPitchP = 0;
    Interval m_transpose;
    String m_musicXmlId;

    bool m_useDrumset = false;
    Drumset* m_drumset = nullptr;
    StringData m_stringData;

    std::list<NamedEventList> m_midiActions;
    std::vector<MidiArticulation> m_articulation;
    std::vector<InstrChannel*> m_channel;        // at least one entry
    std::vector<ClefTypeList> m_clefType;

    bool m_singleNoteDynamics = false;

    Trait m_trait;
    bool m_isPrimary = false;
};

//---------------------------------------------------------
//   InstrumentList
//---------------------------------------------------------

class InstrumentList : public std::map<const int, Instrument*>
{
    OBJECT_ALLOCATOR(engraving, InstrumentList)

public:
    InstrumentList() {}
    const Instrument* instrument(int tick) const;
    Instrument* instrument(int tick);
    void setInstrument(Instrument*, int tick);
    bool contains(const String& instrumentId) const;

private:

    static Instrument defaultInstrument;
};
} // namespace mu::engraving
#endif
