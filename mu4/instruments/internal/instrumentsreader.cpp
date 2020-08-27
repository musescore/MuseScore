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
#include "instrumentsreader.h"

#include <QXmlStreamReader>
#include <QApplication>

#include "libmscore/utils.h"
#include "libmscore/xml.h"

using namespace mu;
using namespace mu::instruments;
using namespace mu::midi;

RetVal<InstrumentsMeta> InstrumentsReader::readMeta(const io::path& path) const
{
    RetVal<InstrumentsMeta> result;

    RetVal<QByteArray> fileBytes = fileSystem()->readFile(path);

    if (!fileBytes.ret) {
        result.ret = fileBytes.ret;
        return result;
    }

    InstrumentsMeta meta;
    Ms::XmlReader reader(fileBytes.val);

    while (reader.readNextStartElement()) {
        if (reader.name() != "museScore") {
            continue;
        }

        while (reader.readNextStartElement()) {
            if (reader.name() == "instrument-group"
                || reader.name() == "InstrumentGroup") {
                GroupMeta groupMeta = readGroupMeta(reader, meta);
                meta.groups.insert(groupMeta.group.id, groupMeta.group);
                meta.instrumentTemplates.unite(groupMeta.templates);
            } else if (reader.name() == "Articulation") {
                MidiArticulation articulation = readArticulation(reader);
                meta.articulations.insert(articulation.name, articulation); // TODO: name?
            } else if (reader.name() == "Genre") {
                InstrumentGenre genre = readGenre(reader);
                meta.genres.insert(genre.id, genre);
            } else {
                reader.skipCurrentElement();
            }
        }
    }

    result.ret = make_ret(Ret::Code::Ok);
    result.val = meta;

    return result;
}

InstrumentsReader::GroupMeta InstrumentsReader::readGroupMeta(Ms::XmlReader& reader, InstrumentsMeta& generalMeta) const
{
    GroupMeta meta;

    InstrumentGroup group;
    group.id = reader.attributes().value("id").toString();
    group.name = qApp->translate("InstrumentsXML", reader.attributes().value("name").toUtf8().data()); // TODO: translate
    group.extended = reader.attributes().value("extended").toInt();

    while (reader.readNextStartElement()) {
        if (reader.name().toString().toLower() == "instrument") {
            InstrumentTemplate _template = readInstrumentTemplate(reader, generalMeta);
            _template.groupId = group.id;
            meta.templates.insert(_template.id, _template);
        } else if (reader.name() == "ref") {
            QString instrumentId = reader.readElementText();
            InstrumentTemplate templateRef = generalMeta.instrumentTemplates[instrumentId];
            InstrumentTemplate newTemplate;
            if (templateRef.isValid()) {
                copyInstrumentTemplate(templateRef, newTemplate);
            }
            generalMeta.instrumentTemplates.insert(newTemplate.id, newTemplate);
        } else if (reader.name() == "name") {
            group.name = qApp->translate("InstrumentsXML", reader.readElementText().toUtf8().data()); // TODO: translate
        } else if (reader.name() == "extended") {
            group.extended = reader.readElementText().toInt();
        } else {
            reader.skipCurrentElement();
        }
    }

    if (group.id.isEmpty()) {
        group.id = group.name.toLower().replace(" ", "-");
    }

    meta.group = group;

    return meta;
}

MidiArticulation InstrumentsReader::readArticulation(Ms::XmlReader& reader) const
{
    MidiArticulation articulation;
    articulation.name = reader.attributes().value("name").toString();

    while (reader.readNextStartElement()) {
        if (reader.name() == "velocity") {
            QString text(reader.readElementText());
            if (text.endsWith("%")) {
                text = text.left(text.size() - 1);
            }
            articulation.velocity = text.toInt();
        } else if (reader.name() == "gateTime") {
            QString text(reader.readElementText());
            if (text.endsWith("%")) {
                text = text.left(text.size() - 1);
            }
            articulation.gateTime = text.toInt();
        } else if (reader.name() == "descr") {
            articulation.descr = reader.readElementText();
        } else {
            reader.skipCurrentElement();
        }
    }

    return articulation;
}

InstrumentGenre InstrumentsReader::readGenre(Ms::XmlReader& reader) const
{
    InstrumentGenre genre;
    genre.id = reader.attributes().value("id").toString();

    while (reader.readNextStartElement()) {
        if (reader.name() == "name") {
            genre.name = qApp->translate("InstrumentsXML", reader.readElementText().toUtf8().data());
        } else {
            reader.skipCurrentElement();
        }
    }

    return genre;
}

