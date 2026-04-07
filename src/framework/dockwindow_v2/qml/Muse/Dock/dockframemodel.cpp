/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "kddockwidgets/src/core/Group.h"
#include "kddockwidgets/src/qtquick/views/Group.h"
#include "kddockwidgets/src/qtquick/views/DockWidget.h"
#include "kddockwidgets/src/qtquick/views/View.h"

#include "ui/qml/Muse/Ui/navigationsection.h"
#include "uicomponents/qml/Muse/UiComponents/abstractmenumodel.h"

#include "docktypes.h"

#include "dockpanelview.h"

using namespace muse::dock;
using namespace muse::actions;
using namespace muse::uicomponents;

DockFrameModel::DockFrameModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
    qApp->installEventFilter(this);
    m_tabsModel = new DockTabsModel(this);
}

bool DockFrameModel::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() != QEvent::DynamicPropertyChange) {
        return QObject::eventFilter(watched, event);
    }

    auto propertyChangeEvent = dynamic_cast<QDynamicPropertyChangeEvent*>(event);
    if (!propertyChangeEvent) {
        return QObject::eventFilter(watched, event);
    }

    if (propertyChangeEvent->propertyName() == CONTEXT_MENU_MODEL_PROPERTY) {
        onContextMenuChanged(watched);
    } else if (propertyChangeEvent->propertyName() == TOOLBAR_COMPONENT_PROPERTY) {
        onToolBarComponentChanged(watched);
    }

    if (propertyChangeEvent->propertyName() == "highlightingRect") {
        emit highlightingVisibleChanged();
    }

    return QObject::eventFilter(watched, event);
}

void DockFrameModel::onContextMenuChanged(QObject* obj)
{
    const auto* dwv = qobject_cast<const KDDockWidgets::QtQuick::DockWidget*>(obj);
    if (!dwv) {
        return;
    }

    m_tabsModel->contextMenuChanged(dwv);

    if (dwv == currentDockWidget()) {
        emit currentDockChanged();
    }
}

void DockFrameModel::onToolBarComponentChanged(QObject* obj)
{
    const auto* dwv = qobject_cast<const KDDockWidgets::QtQuick::DockWidget*>(obj);
    if (!dwv) {
        return;
    }

    m_tabsModel->toolBarComponentChanged(dwv);

    if (dwv == currentDockWidget()) {
        emit currentDockChanged();
    }
}

QQuickItem* DockFrameModel::frame() const
{
    return m_frame;
}

QQmlComponent* DockFrameModel::titleBar() const
{
    return m_titleBar;
}

bool DockFrameModel::titleBarAllowed() const
{
    return m_titleBarAllowed;
}

bool DockFrameModel::isHorizontalPanel() const
{
    return m_isHorizontalPanel;
}

void DockFrameModel::setFrame(QQuickItem* frame)
{
    if (frame == m_frame.data()) {
        return;
    }

    m_frame = qobject_cast<KDDockWidgets::QtQuick::Group*>(frame);
    emit frameChanged(frame);

    listenChangesInFrame();
}

void DockFrameModel::listenChangesInFrame()
{
    if (!m_frame) {
        return;
    }

    connect(m_frame, &KDDockWidgets::QtQuick::Group::currentDockWidgetChanged, this, [this]() {
        auto* group = m_frame ? m_frame->group() : nullptr;
        if (!group) {
            return;
        }

        auto allDocks = group->dockWidgets();
        m_tabsModel->init(allDocks);

        if (allDocks.isEmpty()) {
            setTitleBarAllowed(false);
            updateNavigationSection();
            emit currentDockChanged();
            return;
        }

        DockProperties properties = readPropertiesFromObject(allDocks.first());
        bool isHorizontalPanel = (properties.type == DockType::Panel)
                                 && (properties.location == Location::Top || properties.location == Location::Bottom);
        setIsHorizontalPanel(isHorizontalPanel);

        updateTitleBar();

        bool titleBarAllowed = (properties.type == DockType::Panel) && (properties.floatable || properties.closable);
        setTitleBarAllowed(titleBarAllowed);

        updateNavigationSection();
        emit currentDockChanged();
    });
}

void DockFrameModel::setTitleBarAllowed(bool allowed)
{
    if (allowed == m_titleBarAllowed) {
        return;
    }

    m_titleBarAllowed = allowed;
    emit titleBarAllowedChanged(allowed);
}

void DockFrameModel::setIsHorizontalPanel(bool is)
{
    if (is == m_isHorizontalPanel) {
        return;
    }

    m_isHorizontalPanel = is;
    emit isHorizontalPanelChanged();
}

QObject* DockFrameModel::currentNavigationSection() const
{
    auto dockPanel = currentDockProperty(DOCK_PANEL_PROPERTY).value<DockPanelView*>();
    return dockPanel ? dockPanel->navigationSection_property() : nullptr;
}

void DockFrameModel::updateNavigationSection()
{
    QObject* n = currentNavigationSection();
    if (m_navigationSection != n) {
        m_navigationSection = n;
        emit navigationSectionChanged();
    }
}

QQmlComponent* DockFrameModel::currentTitleBar() const
{
    QQmlComponent* titleBar = currentDockProperty(TITLEBAR_PROPERTY).value<QQmlComponent*>();
    return titleBar;
}

void DockFrameModel::updateTitleBar()
{
    QQmlComponent* tb = currentTitleBar();
    if (m_titleBar != tb) {
        m_titleBar = tb;
        emit titleBarChanged();
    }
}

QObject* DockFrameModel::navigationSection() const
{
    return m_navigationSection;
}

QString DockFrameModel::currentDockUniqueName() const
{
    auto* dock = currentDockWidget();
    return dock ? dock->uniqueName() : QString();
}

QVariant DockFrameModel::currentDockContextMenuModel() const
{
    return currentDockProperty(CONTEXT_MENU_MODEL_PROPERTY);
}

QVariant DockFrameModel::currentDockToolbarComponent() const
{
    return currentDockProperty(TOOLBAR_COMPONENT_PROPERTY);
}

bool DockFrameModel::highlightingVisible() const
{
    return highlightingRect().isValid();
}

QRect DockFrameModel::highlightingRect() const
{
    if (!m_frame) {
        return QRect();
    }

    auto* group = m_frame->group();
    for (auto* dock : group->dockWidgets()) {
        DockProperties properties = readPropertiesFromObject(dock);

        if (properties.highlightingRect.isValid()) {
            return properties.highlightingRect;
        }
    }

    return QRect();
}

KDDockWidgets::QtQuick::DockWidget* DockFrameModel::currentDockWidget() const
{
    if (!m_frame) {
        return nullptr;
    }
    auto* group = m_frame->group();
    if (!group || group->isEmpty()) {
        return nullptr;
    }
    auto* ctrl = group->currentDockWidget();
    return ctrl ? qobject_cast<KDDockWidgets::QtQuick::DockWidget*>(
        KDDockWidgets::QtQuick::asQQuickItem(ctrl)) : nullptr;
}

QVariant DockFrameModel::currentDockProperty(const char* propertyName) const
{
    const QObject* dock = currentDockWidget();
    return dock ? dock->property(propertyName) : QVariant();
}

void DockFrameModel::handleMenuItem(const QString& itemId) const
{
    auto menuModel = currentDockContextMenuModel().value<AbstractMenuModel*>();

    if (menuModel) {
        menuModel->handleMenuItem(itemId);
    }
}
