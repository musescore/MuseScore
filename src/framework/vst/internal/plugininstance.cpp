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

#include "plugininstance.h"

#include <algorithm>

#include "pluginterfaces/gui/iplugview.h"
#include "vstsynthesizer.h"
#include "plugin.h"
#include "log.h"

using namespace mu;
using namespace mu::vst;
using namespace Steinberg;
using namespace Steinberg::Vst;

PluginInstance::PluginInstance(const Plugin* plugin)
    : m_plugin(*plugin),
    m_parameters(), m_events(), m_host(),
    m_effectClass(plugin->m_effectClass),
    m_factory(plugin->m_factory)
{
    init();
    connect();
    initAudioProcessor();
    m_valid = m_component && m_controller && m_audioProcessor;
    initParameters();
}

std::shared_ptr<PluginInstance> PluginInstance::create(const Plugin* plugin)
{
    auto instance = std::make_shared<PluginInstance>(plugin);
    instance->m_id = vstInstanceRegister()->addInstance(instance);
    VSTSynthesizer::create(instance);
    return instance;
}

const Plugin& PluginInstance::plugin() const
{
    return m_plugin;
}

instanceId PluginInstance::id() const
{
    return m_id;
}

bool PluginInstance::isValid() const
{
    return m_valid;
}

PluginInstance::~PluginInstance()
{
    if (m_controller) {
        m_controller->terminate();
    }

    if (m_component) {
        m_component->terminate();
    }
}

IPlugView* PluginInstance::createView()
{
    IF_ASSERT_FAILED(isValid()) {
        LOGE() << "plugin instance is not valid";
        return nullptr;
    }
    auto view = m_controller->createView(ViewType::kEditor);
    if (!view) {
        LOGE() << "plugin hasn't view";
        return nullptr;
    }
    return view;
}

bool PluginInstance::setActive(bool active)
{
    IF_ASSERT_FAILED(m_component->setActive(active) == kResultOk) {
        LOGE() << "can't (de)active plugin";
    }
    m_audioProcessor->setProcessing(m_active);
    m_active = active;
    return isActive();
}

bool PluginInstance::isActive() const
{
    //m_component has not method getActive
    return m_active;
}

void PluginInstance::addMidiEvent(const mu::midi::Event& e)
{
    m_events.addMidiEvent(e);
}

Ret PluginInstance::setSampleRate(int sampleRate)
{
    ProcessSetup setup;
    setup.processMode = kRealtime;
    setup.symbolicSampleSize = kSample32;
    setup.maxSamplesPerBlock = MAX_SAMPLES_PERBLOCK;
    setup.sampleRate = sampleRate;
    if (m_audioProcessor->setupProcessing(setup) != kResultOk) {
        return Ret(Ret::Code::Ok);
    }
    return Ret(Ret::Code::NotSupported);
}

void PluginInstance::process(float* input, float* output, unsigned int samples)
{
    if (!isActive()) {
        return;
    }
    AudioBusBuffers out[m_busInfo.audioOutput.size()], in[m_busInfo.audioInput.size()];
    auto initBuffers = [&samples](AudioBusBuffers* buffers, std::vector<unsigned int>& busInfo, float* stream) {
        for (unsigned int i = 0; i < busInfo.size(); ++i) {
            auto channels = busInfo[i];
            buffers[i].numChannels = channels;
            buffers[i].silenceFlags = 0;
            buffers[i].channelBuffers32 = new Sample32*[channels];
            for (unsigned int j = 0; j < channels; ++j) {
                buffers[i].channelBuffers32[j] = new Sample32[samples];
                if (stream) {
                    //TODO: fill buffer from stream if given (for audio plugins)
                }
            }
        }
    };
    initBuffers(in, m_busInfo.audioInput, input);
    initBuffers(out, m_busInfo.audioOutput, nullptr);

    ProcessData data;
    data.numOutputs = m_busInfo.audioOutput.size();
    data.numInputs = m_busInfo.audioInput.size();
    data.outputs = out;
    data.inputs = in;
    data.processMode = kRealtime;
    data.symbolicSampleSize = kSample32;
    data.numSamples = samples;
    //data.processContext
    data.inputEvents = &m_events;
    //data.outputEvents

    if (m_audioProcessor->process(data) != kResultOk) {
        LOGE() << "VST plugin processing error";
    }
    m_events.clear();

    if (output && m_busInfo.audioOutput.size()) {
        for (unsigned int i = 0; i < samples; ++i) {
            for (unsigned int s = 0; s < audio::synth::AUDIO_CHANNELS; ++s) {
                auto getFromChannel = std::min<unsigned int>(s, out[0].numChannels - 1);
                output[i * audio::synth::AUDIO_CHANNELS + s] = out[0].channelBuffers32[getFromChannel][i];
            }
        }
    }
}

void PluginInstance::flush()
{
    if (!isActive()) {
        return;
    }
    m_events.clear();
    ProcessData data;
    data.numOutputs = 0;
    data.numInputs = 0;
    data.outputs = nullptr;
    data.inputs = nullptr;
    data.processMode = kRealtime;
    data.symbolicSampleSize = kSample32;
    data.numSamples = 0;

    if (m_audioProcessor->process(data) != kResultOk) {
        LOGE() << "VST plugin flush error";
    }
}

std::vector<PluginParameter> PluginInstance::getParameters() const
{
    return m_parameters;
}

