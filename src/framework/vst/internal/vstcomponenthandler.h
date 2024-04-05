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
#ifndef MUSE_VST_VSTCOMPONENTHANDLER_H
#define MUSE_VST_VSTCOMPONENTHANDLER_H

#include "async/notification.h"
#include "async/channel.h"

#include "vsttypes.h"

namespace muse::vst {
class VstAdvancedHandler : public IAdvancedComponentHandler
{
    DECLARE_FUNKNOWN_METHODS
public:
    VstAdvancedHandler(async::Notification notifier);
    virtual ~VstAdvancedHandler() = default;

    Steinberg::tresult setDirty(Steinberg::TBool state) override;
    Steinberg::tresult requestOpenEditor(Steinberg::FIDString name) override;
    Steinberg::tresult startGroupEdit() override;
    Steinberg::tresult finishGroupEdit() override;
private:
    async::Notification m_paramsChanged;
};

class VstComponentHandler : public IComponentHandler
{
    DECLARE_FUNKNOWN_METHODS
public:
    VstComponentHandler();
    virtual ~VstComponentHandler() = default;

    async::Channel<PluginParamId, PluginParamValue> pluginParamChanged() const;
    async::Notification pluginParamsChanged() const;

private:
    Steinberg::tresult beginEdit(Steinberg::Vst::ParamID id) override;
    Steinberg::tresult performEdit(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue valueNormalized) override;
    Steinberg::tresult endEdit(Steinberg::Vst::ParamID id) override;
    Steinberg::tresult restartComponent(Steinberg::int32 flags) override;

    async::Channel<PluginParamId, PluginParamValue> m_paramChanged;
    async::Notification m_paramsChangedNotify;

    Steinberg::FUnknownPtr<VstAdvancedHandler> m_advancedHandler = nullptr;
};
}

#endif // MUSE_VST_VSTCOMPONENTHANDLER_H
