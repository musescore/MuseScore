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
class VstView : public QQuickItem, public Steinberg::IPlugFrame
{
    Q_OBJECT
    Q_PROPERTY(int instanceId READ instanceId WRITE setInstanceId NOTIFY instanceIdChanged FINAL)

    Q_PROPERTY(QString resourceId READ resourceId WRITE setResourceId NOTIFY resourceIdChanged)
    Q_PROPERTY(int trackId READ trackId WRITE setTrackId NOTIFY trackIdChanged)
    Q_PROPERTY(int chainOrder READ chainOrder WRITE setChainOrder NOTIFY chainOrderChanged)

    muse::Inject<IVstInstancesRegister> instancesRegister;

    DECLARE_FUNKNOWN_METHODS

public:
    VstView(QQuickItem* parent = nullptr);

    int instanceId() const;
    void setInstanceId(int newInstanceId);

    QString resourceId() const;
    void setResourceId(const QString &newResourceId);

    int trackId() const;
    void setTrackId(int newTrackId);

    int chainOrder() const;
    void setChainOrder(int newChainOrder);

    Q_INVOKABLE void init();

    // IPlugFrame
    Steinberg::tresult resizeView(Steinberg::IPlugView* view, Steinberg::ViewRect* newSize) override;
    // ----------

signals:
    void instanceIdChanged();

    void resourceIdChanged();

    void trackIdChanged();

    void chainOrderChanged();

private:

    void updateViewGeometry();

    int m_instanceId = -1;
    QString m_resourceId;
    int m_trackId = -1;
    int m_chainOrder = 0;

    IVstPluginInstancePtr m_instance;
    PluginViewPtr m_view;
};
}

