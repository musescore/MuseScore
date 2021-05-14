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

#ifndef CONTROLLERACCESSIBLEINTERFACE_H
#define CONTROLLERACCESSIBLEINTERFACE_H

#include <QAccessibleInterface>

namespace mu::accessibility {
class AccessibilityController;
class AccessibleControllerInterface : public QAccessibleInterface
{
public:
    AccessibleControllerInterface(QObject* o);

    bool isValid() const override;
    QObject* object() const override;
    QWindow* window() const override;
    QRect rect() const override;

    QAccessibleInterface* parent() const override;
    int childCount() const override;
    QAccessibleInterface* child(int i) const override;
    QAccessibleInterface* childAt(int x, int y) const override;
    int indexOfChild(const QAccessibleInterface* iface) const override;

    QAccessible::Role role() const override;
    QAccessible::State state() const override;

    QString text(QAccessible::Text) const override;
    void setText(QAccessible::Text t, const QString& text) override;

private:

    AccessibilityController* m_controller = nullptr;
};
}

#endif // CONTROLLERACCESSIBLEINTERFACE_H
