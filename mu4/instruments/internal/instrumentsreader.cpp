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
            _template.instrument.groupId = group.id;
            meta.templates.insert(_template.id, _template);
        } else if (reader.name() == "ref") {
            QString templateId = reader.readElementText();
            InstrumentTemplate newTemplate = generalMeta.instrumentTemplates[templateId];
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
    Instrument& instrument = instrumentTemplate.instrument;

    instrumentTemplate.id = reader.attributes().value("id").toString();

    bool customDrumset = false;

    while (reader.readNextStartElement()) {
        if (reader.name() == "longName" || reader.name() == "name") {
            int pos = reader.intAttribute("pos", 0);
            for (QList<StaffName>::iterator i = instrument.longNames.begin(); i != instrument.longNames.end(); ++i) {
                if ((*i).pos() == pos) {
                    instrument.longNames.erase(i);
                    break;
                }
            }
            instrument.longNames << StaffName(qApp->translate("InstrumentsXML", reader.readElementText().toUtf8().data()), pos);
        } else if (reader.name() == "shortName" || reader.name() == "short-name") {
            int pos = reader.intAttribute("pos", 0);
            for (QList<StaffName>::iterator i = instrument.shortNames.begin();
                 i != instrument.shortNames.end(); ++i) {
                if ((*i).pos() == pos) {
                    instrument.shortNames.erase(i);
                    break;
                }
            }
            instrument.shortNames << StaffName(qApp->translate("InstrumentsXML", reader.readElementText().toUtf8().data()), pos);
        } else if (reader.name() == "trackName") {
            instrument.name = qApp->translate("InstrumentsXML", reader.readElementText().toUtf8().data());
        } else if (reader.name() == "description") {
            instrument.description = qApp->translate("InstrumentsXML", reader.readElementText().toUtf8().data());
        } else if (reader.name() == "extended") {
            instrument.extended = reader.readElementText().toInt();
        } else if (reader.name() == "staves") {
            instrument.staves = reader.readElementText().toInt();
            instrument.bracketSpan[0] = instrument.staves;
        } else if (reader.name() == "clief") {
            int staffIndex = readStaffIndex(reader);

            QString clef = reader.readElementText();
            bool ok = false;
            int clefNumber = clef.toInt(&ok);
            ClefType ct = ok ? ClefType(clefNumber) : Clef::clefType(clef);
            instrument.clefs[staffIndex]._concertClef = ct;
            instrument.clefs[staffIndex]._transposingClef = ct;
        } else if (reader.name() == "concertClef") {
            int staffIndex = readStaffIndex(reader);

            QString clef = reader.readElementText();
            bool ok = false;
            int clefNumber = clef.toInt(&ok);
            ClefType ct = ok ? ClefType(clefNumber) : Clef::clefType(clef);
            instrument.clefs[staffIndex]._concertClef = ct;
        } else if (reader.name() == "transposingClef") {
            int staffIndex = readStaffIndex(reader);

            QString clef = reader.readElementText();
            bool ok = false;
            int clefNumber = clef.toInt(&ok);
            ClefType ct = ok ? ClefType(clefNumber) : Clef::clefType(clef);
            instrument.clefs[staffIndex]._transposingClef = ct;
        } else if (reader.name() == "stafflines") {
            int staffIndex = readStaffIndex(reader);
            instrument.staffLines[staffIndex] = reader.readElementText().toInt();
        } else if (reader.name() == "smallStaff") {
            int staffIndex = readStaffIndex(reader);
            instrument.smallStaff[staffIndex] = reader.readElementText().toInt();
        } else if (reader.name() == "bracket") {
            int staffIndex = readStaffIndex(reader);
            instrument.bracket[staffIndex] = BracketType(reader.readElementText().toInt());
        } else if (reader.name() == "bracketSpan") {
            int staffIndex = readStaffIndex(reader);
            instrument.bracketSpan[staffIndex] = reader.readElementText().toInt();
        } else if (reader.name() == "barlineSpan") {
            int staffIndex = readStaffIndex(reader);
            int span = reader.readElementText().toInt();
            for (int i = 0; i < span - 1; ++i) {
                instrument.barlineSpan[staffIndex + i] = true;
            }
        } else if (reader.name() == "aPitchRange") {
            instrument.amateurPitchRange = readPitchRange(reader);
        } else if (reader.name() == "pPitchRange") {
            instrument.professionalPitchRange = readPitchRange(reader);
        } else if (reader.name() == "transposition") {
            instrument.transpose.chromatic = reader.readElementText().toInt();
            instrument.transpose.diatonic = Ms::chromatic2diatonic(instrument.transpose.chromatic);
        } else if (reader.name() == "transposeChromatic") {
            instrument.transpose.chromatic = reader.readElementText().toInt();
        } else if (reader.name() == "transposeDiatonic") {
            instrument.transpose.diatonic = reader.readElementText().toInt();
        } else if (reader.name() == "instrumentId") {
            instrument.id = reader.readElementText();
        } else if (reader.name() == "StringData") {
            instrument.stringData = readStringData(reader);
        } else if (reader.name() == "useDrumset") {
            instrument.useDrumset = reader.readElementText().toInt();
            if (instrument.useDrumset) {
                delete instrument.drumset;
                instrument.drumset = new Drumset(*Ms::smDrumset);
            }
        } else if (reader.name() == "Drum") {
            if (!instrument.drumset) {
                instrument.drumset = new Drumset(*Ms::smDrumset);
            }
            if (!customDrumset) {
                const_cast<Drumset*>(instrument.drumset)->clear();
                customDrumset = true;
            }
            const_cast<Drumset*>(instrument.drumset)->load(reader);
        } else if (reader.name() == "MidiAction") {
            MidiAction action = readMidiAction(reader);
            instrument.midiActions << action;
        } else if (reader.name() == "Channel" || reader.name() == "channel") {
            Channel channel;
            channel.read(reader, nullptr);
            instrument.channels << channel;
        } else if (reader.name() == "Articulation") {
            MidiArticulation articulation = readArticulation(reader);
            generalMeta.articulations.insert(articulation.name, articulation);
        } else if (reader.name() == "stafftype") {
            int staffIndex = readStaffIndex(reader);

            QString xmlPresetName = reader.attributes().value("staffTypePreset").toString();
            QString stfGroup = reader.readElementText();
            if (stfGroup == "percussion") {
                instrument.staffGroup = StaffGroup::PERCUSSION;
            } else if (stfGroup == "tablature") {
                instrument.staffGroup = StaffGroup::TAB;
            } else {
                instrument.staffGroup = StaffGroup::STANDARD;
            }
            instrument.staffTypePreset = 0;
            if (!xmlPresetName.isEmpty()) {
                instrument.staffTypePreset = StaffType::presetFromXmlName(xmlPresetName);
            }
            if (!instrument.staffTypePreset || instrument.staffTypePreset->group() != instrument.staffGroup) {
                instrument.staffTypePreset = StaffType::getDefaultPreset(instrument.staffGroup);
            }
            if (instrument.staffTypePreset) {
                instrument.staffLines[staffIndex] = instrument.staffTypePreset->lines();
            }
        } else if (reader.name() == "init") {
            QString templateId = reader.readElementText();
            instrument = generalMeta.instrumentTemplates[templateId].instrument;
        } else if (reader.name() == "musicXMLid") {
            instrument.id = reader.readElementText();
        } else if (reader.name() == "genre") {
            instrument.genreIds << reader.readElementText();
        } else if (reader.name() == "singleNoteDynamics") {
            instrument.singleNoteDynamics = reader.readElementText().toInt();
        } else {
            reader.skipCurrentElement();
        }
    }

    fillByDeffault(instrument);

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

void InstrumentsReader::fillByDeffault(Instrument& instrument) const
{
    if (instrument.channels.empty()) {
        Channel a;
        a.setChorus(0);
        a.setReverb(0);
        a.setName(Channel::DEFAULT_NAME);
        a.setProgram(0);
        a.setBank(0);
        a.setVolume(90);
        a.setPan(0);
        instrument.channels.append(a);
    }
    if (instrument.useDrumset) {
        if (instrument.channels[0].bank() == 0 && instrument.channels[0].synti().toLower() != "zerberus") {
            instrument.channels[0].setBank(128);
        }
    }
    if (instrument.name.isEmpty() && !instrument.longNames.isEmpty()) {
        instrument.name = instrument.longNames[0].name();
    }
    if (instrument.description.isEmpty() && !instrument.longNames.isEmpty()) {
        instrument.description = instrument.longNames[0].name();
    }
    if (instrument.id.isEmpty()) {
        instrument.id = instrument.name.toLower().replace(" ", "-");
    }
}
