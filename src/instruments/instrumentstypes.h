//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_INSTRUMENTS_INSTRUMENTSTYPES_H
#define MU_INSTRUMENTS_INSTRUMENTSTYPES_H

#include <QVariant>
#include <vector>
#include "qobjectdefs.h"

#include "libmscore/interval.h"
#include "libmscore/drumset.h"
#include "libmscore/stringdata.h"
#include "libmscore/clef.h"
#include "libmscore/instrument.h"
#include "framework/midi/miditypes.h"

namespace mu::instruments {
static constexpr int MAX_STAVES  = 4;

using Interval = Ms::Interval;
using Drumset = Ms::Drumset;
using StringData = Ms::StringData;
using Clef = Ms::Clef;
using ClefType = Ms::ClefType;
using ClefTypeList = Ms::ClefTypeList;
using BracketType = Ms::BracketType;
using StaffGroup = Ms::StaffGroup;
using StaffType = Ms::StaffType;
using Channel = Ms::Channel;
using StaffName = Ms::StaffName;
using StaffNameList = Ms::StaffNameList;
using MidiArticulation = Ms::MidiArticulation;

using ChannelList = QList<Channel>;

struct ClefPair
{
    ClefType concertClef = ClefType::G;
    ClefType transposingClef = ClefType::G;
};

struct PitchRange
{
    int min = 0;
    int max = 0;

    PitchRange() = default;
    PitchRange(int min, int max)
        : min(min), max(max) {}

    bool operator ==(const PitchRange& other) const
    {
        return min == other.min && max == other.max;
    }

    bool operator !=(const PitchRange& other) const
    {
        return !operator ==(other);
    }
};

struct MidiAction
{
    QString name;
    QString description;
    std::vector<midi::Event> events;
};
using MidiActionList = QList<MidiAction>;

using MidiArticulationMap = QMap<QString /*id*/, MidiArticulation>;

struct InstrumentGroup
{
    QString id;
    QString name;
    bool extended = false;
    int sequenceOrder = 0;
};

using InstrumentGroupList = QList<InstrumentGroup>;
using InstrumentGroupMap = QMap<QString /*id*/, InstrumentGroup>;

struct InstrumentGenre
{
    QString id;
    QString name;
};
using InstrumentGenreMap = QMap<QString /*id*/, InstrumentGenre>;

struct Transposition
{
    QString id;
    QString name;

    bool isValid() const { return !id.isEmpty(); }
};

struct Instrument
{
    QString id;
    StaffNameList longNames;
    StaffNameList shortNames;
    QString name;
    QString description;

    bool extended = false;
    int staves = 1;

    QString groupId;
    QStringList genreIds;

    PitchRange amateurPitchRange;
    PitchRange professionalPitchRange;

    ClefTypeList clefs[MAX_STAVES];
    int staffLines[MAX_STAVES] = { 0 };
    BracketType bracket[MAX_STAVES] = { BracketType::NO_BRACKET };
    int bracketSpan[MAX_STAVES] = { 0 };
    int barlineSpan[MAX_STAVES] = { 0 };
    bool smallStaff[MAX_STAVES] = { false };

    Interval transpose;

    StaffGroup staffGroup = StaffGroup::STANDARD;
    const StaffType* staffTypePreset = nullptr;

    bool useDrumset = false;
    const Drumset* drumset = nullptr;

    StringData stringData;

    bool singleNoteDynamics = false;

    MidiActionList midiActions;
    QList<MidiArticulation> midiArticulations;

    ChannelList channels;

    bool isValid() const { return !id.isEmpty(); }
    QString abbreviature() const { return !shortNames.isEmpty() ? shortNames.first().name() : QString(); }
};

using InstrumentList = QList<Instrument>;

struct InstrumentTemplate
{
    QString id;
    Transposition transposition;
    Instrument instrument;

    bool isValid() const { return !id.isEmpty(); }
};

using InstrumentTemplateMap = QMap<QString /*id*/, InstrumentTemplate>;
using InstrumentTemplateList = QList<InstrumentTemplate>;

struct InstrumentsMeta
{
    InstrumentTemplateMap instrumentTemplates;
    InstrumentGroupMap groups;
    InstrumentGenreMap genres;
    MidiArticulationMap articulations;
};

class InstrumentTreeItemType
{
    Q_GADGET

public:
    enum class ItemType {
        UNDEFINED = -1,
        ROOT,
        PART,
        INSTRUMENT,
        STAFF,
        CONTROL_ADD_STAFF,
        CONTROL_ADD_DOUBLE_INSTRUMENT
    };

    Q_ENUM(ItemType)
};
}

Q_DECLARE_METATYPE(mu::instruments::Instrument)

#endif // MU_INSTRUMENTS_INSTRUMENTSTYPES_H
