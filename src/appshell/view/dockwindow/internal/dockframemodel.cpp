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
using namespace mu::actions;

static QVariantMap findMenuItem(const QVariantList& menuItems, const QString& itemId)
{
    for (const QVariant& obj : menuItems) {
        QVariantMap item = obj.toMap();

        if (item["id"].toString() == itemId) {
            return item;
        }

        item = findMenuItem(item["subitems"].toList(), itemId);

        if (!item.isEmpty()) {
            return item;
        }
    }

    return QVariantMap();
}

DockFrameModel::DockFrameModel(QObject* parent)
    : QObject(parent)
{
    qApp->installEventFilter(this);
}

bool DockFrameModel::eventFilter(QObject* watched, QEvent* event)
{
    auto propertyChangeEvent = dynamic_cast<QDynamicPropertyChangeEvent*>(event);
    if (!propertyChangeEvent) {
        return QObject::eventFilter(watched, event);
    }

    if (propertyChangeEvent->propertyName() == CONTEXT_MENU_MODEL_PROPERTY) {
        emit tabsChanged();

        if (watched == currentDockObject()) {
            emit currentDockChanged();
        }
    }

    return QObject::eventFilter(watched, event);
}

QQuickItem* DockFrameModel::frame() const
{
    return m_frame;
}

QVariantList DockFrameModel::tabs() const
{
    QVariantList result;

    auto frame = dynamic_cast<KDDockWidgets::Frame*>(m_frame);
    if (!frame || frame->hasSingleDockWidget()) {
        return result;
    }

    for (const KDDockWidgets::DockWidgetBase* dock : frame->dockWidgets()) {
        QVariantMap tab;
        tab["title"] = dock->title();
        tab[CONTEXT_MENU_MODEL_PROPERTY] = dock->property(CONTEXT_MENU_MODEL_PROPERTY).value<QVariant>();

        result << tab;
    }

    return result;
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
        emit tabsChanged();

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

        emit currentDockChanged();
    });

    connect(qApp, &QApplication::aboutToQuit, [numDocksChangedCon, currentDockWidgetChangedCon
#if (defined (_MSCVER) || defined (_MSC_VER)) // prevent bogus compiler warning with MSVC
                                               , this
#endif
            ]() {
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
    auto dockPanel = currentDockProperty(DOCK_PANEL_PROPERY).value<DockPanel*>();
    return dockPanel ? dockPanel->navigationSection() : nullptr;
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

QVariant DockFrameModel::currentDockContextMenuModel() const
{
    return currentDockProperty(CONTEXT_MENU_MODEL_PROPERTY).value<QVariant>();
}

const QObject* DockFrameModel::currentDockObject() const
{
    auto frame = dynamic_cast<KDDockWidgets::Frame*>(m_frame);
    return frame ? frame->currentDockWidget() : nullptr;
}

QVariant DockFrameModel::currentDockProperty(const char* propertyName) const
{
    const QObject* obj = currentDockObject();
    return obj ? obj->property(propertyName) : QVariant();
}

void DockFrameModel::handleMenuItem(const QString& itemId)
{
    QVariantList menuItems = currentDockContextMenuModel().toList();
    QVariantMap item = findMenuItem(menuItems, itemId);
    ActionCode code = codeFromQString(item["code"].toString());

    dispatcher()->dispatch(code);
}
