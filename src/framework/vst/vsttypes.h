//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_VST_VSTTYPES_H
#define MU_VST_VSTTYPES_H

#include <memory>
#include "public.sdk/source/vst/hosting/module.h"
#include "public.sdk/source/vst/hosting/plugprovider.h"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "pluginterfaces/gui/iplugview.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"

namespace mu::vst {
class VstPlugin;
using VstPluginPtr = std::shared_ptr<VstPlugin>;
using ClassInfo = VST3::Hosting::ClassInfo;

using PluginId = std::string;
using PluginModulePtr = VST3::Hosting::Module::Ptr;
using PluginModule = VST3::Hosting::Module;
using PluginFactory = VST3::Hosting::PluginFactory;
using PluginContextFactory = Steinberg::Vst::PluginContextFactory;
using PluginContext = Steinberg::Vst::HostApplication;
using PluginProviderPtr = Steinberg::IPtr<Steinberg::Vst::PlugProvider>;
using PluginProvider = Steinberg::Vst::PlugProvider;
using PluginController = Steinberg::IPtr<Steinberg::Vst::IEditController>;
using PluginView = Steinberg::IPtr<Steinberg::IPlugView>;
using FIDString = Steinberg::FIDString;
namespace PluginEditorViewType = Steinberg::Vst::ViewType;

struct VstPluginMeta {
    PluginId id;
    std::string name;
    std::string path;
};

using VstPluginMetaList = std::vector<VstPluginMeta>;
}
#endif // MU_VST_VSTTYPES_H
