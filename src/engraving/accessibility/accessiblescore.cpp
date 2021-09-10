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
#include "accessiblescore.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::accessibility;
using namespace Ms;

AccessibleScore::AccessibleScore(Ms::Score* score)
    : m_score(score)
{
}

AccessibleScore::~AccessibleScore()
{
    if (m_registred && accessibilityController()) {
        accessibilityController()->unreg(this);
    }
}

void AccessibleScore::setup()
{
    if (!AccessibleItem::enabled) {
        return;
    }

    if (!accessibilityController()) {
        return;
    }

    if (m_score->isPaletteScore()) {
        return;
    }

    accessibilityController()->reg(this);
    m_registred = true;
}

void AccessibleScore::setActive(bool arg)
{
    if (m_active == arg) {
        return;
    }

    m_active = arg;
    m_accessibleStateChanged.send(State::Active, arg);
}

void AccessibleScore::setFocusedElement(AccessibleItem* e)
{
    setActive(true);

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

AccessibleItem* AccessibleScore::focusedElement() const
{
    return m_focusedElement;
}

QRect AccessibleScore::toScreenRect(const QRect&, bool* ok) const
{
    //! TODO Not implemented
    if (ok) {
        *ok = false;
    }
    return QRect();
}

const IAccessible* AccessibleScore::accessibleParent() const
{
    return accessibilityController()->accessibleRoot();
}

size_t AccessibleScore::accessibleChildCount() const
{
    TRACEFUNC;
    size_t count = 0;
    for (const EngravingObject* obj : m_score->EngravingObject::children()) {
        if (obj->isEngravingItem()) {
            AccessibleItem* access = Ms::toEngravingItem(obj)->accessible();
            if (access && access->registered()) {
                ++count;
            }
        }
    }
    return count;
}

const IAccessible* AccessibleScore::accessibleChild(size_t i) const
{
    TRACEFUNC;
    size_t count = 0;
    for (const EngravingObject* obj : m_score->EngravingObject::children()) {
        if (obj->isEngravingItem()) {
            AccessibleItem* access = Ms::toEngravingItem(obj)->accessible();
            if (access && access->registered()) {
                if (count == i) {
                    return access;
                }
                ++count;
            }
        }
    }
    return nullptr;
}

IAccessible::Role AccessibleScore::accessibleRole() const
{
    return IAccessible::Panel;
}

QString AccessibleScore::accessibleName() const
{
    return m_score->title();
}

QString AccessibleScore::accessibleDescription() const
{
    return QString();
}

bool AccessibleScore::accessibleState(State st) const
{
    switch (st) {
    case State::Enabled: return true;
    case State::Active: return m_active;
    default:
        break;
    }
    return false;
}

QRect AccessibleScore::accessibleRect() const
{
    return QRect();
}

mu::async::Channel<IAccessible::Property> AccessibleScore::accessiblePropertyChanged() const
{
    static mu::async::Channel<Property> ch;
    return ch;
}

mu::async::Channel<IAccessible::State, bool> AccessibleScore::accessibleStateChanged() const
{
    return m_accessibleStateChanged;
}
