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
#include <QHash>

#include "../iaccessibilitycontroller.h"
#include "accessibleobject.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "ui/imainwindow.h"

class QAccessibleInterface;
class QAccessibleEvent;

namespace mu::accessibility {
class AccessibilityController : public IAccessibilityController, public IAccessibility, public async::Asyncable,
    public std::enable_shared_from_this<AccessibilityController>
{
    INJECT(accessibility, ui::IMainWindow, mainWindow)

public:
    AccessibilityController() = default;
    ~AccessibilityController();

    void init();

    // IAccessibilityController
    const IAccessibility* rootItem() const override;

    void reg(IAccessibility* item) override;
    void unreg(IAccessibility* item) override;

    void actived(IAccessibility* item, bool isActive) override;
    void focused(IAccessibility* item) override;
    // -----

    // IAccessibility
    IAccessibility* accessibleParent() const override;
    async::Notification accessibleParentChanged() const override;

    size_t accessibleChildCount() const override;
    IAccessibility* accessibleChild(size_t i) const override;

    Role accessibleRole() const override;
    QString accessibleName() const override;
    bool accessibleState(State st) const override;
    QRect accessibleRect() const override;
    // -----

    QAccessibleInterface* parentIface(const IAccessibility* item) const;
    int childCount(const IAccessibility* item) const;
    QAccessibleInterface* child(const IAccessibility* item, int i) const;
    int indexOfChild(const IAccessibility* item, const QAccessibleInterface* iface) const;

private:

    struct Item
    {
        IAccessibility* item = nullptr;
        AccessibleObject* object = nullptr;
        QAccessibleInterface* iface = nullptr;

        bool isValid() const { return item != nullptr; }
    };

    const Item& findItem(const IAccessibility* aitem) const;

    void sendEvent(QAccessibleEvent* ev);

    QHash<const IAccessibility*, Item> m_allItems;

    QList<IAccessibility*> m_children;
};
}

#endif // MU_ACCESSIBILITY_ACCESSIBILITYCONTROLLER_H
