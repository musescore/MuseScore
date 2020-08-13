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
#ifndef MU_VST_PLUGININSTANCE_H
#define MU_VST_PLUGININSTANCE_H

#include <memory>
#include <vector>

#include "internal/hostapplication.h"
#include "internal/connectionproxy.h"
#include "pluginparameter.h"

#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"

namespace mu {
namespace vst {
class Plugin;

class PluginInstance
{
public:
    PluginInstance(const Plugin* plugin);
    ~PluginInstance();

    //! return true if plugin was instantiated successfully
    bool isValid() const { return m_valid; }

    //! create view of plugin's editor
    bool createView();

    //! enable/disable plugin's processing. Return state after applying
    bool setActive(bool active);

    //! true if processing is active
    bool isActive() const;

    //! process audio/midi data
    void process(/*commands, audio buffers etc*/);

    //! returns all parameters of the plugin
    std::vector<PluginParameter> getParameters() const { return m_parameters; }

    //! return ediable parameters of the plugin. Use this for saving and loading parameters from a project
    std::vector<PluginParameter> getEditableParameters() const;

    //! return visible parameters of the plugin. Use this for an UI (if plugin does not has ui)
    std::vector<PluginParameter> getVisibleParameters() const;

    //! return value from 0.0 to 1.0
    double getParameterValue(const uint id) const;

    //! return value from 0.0 to 1.0
    double getParameterValue(const PluginParameter& parameter);

    //! set value for parameter
    void setParameterValue(const PluginParameter& parameter, double value);

    //! set value for parameter
    void setParameterValue(const uint id, double value);

    //! latency of the plugin in samples, should be used in latency compensation of a mixer
    unsigned int getLatency() const;

private:

    //! initialization of interfaces
    void init();

    //! make connections between component and controller interfaces
    void connect();

    //! recieve all parameters from Plugin and store in m_parameters
    void initParameters();

    //! init pointer to the audio processing
    void initAudioProcessor();

    //! recieve information about plugin's busses
    void initBuses(std::vector<unsigned int>& target, Steinberg::Vst::MediaType type, Steinberg::Vst::BusDirection direction);

    //! the flag that initialization and connection was made successfully
    bool m_valid;

    //! current plugin state
    bool m_active;

    //! plugins parameters
    std::vector<PluginParameter> m_parameters;

    struct
    {
        std::vector<unsigned int> audioInput;
        std::vector<unsigned int> audioOutput;
        std::vector<unsigned int> eventInput;
        std::vector<unsigned int> eventOutput;
    } m_busInfo;

    //! instanse of IHostApplication for communication with plugin
    HostApplication m_host;

    //! information about AudioEffectClass of plugin
    Steinberg::PClassInfo2 m_effectClass;

    //! pointer to the factory function of plugin
    Steinberg::IPluginFactory3* m_factory;

    //! interface for processing
    Steinberg::IPtr<Steinberg::Vst::IComponent> m_component;

    //! interface for controller and plugin's UI
    Steinberg::IPtr<Steinberg::Vst::IEditController> m_controller;

    //! connection proxies between component and controller
    std::unique_ptr<ConnectionProxy> m_componentCP, m_controllerCP;

    //! Processing interface
    Steinberg::IPtr<Steinberg::Vst::IAudioProcessor> m_audioProcessor;
};
}
}
#endif // MU_VST_PLUGININSTANCE_H
