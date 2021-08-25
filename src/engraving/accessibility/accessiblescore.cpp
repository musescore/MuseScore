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

AccessibleScore::AccessibleScore(Ms::Score* score)
    : m_score(score)
{
    accessibilityController()->reg(this);
    m_registred = true;
}

AccessibleScore::~AccessibleScore()
{
    accessibilityController()->unreg(this);
}

void AccessibleScore::addChild(AccessibleElement* e)
{
    IF_ASSERT_FAILED(!m_children.contains(e)) {
        return;
    }

    m_children.append(e);
    accessibilityController()->reg(e);
}

void AccessibleScore::removeChild(AccessibleElement* e)
{
    IF_ASSERT_FAILED(m_children.contains(e)) {
        return;
    }
    m_children.removeOne(e);

    accessibilityController()->unreg(e);
}

void AccessibleScore::setActive(bool arg)
{
    if (m_active == arg) {
        return;
    }

    m_active = arg;
    m_accessibleStateChanged.send(State::Active, arg);
}

void AccessibleScore::setFocusedElement(AccessibleElement* e)
{
    m_focusedElement = e;
}

AccessibleElement* AccessibleScore::focusedElement() const
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
    return m_children.size();
}

const IAccessible* AccessibleScore::accessibleChild(size_t i) const
{
    return m_children.at(int(i));
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
