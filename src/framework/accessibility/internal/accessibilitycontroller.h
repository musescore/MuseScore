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
#ifndef MU_ACCESSIBILITY_ACCESSIBILITYCONTROLLER_H
#define MU_ACCESSIBILITY_ACCESSIBILITYCONTROLLER_H

#include <memory>
#include <QObject>
#include <QList>

#include "../iaccessibilitycontroller.h"
#include "accessibleobject.h"

class QAccessibleInterface;
class QAccessibleEvent;

namespace mu::accessibility {
class AccessibilityController : public QObject, public IAccessibilityController,
    public std::enable_shared_from_this<AccessibilityController>
{
    Q_OBJECT
public:
    AccessibilityController() = default;
    ~AccessibilityController();

    void init();

    void created(IAccessibility* parent, IAccessibility* item) override;
    void destroyed(IAccessibility* item) override;
    void actived(IAccessibility* item, bool isActive) override;
    void focused(IAccessibility* item) override;

    int childCount(const IAccessibility* item) const;
    QAccessibleInterface* child(const IAccessibility* item, int i) const;
    int indexOfChild(const IAccessibility* item, const QAccessibleInterface* iface) const;

private:

    struct Item
    {
        IAccessibility* parent = nullptr;
        IAccessibility* item = nullptr;
        AccessibleObject* object = nullptr;
        QAccessibleInterface* iface = nullptr;

        bool isValid() const { return item != nullptr; }
    };

    const Item& findItem(IAccessibility* aitem) const;
    int indexBy(IAccessibility* aitem) const;

    void sendEvent(QAccessibleEvent* ev);

    QList<Item> m_items;
};
}

#endif // MU_ACCESSIBILITY_ACCESSIBILITYCONTROLLER_H
