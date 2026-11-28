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
#pragma once

#include <QObject>

namespace mu::notation {
//! Tells when exactly the chosen modifier keys are held on their own, wherever the focus is in the application.
//! Another key or a click while they're held cancels it until one of them is released.
class ModifierHoldWatcher : public QObject
{
    Q_OBJECT

public:
    explicit ModifierHoldWatcher(QObject* parent = nullptr);

    //! No modifiers: never held
    void setModifiers(Qt::KeyboardModifiers modifiers);

    bool isHeld() const { return m_isHeld; }

signals:
    void heldChanged(bool held);

private:
    bool eventFilter(QObject* watched, QEvent* event) override;

    void onModifiersChanged(Qt::KeyboardModifiers pressed);
    void cancel();
    void reset();
    void setHeld(bool held);

    Qt::KeyboardModifiers m_modifiers;
    Qt::KeyboardModifiers m_pressed;
    bool m_isCancelled = false; // until one of the keys is released
    bool m_isHeld = false;
};
}
