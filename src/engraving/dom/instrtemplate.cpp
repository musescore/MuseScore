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

#include <limits>
#include "instrtemplate.h"

#include "io/file.h"
#include "containers.h"
#include "translation.h"

#include "rw/xmlreader.h"
#include "rw/xmlwriter.h"
#include "rw/write/twrite.h"
#include "rw/read400/tread.h"
#include "types/typesconv.h"

#include "bracket.h"
#include "drumset.h"
#include "scoreorder.h"
#include "stafftype.h"
#include "stringdata.h"
#include "stringutils.h"
#include "utils.h"

#include "log.h"

using namespace mu;
using namespace muse::io;
using namespace mu::engraving;

namespace mu::engraving {
std::vector<const InstrumentGroup*> instrumentGroups;
std::vector<const InstrumentGenre*> instrumentGenres;
std::vector<const InstrumentFamily*> instrumentFamilies;
std::vector<MidiArticulation> midiArticulations;            // global articulations
std::vector<ScoreOrder> instrumentOrders;

InstrumentIndex::InstrumentIndex(int g, int i, const InstrumentTemplate* it)
    : groupIndex{g}, instrIndex{i}, instrTemplate{it}
{
    templateCount = 0;
    for (const InstrumentGroup* ig : instrumentGroups) {
        templateCount += ig->instrumentTemplates.size();
    }
}

const InstrumentGenre* searchInstrumentGenre(const String& id)
{
    for (const InstrumentGenre* ig : instrumentGenres) {
        if (ig->id == id) {
            return ig;
        }
    }
    return nullptr;
}

static const InstrumentFamily* searchInstrumentFamily(const String& name)
{
    for (const InstrumentFamily* fam : instrumentFamilies) {
        if (fam->id == name) {
            return fam;
        }
    }
    return nullptr;
}

static InstrumentGroup* searchInstrumentGroup(const String& id)
{
    for (const InstrumentGroup* g : instrumentGroups) {
        if (g->id == id) {
            return const_cast<InstrumentGroup*>(g);
        }
    }
    return nullptr;
}

static MidiArticulation searchArticulation(const String& name)
{
    for (MidiArticulation a : midiArticulations) {
        if (a.name == name) {
            return a;
        }
    }
    return MidiArticulation();
}

static int readStaffIdx(XmlReader& e)
{
    int idx = e.intAttribute("staff", 1) - 1;
    if (idx >= MAX_STAVES) {
        idx = MAX_STAVES - 1;
    }
    if (idx < 0) {
        idx = 0;
    }
    return idx;
}

static TraitType traitTypeFromString(const String& str)
{
    static const std::unordered_map<String, TraitType> types {
        { u"transposition", TraitType::Transposition },
        { u"tuning", TraitType::Tuning },
        { u"course", TraitType::Course }
    };

    return muse::value(types, str.toLower(), TraitType::Unknown);
}

//---------------------------------------------------------
//   read InstrumentGroup
//---------------------------------------------------------

void InstrumentGroup::read(XmlReader& e)
{
    id       = e.attribute("id");
    name     = muse::mtrc("engraving/instruments/group", e.attribute("name"));
    extended = e.intAttribute("extended", 0);

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "instrument" || tag == "Instrument") {
            String sid = e.attribute("id");
            InstrumentTemplate* t = const_cast<InstrumentTemplate*>(searchTemplate(sid));
            if (t == 0) {
                t = new InstrumentTemplate;
                // init with global articulation
                t->midiArticulations.insert(t->midiArticulations.end(), midiArticulations.begin(), midiArticulations.end());
                t->sequenceOrder = static_cast<int>(instrumentTemplates.size());
                instrumentTemplates.push_back(t);
            }
            t->read(e);
        } else if (tag == "ref") {
            const InstrumentTemplate* ttt = searchTemplate(e.readText());
            if (ttt) {
                InstrumentTemplate* t = new InstrumentTemplate(*ttt);
                instrumentTemplates.push_back(t);
            } else {
                LOGD("instrument reference not found <%s>", e.text().toUtf8().data());
            }
        } else if (tag == "name") {
            name = muse::mtrc("engraving/instruments/group", e.readAsciiText().ascii());
        } else if (tag == "extended") {
            extended = e.readInt();
        } else {
            e.unknown();
        }
    }
    if (id.isEmpty()) {
        id = name.toLower().replace(u" ", u"-");
    }

    for (const InstrumentTemplate* templ : instrumentTemplates) {
        const_cast<InstrumentTemplate*>(templ)->groupId = id;
    }
}

