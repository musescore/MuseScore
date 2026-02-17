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
    staffCount         = 1;
    minPitchA          = MIN_PITCH;
    maxPitchA          = MAX_PITCH;
    minPitchP          = MIN_PITCH;
    maxPitchP          = MAX_PITCH;
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
    longName = t.longName;
    shortName = t.shortName;
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
    glissandoStyle = t.glissandoStyle;
    barlineSpan = t.barlineSpan;

    for (int i = 0; i < MAX_STAVES; ++i) {
        clefTypes[i]   = t.clefTypes[i];
        staffLines[i]  = t.staffLines[i];
        smallStaff[i]  = t.smallStaff[i];
        bracket[i]     = t.bracket[i];
        bracketSpan[i] = t.bracketSpan[i];
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

    write::TWrite::write(longName, xml, "longName");
    write::TWrite::write(shortName, xml, "shortName");

    if (!longName.name().empty()) {
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
            if (i > 0) {
                xml.tag("barlineSpan", { { "staff", i + 1 } }, int { barlineSpan[i] });
            } else {
                xml.tag("barlineSpan", int { barlineSpan[i] });
            }
        }
    }
    if (minPitchA != MIN_PITCH || maxPitchA != MAX_PITCH) {
        xml.tag("aPitchRange", String(u"%1-%2").arg(minPitchA).arg(maxPitchA));
    }
    if (minPitchP != MIN_PITCH || maxPitchP != MAX_PITCH) {
        xml.tag("pPitchRange", String(u"%1-%2").arg(minPitchP).arg(maxPitchP));
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

    xml.tag("glissandoStyle", TConv::toXml(glissandoStyle));

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
            longName = StaffName(translateInstrumentName(id, u"longName", e.readText()), pos);
        } else if (tag == "shortName" || tag == "short-name") {     // "short-name" is obsolete
            int pos = e.intAttribute("pos", 0);
            shortName = StaffName(translateInstrumentName(id, u"shortName", e.readText()), pos);
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
            setPitchRange(e.readText(), minPitchA, maxPitchA);
        } else if (tag == "pPitchRange") {
            setPitchRange(e.readText(), minPitchP, maxPitchP);
        } else if (tag == "transposition") {      // obsolete
            int i = e.readInt();
            transpose.chromatic = i;
            transpose.diatonic = Interval::chromatic2diatonic(i);
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
            // if we see one of these tags, a custom drumset will be created
            if (!drumset) {
                drumset = new Drumset(*smDrumset);
                drumset->clear();
            }
            drumset->loadDrum(e);
        } else if (tag == "percussionPanelColumns") {
            // if we see one of these tags, a custom drumset will be created
            if (!drumset) {
                drumset = new Drumset(*smDrumset);
                drumset->clear();
            }
            drumset->setPercussionPanelColumns(e.readInt());
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
        } else if (tag == "glissandoStyle") {
            glissandoStyle = TConv::fromXml(e.readAsciiText(), GlissandoStyle::CHROMATIC);
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

    if (trackName.isEmpty() && !longName.name().isEmpty()) {
        trackName = longName.name();
    }
    if (description.isEmpty() && !longName.name().isEmpty()) {
        description = longName.name();
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

void InstrumentTemplate::setPitchRange(const String& s, int& a, int& b) const
{
    StringList sl = s.split(u'-');
    if (sl.size() != 2) {
        a = MIN_PITCH;
        b = MAX_PITCH;
        return;
    }
    a = sl.at(0).toInt();
    b = sl.at(1).toInt();
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

// Given an unknown instrument (i.e. one that lacks a valid MuseScore ID),
// compare it to each of our built-in instruments and return the strongest
// available match, or `nullptr` if no match is found.
//
// Match strength is determined according to multiple factors (see code). Feel
// free to add more factors to this function. Please don't create new functions
// that only consider a subset of factors. You can disable matching against a
// specific factor by setting it to its default value in the input instrument.
//
// Use this to recognize instruments in pre-3.6 score files, and in non-native
// formats like MIDI and MusicXML (e.g. see importmusicxmlpass2.cpp). This
// ensures the correct sound and engraving properties are loaded.

const InstrumentTemplate* combinedTemplateSearch(const Instrument& instrument)
{
    const InstrChannel* const channel = instrument.channel(0);
    const int bank = channel->bank();
    const int program = channel->program();
    const int transposeChromatic = instrument.transpose().chromatic;
    const int transposeKey = (transposeChromatic % 12) + (transposeChromatic < 0 ? 12 : 0); // 0=C, 1=C#/Db, 2=D...
    const String& trackName = instrument.trackName();
    const String& traitName = instrument.trait().name;
    String musicXmlId = instrument.musicXmlId();

    if (musicXmlId.isEmpty() && trackName.isEmpty() && bank == 0 && program == -1) {
        return nullptr; // Not enough information to find a matching template.
    }

    if (musicXmlId.isEmpty()) {
        if (trackName.contains(u"drum", muse::CaseInsensitive)) {
            musicXmlId = u"drum.group.set";
        } else if (trackName == u"MusicXML Part" || trackName.contains(u"piano", muse::CaseInsensitive)) {
            musicXmlId = u"keyboard.piano";
        }
    } else {
        // Workaround for old generic instrument templates.
        if ((musicXmlId == u"wind.reed.clarinet" || musicXmlId == u"brass.trumpet")
            && (traitName == u"B♭" || transposeKey == 10)
            ) {
            musicXmlId.append(u".bflat");
        }
    }

    // Perform a weighted search over instrument names. Current limitations:
    // * We only check in the Preferences language. (Ideally we'd check English, German & Italian.)
    // * We don't consider aliases (e.g. we look for "Violoncello" but not "Double Bass").
    static constexpr int TRACK_NAME_WEIGHT = 128;
    static constexpr int LONG_NAME_WEIGHT = 64;
    static constexpr int SHORT_NAME_WEIGHT = 16; // Not standardized outside MuseScore.
    static constexpr int TRAIT_NAME_WEIGHT = 4; // More reliable than transposition interval.

    // Also search over these parameters, which don't depend on the language.
    static constexpr int MUSICXML_ID_WEIGHT = 32; // Not unique. Newer, more specialist IDs aren't in old files.
    static constexpr int MIDI_WEIGHT = 8; // Standardization limited to General MIDI Level 2.
    static constexpr int TRANSPOSITION_KEY_WEIGHT = 2; // Unreliable (e.g. in concert-pitch MusicXML).
    static constexpr int TRANSPOSITION_OCTAVE_WEIGHT = 1;

    // Optimize performance: Ignore low-strength matches.
    static constexpr int MIN_MATCH_STRENGTH = MIDI_WEIGHT;

    // Also consider name almost-matches (i.e. fuzzy text search).
    static constexpr size_t INITIAL_LEVENSHTEIN_DISTANCE = std::numeric_limits<size_t>::max();

    const InstrumentTemplate* bestMatch = nullptr;
    int bestMatchStrength = MIN_MATCH_STRENGTH;
    auto bestMatchLevDist = INITIAL_LEVENSHTEIN_DISTANCE;

    for (const InstrumentGroup* group : instrumentGroups) {
        for (const InstrumentTemplate* templ : group->instrumentTemplates) {
            if (templ->trait.name == u"[hide]") {
                continue;
            }

            int matchStrength = 0;

            if (!trackName.isEmpty() && trackName == templ->trackName) {
                matchStrength += TRACK_NAME_WEIGHT;
            }

            if (instrument.longName() == templ->longName) {
                matchStrength += LONG_NAME_WEIGHT;
            }

            if (instrument.shortName() == templ->shortName) {
                matchStrength += SHORT_NAME_WEIGHT;
            }

            if (!traitName.isEmpty() && traitName == templ->trait.name) {
                matchStrength += TRAIT_NAME_WEIGHT;
            }

            if (!musicXmlId.empty() && musicXmlId == templ->musicXmlId) {
                matchStrength += MUSICXML_ID_WEIGHT;
            }

            for (const InstrChannel& templChannel : templ->channel) {
                if (bank == templChannel.bank() && program == templChannel.program()) {
                    matchStrength += MIDI_WEIGHT;
                    break;
                }
            }

            const int chromaticDiff = std::abs(transposeChromatic - templ->transpose.chromatic);

            if (chromaticDiff % 12 == 0) { // Same key.
                matchStrength += TRANSPOSITION_KEY_WEIGHT;
            }

            if (chromaticDiff < 12) { // Same octave.
                matchStrength += TRANSPOSITION_OCTAVE_WEIGHT;
            }

            if (matchStrength < bestMatchStrength) {
                continue; // Keep looking.
            }

            if (matchStrength > bestMatchStrength) {
                bestMatch = templ;
                bestMatchStrength = matchStrength;
                bestMatchLevDist = INITIAL_LEVENSHTEIN_DISTANCE; // Reset.
            }

            // Find smallest lev. dist. among templates with match strength equal to best.
            auto levDist = INITIAL_LEVENSHTEIN_DISTANCE;

            if (!trackName.isEmpty()) {
                levDist = muse::strings::levenshteinDistance(
                    trackName.toStdString(),
                    templ->trackName.toStdString()
                    );
            }

            levDist
                = std::min(levDist,
                           muse::strings::levenshteinDistance(instrument.longName().name().toStdString(),
                                                              templ->longName.name().toStdString()));

            if (levDist < bestMatchLevDist) {
                bestMatch = templ;
                bestMatchLevDist = levDist;
            }
        }
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

const InstrumentTemplate* searchTemplateForInstrNameList(const std::vector<String>& nameList, bool useDrumset, bool caseSensitive)
{
    const InstrumentTemplate* bestMatch = nullptr; // default if no matches
    int bestMatchStrength = 0; // higher for better matches

    for (const String& name : nameList) {
        if (name.isEmpty()) {
            continue;
        }

        auto stringEqualsName = caseSensitive
                                ? std::function([&name](const String& s) { return s == name; })
                                : std::function([&name](const String& s) { return s.isEqualIgnoreCase(name); });

        auto staffNameEqualsName = caseSensitive
                                   ? std::function([&name](const StaffName& sn) { return sn.name() == name; })
                                   : std::function([&name](const StaffName& sn) { return sn.name().isEqualIgnoreCase(name); });

        for (const InstrumentGroup* g : instrumentGroups) {
            for (const InstrumentTemplate* it : g->instrumentTemplates) {
                if (it->useDrumset != useDrumset) {
                    continue;
                }

                int matchStrength
                    = 0
                      + (4 * (stringEqualsName(it->trackName) ? 1 : 0)) // most weight to track name since there are fewer duplicates
                      + (2 * (staffNameEqualsName(it->longName) ? 1 : 0))
                      + (1 * (staffNameEqualsName(it->shortName) ? 1 : 0)); // least weight to short name
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
        static const std::wregex drumsetRegex(L"drum ?(set|kit)", std::regex_constants::icase);

        for (const String& name : nameList) {
            if (name.contains(drumsetRegex)) {
                return searchTemplate(u"drumset"); // Large Drum Kit
            }

            if (name.contains(u"drum", muse::CaseInsensitive) || name.contains(u"percussion", muse::CaseInsensitive)) {
                return searchTemplate(u"percussion-synthesizer"); // General MIDI percussion
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
