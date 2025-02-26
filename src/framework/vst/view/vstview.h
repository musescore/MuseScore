/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include <QQuickItem>

#include "global/modularity/ioc.h"
#include "../ivstinstancesregister.h"

namespace muse::vst {
class RunLoop;
class VstView : public QQuickItem, public Steinberg::IPlugFrame
{
    Q_OBJECT
    Q_PROPERTY(int instanceId READ instanceId WRITE setInstanceId NOTIFY instanceIdChanged FINAL)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged FINAL)

    muse::Inject<IVstInstancesRegister> instancesRegister;

    DECLARE_FUNKNOWN_METHODS

public:
    VstView(QQuickItem* parent = nullptr);
    ~VstView();

    int instanceId() const;
    void setInstanceId(int newInstanceId);

    Q_INVOKABLE void init();
    Q_INVOKABLE void deinit();

    // IPlugFrame
    Steinberg::tresult resizeView(Steinberg::IPlugView* view, Steinberg::ViewRect* requiredSize) override;
    // ----------

    QString title() const;

signals:
    void instanceIdChanged();
    void titleChanged();

private:

    struct ScreenMetrics {
        QSize availableSize;
        double devicePixelRatio = 0.0;
    };

    void updateScreenMetrics();
    void updateViewGeometry();

    int m_instanceId = -1;
    IVstPluginInstancePtr m_instance;
    QWindow* m_window = nullptr;
    ScreenMetrics m_screenMetrics;
    PluginViewPtr m_view;
    QString m_title;
    RunLoop* m_runLoop = nullptr;
};
}
