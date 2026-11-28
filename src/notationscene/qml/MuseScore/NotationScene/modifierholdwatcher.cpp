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
#include "modifierholdwatcher.h"

#include <QGuiApplication>
#include <QKeyEvent>

#include "shortcuts_v2/shortcutstypes.h"

using namespace mu::notation;

static constexpr Qt::KeyboardModifiers WATCHED_MODIFIERS
    = Qt::ControlModifier | Qt::AltModifier | Qt::ShiftModifier | Qt::MetaModifier;

ModifierHoldWatcher::ModifierHoldWatcher(QObject* parent)
    : QObject(parent)
{
    // Keys released while another application has the focus never reach us
    connect(qGuiApp, &QGuiApplication::applicationStateChanged, this, [this](Qt::ApplicationState state) {
        if (state != Qt::ApplicationActive) {
            reset();
        }
    });

    qGuiApp->installEventFilter(this);
}

void ModifierHoldWatcher::setModifiers(Qt::KeyboardModifiers modifiers)
{
    m_modifiers = modifiers & WATCHED_MODIFIERS;
    reset();
}

bool ModifierHoldWatcher::eventFilter(QObject* watched, QEvent* event)
{
    // A key event can pass through here several times as it propagates; each pass gives the same result
    switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
        const QKeyEvent* keyEvent = static_cast<const QKeyEvent*>(event);
        const Qt::KeyboardModifier modifier = muse::shortcuts::modifierOfKey(keyEvent->key());

        if (modifier == Qt::NoModifier) {
            if (event->type() == QEvent::KeyPress) {
                cancel();
            }

            break;
        }

        // The event's modifiers are those from before the key changed: add or remove the key itself
        Qt::KeyboardModifiers pressed = keyEvent->modifiers() & WATCHED_MODIFIERS;

        if (event->type() == QEvent::KeyPress) {
            pressed |= modifier;
        } else {
            pressed &= ~modifier;
        }

        onModifiersChanged(pressed);
        break;
    }
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
    case QEvent::TouchBegin:
    case QEvent::TabletPress:
        cancel();
        break;
    default:
        break;
    }

    return QObject::eventFilter(watched, event);
}

void ModifierHoldWatcher::onModifiersChanged(Qt::KeyboardModifiers pressed)
{
    m_pressed = pressed;

    if (m_modifiers == Qt::NoModifier || m_pressed != m_modifiers) {
        // Only releasing one of the configured keys ends a cancellation: pressing and releasing an extra
        // modifier (e.g. Meta while Ctrl+Alt+Shift stay down) brings back the same keys, still cancelled
        if ((m_pressed & m_modifiers) != m_modifiers) {
            m_isCancelled = false;
        }

        setHeld(false);

        return;
    }

    // On the way to a shortcut that starts with the same keys (e.g. Ctrl+Alt+Shift+F) this flashes until the last
    // key cancels it, which does no harm: nothing moves or changes
    if (!m_isCancelled) {
        setHeld(true);
    }
}

void ModifierHoldWatcher::cancel()
{
    if (m_modifiers == Qt::NoModifier || m_pressed != m_modifiers) {
        return;
    }

    m_isCancelled = true;
    setHeld(false);
}

void ModifierHoldWatcher::reset()
{
    m_pressed = Qt::NoModifier;
    m_isCancelled = false;
    setHeld(false);
}

void ModifierHoldWatcher::setHeld(bool held)
{
    if (m_isHeld == held) {
        return;
    }

    m_isHeld = held;
    emit heldChanged(held);
}