InstrumentTemplate InstrumentsReader::readInstrumentTemplate(Ms::XmlReader& reader, InstrumentsMeta& generalMeta) const
{
    InstrumentTemplate instrumentTemplate;
    instrumentTemplate.id = reader.attributes().value("id").toString();

    bool customDrumset = false;

    while (reader.readNextStartElement()) {
        if (reader.name() == "longName" || reader.name() == "name") {
            int pos = reader.intAttribute("pos", 0);
            for (QList<StaffName>::iterator i = instrumentTemplate.longNames.begin(); i != instrumentTemplate.longNames.end(); ++i) {
                if ((*i).pos() == pos) {
                    instrumentTemplate.longNames.erase(i);
                    break;
                }
            }
            instrumentTemplate.longNames << StaffName(qApp->translate("InstrumentsXML", reader.readElementText().toUtf8().data()), pos);
        } else if (reader.name() == "shortName" || reader.name() == "short-name") {
            int pos = reader.intAttribute("pos", 0);
            for (QList<StaffName>::iterator i = instrumentTemplate.shortNames.begin();
                 i != instrumentTemplate.shortNames.end(); ++i) {
                if ((*i).pos() == pos) {
                    instrumentTemplate.shortNames.erase(i);
                    break;
                }
            }
            instrumentTemplate.shortNames << StaffName(qApp->translate("InstrumentsXML", reader.readElementText().toUtf8().data()), pos);
        } else if (reader.name() == "trackName") {
            instrumentTemplate.trackName = qApp->translate("InstrumentsXML", reader.readElementText().toUtf8().data());
        } else if (reader.name() == "description") {
            instrumentTemplate.description = qApp->translate("InstrumentsXML", reader.readElementText().toUtf8().data());
        } else if (reader.name() == "extended") {
            instrumentTemplate.extended = reader.readElementText().toInt();
        } else if (reader.name() == "staves") {
            instrumentTemplate.staves = reader.readElementText().toInt();
            instrumentTemplate.bracketSpan[0] = instrumentTemplate.staves;
        } else if (reader.name() == "clief") {
            int staffIndex = readStaffIndex(reader);

            QString clef = reader.readElementText();
            bool ok = false;
            int clefNumber = clef.toInt(&ok);
            ClefType ct = ok ? ClefType(clefNumber) : Clef::clefType(clef);
            instrumentTemplate.clefs[staffIndex]._concertClef = ct;
            instrumentTemplate.clefs[staffIndex]._transposingClef = ct;
        } else if (reader.name() == "concertClef") {
            int staffIndex = readStaffIndex(reader);

            QString clef = reader.readElementText();
            bool ok = false;
            int clefNumber = clef.toInt(&ok);
            ClefType ct = ok ? ClefType(clefNumber) : Clef::clefType(clef);
            instrumentTemplate.clefs[staffIndex]._concertClef = ct;
        } else if (reader.name() == "transposingClef") {
            int staffIndex = readStaffIndex(reader);

            QString clef = reader.readElementText();
            bool ok = false;
            int clefNumber = clef.toInt(&ok);
            ClefType ct = ok ? ClefType(clefNumber) : Clef::clefType(clef);
            instrumentTemplate.clefs[staffIndex]._transposingClef = ct;
        } else if (reader.name() == "stafflines") {
            int staffIndex = readStaffIndex(reader);
            instrumentTemplate.staffLines[staffIndex] = reader.readElementText().toInt();
        } else if (reader.name() == "smallStaff") {
            int staffIndex = readStaffIndex(reader);
            instrumentTemplate.smallStaff[staffIndex] = reader.readElementText().toInt();
        } else if (reader.name() == "bracket") {
            int staffIndex = readStaffIndex(reader);
            instrumentTemplate.bracket[staffIndex] = BracketType(reader.readElementText().toInt());
        } else if (reader.name() == "bracketSpan") {
            int staffIndex = readStaffIndex(reader);
            instrumentTemplate.bracketSpan[staffIndex] = reader.readElementText().toInt();
        } else if (reader.name() == "barlineSpan") {
            int staffIndex = readStaffIndex(reader);
            int span = reader.readElementText().toInt();
            for (int i = 0; i < span - 1; ++i) {
                instrumentTemplate.barlineSpan[staffIndex + i] = true;
            }
        } else if (reader.name() == "aPitchRange") {
            instrumentTemplate.aPitchRange = readPitchRange(reader);
        } else if (reader.name() == "pPitchRange") {
            instrumentTemplate.pPitchRange = readPitchRange(reader);
        } else if (reader.name() == "transposition") {
            instrumentTemplate.transpose.chromatic = reader.readElementText().toInt();
            instrumentTemplate.transpose.diatonic = Ms::chromatic2diatonic(instrumentTemplate.transpose.chromatic);
        } else if (reader.name() == "transposeChromatic") {
            instrumentTemplate.transpose.chromatic = reader.readElementText().toInt();
        } else if (reader.name() == "transposeDiatonic") {
            instrumentTemplate.transpose.diatonic = reader.readElementText().toInt();
        } else if (reader.name() == "instrumentId") {
            instrumentTemplate.id = reader.readElementText();
        } else if (reader.name() == "StringData") {
            instrumentTemplate.stringData = readStringData(reader);
        } else if (reader.name() == "useDrumset") {
            instrumentTemplate.useDrumset = reader.readElementText().toInt();
            if (instrumentTemplate.useDrumset) {
                delete instrumentTemplate.drumset;
                instrumentTemplate.drumset = new Drumset(*Ms::smDrumset);
            }
        } else if (reader.name() == "Drum") {
            if (!instrumentTemplate.drumset) {
                instrumentTemplate.drumset = new Drumset(*Ms::smDrumset);
            }
            if (!customDrumset) {
                const_cast<Drumset*>(instrumentTemplate.drumset)->clear();
                customDrumset = true;
            }
            const_cast<Drumset*>(instrumentTemplate.drumset)->load(reader);
        } else if (reader.name() == "MidiAction") {
            MidiAction action = readMidiAction(reader);
            instrumentTemplate.midiActions << action;
        } else if (reader.name() == "Channel" || reader.name() == "channel") {
            Channel channel;
            channel.read(reader, nullptr);
            instrumentTemplate.channels << channel;
        } else if (reader.name() == "Articulation") {
            MidiArticulation articulation = readArticulation(reader);
            generalMeta.articulations.insert(articulation.name, articulation);
        } else if (reader.name() == "stafftype") {
            int staffIndex = readStaffIndex(reader);

            QString xmlPresetName = reader.attributes().value("staffTypePreset").toString();
            QString stfGroup = reader.readElementText();
            if (stfGroup == "percussion") {
                instrumentTemplate.staffGroup = StaffGroup::PERCUSSION;
            } else if (stfGroup == "tablature") {
                instrumentTemplate.staffGroup = StaffGroup::TAB;
            } else {
                instrumentTemplate.staffGroup = StaffGroup::STANDARD;
            }
            instrumentTemplate.staffTypePreset = 0;
            if (!xmlPresetName.isEmpty()) {
                instrumentTemplate.staffTypePreset = StaffType::presetFromXmlName(xmlPresetName);
            }
            if (!instrumentTemplate.staffTypePreset || instrumentTemplate.staffTypePreset->group() != instrumentTemplate.staffGroup) {
                instrumentTemplate.staffTypePreset = StaffType::getDefaultPreset(instrumentTemplate.staffGroup);
            }
            if (instrumentTemplate.staffTypePreset) {
                instrumentTemplate.staffLines[staffIndex] = instrumentTemplate.staffTypePreset->lines();
            }
        } else if (reader.name() == "init") {
            QString initInstrumentId = reader.readElementText();
            copyInstrumentTemplate(generalMeta.instrumentTemplates[initInstrumentId], instrumentTemplate);
        } else if (reader.name() == "musicXMLid") {
            instrumentTemplate.musicXMLId = reader.readElementText();
        } else if (reader.name() == "genre") {
            instrumentTemplate.genreIds << reader.readElementText();
        } else if (reader.name() == "singleNoteDynamics") {
            instrumentTemplate.singleNoteDynamics = reader.readElementText().toInt();
        } else {
            reader.skipCurrentElement();
        }
    }

    fillByDeffault(instrumentTemplate);

    return instrumentTemplate;
}

