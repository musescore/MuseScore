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
using IAudioProcessorPtr = Steinberg::FUnknownPtr<Steinberg::Vst::IAudioProcessor>;
using FIDString = Steinberg::FIDString;

enum class VstPluginType {
    Undefined,
    Instrument,
    Fx
};

/// @see https://steinbergmedia.github.io/vst3_doc/vstinterfaces/namespaceSteinberg_1_1Vst_1_1PlugType.html
namespace PluginCategory {
static const std::string Analyzer = "Analyzer";
static const std::string Delay = "Delay";
static const std::string Distortion = "Distortion";
static const std::string Dynamics = "Dynamics";
static const std::string Equalizer = "EQ";
static const std::string Filter = "Filter";
static const std::string Generator = "Generator";
static const std::string Mastering = "Mastering";
static const std::string Modulation = "Modulation";
static const std::string PitchShift = "Pitch Shift";
static const std::string Restoration = "Restoration";
static const std::string Reverb = "Reverb";
static const std::string Surround = "Surround";
static const std::string Tools = "Tools";
}

using VstEventList = Steinberg::Vst::EventList;
using VstEvent = Steinberg::Vst::Event;
using VstProcessData = Steinberg::Vst::HostProcessData;
using VstProcessContext = Steinberg::Vst::ProcessContext;
using VstProcessSetup = Steinberg::Vst::ProcessSetup;

namespace PluginEditorViewType = Steinberg::Vst::ViewType;
}
#endif // MU_VST_VSTTYPES_H
