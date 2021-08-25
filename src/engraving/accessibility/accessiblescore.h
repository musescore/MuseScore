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
#ifndef MU_ENGRAVING_ACCESSIBLESCORE_H
#define MU_ENGRAVING_ACCESSIBLESCORE_H

#include "modularity/ioc.h"
#include "accessibility/iaccessible.h"
#include "accessibility/iaccessibilitycontroller.h"

#include "accessibleelement.h"
#include "libmscore/masterscore.h"

namespace mu::engraving {
class AccessibleScore : public accessibility::IAccessible
{
    INJECT(score, accessibility::IAccessibilityController, accessibilityController)

public:
    AccessibleScore(Ms::Score* score);
    ~AccessibleScore();

    void addChild(AccessibleElement* e);
    void removeChild(AccessibleElement* e);

    void setActive(bool arg);

    void setFocusedElement(AccessibleElement* e);
    AccessibleElement* focusedElement() const;

    QRect toScreenRect(const QRect&, bool* ok = nullptr) const;

    // IAccessible
    const IAccessible* accessibleParent() const override;
    size_t accessibleChildCount() const override;
    const IAccessible* accessibleChild(size_t i) const override;
    Role accessibleRole() const override;
    QString accessibleName() const override;
    QString accessibleDescription() const override;
    bool accessibleState(State st) const override;
    QRect accessibleRect() const override;
    mu::async::Channel<Property> accessiblePropertyChanged() const override;
    mu::async::Channel<State, bool> accessibleStateChanged() const override;

private:

    Ms::Score* m_score = nullptr;
    bool m_registred = false;
    bool m_active = false;
    mu::async::Channel<IAccessible::State, bool> m_accessibleStateChanged;
    QList<AccessibleElement*> m_children;
    AccessibleElement* m_focusedElement = nullptr;
};
}

#endif // MU_ENGRAVING_ACCESSIBLESCORE_H
