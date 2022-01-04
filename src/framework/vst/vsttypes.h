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
#ifndef MU_VST_VSTTYPES_H
#define MU_VST_VSTTYPES_H

#include <memory>
#include <vector>

#include "public.sdk/source/vst/hosting/module.h"
#include "public.sdk/source/vst/hosting/plugprovider.h"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "public.sdk/source/vst/hosting/eventlist.h"
#include "public.sdk/source/vst/hosting/processdata.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include "public.sdk/source/common/memorystream.h"
#include "pluginterfaces/gui/iplugview.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"

#include "framework/midi/miditypes.h"

namespace mu::vst {
class VstPlugin;
using VstPluginPtr = std::shared_ptr<VstPlugin>;
using ClassInfo = VST3::Hosting::ClassInfo;

using PluginId = std::string;
using PluginModulePtr = VST3::Hosting::Module::Ptr;
using PluginModule = VST3::Hosting::Module;
using PluginModuleList = std::vector<PluginModulePtr>;
using PluginFactory = VST3::Hosting::PluginFactory;
using PluginSubcategories = ClassInfo::SubCategories;
using PluginContextFactory = Steinberg::Vst::PluginContextFactory;
using PluginContext = Steinberg::Vst::HostApplication;
using PluginProviderPtr = Steinberg::IPtr<Steinberg::Vst::PlugProvider>;
using PluginProvider = Steinberg::Vst::PlugProvider;
using PluginControllerPtr = Steinberg::IPtr<Steinberg::Vst::IEditController>;
using PluginComponentPtr = Steinberg::IPtr<Steinberg::Vst::IComponent>;
using PluginViewPtr = Steinberg::IPtr<Steinberg::IPlugView>;
using PluginParamInfo = Steinberg::Vst::ParameterInfo;
using PluginParamId = Steinberg::Vst::ParamID;
using PluginParamValue = Steinberg::Vst::ParamValue;
using IAudioProcessorPtr = Steinberg::FUnknownPtr<Steinberg::Vst::IAudioProcessor>;
using IComponentHandler = Steinberg::Vst::IComponentHandler;
using IAdvancedComponentHandler = Steinberg::Vst::IComponentHandler2;
using FIDString = Steinberg::FIDString;

enum class VstPluginType {
    Undefined,
    Instrument,
    Fx
};

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

using VstEventList = Steinberg::Vst::EventList;
using VstEvent = Steinberg::Vst::Event;
using VstProcessData = Steinberg::Vst::HostProcessData;
using VstProcessContext = Steinberg::Vst::ProcessContext;
using VstProcessSetup = Steinberg::Vst::ProcessSetup;
using VstMemoryStream = Steinberg::MemoryStream;

namespace PluginEditorViewType = Steinberg::Vst::ViewType;
}
#endif // MU_VST_VSTTYPES_H
