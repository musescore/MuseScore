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
#include "accessibleroot.h"

#include "../libmscore/score.h"

using namespace mu::engraving;
using namespace mu::accessibility;

AccessibleRoot::AccessibleRoot(RootItem* e)
    : AccessibleItem(e)
{
}

void AccessibleRoot::setFocusedElement(AccessibleItem* e)
{
    AccessibleItem* old = m_focusedElement;
    m_focusedElement = nullptr;

    if (old) {
        old->notifyAboutFocus(false);
    }

    m_focusedElement = e;
    if (m_focusedElement) {
        m_focusedElement->notifyAboutFocus(true);
    }
}

AccessibleItem* AccessibleRoot::focusedElement() const
{
    return m_focusedElement;
}

QRect AccessibleRoot::toScreenRect(const QRect&, bool* ok) const
{
    //! TODO Not implemented
    if (ok) {
        *ok = false;
    }
    return QRect();
}

const IAccessible* AccessibleRoot::accessibleParent() const
{
    return accessibilityController()->accessibleRoot();
}

IAccessible::Role AccessibleRoot::accessibleRole() const
{
    return IAccessible::Panel;
}

QString AccessibleRoot::accessibleName() const
{
    return element()->score()->title();
}