int InstrumentsReader::readStaffIndex(Ms::XmlReader& reader) const
{
    int staffIndex = reader.attributes().value("staff").toInt() - 1;
    if (staffIndex >= MAX_STAVES) {
        staffIndex = MAX_STAVES - 1;
    } else if (staffIndex < 0) {
        staffIndex = 0;
    }

    return staffIndex;
}

PitchRange InstrumentsReader::readPitchRange(Ms::XmlReader& reader) const
{
    PitchRange range;
    QStringList ranges = reader.readElementText().split("-");
    if (ranges.size() != 2) {
        range.min = 0;
        range.max = 127;
        return range;
    }
    range.min = ranges[0].toInt();
    range.max = ranges[1].toInt();

    return range;
}

MidiAction InstrumentsReader::readMidiAction(Ms::XmlReader& reader) const
{
    MidiAction action;
    action.name = reader.attributes().value("name").toString();

    while (reader.readNextStartElement()) {
        if (reader.name() == "program") {
            Event event(0 /*TODO*/, EventType::ME_CONTROLLER, CntrType::CTRL_PROGRAM,
                        reader.attributes().value("value").toInt());
            action.events.push_back(event);
            reader.skipCurrentElement();
        } else if (reader.name() == "controller") {
            Event event(0 /*TODO*/, EventType::ME_CONTROLLER, reader.attributes().value("ctrl").toInt(),
                        reader.attributes().value("value").toInt());
            action.events.push_back(event);
            reader.skipCurrentElement();
        } else if (reader.name() == "descr") {
            action.description = reader.readElementText();
        } else {
            reader.skipCurrentElement();
        }
    }

    return action;
}

