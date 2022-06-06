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

#ifndef __INSTRUMENT_H__
#define __INSTRUMENT_H__

#include <QtGlobal>
#include <QString>
#include <list>

#include "containers.h"

#include "stringdata.h"
#include "mscore.h"
#include "notifier.hpp"
#include "interval.h"
#include "clef.h"

#include "compat/midi/midicoreevent.h"

namespace mu::engraving {
class InstrumentTemplate;
class MasterScore;
class XmlWriter;
class XmlReader;
class Drumset;
class StringData;
class ChannelListener;
class Synthesizer;

//---------------------------------------------------------
//   StaffName
//---------------------------------------------------------

class StaffName
{
    QString _name;      // html string
    int _pos = 0;       // even number -> between staves

public:
    StaffName() {}
    StaffName(const QString& s, int p=0);

    bool operator==(const StaffName&) const;
    QString toString() const;
    void read(XmlReader&);
    void write(XmlWriter& xml, const char* name) const;
    int pos() const { return _pos; }
    QString name() const { return _name; }
};

//---------------------------------------------------------
//   StaffNameList
//---------------------------------------------------------

class StaffNameList : public std::list<StaffName>
{
public:
    void write(XmlWriter& xml, const char* name) const;
    std::list<QString> toStringList() const;
};

//---------------------------------------------------------
//   NamedEventList
//---------------------------------------------------------

struct NamedEventList {
    QString name;
    QString descr;
    std::vector<MidiCoreEvent> events;

    void write(XmlWriter&, const AsciiString& name) const;
    void read(XmlReader&);
    bool operator==(const NamedEventList& i) const { return i.name == name && i.events == events; }
};

//---------------------------------------------------------
//   MidiArticulation
//---------------------------------------------------------

struct MidiArticulation {
    QString name;
    QString descr;
    int velocity = 0;         // velocity change: -100% - +100%
    int gateTime = 0;         // gate time change: -100% - +100%
    void write(XmlWriter&) const;
    void read(XmlReader&);

    MidiArticulation() {}
    MidiArticulation(const QString& n, const QString& d, int v, int g)
        : name(n), descr(d), velocity(v), gateTime(g) {}
    bool operator==(const MidiArticulation& i) const;
};

//---------------------------------------------------------
//   Channel
//---------------------------------------------------------

class InstrChannel
{
    // this are the indexes of controllers which are always present in
    // Channel init EventList (maybe zero)
    QString _name;
    QString _descr;

    static const int DEFAULT_COLOR = 0x3399ff;
    int _color;    //rgb

    QString _synti;

    char _volume;
    char _pan;

    char _chorus;
    char _reverb;

    int _program;       // current values as shown in mixer
    int _bank;          // initialized from "init"
    int _channel { 0 };        // mscore channel number, mapped to midi port/channel

    bool _soloMute;
    bool _mute;
    bool _solo;

    // MuseScore General-specific SND flags:
    //! TODO Needs porting to MU4
    bool _userBankController = false;     // if the user has changed the bank controller as opposed to switchExpressive
    //bool _switchedToExpressive = false;   // if the patch has been automatically switched to an expr variant

    mutable std::vector<MidiCoreEvent> _init;
    mutable bool _mustUpdateInit = true;

public:
    static const char* DEFAULT_NAME;
    static const char* HARMONY_NAME;
    static constexpr char defaultVolume = 100;

    enum class A : char {
        HBANK, LBANK, PROGRAM, VOLUME, PAN, CHORUS, REVERB,
        INIT_COUNT
    };

    enum class Prop : char {
        VOLUME, PAN, CHORUS, REVERB, NAME, DESCR, PROGRAM, BANK, COLOR,
        SOLOMUTE, SOLO, MUTE, SYNTI, CHANNEL, USER_BANK_CONTROL
    };

private:
    Notifier<InstrChannel::Prop> _notifier;
    void firePropertyChanged(InstrChannel::Prop prop) { _notifier.notify(prop); }

public:
    std::vector<MidiCoreEvent>& initList() const;

    QString name() const { return _name; }
    void setName(const QString& value);
    QString descr() const { return _descr; }
    void setDescr(const QString& value);
    QString synti() const { return _synti; }
    void setSynti(const QString& value);
    int color() const { return _color; }
    void setColor(int value);

