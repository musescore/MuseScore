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
#ifndef MU_ENGRAVING_ACCESSIBLEITEM_H
#define MU_ENGRAVING_ACCESSIBLEITEM_H

#include "global/allocator.h"

#include "accessibility/iaccessible.h"
#include "modularity/ioc.h"
#include "accessibility/iaccessibilitycontroller.h"

#include "libmscore/engravingitem.h"
#include "libmscore/textbase.h"

//! NOTE At the moment this is just a concept, not a production-ready system, a lot of work yet.

namespace mu::engraving {
class AccessibleRoot;
class AccessibleItem : public accessibility::IAccessible, public std::enable_shared_from_this<AccessibleItem>
{
    OBJECT_ALLOCATOR(engraving, AccessibleItem)

    INJECT_STATIC(engraving, accessibility::IAccessibilityController, accessibilityController)

public:
    AccessibleItem(EngravingItem* e, Role role = Role::ElementOnScore);
    virtual ~AccessibleItem();
    virtual AccessibleItem* clone(EngravingItem* e) const;

    virtual void setup();

    AccessibleRoot* accessibleRoot() const;

    const EngravingItem* element() const;

    bool registered() const;

    void notifyAboutFocus(bool focused);

    // IAccessible
    const IAccessible* accessibleParent() const override;
    size_t accessibleChildCount() const override;
    const IAccessible* accessibleChild(size_t i) const override;
    QWindow* accessibleWindow() const override;

    Role accessibleRole() const override;
    QString accessibleName() const override;
    QString accessibleDescription() const override;
    bool accessibleState(State st) const override;
    QRect accessibleRect() const override;
    bool accessibleIgnored() const override;

    QVariant accessibleValue() const override;
    QVariant accessibleMaximumValue() const override;
    QVariant accessibleMinimumValue() const override;
    QVariant accessibleValueStepSize() const override;

    void accessibleSelection(int selectionIndex, int* startOffset, int* endOffset) const override;
    int accessibleSelectionCount() const override;

    int accessibleCursorPosition() const override;

    QString accessibleText(int startOffset, int endOffset) const override;
    QString accessibleTextBeforeOffset(int offset, TextBoundaryType boundaryType, int* startOffset, int* endOffset) const override;
    QString accessibleTextAfterOffset(int offset, TextBoundaryType boundaryType, int* startOffset, int* endOffset) const override;
    QString accessibleTextAtOffset(int offset, TextBoundaryType boundaryType, int* startOffset, int* endOffset) const override;
    int accessibleCharacterCount() const override;

    async::Channel<Property, Val> accessiblePropertyChanged() const override;
    async::Channel<State, bool> accessibleStateChanged() const override;

    void setState(State state, bool arg) override;
    // ---

    static bool enabled;

private:
    TextCursor* textCursor() const;

protected:

    EngravingItem* m_element = nullptr;
    bool m_registred = false;

    Role m_role = Role::ElementOnScore;

    mu::async::Channel<IAccessible::Property, Val> m_accessiblePropertyChanged;
    mu::async::Channel<IAccessible::State, bool> m_accessibleStateChanged;
};
using AccessibleItemPtr = std::shared_ptr<AccessibleItem>;
using AccessibleItemWeakPtr = std::weak_ptr<AccessibleItem>;
}

#endif // MU_ENGRAVING_ACCESSIBLEITEM_H