StringData InstrumentsReader::readStringData(Ms::XmlReader& reader) const
{
    int frets = 0;
    QList<Ms::instrString> strings;

    while (reader.readNextStartElement()) {
        if (reader.name() == "frets") {
            frets = reader.readElementText().toInt();
        } else if (reader.name() == "string") {
            Ms::instrString strg;
            strg.open  = reader.attributes().value("open").toInt();
            strg.pitch = reader.readElementText().toInt();
            strings << strg;
        } else {
            reader.skipCurrentElement();
        }
    }

    return StringData(frets, strings);
}

void InstrumentsReader::copyInstrumentTemplate(const InstrumentTemplate& from, InstrumentTemplate& to) const
{
    to.longNames = from.longNames;
    to.shortNames = from.shortNames;
    to.musicXMLId = from.musicXMLId;
    to.staves = from.staves;
    to.extended = from.extended;

    for (int i = 0; i < MAX_STAVES; ++i) {
        to.clefs[i] = from.clefs[i];
        to.staffLines[i] = from.staffLines[i];
        to.smallStaff[i] = from.smallStaff[i];
        to.bracket[i] = from.bracket[i];
        to.bracketSpan[i] = from.bracketSpan[i];
        to.barlineSpan[i] = from.barlineSpan[i];
    }

    to.aPitchRange = from.aPitchRange;
    to.pPitchRange = from.pPitchRange;
    to.transpose  = from.transpose;
    to.staffGroup = from.staffGroup;
    to.staffTypePreset = from.staffTypePreset;
    to.useDrumset = from.useDrumset;
    if (from.drumset) {
        to.drumset = new Drumset(*from.drumset);
    } else {
        to.drumset = 0;
    }
    to.stringData  = from.stringData;
    to.midiActions = from.midiActions;
    to.channels = from.channels;
    to.singleNoteDynamics = from.singleNoteDynamics;
}

void InstrumentsReader::fillByDeffault(InstrumentTemplate& instrumentTemplate) const
{
    if (instrumentTemplate.channels.empty()) {
        Channel a;
        a.setChorus(0);
        a.setReverb(0);
        a.setName(Channel::DEFAULT_NAME);
        a.setProgram(0);
        a.setBank(0);
        a.setVolume(90);
        a.setPan(0);
        instrumentTemplate.channels.append(a);
    }
    if (instrumentTemplate.useDrumset) {
        if (instrumentTemplate.channels[0].bank() == 0 && instrumentTemplate.channels[0].synti().toLower() != "zerberus") {
            instrumentTemplate.channels[0].setBank(128);
        }
    }
    if (instrumentTemplate.trackName.isEmpty() && !instrumentTemplate.longNames.isEmpty()) {
        instrumentTemplate.trackName = instrumentTemplate.longNames[0].name();
    }
    if (instrumentTemplate.description.isEmpty() && !instrumentTemplate.longNames.isEmpty()) {
        instrumentTemplate.description = instrumentTemplate.longNames[0].name();
    }
    if (instrumentTemplate.id.isEmpty()) {
        instrumentTemplate.id = instrumentTemplate.trackName.toLower().replace(" ", "-");
    }
}
