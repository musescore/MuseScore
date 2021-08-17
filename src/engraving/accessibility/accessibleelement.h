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
#ifndef MU_ENGRAVING_ACCESSIBLEELEMENT_H
#define MU_ENGRAVING_ACCESSIBLEELEMENT_H

#include "accessibility/iaccessible.h"
#include "async/asyncable.h"

#include "libmscore/element.h"

//! NOTE At the moment this is just a concept, not a production-ready system, a lot of work yet.

namespace mu::engraving {
class AccessibleScore;
class AccessibleElement : public accessibility::IAccessible, public async::Asyncable
{
public:
    AccessibleElement(Ms::Element* e = nullptr);
    virtual ~AccessibleElement();

    virtual AccessibleElement* clone(Ms::Element* e) const;

    void setElement(Ms::Element* e);
    const Ms::Element* element() const;
    void focused();

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

private:

    bool isAvalaible() const;
    AccessibleScore* accessibleScore() const;

    Ms::Element* m_element = nullptr;
    bool m_registred = false;

    mu::async::Channel<IAccessible::State, bool> m_accessibleStateChanged;
};
}

#endif // MU_ENGRAVING_ACCESSIBLEELEMENT_H