//---------------------------------------------------------
//   clear InstrumentGroup
//---------------------------------------------------------

void InstrumentGroup::clear()
{
    muse::DeleteAll(instrumentTemplates);
    instrumentTemplates.clear();
}

//---------------------------------------------------------
//   InstrumentTemplate
//---------------------------------------------------------

InstrumentTemplate::InstrumentTemplate()
{
    staffCount        = 1;
    minPitchA          = 0;
    maxPitchA          = 127;
    minPitchP          = 0;
    maxPitchP          = 127;
    staffGroup         = StaffGroup::STANDARD;
    staffTypePreset    = 0;
    useDrumset         = false;
    drumset            = 0;
    extended           = false;
    singleNoteDynamics = true;
    family             = nullptr;

    for (int i = 0; i < MAX_STAVES; ++i) {
        clefTypes[i].concertClef = ClefType::G;
        clefTypes[i].transposingClef = ClefType::G;
        staffLines[i]  = 5;
        smallStaff[i]  = false;
        bracket[i]     = BracketType::NO_BRACKET;
        bracketSpan[i] = 0;
        barlineSpan[i] = false;
    }
    transpose.diatonic   = 0;
    transpose.chromatic  = 0;
}

InstrumentTemplate::InstrumentTemplate(const InstrumentTemplate& t)
{
    init(t);
}

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void InstrumentTemplate::init(const InstrumentTemplate& t)
{
    id = t.id;
    soundId = t.soundId;
    trackName = t.trackName;
    longNames = t.longNames;
    shortNames = t.shortNames;
    description = t.description;
    musicXmlId = t.musicXmlId;
    staffCount = t.staffCount;
    extended = t.extended;
    minPitchA = t.minPitchA;
    maxPitchA = t.maxPitchA;
    minPitchP = t.minPitchP;
    maxPitchP = t.maxPitchP;
    transpose = t.transpose;
    staffGroup = t.staffGroup;
    staffTypePreset = t.staffTypePreset;
    useDrumset = t.useDrumset;
    drumset = t.drumset ? new Drumset(*t.drumset) : nullptr;
    stringData = t.stringData;
    midiActions = t.midiActions;
    genres = t.genres;
    channel = t.channel;
    family = t.family;
    singleNoteDynamics = t.singleNoteDynamics;
    sequenceOrder = t.sequenceOrder;
    trait = t.trait;
    groupId = t.groupId;

    for (int i = 0; i < MAX_STAVES; ++i) {
        clefTypes[i]   = t.clefTypes[i];
        staffLines[i]  = t.staffLines[i];
        smallStaff[i]  = t.smallStaff[i];
        bracket[i]     = t.bracket[i];
        bracketSpan[i] = t.bracketSpan[i];
        barlineSpan[i] = t.barlineSpan[i];
    }
}

InstrumentTemplate::~InstrumentTemplate()
{
    delete drumset;
    drumset = nullptr;
}

InstrumentTemplate& InstrumentTemplate::operator=(const InstrumentTemplate& templ)
{
    init(templ);
    return *this;
}

