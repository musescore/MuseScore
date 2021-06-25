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
#include "libmscore/scoreorder.h"
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
using Transposition = Ms::Transposition;
using TranspositionType = Ms::TranspositionType;

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

using MidiArticulations = QList<Ms::MidiArticulation>;

struct InstrumentGroup
{
    QString id;
    QString name;
    bool extended = false;
    int sequenceOrder = 0;
};

using InstrumentGroups = QList<InstrumentGroup>;

struct InstrumentGenre
{
    QString id;
    QString name;
};
using InstrumentGenres = QList<InstrumentGenre>;

static const QString COMMON_GENRE_ID("common");

struct Instrument
{
    QString id;
    StaffNameList longNames;
    StaffNameList shortNames;
    QString name;
    QString musicXMLid;
    QString description;
    int sequenceOrder = 0;

    bool extended = false;
    int staves = 1;

    QString groupId;
    QStringList genreIds;
    QString familyId;

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

    Transposition transposition;

    bool isValid() const { return !id.isEmpty(); }
    QString abbreviature() const { return !shortNames.isEmpty() ? shortNames.first().name() : QString(); }
};

struct PartInstrument
{
    QString partId;
    Instrument instrument;

    bool isExistingPart = false;
    bool isSoloist = false;
};

using PartInstrumentList = QList<PartInstrument>;

struct ScoreOrderGroup
{
    QString family;
    QString section;
    QString unsorted;

    bool bracket = false;
    bool showSystemMarkings = false;
    bool barLineSpan = false;
    bool thinBracket = false;
};

using InstrumentOverwrite = Ms::InstrumentOverwrite;

struct ScoreOrder
{
    QString id;
    QString name;
    QMap<QString, InstrumentOverwrite> instrumentMap;
    QList<ScoreOrderGroup> groups;

    bool isValid() { return !groups.empty(); }
};

using ScoreOrders = QList<ScoreOrder>;

struct PartInstrumentListScoreOrder
{
    PartInstrumentList instruments;
    ScoreOrder scoreOrder;
};

struct InstrumentTemplate
{
    QString id;
    Instrument instrument;

    bool isValid() const { return !id.isEmpty(); }
};

using InstrumentTemplates = QList<InstrumentTemplate>;

struct InstrumentsMeta
{
    InstrumentTemplates instrumentTemplates;
    InstrumentGroups groups;
    InstrumentGenres genres;
    MidiArticulations articulations;
    ScoreOrders scoreOrders;
};

class InstrumentsTreeItemType
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
Q_DECLARE_METATYPE(mu::instruments::ScoreOrder)

#endif // MU_INSTRUMENTS_INSTRUMENTSTYPES_H
