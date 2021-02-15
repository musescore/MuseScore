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
#include "vstdevtools.h"
#include <QStandardItemModel>
#include <QStandardItem>
#include "log.h"
#include "internal/vstsynthesizer.h"
#include "internal/plugininstance.h"

using namespace mu::vst;

VSTDevTools::VSTDevTools(QObject* parent)
    : QObject(parent), m_pluginsListModel(new PluginListModel(vstScanner(), this))
{
}

PluginListModel* VSTDevTools::plugins()
{
    return m_pluginsListModel;
}

void VSTDevTools::addInstance(unsigned int index)
{
    auto plugin = m_pluginsListModel->item(index);
    if (plugin.getType() == Plugin::UNKNOWN) {
        LOGE() << "UNKNOW plugin type of instance";
        return;
    }
    auto instance = plugin.createInstance();
    instance->setActive(true);
    emit instancesChanged();
}

QString VSTDevTools::pluginName(int index)
{
    auto instance = vstInstanceRegister()->instance(index);
    return QString::fromStdString(instance->plugin().getName());
}

void VSTDevTools::play(int index)
{
    auto instance = vstInstanceRegister()->instance(index);
    if (!instance) {
        LOGE() << "instance not found";
        return;
    }

    if (!m_midiStream) {
        makeArpeggio();
    }
    auto synth = VSTSynthesizer::create(instance);
    m_midiStream->initData.synthMap[0] = synth->name();
    sequencer()->instantlyPlayMidi(m_midiStream->initData);
}

void VSTDevTools::makeArpeggio()
{
    using namespace midi;
    if (m_midiStream) {
        return;
    }

    m_midiStream = std::make_shared<midi::MidiStream>();

    Track t;
    t.num = 0;
    t.channels.push_back(0);
    m_midiStream->initData.tracks.push_back(t);

    midi::Event e(midi::Event::Opcode::ProgramChange);
    m_midiStream->initData.initEvents.push_back(e);

    auto makeChunk = [](Chunk& chunk, uint32_t tick, bool pitch) {
        /* notes of the arpeggio */
        static std::vector<int> notes = { 60, 64, 67, 72, 76, 79, 84, 79, 76, 72, 67, 64 };
        static uint32_t duration = 4440;
        uint16_t velocity = 65535;
        chunk.beginTick = tick;

        uint32_t note_duration = duration / notes.size();
        uint32_t note_time = tick + (tick > 0 ? note_duration : 0);

        for (int n : notes) {
            auto noteOn = midi::Event(midi::Event::Opcode::NoteOn);

            noteOn.setNote(n);
            noteOn.setVelocity(velocity);
            if (pitch) {
                noteOn.setPitchNote(n + 12, 1.f);                 //1 octave + 1 semitone
            }
            velocity -= 5461;
            chunk.events.insert({ note_time, noteOn });
            note_time += note_duration;

            //NoteOff should copy noteId and pitch from NoteOn
            auto noteOff = noteOn;
            noteOff.setOpcode(midi::Event::Opcode::NoteOff);
            chunk.events.insert({ note_time, noteOff });
        }
        chunk.endTick = note_time + note_duration;
    };

    Chunk chunk;
    makeChunk(chunk, 0, false);
    makeChunk(chunk, chunk.endTick, true);
    chunk.beginTick = 0;
    m_midiStream->lastTick = chunk.endTick;
    m_midiStream->initData.chunks.insert({ chunk.beginTick, std::move(chunk) });
}

void VSTDevTools::showEditor(int index)
{
    UriQuery uri("musescore://vst/editor");
    uri.addParam("instanceId", Val(index));
    interactive()->open(uri);
}

QQmlListProperty<VSTInstanceEditorModel> VSTDevTools::instances()
{
    return {
        this,
        this,
        &VSTDevTools::instancesCount,
        &VSTDevTools::instanceAt
    };
}

int VSTDevTools::instancesCount(QQmlListProperty<VSTInstanceEditorModel>* list)
{
    auto devTools = reinterpret_cast<VSTDevTools*>(list->data);
    auto instanceRegister = devTools->vstInstanceRegister();
    return instanceRegister->count();
}

VSTInstanceEditorModel* VSTDevTools::instanceAt(QQmlListProperty<VSTInstanceEditorModel>* list, int index)
{
    auto devTools = reinterpret_cast<VSTDevTools*>(list->data);
    auto instanceModel = new VSTInstanceEditorModel(devTools);
    instanceModel->setId(index);
    return instanceModel;
}
