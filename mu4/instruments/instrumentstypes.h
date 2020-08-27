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

#include <QString>
#include <vector>

#include "libmscore/interval.h"
#include "libmscore/drumset.h"
#include "libmscore/stringdata.h"
#include "libmscore/clef.h"
#include "libmscore/instrument.h"
#include "framework/midi/miditypes.h"

namespace mu {
namespace instruments {
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

struct ClefPair {
    ClefType concertClef = ClefType::G;
    ClefType transposingClef = ClefType::G;
};

struct PitchRange
{
    int min = 0;
    int max = 0;
};

struct MidiAction
{
    QString name;
    QString description;
    std::vector<midi::Event> events;
};
using MidiActionList = QList<MidiAction>;

using MidiArticulationHash = QHash<QString /*id*/, MidiArticulation>;

struct InstrumentGroup
{
    QString id;
    QString name;
    bool extended;
};
using InstrumentGroupHash = QHash<QString /*id*/, InstrumentGroup>;

struct InstrumentGenre
{
    QString id;
    QString name;
};
using InstrumentGenreHash = QHash<QString /*id*/, InstrumentGenre>;

struct Transposition {
    QString id;
    QString name;

    bool isValid() const { return !id.isEmpty(); }
};

struct InstrumentTemplate
{
    QString id;
    StaffNameList longNames;
    StaffNameList shortNames;
    QString trackName;
    QString description;

    bool extended = false;
    int staves = 0;

    QString groupId;
    QStringList genreIds;

    QString musicXMLId;

    PitchRange aPitchRange;
    PitchRange pPitchRange;

    ClefTypeList clefs[MAX_STAVES];
    int staffLines[MAX_STAVES];
    BracketType bracket[MAX_STAVES];
    int bracketSpan[MAX_STAVES];
    int barlineSpan[MAX_STAVES];
    bool smallStaff[MAX_STAVES];

    Interval transpose;

    StaffGroup staffGroup;
    const StaffType* staffTypePreset;

    bool useDrumset = false;
    Drumset* drumset = 0;

    StringData stringData;

    bool singleNoteDynamics = false;

    MidiActionList midiActions;
    QList<MidiArticulation> midiArticulations;

    ChannelList channels;

    Transposition transposition;

    bool isValid() const { return !id.isEmpty(); }
};

using InstrumentTemplateHash = QHash<QString /*id*/, InstrumentTemplate>;

struct InstrumentsMeta
{
    InstrumentTemplateHash instrumentTemplates;
    InstrumentGroupHash groups;
    InstrumentGenreHash genres;
    MidiArticulationHash articulations;
};
}
}

#endif // MU_INSTRUMENTS_INSTRUMENTSTYPES_H