bool InstrumentTemplate::isValid() const
{
    return !id.empty();
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void InstrumentTemplate::write(XmlWriter& xml) const
{
    xml.startElement("Instrument",  { { "id", id } });

    if (!soundId.empty()) {
        xml.tag("soundId", soundId);
    }

    write::TWrite::write(&longNames, xml, "longName");
    write::TWrite::write(&shortNames, xml, "shortName");

    if (longNames.size() > 1) {
        xml.tag("trackName", trackName);
    }
    xml.tag("description", description);
    xml.tag("musicXMLid", musicXmlId);
    if (extended) {
        xml.tag("extended", extended);
    }
    write::TWrite::write(&stringData, xml);
    if (staffCount > 1) {
        xml.tag("staves", static_cast<int>(staffCount));
    }
    for (staff_idx_t i = 0; i < staffCount; ++i) {
        if (clefTypes[i].concertClef == clefTypes[i].transposingClef) {
            if (i) {
                xml.tag("clef", { { "staff", i + 1 } }, TConv::toXml(clefTypes[i].concertClef));
            } else {
                xml.tag("clef", TConv::toXml(clefTypes[i].concertClef));
            }
        } else {
            if (i) {
                xml.tag("concertClef", { { "staff", i + 1 } }, TConv::toXml(clefTypes[i].concertClef));
                xml.tag("transposingClef", { { "staff", i + 1 } }, TConv::toXml(clefTypes[i].transposingClef));
            } else {
                xml.tag("concertClef", TConv::toXml(clefTypes[i].concertClef));
                xml.tag("transposingClef", TConv::toXml(clefTypes[i].transposingClef));
            }
        }
        if (staffLines[i] != 5) {
            if (i) {
                xml.tag("stafflines", { { "staff", i + 1 } }, staffLines[i]);
            } else {
                xml.tag("stafflines", staffLines[i]);
            }
        }
        if (smallStaff[i]) {
            if (i) {
                xml.tag("smallStaff", { { "staff", i + 1 } }, smallStaff[i]);
            } else {
                xml.tag("smallStaff", smallStaff[i]);
            }
        }

        if (bracket[i] != BracketType::NO_BRACKET) {
            if (i) {
                xml.tag("bracket", { { "staff", i + 1 } }, int(bracket[i]));
            } else {
                xml.tag("bracket", int(bracket[i]));
            }
        }
        if (bracketSpan[i] != 0) {
            if (i) {
                xml.tag("bracketSpan", { { "staff", i + 1 } }, bracketSpan[i]);
            } else {
                xml.tag("bracketSpan", bracketSpan[i]);
            }
        }
        if (barlineSpan[i]) {
            if (i) {
                xml.tag("barlineSpan", { { "staff", i + 1 } }, barlineSpan[i]);
            } else {
                xml.tag("barlineSpan", barlineSpan[i]);
            }
        }
    }
    if (minPitchA != 0 || maxPitchA != 127) {
        xml.tag("aPitchRange", String(u"%1-%2").arg(int(minPitchA)).arg(int(maxPitchA)));
    }
    if (minPitchP != 0 || maxPitchP != 127) {
        xml.tag("pPitchRange", String(u"%1-%2").arg(int(minPitchP)).arg(int(maxPitchP)));
    }
    if (transpose.diatonic) {
        xml.tag("transposeDiatonic", transpose.diatonic);
    }
    if (transpose.chromatic) {
        xml.tag("transposeChromatic", transpose.chromatic);
    }
    if (useDrumset) {
        xml.tag("drumset", int(useDrumset));
    }
    if (drumset) {
        drumset->save(xml);
    }

    if (!singleNoteDynamics) {      // default is true
        xml.tag("singleNoteDynamics", singleNoteDynamics);
    }

    for (const NamedEventList& a : midiActions) {
        write::TWrite::write(&a, xml, "MidiAction");
    }
    for (const InstrChannel& a : channel) {
        write::TWrite::write(&a, xml, nullptr);
    }
    for (const MidiArticulation& ma : midiArticulations) {
        bool isGlobal = false;
        for (const MidiArticulation& ga : mu::engraving::midiArticulations) {
            if (ma == ga) {
                isGlobal = true;
                break;
            }
        }
        if (!isGlobal) {
            write::TWrite::write(&ma, xml);
        }
    }
    if (family) {
        xml.tag("family", family->id);
    }
    xml.endElement();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

String translateInstrumentName(const String& instrumentId, const String& nameType, const String& text)
{
    String disambiguation = instrumentId + u' ' + nameType;
    return muse::mtrc("engraving/instruments", text, disambiguation);
}

void InstrumentTemplate::read(XmlReader& e)
{
    id = e.attribute("id");

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "soundId") {
            soundId = e.readText();
        } else if (tag == "longName" || tag == "name") {                   // "name" is obsolete
            int pos = e.intAttribute("pos", 0);
            for (std::list<StaffName>::iterator i = longNames.begin(); i != longNames.end(); ++i) {
                if ((*i).pos() == pos) {
                    longNames.erase(i);
                    break;
                }
            }
            longNames.push_back(StaffName(translateInstrumentName(id, u"longName", e.readText()), pos));
        } else if (tag == "shortName" || tag == "short-name") {     // "short-name" is obsolete
            int pos = e.intAttribute("pos", 0);
            for (std::list<StaffName>::iterator i = shortNames.begin(); i != shortNames.end(); ++i) {
                if ((*i).pos() == pos) {
                    shortNames.erase(i);
                    break;
                }
            }
            shortNames.push_back(StaffName(translateInstrumentName(id, u"shortName", e.readText()), pos));
        } else if (tag == "trackName") {
            trackName = translateInstrumentName(id, u"trackName", e.readText());
        } else if (tag == "description") {
            description = translateInstrumentName(id, u"description", e.readText());
        } else if (tag == "extended") {
            extended = e.readInt();
        } else if (tag == "staves") {
            staffCount = e.readInt();
            bracketSpan[0] = static_cast<int>(staffCount);
//                  for (int i = 0; i < staves-1; ++i)
//                        barlineSpan[i] = true;
        } else if (tag == "clef") {             // sets both transposing and concert clef
            int idx = readStaffIdx(e);
            ClefType ct = TConv::fromXml(e.readAsciiText(), ClefType::G);
            clefTypes[idx].concertClef = ct;
            clefTypes[idx].transposingClef = ct;
        } else if (tag == "concertClef") {
            int idx = readStaffIdx(e);
            clefTypes[idx].concertClef = TConv::fromXml(e.readAsciiText(), ClefType::G);
        } else if (tag == "transposingClef") {
            int idx = readStaffIdx(e);
            clefTypes[idx].transposingClef = TConv::fromXml(e.readAsciiText(), ClefType::G);
        } else if (tag == "stafflines") {
            int idx = readStaffIdx(e);
            staffLines[idx] = e.readInt();
        } else if (tag == "smallStaff") {
            int idx = readStaffIdx(e);
            smallStaff[idx] = e.readInt();
        } else if (tag == "bracket") {
            int idx = readStaffIdx(e);
            bracket[idx] = BracketType(e.readInt());
        } else if (tag == "bracketSpan") {
            int idx = readStaffIdx(e);
            bracketSpan[idx] = e.readInt();
        } else if (tag == "barlineSpan") {
            int idx = readStaffIdx(e);
            int span = e.readInt();
            for (int i = 0; i < span - 1; ++i) {
                barlineSpan[idx + i] = true;
            }
        } else if (tag == "aPitchRange") {
            setPitchRange(e.readText(), &minPitchA, &maxPitchA);
        } else if (tag == "pPitchRange") {
            setPitchRange(e.readText(), &minPitchP, &maxPitchP);
        } else if (tag == "transposition") {      // obsolete
            int i = e.readInt();
            transpose.chromatic = i;
            transpose.diatonic = chromatic2diatonic(i);
        } else if (tag == "transposeChromatic") {
            transpose.chromatic = e.readInt();
        } else if (tag == "transposeDiatonic") {
            transpose.diatonic = e.readInt();
        } else if (tag == "traitName") {
            trait.type = traitTypeFromString(e.attribute("type"));
            String traitName = translateInstrumentName(id, u"traitName", e.readText());
            trait.isDefault = traitName.contains(u'*');
            trait.isHiddenOnScore = traitName.contains(u'(') && traitName.contains(u')');
            trait.name = traitName.remove(u'*').remove(u'(').remove(u')');
        } else if (tag == "StringData") {
            read400::TRead::read(&stringData, e);
        } else if (tag == "drumset") {
            useDrumset = e.readInt();
        } else if (tag == "Drum") {
            // if we see one of this tags, a custom drumset will
            // be created
            if (drumset == 0) {
                drumset = new Drumset(*smDrumset);
                drumset->clear();
            }
            drumset->load(e);
        } else if (tag == "MidiAction") {
            NamedEventList a;
            read400::TRead::read(&a, e);
            midiActions.push_back(a);
        } else if (tag == "Channel" || tag == "channel") {
            InstrChannel a;
            InstrumentTrackId tId;
            read400::ReadContext rctx(nullptr);
            read400::TRead::read(&a, e, rctx, nullptr, tId);
            channel.push_back(a);
        } else if (tag == "Articulation") {
            MidiArticulation a;
            read400::TRead::read(&a, e);
            size_t n = midiArticulations.size();
            size_t i;
            for (i = 0; i < n; ++i) {
                if (midiArticulations[i].name == a.name) {
                    midiArticulations[i] = a;
                    break;
                }
            }
            if (i == n) {
                midiArticulations.push_back(a);
            }
        } else if (tag == "stafftype") {
            int staffIdx = readStaffIdx(e);
            String xmlPresetName = e.attribute("staffTypePreset");
            String stfGroup = e.readText();
            if (stfGroup == "percussion") {
                staffGroup = StaffGroup::PERCUSSION;
            } else if (stfGroup == "tablature") {
                staffGroup = StaffGroup::TAB;
            } else {
                staffGroup = StaffGroup::STANDARD;
            }
            staffTypePreset = 0;
            if (!xmlPresetName.isEmpty()) {
                staffTypePreset = StaffType::presetFromXmlName(xmlPresetName);
            }
            if (!staffTypePreset || staffTypePreset->group() != staffGroup) {
                staffTypePreset = StaffType::getDefaultPreset(staffGroup);
            }
            if (staffTypePreset) {
                staffLines[staffIdx] = staffTypePreset->lines();
            }
        } else if (tag == "init") {
            String val(e.readText());
            const InstrumentTemplate* ttt = searchTemplate(val);
            if (ttt) {
                String id_ = id;
                init(*ttt);
                id = id_;
            } else {
                LOGD("InstrumentTemplate:: init instrument <%s> not found", muPrintable(val));
            }
        } else if (tag == "musicXMLid") {
            musicXmlId = e.readText();
        } else if (tag == "family") {
            family = searchInstrumentFamily(e.readText());
        } else if (tag == "genre") {
            String val(e.readText());
            linkGenre(val);
        } else if (tag == "singleNoteDynamics") {
            singleNoteDynamics = e.readBool();
        } else {
            e.unknown();
        }
    }
    if (channel.empty()) {
        InstrChannel a;
        a.setChorus(0);
        a.setReverb(0);
        a.setName(String::fromUtf8(InstrChannel::DEFAULT_NAME));
        a.setProgram(0);
        a.setBank(0);
        a.setVolume(90);
        a.setPan(0);
        channel.push_back(a);
    }

    if (trackName.isEmpty() && !longNames.empty()) {
        trackName = longNames.front().name();
    }
    if (description.isEmpty() && !longNames.empty()) {
        description = longNames.front().name();
    }
    if (id.empty()) {
        id = trackName.toLower().replace(u' ', u'-');
    }

    if (staffCount == 0) {
        LOGD(" 2Instrument: staves == 0 <%s>", muPrintable(id));
    }
}

