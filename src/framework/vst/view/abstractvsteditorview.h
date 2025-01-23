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

#ifndef MUSE_VST_ABSTRACTVSTEDITORVIEW_H
#define MUSE_VST_ABSTRACTVSTEDITORVIEW_H

#include "uicomponents/view/topleveldialog.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "../ivstinstancesregister.h"

class QWidget;

namespace muse::vst {
class AbstractVstEditorView : public uicomponents::TopLevelDialog, public Steinberg::IPlugFrame, public async::Asyncable
{
    Q_OBJECT

    DECLARE_FUNKNOWN_METHODS

    //! NOTE To identify the plugin instance that needs to be shown,
    //! we can explicitly specify the instance ID
    //! or the context (resourceId, trackId...) by which the instance can be determined.
    Q_PROPERTY(int instanceId READ instanceId WRITE setInstanceId NOTIFY instanceIdChanged)

    Q_PROPERTY(int trackId READ trackId WRITE setTrackId NOTIFY trackIdChanged)
    Q_PROPERTY(QString resourceId READ resourceId WRITE setResourceId NOTIFY resourceIdChanged)

    muse::Inject<IVstInstancesRegister> instancesRegister;

public:
    AbstractVstEditorView(QWidget* parent = nullptr);
    ~AbstractVstEditorView() override;

    Steinberg::tresult resizeView(Steinberg::IPlugView* view, Steinberg::ViewRect* newSize) override;

    int trackId() const;
    void setTrackId(int newTrackId);

    const QString& resourceId() const;
    void setResourceId(const QString& newResourceId);

    int instanceId() const;
    void setInstanceId(int newInstanceId);

signals:
    void trackIdChanged();
    void resourceIdChanged();

    void instanceIdChanged();

protected:
    virtual bool isAbleToWrapPlugin() const = 0;

    IVstPluginInstancePtr getInstance() const;
    virtual IVstPluginInstancePtr determineInstance() const = 0;

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

    IVstPluginInstancePtr m_pluginPtr = nullptr;
    PluginViewPtr m_view = nullptr;

    muse::audio::TrackId m_trackId = -1;
    QString m_resourceId;
    muse::audio::AudioFxChainOrder m_chainOrder = -1;
    VstPluginInstanceId m_instanceId = -1;
};
}

#endif // MUSE_VST_ABSTRACTVSTEDITORVIEW_H
