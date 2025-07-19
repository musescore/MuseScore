/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MUSE_VST_VSTTYPES_H
#define MUSE_VST_VSTTYPES_H

#include <memory>
#include <unordered_map>
#include <vector>

#include "public.sdk/source/vst/hosting/module.h"
#include "public.sdk/source/vst/hosting/plugprovider.h"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "public.sdk/source/vst/hosting/eventlist.h"
#include "public.sdk/source/vst/hosting/processdata.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include "public.sdk/source/vst/vstpresetfile.h"
#include "public.sdk/source/common/memorystream.h"
#include "pluginterfaces/gui/iplugview.h"
#include "pluginterfaces/gui/iplugviewcontentscalesupport.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h" // IWYU pragma: export

#include "io/path.h"
#include "log.h"

namespace muse::vst {
class IVstPluginInstance;
using IVstPluginInstancePtr = std::shared_ptr<IVstPluginInstance>;
using VstPluginInstanceId = int;
using ClassInfo = VST3::Hosting::ClassInfo;

using PluginId = std::string;
using PluginModulePtr = VST3::Hosting::Module::Ptr;
using PluginModule = VST3::Hosting::Module;
using PluginModuleList = std::vector<PluginModulePtr>;
using PluginFactory = VST3::Hosting::PluginFactory;
using PluginSubcategories = ClassInfo::SubCategories;
using PluginContextFactory = Steinberg::Vst::PluginContextFactory;
using PluginContext = Steinberg::Vst::HostApplication;
using PluginControllerPtr = Steinberg::IPtr<Steinberg::Vst::IEditController>;
using PluginComponentPtr = Steinberg::IPtr<Steinberg::Vst::IComponent>;
using PluginViewPtr = Steinberg::IPtr<Steinberg::IPlugView>;
using PluginParamInfo = Steinberg::Vst::ParameterInfo;
using PluginParamId = Steinberg::Vst::ParamID;
using PluginParamValue = Steinberg::Vst::ParamValue;
using PluginPreset = Steinberg::Vst::PresetFile;
using ControlIdx = Steinberg::Vst::CtrlNumber;
using IAudioProcessorPtr = Steinberg::FUnknownPtr<Steinberg::Vst::IAudioProcessor>;
using IComponentHandler = Steinberg::Vst::IComponentHandler;
using IAdvancedComponentHandler = Steinberg::Vst::IComponentHandler2;
using IPluginContentScaleHandler = Steinberg::IPlugViewContentScaleSupport;
using FIDString = Steinberg::FIDString;
using BusInfo = Steinberg::Vst::BusInfo;
using BusDirection = Steinberg::Vst::BusDirections;
using BusType = Steinberg::Vst::BusTypes;
using BusMediaType = Steinberg::Vst::MediaTypes;
using PluginMidiMappingPtr = Steinberg::IPtr<Steinberg::Vst::IMidiMapping>;
using ParamsMapping = std::unordered_map<ControlIdx, PluginParamId>;

//@see https://developer.steinberg.help/pages/viewpage.action?pageId=9798275
static const std::string VST3_PACKAGE_EXTENSION = "vst3";
static const std::string VST3_PACKAGE_FILTER = "*." + VST3_PACKAGE_EXTENSION;

/// @see https://steinbergmedia.github.io/vst3_doc/vstinterfaces/namespaceSteinberg_1_1Vst_1_1PlugType.html
namespace PluginCategory {
static constexpr std::string_view Analyzer { "Analyzer" };
static constexpr std::string_view Delay { "Delay" };
static constexpr std::string_view Distortion { "Distortion" };
static constexpr std::string_view Dynamics { "Dynamics" };
static constexpr std::string_view Equalizer { "EQ" };
static constexpr std::string_view Filter { "Filter" };
static constexpr std::string_view Generator { "Generator" };
static constexpr std::string_view Mastering { "Mastering" };
static constexpr std::string_view Modulation { "Modulation" };
static constexpr std::string_view PitchShift { "Pitch Shift" };
static constexpr std::string_view Restoration { "Restoration" };
static constexpr std::string_view Reverb { "Reverb" };
static constexpr std::string_view Surround { "Surround" };
static constexpr std::string_view Tools { "Tools" };
}

namespace PluginSubCategory {
static constexpr std::string_view Synth { "Synth" };
static constexpr std::string_view Piano { "Piano" };
static constexpr std::string_view Drum { "Drum" };
static constexpr std::string_view External { "External" };
}

using VstEventList = Steinberg::Vst::EventList;
using VstEvent = Steinberg::Vst::Event;
using VstParameterChanges = Steinberg::Vst::ParameterChanges;
using VstProcessData = Steinberg::Vst::HostProcessData;
using VstProcessContext = Steinberg::Vst::ProcessContext;
using VstProcessSetup = Steinberg::Vst::ProcessSetup;
using VstMemoryStream = Steinberg::MemoryStream;
using VstBufferStream = Steinberg::Vst::BufferStream;

namespace PluginEditorViewType = Steinberg::Vst::ViewType;

inline PluginModulePtr createModule(const io::path_t& path)
{
    std::string errorString;
    PluginModulePtr result = nullptr;

    try {
        result = PluginModule::create(path.toStdString(), errorString);
    }  catch (...) {
        LOGE() << "Unable to load a new VST Module, error string: " << errorString;
    }

    return result;
}

struct ParamChangeEvent {
    PluginParamId paramId;
    PluginParamValue value = 0.;
};
}

template<>
struct std::less<muse::vst::ParamChangeEvent>
{
    bool operator()(const muse::vst::ParamChangeEvent& first,
                    const muse::vst::ParamChangeEvent& second) const
    {
        return first.paramId < second.paramId
               && first.value < second.value;
    }
};

template<>
struct std::less<Steinberg::Vst::NoteOnEvent>
{
    bool operator()(const Steinberg::Vst::NoteOnEvent& first,
                    const Steinberg::Vst::NoteOnEvent& second) const
    {
        return first.channel < second.channel
               || first.pitch < second.pitch
               || first.tuning < second.tuning
               || first.velocity < second.velocity
               || first.length < second.length
               || first.noteId < second.noteId;
    }
};

template<>
struct std::less<Steinberg::Vst::NoteOffEvent>
{
    bool operator()(const Steinberg::Vst::NoteOffEvent& first,
                    const Steinberg::Vst::NoteOffEvent& second) const
    {
        return first.channel < second.channel
               || first.pitch < second.pitch
               || first.tuning < second.tuning
               || first.velocity < second.velocity
               || first.noteId < second.noteId;
    }
};

template<>
struct std::less<muse::vst::VstEvent>
{
    bool operator()(const muse::vst::VstEvent& first,
                    const muse::vst::VstEvent& second) const
    {
        if (first.type < second.type || first.busIndex < second.busIndex) {
            return true;
        }

        if (first.type == Steinberg::Vst::Event::kNoteOnEvent && first.type == second.type) {
            return std::less<Steinberg::Vst::NoteOnEvent> {}(first.noteOn, second.noteOn);
        }

        if (first.type == Steinberg::Vst::Event::kNoteOffEvent && first.type == second.type) {
            return std::less<Steinberg::Vst::NoteOffEvent> {}(first.noteOff, second.noteOff);
        }

        return false;
    }
};

#endif // MUSE_VST_VSTTYPES_H