//---------------------------------------------------------
//   setPitchRange
//---------------------------------------------------------

void InstrumentTemplate::setPitchRange(const String& s, char* a, char* b) const
{
    StringList sl = s.split(u'-');
    if (sl.size() != 2) {
        *a = 0;
        *b = 127;
        return;
    }
    *a = sl.at(0).toInt();
    *b = sl.at(1).toInt();
}

//---------------------------------------------------------
//   clearInstrumentTemplates
//---------------------------------------------------------

void clearInstrumentTemplates()
{
    for (const InstrumentGroup* g : instrumentGroups) {
        const_cast<InstrumentGroup*>(g)->clear();
    }
    muse::DeleteAll(instrumentGroups);
    instrumentGroups.clear();
    muse::DeleteAll(instrumentGenres);
    instrumentGenres.clear();
    muse::DeleteAll(instrumentFamilies);
    instrumentFamilies.clear();
    midiArticulations.clear();
    instrumentOrders.clear();
}

//---------------------------------------------------------
//   loadInstrumentTemplates
//---------------------------------------------------------

bool loadInstrumentTemplates(const muse::io::path_t& instrTemplatesPath)
{
    File qf(instrTemplatesPath);
    if (!qf.open(IODevice::ReadOnly)) {
        LOGE() << "Could not load instrument templates from " << instrTemplatesPath;
        return false;
    }

    XmlReader e(&qf);
    while (e.readNextStartElement()) {
        if (e.name() == "museScore") {
            while (e.readNextStartElement()) {
                const AsciiStringView tag(e.name());
                if (tag == "instrument-group" || tag == "InstrumentGroup") {
                    String idGroup(e.attribute("id"));
                    InstrumentGroup* group = searchInstrumentGroup(idGroup);
                    if (group == 0) {
                        group = new InstrumentGroup;
                        instrumentGroups.push_back(group);
                    }
                    group->read(e);
                } else if (tag == "Articulation") {
                    // read global articulation
                    String name(e.attribute("name"));
                    MidiArticulation a = searchArticulation(name);
                    read400::TRead::read(&a, e);
                    midiArticulations.push_back(a);
                } else if (tag == "Genre") {
                    String idGenre(e.attribute("id"));
                    InstrumentGenre* genre = const_cast<InstrumentGenre*>(searchInstrumentGenre(idGenre));
                    if (!genre) {
                        genre = new InstrumentGenre;
                        instrumentGenres.push_back(genre);
                    }
                    genre->read(e);
                } else if (tag == "Family") {
                    String idFamily(e.attribute("id"));
                    InstrumentFamily* fam = const_cast<InstrumentFamily*>(searchInstrumentFamily(idFamily));
                    if (!fam) {
                        fam = new InstrumentFamily;
                        instrumentFamilies.push_back(fam);
                    }
                    fam->read(e);
                } else if (tag == "Order") {
                    ScoreOrder order;
                    order.read(e);
                    instrumentOrders.push_back(order);
                } else {
                    e.unknown();
                }
            }
        }
    }

    return true;
}