    char volume() const { return _volume; }
    void setVolume(char value);
    char pan() const { return _pan; }
    void setPan(char value);
    char chorus() const { return _chorus; }
    void setChorus(char value);
    char reverb() const { return _reverb; }
    void setReverb(char value);

    int program() const { return _program; }
    void setProgram(int value);
    int bank() const { return _bank; }
    void setBank(int value);
    int channel() const { return _channel; }
    void setChannel(int value);

    bool soloMute() const { return _soloMute; }
    void setSoloMute(bool value);
    bool mute() const { return _mute; }
    void setMute(bool value);
    bool solo() const { return _solo; }
    void setSolo(bool value);

    // If the bank controller is set by the user or not
    bool userBankController() const { return _userBankController; }
    void setUserBankController(bool val);

    bool isHarmonyChannel() const { return _name == InstrChannel::HARMONY_NAME; }

    std::list<NamedEventList> midiActions;
    std::vector<MidiArticulation> articulation;

    InstrChannel();
    void write(XmlWriter&, const Part* part) const;
    void read(XmlReader&, Part* part);
    void updateInitList() const;
    bool operator==(const InstrChannel& c) const { return (_name == c._name) && (_channel == c._channel); }
    bool operator!=(const InstrChannel& c) const { return !(*this == c); }

    void addListener(ChannelListener* l);
    void removeListener(ChannelListener* l);

    void switchExpressive(Synthesizer* synth, bool expressive, bool force = false);
};

//---------------------------------------------------------
//   ChannelListener
//---------------------------------------------------------

class ChannelListener : public Listener<InstrChannel::Prop>
{
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
    // A list of properties which may vary for different excerpts.
    static const std::initializer_list<InstrChannel::Prop> excerptProperties;

private:
    InstrChannel* _main;
    InstrChannel* _bound;
    bool _excerpt;

    static bool isExcerptProperty(InstrChannel::Prop p)
    {
        return std::find(excerptProperties.begin(), excerptProperties.end(), p) != excerptProperties.end();
    }

    static void applyProperty(InstrChannel::Prop p, const InstrChannel* from, InstrChannel* to);
    void propertyChanged(InstrChannel::Prop p) override;

public:
    PartChannelSettingsLink()
        : _main(nullptr), _bound(nullptr), _excerpt(false) {}
    PartChannelSettingsLink(InstrChannel* main, InstrChannel* bound, bool excerpt);
    PartChannelSettingsLink(const PartChannelSettingsLink&) = delete;
    PartChannelSettingsLink(PartChannelSettingsLink&&);
    PartChannelSettingsLink& operator=(const PartChannelSettingsLink&) = delete;
    PartChannelSettingsLink& operator=(PartChannelSettingsLink&&);
    ~PartChannelSettingsLink() {}

    friend void swap(PartChannelSettingsLink&, PartChannelSettingsLink&);
};

//---------------------------------------------------------
//   Trait
//---------------------------------------------------------

enum class TraitType
{
    Unknown,
    Tuning,
    Transposition,
    Course
};

struct Trait
{
    QString name;

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
    StaffNameList _longNames;
    StaffNameList _shortNames;
    QString _trackName;
    QString _id;

    int _minPitchA = 0;
    int _maxPitchA = 0;
    int _minPitchP = 0;
    int _maxPitchP = 0;
    Interval _transpose;
    QString _instrumentId;

    bool _useDrumset = false;
    Drumset* _drumset = nullptr;
    StringData _stringData;

    std::list<NamedEventList> _midiActions;
    std::vector<MidiArticulation> _articulation;
    std::vector<InstrChannel*> _channel;        // at least one entry
    std::vector<ClefTypeList> _clefType;

    bool _singleNoteDynamics = false;

    Trait _trait;

public:
    Instrument(QString id = "");
    Instrument(const Instrument&);
    ~Instrument();

    void read(XmlReader&, Part* part);
    bool readProperties(XmlReader&, Part*, bool* customDrumset);
    void write(XmlWriter& xml, const Part* part) const;
    NamedEventList* midiAction(const QString& s, int channel) const;
    int channelIdx(const QString& s) const;
    void updateVelocity(int* velocity, int channel, const QString& name);
    qreal getVelocityMultiplier(const QString& name);
    void updateGateTime(int* gateTime, int channelIdx, const QString& name);

