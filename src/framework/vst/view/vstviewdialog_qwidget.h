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

#pragma once

#include "uicomponents/view/topleveldialog.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "../ivstinstancesregister.h"

class QWidget;

namespace muse::vst {
class VstViewDialog : public uicomponents::TopLevelDialog, public Steinberg::IPlugFrame, public async::Asyncable
{
    Q_OBJECT

    DECLARE_FUNKNOWN_METHODS

    Q_PROPERTY(int instanceId READ instanceId WRITE setInstanceId NOTIFY instanceIdChanged)

    muse::Inject<IVstInstancesRegister> instancesRegister;

public:
    VstViewDialog(QWidget* parent = nullptr);
    ~VstViewDialog() override;

    Steinberg::tresult resizeView(Steinberg::IPlugView* view, Steinberg::ViewRect* newSize) override;

    int instanceId() const;
    void setInstanceId(int newInstanceId);

signals:
    void instanceIdChanged();

protected:
    void wrapPluginView();

private:
    void attachView(IVstPluginInstancePtr instance);

    void updateViewGeometry();
    void moveViewToMainWindowCenter();

    void showEvent(QShowEvent* ev) override;
    void closeEvent(QCloseEvent* ev) override;
    bool event(QEvent* ev) override;

    void deinit();

    FIDString currentPlatformUiType() const;

    VstPluginInstanceId m_instanceId = -1;
    IVstPluginInstancePtr m_instance = nullptr;
    PluginViewPtr m_view = nullptr;
};
}