const InstrumentTemplate* searchTemplate(const String& name)
{
    for (const InstrumentGroup* g : instrumentGroups) {
        for (const InstrumentTemplate* it : g->instrumentTemplates) {
            if (it->id == name) {
                return it;
            }
        }
    }
    return 0;
}

const InstrumentTemplate* combinedTemplateSearch(const String& mxmlId, const String& name, const int transposition, int bank,
                                                 int program)
{
    size_t minLevenshteinDistance = std::numeric_limits<size_t>::max();
    const InstrumentTemplate* templateWithMinLevenshteinDistance = nullptr;

    if (mxmlId.empty() && name.empty() && bank == 0 && program == -1) {
        // No instrument information provided
        return nullptr;
    }

    String id = mxmlId;
    if (mxmlId.empty()) {
        if (name.contains(u"drum", muse::CaseInsensitive)) {
            id = u"drum.group.set";
        } else if (name.contains(u"piano", muse::CaseInsensitive)) {
            id = u"keyboard.piano";
        }
    }

    // This is to workaround old generic instrument templates
    if ((mxmlId == u"wind.reed.clarinet" || mxmlId == u"brass.trumpet") && transposition == 10) {
        id.append(u".bflat");
    }

    // Perform a weighted search over musicxml ID, instrument name, transposition, and midi program
    static const int MXML_ID_WEIGHT = 4;
    static const int TRACK_NAME_WEIGHT = 32;
    static const int LONG_NAME_WEIGHT = 16;
    static const int SHORT_NAME_WEIGHT = 8;
    static const int MIDI_WEIGHT = 2;
    static const int TRANSPOSITION_WEIGHT = 1;

    // Exclude text weights from a perfect score as we only have one string to match, and it won't match all three track, long and short names
    int perfectMatchStrength = 0 + (id.isEmpty() ? 0 : MXML_ID_WEIGHT)
                               + (program == -1 ? 0 : MIDI_WEIGHT)
                               + TRANSPOSITION_WEIGHT;
    const InstrumentTemplate* bestMatch = nullptr;
    int bestMatchStrength = 0;
    for (const InstrumentGroup* g : instrumentGroups) {
        for (const InstrumentTemplate* it : g->instrumentTemplates) {
            if (it->trait.name == u"[hide]") {
                continue;
            }
            int matchStrength = 0;
            int nameWeight = 0;

            // MusicXML ID
            if (!it->musicXmlId.empty() && it->musicXmlId == id) {
                matchStrength += MXML_ID_WEIGHT;
            }

            // Instrument names
            if (!name.isEmpty()) {
                nameWeight = 0 + (TRACK_NAME_WEIGHT * (it->trackName == name ? 1 : 0))
                             + (LONG_NAME_WEIGHT * (muse::contains(it->longNames, StaffName(name)) ? 1 : 0))
                             + (SHORT_NAME_WEIGHT * (muse::contains(it->shortNames, StaffName(name)) ? 1 : 0));
                matchStrength += nameWeight;
            }

            // Midi program
            for (const InstrChannel& channel : it->channel) {
                if (channel.bank() == bank && channel.program() == program) {
                    matchStrength += MIDI_WEIGHT;
                    break;
                }
            }

            // We aren't concerned about the octave of transpositions
            if (transposition == (it->transpose.chromatic + 12) % 12) {
                matchStrength += TRANSPOSITION_WEIGHT;
            }

            if (matchStrength > bestMatchStrength) {
                bestMatch = it;
                bestMatchStrength = matchStrength;

                if (bestMatchStrength - nameWeight == perfectMatchStrength && nameWeight > 0) {
                    return bestMatch; // stop looking for matches
                } else {
                    // We reset the distance
                    minLevenshteinDistance = std::numeric_limits<int>::max();
                    templateWithMinLevenshteinDistance = nullptr;
                }
            }

            // We look for the shortest distance
            if ((matchStrength == bestMatchStrength) && (matchStrength > 0)) {
                // if the name has some meaning we calculate the distance
                if ((!name.isEmpty()) && (name != u"MusicXML Part") && (name != u"Staff")) {
                    // We keep the lowest distance with trackName ...
                    size_t levenshteinDistance = muse::strings::levenshteinDistance(
                        StaffName(name).toString().toStdString(), it->trackName.toStdString());

                    // ... and longNames
                    for (const muse::String& instLongName : it->longNames.toStringList()) {
                        levenshteinDistance = std::min(levenshteinDistance,
                                                       muse::strings::levenshteinDistance(
                                                           StaffName(name).toString().toStdString(), instLongName.toStdString()));
                    }
                    // If we have a shortest distance we keep this element
                    if (levenshteinDistance < minLevenshteinDistance) {
                        minLevenshteinDistance = levenshteinDistance;
                        templateWithMinLevenshteinDistance = it;
                    }
                }
            }
        }
    }

    // If we have improved the Levenshtein distance we change the best match
    if (minLevenshteinDistance < std::numeric_limits<int>::max()) {
        bestMatch = templateWithMinLevenshteinDistance;
    }

    return bestMatch;
}

