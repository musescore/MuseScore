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
#include "../libmscore/staff.h"
#include "../libmscore/part.h"

#include "context/uicontext.h"

#include "translation.h"

using namespace mu::engraving;
using namespace mu::accessibility;

AccessibleRoot::AccessibleRoot(RootItem* e)
    : AccessibleItem(e)
{
}

AccessibleRoot::~AccessibleRoot()
{
    if (m_registred && accessibilityController()) {
        accessibilityController()->unreg(this);
        m_registred = false;
    }

    m_element = nullptr;
}

void AccessibleRoot::setFocusedElement(AccessibleItem* e)
{
    AccessibleItem* old = m_focusedElement;
    m_focusedElement = nullptr;

    updateStaffInfo(e, old);

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

mu::RectF AccessibleRoot::toScreenRect(const RectF& rect, bool* ok) const
{
    RectF result;
    if (m_accessibleMapToScreenFunc) {
        result = m_accessibleMapToScreenFunc(rect);
    }

    if (ok) {
        *ok = m_accessibleMapToScreenFunc != nullptr;
    }

    return result;
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
    return qtrc("engraving", "Score") + " " + element()->score()->name();
}

bool AccessibleRoot::enabled() const
{
    return m_enabled;
}

void AccessibleRoot::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

QString AccessibleRoot::staffInfo() const
{
    return m_staffInfo;
}

void AccessibleRoot::updateStaffInfo(const AccessibleItem* newAccessibleItem, const AccessibleItem* oldAccessibleItem)
{
    m_staffInfo = "";

    if (newAccessibleItem && newAccessibleItem->element()->hasStaff()) {
        staff_idx_t newStaffIdx = newAccessibleItem->element()->staffIdx();
        staff_idx_t oldStaffIdx = oldAccessibleItem && oldAccessibleItem->registered() ? oldAccessibleItem->element()->staffIdx() : nidx;

        if (newStaffIdx != oldStaffIdx) {
            auto element = newAccessibleItem->element();
            QString staff = qtrc("engraving", "Staff %1").arg(QString::number(element->staffIdx() + 1));

            QString staffName = element->staff()->part()->longName(element->tick());
            if (staffName.isEmpty()) {
                staffName = element->staff()->partName();
            }

            if (staffName.isEmpty()) {
                m_staffInfo = staff;
            } else {
                m_staffInfo = QString("%2 (%3)").arg(staff).arg(staffName);
            }
        }
    }
}

void AccessibleRoot::setMapToScreenFunc(const AccessibleMapToScreenFunc& func)
{
    m_accessibleMapToScreenFunc = func;
}