void PluginInstance::init()
{
    tresult resultCode;

    //create instance of processing class
    Steinberg::Vst::IComponent* component;
    resultCode = m_factory->createInstance(
        m_effectClass.cid,
        INLINE_UID_OF(Steinberg::Vst::IComponent),
        reinterpret_cast<void**>(&component)
        );
    m_component = owned(component);
    if (resultCode != Steinberg::kResultTrue) {
        return;
    }

    //init processing interface
    resultCode = m_component->initialize(static_cast<Steinberg::Vst::IHostApplication*>(&m_host));
    if (resultCode != Steinberg::kResultTrue) {
        return;
    }

    //try to recieve controller from component
    resultCode = m_component->queryInterface(INLINE_UID_OF(Steinberg::Vst::IEditController), (void**)&m_controller);
    if (resultCode != Steinberg::kResultTrue) {
        //else create controller by CID

        // ask for the associated controller class ID
        TUID controllerCID { 0 };
        resultCode = m_component->getControllerClassId(controllerCID);
        if (resultCode != Steinberg::kResultTrue) {
            return;
        }

        // create its controller part from the factory
        auto const classID = FUID::fromTUID(controllerCID);
        Steinberg::Vst::IEditController* controller;
        resultCode
            = m_factory->createInstance(classID /*uid.data()*/, INLINE_UID_OF(IEditController), reinterpret_cast<void**>(&controller));
        m_controller = Steinberg::owned(controller);

        if ((resultCode != Steinberg::kResultTrue) | !m_controller) {
            return;
        }

        if (m_controller->initialize(static_cast<Steinberg::Vst::IHostApplication*>(&m_host)) != kResultTrue) {
            LOGE() << "can't create controller for the plugin";
            return;
        }

        resultCode = m_controller->queryInterface(INLINE_UID_OF(Steinberg::Vst::INoteExpressionController), (void**)&m_noteexpression);
        if (resultCode != Steinberg::kResultTrue) {
            m_noteexpression = nullptr;
            LOGI() << plugin().getName() << " hasn't note expression interface";
        }
    }
}

void PluginInstance::connect()
{
    if (!m_component || !m_controller) {
        return;
    }

    FUnknownPtr<IConnectionPoint> componentConnectionPoint(m_component);
    FUnknownPtr<IConnectionPoint> controllerConnectionPoint(m_controller);

    m_componentCP = std::unique_ptr<ConnectionProxy>(new ConnectionProxy(componentConnectionPoint));
    m_controllerCP = std::unique_ptr<ConnectionProxy>(new ConnectionProxy(controllerConnectionPoint));

    //component to controller
    if (m_componentCP->connect(controllerConnectionPoint) != kResultTrue) {
        LOGE() << "can't create connection";
        return;
    }

    //controller to component
    if (m_controllerCP->connect(componentConnectionPoint) != kResultTrue) {
        LOGE() << "can't create connection";
        return;
    }
}

void PluginInstance::initParameters()
{
    if (!isValid()) {
        return;
    }

    auto count = m_controller->getParameterCount();
    m_parameters.resize(count);

    for (decltype(count) i = 0; i < count; ++i) {
        ParameterInfo info;
        m_controller->getParameterInfo(i, info);
        m_parameters[i] = info;
    }
}

DEF_CLASS_IID(IAudioProcessor)
void PluginInstance::initAudioProcessor()
{
    if (!m_component) {
        return;
    }

    //init audio processor interface
    IAudioProcessor* audioProcessor;
    IF_ASSERT_FAILED(m_component->queryInterface(IAudioProcessor::iid, reinterpret_cast<void**>(&audioProcessor)) == kResultOk) {
        LOGE() << "can't get Audio Processor interface from component";
        return;
    }
    m_audioProcessor = audioProcessor;

    initBuses(m_busInfo.audioInput, kAudio, kInput);
    initBuses(m_busInfo.audioOutput, kAudio, kOutput);
    initBuses(m_busInfo.eventInput, kEvent, kInput);
    initBuses(m_busInfo.eventOutput, kEvent, kOutput);
}

void PluginInstance::initBuses(std::vector<unsigned int>& target, MediaType type, BusDirection direction)
{
    target.resize(m_component->getBusCount(type, direction), 0);

    for (auto& bus : target) {
        //each bit in SpeakerArrangement means one channel
        SpeakerArrangement arr(0);
        if (m_audioProcessor->getBusArrangement(direction, 0, arr) == kResultTrue) {
            do {
                if (arr & 0x01) {
                    ++bus;
                }
            } while (arr >>= 1);
        } else {
            bus = 0;
        }
    }
}

std::vector<PluginParameter> PluginInstance::getEditableParameters() const
{
    std::vector<PluginParameter> params;
    for (auto p : m_parameters) {
        if (p.isEditable()) {
            params.push_back(p);
        }
    }
    return params;
}

std::vector<PluginParameter> PluginInstance::getVisibleParameters() const
{
    std::vector<PluginParameter> params;
    for (auto p : m_parameters) {
        if (p.isVisible()) {
            params.push_back(p);
        }
    }
    return params;
}

double PluginInstance::getParameterValue(const uint id) const
{
    IF_ASSERT_FAILED(id < m_parameters.size()) {
        LOGE() << "paremeter is not exists";
        return 0;
    }
    return m_controller->getParamNormalized(id);
}

double PluginInstance::getParameterValue(const PluginParameter& parameter)
{
    return getParameterValue(parameter.id());
}

void PluginInstance::setParameterValue(const PluginParameter& parameter, double value)
{
    setParameterValue(parameter.id(), value);
}

void PluginInstance::setParameterValue(const uint id, double value)
{
    IF_ASSERT_FAILED(id < m_parameters.size()) {
        LOGE() << "paremeter is not exists";
        return;
    }
    IF_ASSERT_FAILED(m_controller->setParamNormalized(id, value) == kResultOk) {
        LOGE() << "can't apply parameter's value";
    }
}

unsigned int PluginInstance::getLatency() const
{
    IF_ASSERT_FAILED(isValid()) {
        return 0;
    }
    return m_audioProcessor->getLatencySamples();
}
