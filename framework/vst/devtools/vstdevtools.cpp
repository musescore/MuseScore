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

using namespace mu::vst;

VSTDevTools::VSTDevTools(QObject* parent)
    : QObject(parent), m_pluginsListModel(new PluginListModel(vstScanner(), this))
{
    qmlRegisterType<PluginListModel>("MuseScore.VST", 1, 0, "VSTPluginListModel");
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
    plugin.createInstance();
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

    if (!m_midiSource) {
        makeArpeggio();
        m_midiSource = std::make_shared<mu::audio::MidiSource>();
    }
    auto synth = VSTSynthesizer::create(instance);
    m_midiStream->initData.synthMap[0] = synth->name();
    m_midiSource->loadMIDI(m_midiStream);
    audioEngine()->play(m_midiSource);
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
    Event e;
    e.channel = 0;
    e.type = EventType::ME_PROGRAM;
    e.a = 0;
    m_midiStream->initData.initEvents.push_back(e);

    auto makeEvents = [](Events& events, uint32_t tick, int pitch) {
                          /* notes of the arpeggio */
                          static std::vector<int> notes = { 60, 64, 67, 72, 76, 79, 84, 79, 76, 72, 67, 64 };
                          static uint32_t duration = 4440;

                          uint32_t note_duration = duration / notes.size();
                          uint32_t note_time = tick;

                          for (int n : notes) {
                              events.insert({ note_time, Event(0, EventType::ME_NOTEON, n + pitch, 100) });
                              note_time += note_duration;
                              events.insert({ note_time, Event(0, EventType::ME_NOTEOFF, n + pitch, 100) });
                          }
                      };

    makeEvents(m_midiStream->initData.events, 0, 0);

    m_midiStream->request.onReceive(this, [this, makeEvents](uint32_t tick) {
        static int pitch = -11;
        ++pitch;
        if (pitch > 11) {
            pitch = -10;
        }

        if (tick > 20000) {
            m_midiStream->stream.close();
            return;
        }

        MidiData data;
        makeEvents(data.events, tick, pitch);

        m_midiStream->stream.send(data);
    });
}

void VSTDevTools::showEditor(int index)
{
    UriQuery uri("musescore://vst/editor");
    uri.addParam("instance_id", Val(index));
    interactive()->open(uri);
}

QQmlListProperty<PluginInstance> VSTDevTools::instances()
{
    return {
        this,
        vstInstanceRegister().get(),
        &VSTDevTools::instancesCount,
        &VSTDevTools::instanceAt
    };
}

int VSTDevTools::instancesCount(QQmlListProperty<PluginInstance>* list)
{
    auto instanceRegister = reinterpret_cast<IVSTInstanceRegister*>(list->data);
    return instanceRegister->count();
}

PluginInstance* VSTDevTools::instanceAt(QQmlListProperty<PluginInstance>* list, int index)
{
    auto instanceRegister = reinterpret_cast<IVSTInstanceRegister*>(list->data);
    return instanceRegister->instance(index).get();
}
