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

#ifndef __INSTRTEMPLATE_H__
#define __INSTRTEMPLATE_H__

#include <list>

#include "mscore.h"
#include "instrument.h"
#include "clef.h"
#include "stringdata.h"

namespace mu::engraving {
class XmlWriter;
class Part;
class Staff;
class StringData;
class StaffType;
struct ScoreOrder;

//---------------------------------------------------------
//   InstrumentGenre
//---------------------------------------------------------

class InstrumentGenre
{
public:
    String id;
    String name;

    InstrumentGenre() {}
    void write(XmlWriter& xml) const;
    void write1(XmlWriter& xml) const;
    void read(XmlReader&);
};

//---------------------------------------------------------
//   InstrumentFamily
//---------------------------------------------------------

class InstrumentFamily
{
public:
    String id;
    String name;

    InstrumentFamily() {}
    void write(XmlWriter& xml) const;
    void write1(XmlWriter& xml) const;
    void read(XmlReader&);
};

//---------------------------------------------------------
//   InstrumentTemplate
//---------------------------------------------------------

class InstrumentTemplate
{
public:
    InstrumentTemplate();
    InstrumentTemplate(const InstrumentTemplate&);
    ~InstrumentTemplate();
    InstrumentTemplate& operator=(const InstrumentTemplate&);

    String id;
    String trackName;
    StaffNameList longNames;     ///< shown on first system
    StaffNameList shortNames;    ///< shown on followup systems
    String musicXMLid;          ///< used in MusicXML 3.0
    String description;         ///< a longer description of the instrument

    size_t staffCount = 0;
    int sequenceOrder = 0;

    Trait trait;

    char minPitchA = 0;           // pitch range playable by an amateur
    char maxPitchA = 0;
    char minPitchP = 0;           // pitch range playable by professional
    char maxPitchP = 0;

    Interval transpose;       // for transposing instruments

    StaffGroup staffGroup;
    const StaffType* staffTypePreset = nullptr;
    bool useDrumset = false;
    Drumset* drumset = nullptr;

    StringData stringData;

    std::list<NamedEventList> midiActions;
    std::vector<MidiArticulation> midiArticulations;
    std::vector<InstrChannel> channel;
    std::list<InstrumentGenre*> genres;       //; list of genres this instrument belongs to
    InstrumentFamily* family = nullptr;   //; family the instrument belongs to

    ClefTypeList clefTypes[MAX_STAVES];
    int staffLines[MAX_STAVES];
    BracketType bracket[MAX_STAVES];              // bracket type (NO_BRACKET)
    int bracketSpan[MAX_STAVES];
    int barlineSpan[MAX_STAVES];
    bool smallStaff[MAX_STAVES];

    bool extended = false;            // belongs to extended instrument set if true
    bool singleNoteDynamics = false;

    String groupId;

    bool isValid() const;

    void write(XmlWriter& xml) const;
    void write1(XmlWriter& xml) const;
    void read(XmlReader&);
    ClefTypeList clefType(staff_idx_t staffIdx) const;
    String familyId() const;
    bool containsGenre(const String& genreId) const;

private:
    void init(const InstrumentTemplate&);
    void setPitchRange(const String& s, char* a, char* b) const;
    void linkGenre(const String&);
};

//---------------------------------------------------------
//   InstrumentGroup
//---------------------------------------------------------

struct InstrumentGroup {
    String id;
    String name;
    bool extended;            // belongs to extended instruments set if true
    std::list<InstrumentTemplate*> instrumentTemplates;
    void read(XmlReader&);
    void clear();

    InstrumentGroup() { extended = false; }
};

//---------------------------------------------------------
//   InstrumentIndex
//---------------------------------------------------------

struct InstrumentIndex {
    int groupIndex = 0;
    int instrIndex = 0;
    size_t templateCount = 0;
    InstrumentTemplate* instrTemplate = nullptr;

    InstrumentIndex(int g, int i, InstrumentTemplate* it);
};

extern std::vector<InstrumentGenre*> instrumentGenres;
extern std::vector<InstrumentFamily*> instrumentFamilies;
extern std::vector<MidiArticulation> midiArticulations;
extern std::vector<InstrumentGroup*> instrumentGroups;
extern std::vector<ScoreOrder> instrumentOrders;
extern void clearInstrumentTemplates();
extern bool loadInstrumentTemplates(const String& instrTemplates);
extern InstrumentTemplate* searchTemplate(const String& name);
extern InstrumentIndex searchTemplateIndexForTrackName(const String& trackName);
extern InstrumentIndex searchTemplateIndexForId(const String& id);
extern InstrumentTemplate* searchTemplateForMusicXmlId(const String& mxmlId);
extern InstrumentTemplate* searchTemplateForInstrNameList(const std::list<String>& nameList);
extern InstrumentTemplate* searchTemplateForMidiProgram(int midiProgram, const bool useDrumKit = false);
extern InstrumentTemplate* guessTemplateByNameData(const std::list<String>& nameDataList);
extern InstrumentGroup* searchInstrumentGroup(const String& name);
extern ClefType defaultClef(int patch);
} // namespace mu::engraving
#endif
