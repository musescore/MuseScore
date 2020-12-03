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
#include "synthssettingsmodel.h"

#include "log.h"

using namespace mu::midi;

SynthsSettingsModel::SynthsSettingsModel(QObject* parent)
    : QObject(parent)
{
}

void SynthsSettingsModel::load()
{
    m_state = configuration()->synthesizerState();

    auto doLoad = [this](const QString& name) {
                      auto synth = synthRegister()->synthesizer(name.toStdString());
                      std::vector<io::path> avalaibleSFPaths = sfprovider()->soundFontPaths(synth->soundFontFormats());
                      for (const io::path& path : avalaibleSFPaths) {
                          m_avalaibleSoundFonts[name] << io::filename(path).toQString();
                      }
                      emit avalaibleChanged(name);
                      emit selectedChanged(name);
                  };

    doLoad("Fluid");
    doLoad("Zerberus");
}

void SynthsSettingsModel::apply()
{
    configuration()->saveSynthesizerState(m_state);
}

QStringList SynthsSettingsModel::selectedSoundFonts(const QString& synth) const
{
    auto it = m_state.groups.find(synth.toStdString());
    if (it == m_state.groups.end()) {
        return QStringList();
    }
    const SynthesizerState::Group& g = it->second;

    QStringList list;
    for (const SynthesizerState::Val& val : g.vals) {
        if (val.id == SynthesizerState::SoundFontID) {
            list << QString::fromStdString(val.val);
        }
    }

    return list;
}

QStringList SynthsSettingsModel::avalaibleSoundFonts(const QString& synth) const
{
    return m_avalaibleSoundFonts.value(synth);
}

void SynthsSettingsModel::soundFontUp(int selectedIndex_, const QString& synth)
{
    size_t selectedIndex = selectedIndex_;
    auto it = m_state.groups.find(synth.toStdString());
    if (it == m_state.groups.end()) {
        return;
    }
    SynthesizerState::Group& g = it->second;

    if (!(selectedIndex > 0 && selectedIndex < g.vals.size())) {
        return;
    }

    std::swap(g.vals[selectedIndex], g.vals[selectedIndex - 1]);

    emit selectedChanged(synth);
}

void SynthsSettingsModel::soundFontDown(int selectedIndex_, const QString& synth)
{
    size_t selectedIndex = selectedIndex_;
    auto it = m_state.groups.find(synth.toStdString());
    if (it == m_state.groups.end()) {
        return;
    }
    SynthesizerState::Group& g = it->second;

    if (!(selectedIndex > 0 && selectedIndex < (g.vals.size() - 1))) {
        return;
    }

    std::swap(g.vals[selectedIndex], g.vals[selectedIndex + 1]);

    emit selectedChanged(synth);
}

void SynthsSettingsModel::removeSoundFont(int selectedIndex_, const QString& synth)
{
    size_t selectedIndex = selectedIndex_;
    auto it = m_state.groups.find(synth.toStdString());
    if (it == m_state.groups.end()) {
        return;
    }
    SynthesizerState::Group& g = it->second;

    if (!(selectedIndex < g.vals.size())) {
        return;
    }

    g.vals.erase(g.vals.begin() + selectedIndex);

    emit selectedChanged(synth);
}

void SynthsSettingsModel::addSoundFont(int avalableIndex_, const QString& synth)
{
    size_t avalableIndex = avalableIndex_;
    const QStringList& avalaible = m_avalaibleSoundFonts[synth];
    std::string name = avalaible.at(avalableIndex).toStdString();

    auto it = m_state.groups.find(synth.toStdString());
    if (it == m_state.groups.end()) {
        return;
    }
    SynthesizerState::Group& g = it->second;

    auto vit = std::find_if(g.vals.cbegin(), g.vals.cend(), [name](const SynthesizerState::Val& val) {
        return val.val == name;
    });

    if (vit != g.vals.cend()) {
        LOGI() << "already added font: " << name;
        return;
    }

    g.vals.push_back(SynthesizerState::Val(SynthesizerState::SoundFontID, name));

    emit selectedChanged(synth);
}
