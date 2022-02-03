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

#include "async/asyncable.h"
#include "global/iapplication.h"

#include "modularity/ioc.h"
#include "ui/imainwindow.h"
#include "ui/iinteractiveprovider.h"
#include "../iaccessibilityconfiguration.h"
#include "../iqaccessibleinterfaceregister.h"
#include "../iaccessibilitycontroller.h"
#include "accessibleobject.h"

class QAccessibleInterface;
class QAccessibleEvent;

namespace mu::diagnostics {
class DiagnosticAccessibleModel;
}

namespace mu::accessibility {
class AccessibilityController : public IAccessibilityController, public IAccessible, public async::Asyncable,
    public std::enable_shared_from_this<AccessibilityController>
{
    INJECT(accessibility, framework::IApplication, application)
    INJECT(accessibility, ui::IMainWindow, mainWindow)
    INJECT(accessibility, ui::IInteractiveProvider, interactiveProvider)
    INJECT(accessibility, IAccessibilityConfiguration, configuration)

public:
    AccessibilityController() = default;
    ~AccessibilityController() override;

    static QAccessibleInterface* accessibleInterface(QObject* object);

    // IAccessibilityController
    void reg(IAccessible* item) override;
    void unreg(IAccessible* item) override;

    const IAccessible* accessibleRoot() const override;
    const IAccessible* lastFocused() const override;
    // -----

    // IAccessibility (root)
    IAccessible* accessibleParent() const override;

    size_t accessibleChildCount() const override;
    IAccessible* accessibleChild(size_t i) const override;

    Role accessibleRole() const override;
    QString accessibleName() const override;
    QString accessibleDescription() const override;
    bool accessibleState(State st) const override;
    QRect accessibleRect() const override;

    // Value Interface
    QVariant accessibleValue() const override;
    QVariant accessibleMaximumValue() const override;
    QVariant accessibleMinimumValue() const override;
    QVariant accessibleValueStepSize() const override;

    // Text Interface
    void accessibleSelection(int selectionIndex, int* startOffset, int* endOffset) const override;
    int accessibleSelectionCount() const override;

    int accessibleCursorPosition() const override;

    QString accessibleText(int startOffset, int endOffset) const override;
    QString accessibleTextAtOffset(int offset, TextBoundaryType boundaryType, int* startOffset, int* endOffset) const override;
    int accessibleCharacterCount() const override;

    async::Channel<Property, Val> accessiblePropertyChanged() const override;
    async::Channel<State, bool> accessibleStateChanged() const override;
    // -----

    QAccessibleInterface* parentIface(const IAccessible* item) const;
    int childCount(const IAccessible* item) const;
    QAccessibleInterface* child(const IAccessible* item, int i) const;
    int indexOfChild(const IAccessible* item, const QAccessibleInterface* iface) const;
    QAccessibleInterface* focusedChild(const IAccessible* item) const;

    async::Channel<QAccessibleEvent*> eventSent() const;

private:

    friend class mu::diagnostics::DiagnosticAccessibleModel;

    struct Item
    {
        IAccessible* item = nullptr;
        AccessibleObject* object = nullptr;
        QAccessibleInterface* iface = nullptr;

        bool isValid() const { return item != nullptr; }
    };

    void init();

    const Item& findItem(const IAccessible* aitem) const;

    void propertyChanged(IAccessible* item, IAccessible::Property property, const Val& value);
    void stateChanged(IAccessible* item, IAccessible::State state, bool arg);

    void sendEvent(QAccessibleEvent* ev);

    void cancelPreviousReading();

    QHash<const IAccessible*, Item> m_allItems;

    QList<IAccessible*> m_children;
    async::Channel<QAccessibleEvent*> m_eventSent;
    IAccessible* m_lastFocused = nullptr;

    bool m_inited = false;
};
}

#endif // MU_ACCESSIBILITY_ACCESSIBILITYCONTROLLER_H
