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

#ifndef MU_VST_VSTIEDITORVIEW_H
#define MU_VST_VSTIEDITORVIEW_H

#include <QDialog>
#include <QWidget>

#include "async/asyncable.h"
#include "modularity/ioc.h"

#include "ivstpluginsregister.h"

namespace mu::vst {
class VstiEditorView : public QDialog, public Steinberg::IPlugFrame, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(int trackId READ trackId WRITE setTrackId NOTIFY trackIdChanged)
    Q_PROPERTY(QString resourceId READ resourceId WRITE setResourceId NOTIFY resourceIdChanged)

    INJECT(vst, IVstPluginsRegister, pluginsRegister)

    DECLARE_FUNKNOWN_METHODS
public:
    VstiEditorView(QWidget* parent = nullptr);
    VstiEditorView(const VstiEditorView& copy);
    ~VstiEditorView();

    Steinberg::tresult resizeView(Steinberg::IPlugView* view, Steinberg::ViewRect* newSize) override;

    int trackId() const;
    void setTrackId(int newTrackId);

    const QString& resourceId() const;
    void setResourceId(const QString& newResourceId);

signals:
    void trackIdChanged();

    void resourceIdChanged();

private:
    void wrapPluginView();
    void attachView(VstPluginPtr pluginPtr);

    FIDString currentPlatformUiType() const;

    VstPluginPtr m_pluginPtr = nullptr;
    PluginViewPtr m_view = nullptr;

    audio::TrackId m_trackId = -1;
    QString m_resourceId;
};
}

Q_DECLARE_METATYPE(mu::vst::VstiEditorView)

#endif // VSTIEDITORVIEW_H