const InstrumentTemplate* searchTemplateForMusicXmlId(const String& mxmlId)
{
    for (const InstrumentGroup* g : instrumentGroups) {
        for (const InstrumentTemplate* it : g->instrumentTemplates) {
            if (it->musicXmlId == mxmlId) {
                return it;
            }
        }
    }
    return 0;
}

const InstrumentTemplate* searchTemplateForInstrNameList(const std::list<String>& nameList, bool useDrumset, bool caseSensitive)
{
    const InstrumentTemplate* bestMatch = nullptr; // default if no matches
    int bestMatchStrength = 0; // higher for better matches

    for (String name : nameList) {
        if (name.isEmpty()) {
            continue;
        }

        if (!caseSensitive) {
            name = name.toLower();
        }
        StaffName instrName(name);

        for (const InstrumentGroup* g : instrumentGroups) {
            for (const InstrumentTemplate* it : g->instrumentTemplates) {
                if (it->useDrumset != useDrumset) {
                    continue;
                }

                String trackName = it->trackName;
                StaffNameList longNames = it->longNames;
                StaffNameList shortNames = it->shortNames;

                if (!caseSensitive) {
                    trackName = trackName.toLower();
                    for (StaffName& n : longNames) {
                        n.setName(n.name().toLower());
                    }
                    for (StaffName& n : shortNames) {
                        n.setName(n.name().toLower());
                    }
                }

                int matchStrength = 0
                                    + (4 * (trackName == name ? 1 : 0)) // most weight to track name since there are fewer duplicates
                                    + (2 * (muse::contains(longNames, instrName) ? 1 : 0))
                                    + (1 * (muse::contains(shortNames, instrName) ? 1 : 0)); // least weight to short name
                const int perfectMatchStrength = 7;
                assert(matchStrength <= perfectMatchStrength);
                if (matchStrength > bestMatchStrength) {
                    bestMatch = it;
                    bestMatchStrength = matchStrength;
                    if (bestMatchStrength == perfectMatchStrength) {
                        break; // stop looking for matches
                    }
                }
            }
        }
    }

    if (!bestMatch) {
        for (const String& name : nameList) {
            if (name.contains(u"drum", muse::CaseInsensitive)) {
                return searchTemplate(u"drumset");
            }

            if (name.contains(u"piano", muse::CaseInsensitive)) {
                return searchTemplate(u"piano");
            }
        }
    }

    return bestMatch; // nullptr if no matches found
}

