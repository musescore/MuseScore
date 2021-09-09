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

#include "accessibility/iaccessible.h"
#include "modularity/ioc.h"
#include "accessibility/iaccessibilitycontroller.h"

#include "libmscore/engravingitem.h"

//! NOTE At the moment this is just a concept, not a production-ready system, a lot of work yet.

namespace mu::engraving {
class AccessibleScore;
class AccessibleItem : public accessibility::IAccessible
{
    INJECT_STATIC(engraving, accessibility::IAccessibilityController, accessibilityController)

public:
    AccessibleItem(Ms::EngravingItem* e);
    virtual ~AccessibleItem();
    virtual AccessibleItem* clone(Ms::EngravingItem* e) const;

    virtual void setup();
    bool isAvalaible() const;

    const Ms::EngravingItem* element() const;

    void setRegistred(bool arg);
    bool registred() const;

    void setFocus();
    void notifyAboutFocus(bool focused);

    // IAccessible
    const IAccessible* accessibleParent() const override;
    size_t accessibleChildCount() const override;
    const IAccessible* accessibleChild(size_t i) const override;

    Role accessibleRole() const override;
    QString accessibleName() const override;
    QString accessibleDescription() const override;
    bool accessibleState(State st) const override;
    QRect accessibleRect() const override;

    async::Channel<Property> accessiblePropertyChanged() const override;
    async::Channel<State, bool> accessibleStateChanged() const override;
    // ---

    static bool enabled;

private:

    AccessibleScore* accessibleScore() const;

    Ms::EngravingItem* m_element = nullptr;
    bool m_registred = false;

    mu::async::Channel<IAccessible::State, bool> m_accessibleStateChanged;
};
}

#endif // MU_ENGRAVING_ACCESSIBLEITEM_H