    QString recognizeInstrumentId() const;
    QString recognizeId() const;
    int recognizeMidiProgram() const;

    void operator=(const Instrument&);
    bool operator==(const Instrument&) const;
    bool operator!=(const Instrument&) const;

    bool isDifferentInstrument(const Instrument& i) const;

    QString id() const { return _id; }
    QString family() const;
    void setId(const QString& id) { _id = id; }
    void setMinPitchP(int v) { _minPitchP = v; }
    void setMaxPitchP(int v) { _maxPitchP = v; }
    void setMinPitchA(int v) { _minPitchA = v; }
    void setMaxPitchA(int v) { _maxPitchA = v; }
    Interval transpose() const { return _transpose; }
    void setTranspose(const Interval& v) { _transpose = v; }
    QString instrumentId() { return _instrumentId; }
    void setInstrumentId(const QString& instrumentId) { _instrumentId = instrumentId; }

    void setDrumset(const Drumset* ds);
    const Drumset* drumset() const { return _drumset; }
    Drumset* drumset() { return _drumset; }
    bool useDrumset() const { return _useDrumset; }
    void setUseDrumset(bool val);
    void setAmateurPitchRange(int a, int b) { _minPitchA = a; _maxPitchA = b; }
    void setProfessionalPitchRange(int a, int b) { _minPitchP = a; _maxPitchP = b; }
    InstrChannel* channel(int idx) { return _channel[idx]; }
    const InstrChannel* channel(int idx) const { return _channel.at(idx); }
    InstrChannel* playbackChannel(int idx, MasterScore*);
    const InstrChannel* playbackChannel(int idx, const MasterScore*) const;
    size_t cleffTypeCount() const;
    ClefTypeList clefType(size_t staffIdx) const;
    void setClefType(size_t staffIdx, const ClefTypeList& c);

    const std::list<NamedEventList>& midiActions() const { return _midiActions; }
    const std::vector<MidiArticulation>& articulation() const { return _articulation; }

    const std::vector<InstrChannel*>& channel() const { return _channel; }
    void appendChannel(InstrChannel* c) { _channel.push_back(c); }
    void removeChannel(InstrChannel* c) { mu::remove(_channel, c); }
    void clearChannels() { _channel.clear(); }

    void setMidiActions(const std::list<NamedEventList>& l) { _midiActions = l; }
    void setArticulation(const std::vector<MidiArticulation>& l) { _articulation = l; }
    const StringData* stringData() const { return &_stringData; }
    void setStringData(const StringData& d) { _stringData.set(d); }

    void setLongName(const QString& f);
    void setShortName(const QString& f);

    void addLongName(const StaffName& f);
    void addShortName(const StaffName& f);

    int minPitchP() const;
    int maxPitchP() const;
    int minPitchA() const;
    int maxPitchA() const;
    QString instrumentId() const;

    const std::list<StaffName>& longNames() const;
    const std::list<StaffName>& shortNames() const;
    std::list<StaffName>& longNames();
    std::list<StaffName>& shortNames();

    QString trackName() const;
    void setTrackName(const QString& s);
    QString name() const;
    QString abbreviature() const;
    static Instrument fromTemplate(const InstrumentTemplate* t);

    Trait trait() const;
    void setTrait(const Trait& trait);

    void updateInstrumentId();

    bool singleNoteDynamics() const { return _singleNoteDynamics; }
    void setSingleNoteDynamics(bool val) { _singleNoteDynamics = val; }
    void setSingleNoteDynamicsFromTemplate();
    bool getSingleNoteDynamicsFromTemplate() const;
    void switchExpressive(MasterScore* score, Synthesizer* synth, bool expressive, bool force = false);
};

//---------------------------------------------------------
//   InstrumentList
//---------------------------------------------------------

class InstrumentList : public std::map<const int, Instrument*>
{
    static Instrument defaultInstrument;

public:
    InstrumentList() {}
    const Instrument* instrument(int tick) const;
    Instrument* instrument(int tick);
    void setInstrument(Instrument*, int tick);
    bool contains(const std::string& instrumentId) const;
};
} // namespace mu::engraving
#endif
