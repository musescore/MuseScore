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

#include "dockframemodel.h"

#include "../docktypes.h"

#include "thirdparty/KDDockWidgets/src/private/Frame_p.h"

#include <QQuickItem>

using namespace mu::dock;

DockFrameModel::DockFrameModel(QObject* parent)
    : QObject(parent)
{
}

QQuickItem* DockFrameModel::frame() const
{
    return m_frame;
}

bool DockFrameModel::titleBarVisible() const
{
    return m_titleBarVisible;
}

void DockFrameModel::setFrame(QQuickItem* frame)
{
    if (frame == m_frame) {
        return;
    }

    m_frame = frame;
    emit frameChanged(frame);

    listenChangesInFrame();
}

void DockFrameModel::listenChangesInFrame()
{
    auto frame = dynamic_cast<KDDockWidgets::Frame*>(m_frame);

    if (!frame) {
        return;
    }

    connect(frame, &KDDockWidgets::Frame::numDockWidgetsChanged, [this, frame]() {
        auto docks = frame->dockWidgets();

        if (docks.size() != 1) {
            setTitleBarVisible(false);
            return;
        }

        DockProperties properties = readPropertiesFromObject(docks.first());
        bool visible = (properties.type == DockType::Panel && properties.allowedAreas != Qt::NoDockWidgetArea);
        setTitleBarVisible(visible);
    });
}

void DockFrameModel::setTitleBarVisible(bool visible)
{
    if (visible == m_titleBarVisible) {
        return;
    }

    m_titleBarVisible = visible;
    emit titleBarVisibleChanged(visible);
}
