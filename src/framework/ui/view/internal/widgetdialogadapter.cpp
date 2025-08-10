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
#include "widgetdialogadapter.h"

#include <QGuiApplication>

#include "log.h"

using namespace muse::ui;

WidgetDialogAdapter::WidgetDialogAdapter(QDialog* parent)
    : QObject(parent), m_dialog(parent)
{
    IF_ASSERT_FAILED(m_dialog) {
        return;
    }

    m_dialog->installEventFilter(this);
}

bool WidgetDialogAdapter::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::Show && m_onShownCallBack) {
        m_onShownCallBack();
    }

    if (event->type() == QEvent::Hide && m_onHideCallBack) {
        m_onHideCallBack();
    }

    if (event->type() == QEvent::ShortcutOverride) {
        if (QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event)) {
            if (keyEvent->key() == Qt::Key_Escape && keyEvent->modifiers() == Qt::NoModifier) {
                m_dialog->close();
                return true;
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

WidgetDialogAdapter& WidgetDialogAdapter::onShow(const std::function<void()>& func)
{
    m_onShownCallBack = func;
    return *this;
}

WidgetDialogAdapter& WidgetDialogAdapter::onHide(const std::function<void()>& func)
{
    m_onHideCallBack = func;
    return *this;
}
