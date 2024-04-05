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
#include "ivstpluginsregister.h"

class QWidget;

namespace muse::vst {
class AbstractVstEditorView : public uicomponents::TopLevelDialog, public Steinberg::IPlugFrame, public async::Asyncable
{
    Q_OBJECT

    DECLARE_FUNKNOWN_METHODS

    Q_PROPERTY(int trackId READ trackId WRITE setTrackId NOTIFY trackIdChanged)
    Q_PROPERTY(QString resourceId READ resourceId WRITE setResourceId NOTIFY resourceIdChanged)

    INJECT(IVstPluginsRegister, pluginsRegister)

public:
    AbstractVstEditorView(QWidget* parent = nullptr);
    ~AbstractVstEditorView() override;

    Steinberg::tresult resizeView(Steinberg::IPlugView* view, Steinberg::ViewRect* newSize) override;

    int trackId() const;
    void setTrackId(int newTrackId);

    const QString& resourceId() const;
    void setResourceId(const QString& newResourceId);

signals:
    void trackIdChanged();
    void resourceIdChanged();

protected:
    virtual bool isAbleToWrapPlugin() const = 0;
    virtual VstPluginPtr getPluginPtr() const = 0;

    void wrapPluginView();

private:
    void attachView(VstPluginPtr pluginPtr);

    void updateViewGeometry();
    void moveViewToMainWindowCenter();

    void showEvent(QShowEvent* ev) override;
    void closeEvent(QCloseEvent* ev) override;
    bool event(QEvent* ev) override;

    void deinit();

    FIDString currentPlatformUiType() const;

    VstPluginPtr m_pluginPtr = nullptr;
    PluginViewPtr m_view = nullptr;

    muse::audio::TrackId m_trackId = -1;
    QString m_resourceId;
    muse::audio::AudioFxChainOrder m_chainOrder = -1;
};
}

#endif // MUSE_VST_ABSTRACTVSTEDITORVIEW_H
