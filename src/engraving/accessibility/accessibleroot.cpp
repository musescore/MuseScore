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

#include "translation.h"

using namespace mu::engraving;
using namespace mu::accessibility;

AccessibleRoot::AccessibleRoot(RootItem* e, Role role)
    : AccessibleItem(e, role)
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

void AccessibleRoot::setFocusedElement(AccessibleItemPtr e, bool voiceStaffInfoChange)
{
    AccessibleItemWeakPtr old = m_focusedElement;
    updateStaffInfo(e, old, voiceStaffInfoChange);

    if (auto oldItem = old.lock()) {
        oldItem->notifyAboutFocus(false);
    }

    m_focusedElement = e;
    if (auto newItem = m_focusedElement.lock()) {
        newItem->notifyAboutFocus(true);
    }
}

AccessibleItemWeakPtr AccessibleRoot::focusedElement() const
{
    return m_focusedElement;
}

void AccessibleRoot::notifyAboutFocusedElementNameChanged()
{
    m_staffInfo = "";

    if (auto focusedElement = m_focusedElement.lock()) {
        focusedElement->accessiblePropertyChanged().send(accessibility::IAccessible::Property::Name, Val());
    }
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

void AccessibleRoot::updateStaffInfo(const AccessibleItemWeakPtr newAccessibleItem, const AccessibleItemWeakPtr oldAccessibleItem,
                                     bool voiceStaffInfoChange)
{
    m_staffInfo = "";

    if (!voiceStaffInfoChange) {
        return;
    }

    AccessibleItemPtr newItem = newAccessibleItem.lock();
    AccessibleItemPtr oldItem = oldAccessibleItem.lock();

    if (newItem && newItem->element()->hasStaff()) {
        staff_idx_t newStaffIdx = newItem->element()->staffIdx();
        staff_idx_t oldStaffIdx = oldItem ? oldItem->element()->staffIdx() : nidx;

        if (newStaffIdx != oldStaffIdx) {
            auto element = newItem->element();
            QString staff = qtrc("engraving", "Staff %1").arg(QString::number(element->staffIdx() + 1));

            QString staffName = element->staff()->part()->longName(element->tick());
            if (staffName.isEmpty()) {
                staffName = element->staff()->partName();
            }

            if (staffName.isEmpty()) {
                m_staffInfo = staff;
            } else {
                m_staffInfo = QString("%2 (%3)").arg(staff, staffName);
            }
        }
    }
}

QString AccessibleRoot::commandInfo() const
{
    return m_commandInfo;
}

void AccessibleRoot::setCommandInfo(const QString& command)
{
    m_commandInfo = command;

    if (!m_commandInfo.isEmpty()) {
        notifyAboutFocusedElementNameChanged();
    }
}

void AccessibleRoot::setMapToScreenFunc(const AccessibleMapToScreenFunc& func)
{
    m_accessibleMapToScreenFunc = func;
}
