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

#include <list>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>

#include "log.h"
#include "settings.h"
#include "stringutils.h"

using namespace mu;
using namespace mu::midi;
using namespace mu::framework;

static const std::string module_name("midi");

static const Settings::Key MY_SOUNDFONTS(module_name, "application/paths/mySoundfonts");

//! FIXME Temporary for tests
static const std::string DEFAULT_FLUID_SOUNDFONT = "MuseScore_General.sf3"; // "GeneralUser GS v1.471.sf2"; // "MuseScore_General.sf3";
static const std::string DEFAULT_ZERBERUS_SOUNDFONT = "FM-Piano1-20190916.sfz"; // "";

std::vector<io::path> MidiConfiguration::soundFontPaths() const
{
    std::string pathsStr = settings()->value(MY_SOUNDFONTS).toString();
    std::vector<io::path> paths = io::path::pathsFromString(pathsStr, ";");
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
        state.groups.insert({ gf.name, std::move(gf) });

        SynthesizerState::Group gz;
        gz.name = "Zerberus";
        gz.vals.push_back(SynthesizerState::Val(SynthesizerState::SoundFontID, DEFAULT_ZERBERUS_SOUNDFONT));
        state.groups.insert({ gz.name, std::move(gz) });
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
        LOGW() << "failed read synthesizer state, file: " << stateFilePath();
        m_state = defaultSynthesizerState();
    }

    return m_state;
}

Ret MidiConfiguration::saveSynthesizerState(const SynthesizerState& state)
{
    std::list<std::string> changedGroups;
    for (auto it = m_state.groups.cbegin(); it != m_state.groups.cend(); ++it) {
        auto nit = state.groups.find(it->first);
        if (nit == state.groups.cend()) {
            continue;
        }

        if (it->second != nit->second) {
            changedGroups.push_back(it->first);
        }
    }

    Ret ret = writeState(stateFilePath(), state);
    if (!ret) {
        LOGE() << "failed write synthesizer state, file: " << stateFilePath();
        return ret;
    }

    m_state = state;
    m_synthesizerStateChanged.notify();
    for (const std::string& gname : changedGroups) {
        m_synthesizerStateGroupChanged[gname].notify();
    }

    return make_ret(Ret::Code::Ok);
}

async::Notification MidiConfiguration::synthesizerStateChanged() const
{
    return m_synthesizerStateChanged;
}

async::Notification MidiConfiguration::synthesizerStateGroupChanged(const std::string& gname) const
{
    return m_synthesizerStateGroupChanged[gname];
}

io::path MidiConfiguration::stateFilePath() const
{
    return globalConfiguration()->dataPath() + "/synthesizer.xml";
}

bool MidiConfiguration::readState(const io::path& path, SynthesizerState& state) const
{
    QFile file(path.toQString());
    if (!file.open(QIODevice::ReadOnly)) {
        LOGE() << "failed open " << path;
        return false;
    }

    QXmlStreamReader xml(&file);

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartDocument) {
            continue;
        }

        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == "Synthesizer") {
                continue;
            }

            while (!(xml.tokenType() == QXmlStreamReader::EndElement)) {
                SynthesizerState::Group g;
                g.name = xml.name().toString().toStdString();

                xml.readNext();

                while (!(xml.tokenType() == QXmlStreamReader::EndElement)) {
                    if (xml.tokenType() == QXmlStreamReader::StartElement) {
                        if (xml.name() == "val") {
                            SynthesizerState::ValID id = static_cast<SynthesizerState::ValID>(xml.attributes().value("id").toInt());
                            g.vals.push_back(SynthesizerState::Val(id, xml.readElementText().toStdString()));
                        } else {
                            xml.skipCurrentElement();
                        }
                    }
                    xml.readNext();
                }

                state.groups.insert({ g.name, std::move(g) });
            }
        }
    }

    if (xml.hasError()) {
        LOGE() << "failed parse xml, error: " << xml.errorString() << ", path: " << path;
        return false;
    }

    return true;
}

bool MidiConfiguration::writeState(const io::path& path, const SynthesizerState& state)
{
    QFile file(path.toQString());
    if (!file.open(QIODevice::WriteOnly)) {
        LOGE() << "failed open " << path;
        return false;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();

    xml.writeStartElement("Synthesizer");

    for (auto it = state.groups.cbegin(); it != state.groups.cend(); ++it) {
        const SynthesizerState::Group& g = it->second;

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
    xml.writeEndDocument();

    if (xml.hasError()) {
        LOGE() << "failed write xml";
        return false;
    }

    if (!file.flush()) {
        LOGE() << "failed flush data, file: " << path;
        return false;
    }

    return true;
}
