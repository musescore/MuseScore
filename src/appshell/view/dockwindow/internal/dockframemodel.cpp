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

#include <QQuickItem>
#include <QApplication>

#include "thirdparty/KDDockWidgets/src/private/Frame_p.h"

#include "../docktypes.h"
#include "../dockpanel.h"

#include "log.h"

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

    auto numDocksChangedCon = connect(frame, &KDDockWidgets::Frame::numDockWidgetsChanged, [this, frame]() {
        auto currentDock = frame->currentDockWidget();
        auto allDocks = frame->dockWidgets();

        if (!allDocks.contains(currentDock)) {
            frame->setCurrentTabIndex(0);
        }

        if (allDocks.size() != 1) {
            setTitleBarVisible(false);
            return;
        }

        DockProperties properties = readPropertiesFromObject(allDocks.first());
        bool visible = (properties.type == DockType::Panel && properties.allowedAreas != Qt::NoDockWidgetArea);
        setTitleBarVisible(visible);

        updateNavigationSection();
    });

    auto currentDockWidgetChangedCon = connect(frame, &KDDockWidgets::Frame::currentDockWidgetChanged, [this]() {
        updateNavigationSection();

        emit currentDockUniqueNameChanged();
    });

    connect(qApp, &QApplication::aboutToQuit, [numDocksChangedCon, currentDockWidgetChangedCon]() {
        disconnect(numDocksChangedCon);
        disconnect(currentDockWidgetChangedCon);
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

QObject* DockFrameModel::currentNavigationSection() const
{
    auto frame = dynamic_cast<KDDockWidgets::Frame*>(m_frame);
    if (!frame) {
        return nullptr;
    }

    KDDockWidgets::DockWidgetBase* w = frame->currentDockWidget();
    if (!w) {
        return nullptr;
    }

    DockPanel* dockPanel = w->property("dockPanel").value<DockPanel*>();
    if (!dockPanel) {
        return nullptr;
    }

    return dockPanel->navigationSection();
}

void DockFrameModel::updateNavigationSection()
{
    QObject* n = currentNavigationSection();
    if (m_navigationSection != n) {
        m_navigationSection = n;
        emit navigationSectionChanged();
    }
}

QObject* DockFrameModel::navigationSection() const
{
    return m_navigationSection;
}

QString DockFrameModel::currentDockUniqueName() const
{
    auto frame = dynamic_cast<KDDockWidgets::Frame*>(m_frame);
    if (frame && frame->currentDockWidget()) {
        return frame->currentDockWidget()->uniqueName();
    }

    return QString();
}
