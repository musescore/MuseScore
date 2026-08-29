/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "videopanelcontextmenumodel.h"

#include <QGuiApplication>
#include <QScreen>

#include "types/translatablestring.h"

using namespace mu::playback;
using namespace muse;
using namespace muse::ui;
using namespace muse::uicomponents;
using namespace muse::actions;

static const ActionCode TOGGLE_FULL_SCREEN_ACTION("video-panel-toggle-fullscreen");

VideoPanelContextMenuModel::VideoPanelContextMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

void VideoPanelContextMenuModel::load()
{
    AbstractMenuModel::load();

    dispatcher()->reg(this, TOGGLE_FULL_SCREEN_ACTION, [this]() {
        emit toggleFullScreenRequested();
    });

    updateItems();
}

bool VideoPanelContextMenuModel::floating() const
{
    return m_floating;
}

void VideoPanelContextMenuModel::setFloating(bool floating)
{
    if (m_floating == floating) {
        return;
    }

    m_floating = floating;
    emit floatingChanged();

    updateItems();
}

bool VideoPanelContextMenuModel::isFullScreen() const
{
    return m_isFullScreen;
}

void VideoPanelContextMenuModel::setIsFullScreen(bool isFullScreen)
{
    if (m_isFullScreen == isFullScreen) {
        return;
    }

    m_isFullScreen = isFullScreen;
    emit isFullScreenChanged();

    updateItems();
}

QVariantMap VideoPanelContextMenuModel::screenAvailableGeometry(int windowX, int windowY) const
{
    QScreen* screen = QGuiApplication::screenAt(QPoint(windowX, windowY));
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }

    QVariantMap result;
    if (!screen) {
        return result;
    }

    const QRect geometry = screen->availableGeometry();
    result["x"] = geometry.x();
    result["y"] = geometry.y();
    result["width"] = geometry.width();
    result["height"] = geometry.height();
    return result;
}

void VideoPanelContextMenuModel::updateItems()
{
    if (!m_floating) {
        setItems({});
        return;
    }

    UiAction action;
    action.title = m_isFullScreen ? TranslatableString("playback", "Exit full screen") : TranslatableString("playback", "Full screen");
    action.code = TOGGLE_FULL_SCREEN_ACTION;

    MenuItem* item = new MenuItem(action, this);
    item->setId("video-panel-fullscreen");
    item->setState(UiActionState::make_enabled());

    setItems({ item });
}