const InstrumentTemplate* searchTemplateForMidiProgram(int bank, int program, bool useDrumset)
{
    for (const InstrumentGroup* g : instrumentGroups) {
        for (const InstrumentTemplate* it : g->instrumentTemplates) {
            if (it->useDrumset != useDrumset) {
                continue;
            }

            for (const InstrChannel& channel : it->channel) {
                if (channel.bank() == bank && channel.program() == program) {
                    return it;
                }
            }
        }
    }

    return nullptr;
}

void addTemplateToGroup(const InstrumentTemplate* templ, const String& groupId)
{
    IF_ASSERT_FAILED(templ) {
        return;
    }

    InstrumentGroup* group = searchInstrumentGroup(groupId);
    if (group) {
        group->instrumentTemplates.push_back(templ);
    }
}

//---------------------------------------------------------
//   searchTemplateIndexForTrackName
//---------------------------------------------------------

InstrumentIndex searchTemplateIndexForTrackName(const String& trackName)
{
    int instIndex = 0;
    int grpIndex = 0;
    for (const InstrumentGroup* g : instrumentGroups) {
        for (const InstrumentTemplate* it : g->instrumentTemplates) {
            if (it->trackName == trackName) {
                return InstrumentIndex(grpIndex, instIndex, it);
            }
            ++instIndex;
        }
        ++grpIndex;
    }
    return InstrumentIndex(-1, -1, nullptr);
}

