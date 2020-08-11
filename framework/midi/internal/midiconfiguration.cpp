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
#include "midiconfiguration.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>

#include "log.h"
#include "settings.h"
#include "stringutils.h"

using namespace mu;
using namespace mu::midi;
using namespace mu::framework;

static std::string module_name("midi");

static const Settings::Key MY_SOUNDFONTS(module_name, "application/paths/mySoundfonts");

//! FIXME Temporary for tests
static const std::string DEFAULT_FLUID_SOUNDFONT = "MuseScore_General.sf3"; // "GeneralUser GS v1.471.sf2"; // "MuseScore_General.sf3";
static const std::string DEFAULT_ZERBERUS_SOUNDFONT = "FM-Piano1-20190916.sfz"; // "";

std::vector<io::path> MidiConfiguration::soundFontPaths() const
{
    std::string pathsStr = settings()->value(MY_SOUNDFONTS).toString();
    std::vector<io::path> paths;
    strings::split(pathsStr, paths, ";");

    paths.push_back(globalConfiguration()->sharePath());

    //! TODO Implement me
    // append extensions directory
    //QStringList extensionsDir = Ms::Extension::getDirectoriesByType(Ms::Extension::soundfontsDir);

    return paths;
}

const SynthesizerState& MidiConfiguration::defaultSynthesizerState() const
{
    static SynthesizerState state;
    if (state.isNull()) {
        SynthesizerState::Group gf;
        gf.name = "Fluid";
        gf.vals.push_back(SynthesizerState::Val(SynthesizerState::SoundFontID, DEFAULT_FLUID_SOUNDFONT));
        state.groups.push_back(std::move(gf));

        SynthesizerState::Group gz;
        gz.name = "Zerberus";
        gz.vals.push_back(SynthesizerState::Val(SynthesizerState::SoundFontID, DEFAULT_ZERBERUS_SOUNDFONT));
        state.groups.push_back(std::move(gz));
    }

    return state;
}

const SynthesizerState& MidiConfiguration::synthesizerState() const
{
    if (!m_state.isNull()) {
        return m_state;
    }

    bool ok = readState(stateFilePath(), m_state);
    if (!ok) {
        m_state = defaultSynthesizerState();
    }

    return m_state;
}

io::path MidiConfiguration::stateFilePath() const
{
    return globalConfiguration()->dataPath() + "/synthesizer.xml";
}

bool MidiConfiguration::readState(const io::path& path, SynthesizerState& state) const
{
    QXmlStreamReader xml(io::pathToQString(path));

    while (xml.readNextStartElement()) {
        if (xml.name() == "Synthesizer") {
            SynthesizerState::Group g;
            g.name = xml.name().toString().toStdString();

            while (xml.readNextStartElement()) {
                if (xml.name() == "val") {
                    SynthesizerState::ValID id = static_cast<SynthesizerState::ValID>(xml.attributes().value("id").toInt());
                    g.vals.push_back(SynthesizerState::Val(id, xml.readElementText().toStdString()));
                } else {
                    xml.skipCurrentElement();
                }
            }

            state.groups.push_back(std::move(g));
        } else {
            LOGE() << "unknown format";
            return false;
        }
    }

    return !xml.hasError();
}

bool MidiConfiguration::writeState(const io::path& path, const SynthesizerState& state)
{
    QFile file(io::pathToQString(path));
    if (!file.open(QIODevice::WriteOnly)) {
        LOGE() << "failed open " << path;
        return false;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();

    xml.writeStartElement("Synthesizer");

    for (const SynthesizerState::Group& g : state.groups) {
        if (!g.name.empty()) {
            xml.writeStartElement(QString::fromStdString(g.name));
            for (const SynthesizerState::Val& v : g.vals) {
                xml.writeStartElement("val");
                xml.writeAttribute("id", QString::number(static_cast<int>(v.id)));
                xml.writeCharacters(QString::fromStdString(v.val));
                xml.writeEndElement();
            }
            xml.writeEndElement();
        }
    }

    xml.writeEndElement();

    return !xml.hasError();
}