//---------------------------------------------------------
//   searchTemplateIndexForId
//---------------------------------------------------------

InstrumentIndex searchTemplateIndexForId(const String& id)
{
    int instIndex = 0;
    int grpIndex = 0;
    for (const InstrumentGroup* g : instrumentGroups) {
        for (const InstrumentTemplate* it : g->instrumentTemplates) {
            if (it->id == id) {
                return InstrumentIndex(grpIndex, instIndex, it);
            }
            ++instIndex;
        }
        ++grpIndex;
    }
    return InstrumentIndex(-1, instIndex, nullptr);
}

//---------------------------------------------------------
//   linkGenre
//      link the current instrument template to the genre list specified by "genre"
//      Each genre is a list of pointers to instrument templates
//      The list of genres is at application level
//---------------------------------------------------------

void InstrumentTemplate::linkGenre(const String& genre)
{
    const InstrumentGenre* ig = searchInstrumentGenre(genre);
    if (ig) {
        genres.push_back(ig);
    }
}

void InstrumentGenre::write(XmlWriter& xml) const
{
    xml.startElement("Genre", { { "id", id } });
    xml.tag("name", name);
    xml.endElement();
}

void InstrumentGenre::read(XmlReader& e)
{
    id = e.attribute("id");
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "name") {
            name = muse::mtrc("engraving/instruments/genre", e.readText());
        } else {
            e.unknown();
        }
    }
}

void InstrumentFamily::write(XmlWriter& xml) const
{
    xml.startElement("Family", { { "id", id } });
    xml.tag("name", name);
    xml.endElement();
}

void InstrumentFamily::read(XmlReader& e)
{
    id = e.attribute("id");
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "name") {
            name = muse::mtrc("engraving/instruments/family", e.readText());
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   clefType
//---------------------------------------------------------

ClefTypeList InstrumentTemplate::clefType(staff_idx_t staffIdx) const
{
    if (staffIdx < staffCount) {
        return clefTypes[staffIdx];
    }
    return clefTypes[0];
}

String InstrumentTemplate::familyId() const
{
    return family ? family->id : String();
}

bool InstrumentTemplate::containsGenre(const String& genreId) const
{
    for (const InstrumentGenre* genre : genres) {
        if (genre->id == genreId) {
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------
//   defaultClef
//    traverse the instrument list for first instrument
//    with midi patch 'program'. Return the default clef
//    for this instrument.
//---------------------------------------------------------

ClefType defaultClef(int program)
{
    if (program >= 24 && program < 32) {              // this are guitars
        return ClefType::G8_VB;
    } else if (program >= 32 && program < 40) {       // this is bass
        return ClefType::F8_VB;
    }

    for (const InstrumentGroup* g : instrumentGroups) {
        for (const InstrumentTemplate* it : g->instrumentTemplates) {
            if (it->channel[0].bank() == 0 && it->channel[0].program() == program) {
                return it->clefTypes[0].concertClef;
            }
        }
    }
    return ClefType::G;
}
}
