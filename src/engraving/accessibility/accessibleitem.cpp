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
#include "accessibleitem.h"

#include "accessiblescore.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::accessibility;
using namespace Ms;

bool AccessibleItem::enabled = true;

AccessibleItem::AccessibleItem(Ms::EngravingItem* e)
    : m_element(e)
{
}

AccessibleItem::~AccessibleItem()
{
    AccessibleScore* ascore = accessibleScore();

    if (!ascore) {
        return;
    }

    if (m_registred && accessibilityController()) {
        accessibilityController()->unreg(this);
        m_registred = false;
    }

    if (ascore->focusedElement() == this) {
        ascore->setFocusedElement(nullptr);
    }

    m_element = nullptr;
}

AccessibleItem* AccessibleItem::clone(Ms::EngravingItem* e) const
{
    return new AccessibleItem(e);
}

void AccessibleItem::setup()
{
    if (!AccessibleItem::enabled) {
        return;
    }

    if (!accessibilityController()) {
        return;
    }

    accessibilityController()->reg(this);
    m_registred = true;
}

AccessibleScore* AccessibleItem::accessibleScore() const
{
    if (!m_element) {
        return nullptr;
    }

    if (!m_element->score()) {
        return nullptr;
    }

    return m_element->score()->accessible();
}

const Ms::EngravingItem* AccessibleItem::element() const
{
    return m_element;
}

void AccessibleItem::setRegistred(bool arg)
{
    m_registred = arg;
}

bool AccessibleItem::registred() const
{
    return m_registred;
}

void AccessibleItem::setFocus()
{
}

void AccessibleItem::notifyAboutFocus(bool focused)
{
    m_accessibleStateChanged.send(IAccessible::State::Focused, focused);
}

const IAccessible* AccessibleItem::accessibleParent() const
{
    Ms::EngravingItem* p = m_element->parentElement();
    if (!p) {
        return nullptr;
    }
    return p->accessible();
}

size_t AccessibleItem::accessibleChildCount() const
{
    TRACEFUNC;
    size_t count = 0;
    for (const EngravingObject* obj : m_element->children()) {
        if (obj->isEngravingItem()) {
            AccessibleItem* access = Ms::toEngravingItem(obj)->accessible();
            if (access && access->registred()) {
                ++count;
            }
        }
    }
    return count;
}

const IAccessible* AccessibleItem::accessibleChild(size_t i) const
{
    TRACEFUNC;
    size_t count = 0;
    for (const EngravingObject* obj : m_element->children()) {
        if (obj->isEngravingItem()) {
            AccessibleItem* access = Ms::toEngravingItem(obj)->accessible();
            if (access && access->registred()) {
                if (count == i) {
                    return access;
                }
                ++count;
            }
        }
    }
    return nullptr;
}

IAccessible::Role AccessibleItem::accessibleRole() const
{
    return IAccessible::Role::ElementOnScore;
}

QString AccessibleItem::accessibleName() const
{
    return m_element->accessibleInfo();
}

QString AccessibleItem::accessibleDescription() const
{
    return m_element->accessibleExtraInfo();
}

bool AccessibleItem::accessibleState(State st) const
{
    if (!registred()) {
        return false;
    }

    switch (st) {
    case IAccessible::State::Enabled: return true;
    case IAccessible::State::Focused: return accessibleScore()->focusedElement() == this;
    case IAccessible::State::Selected: return m_element->selected();
    default:
        break;
    }
    return false;
}

QRect AccessibleItem::accessibleRect() const
{
    if (!registred()) {
        return QRect();
    }

    RectF bbox = m_element->bbox();
    PointF canvasPos = m_element->canvasPos();
    QRect canvasRect = QRectF(canvasPos.toQPointF(), bbox.size().toQSizeF()).toRect();

    return accessibleScore()->toScreenRect(canvasRect);
}

mu::async::Channel<IAccessible::Property> AccessibleItem::accessiblePropertyChanged() const
{
    static async::Channel<IAccessible::Property> ch;
    return ch;
}

mu::async::Channel<IAccessible::State, bool> AccessibleItem::accessibleStateChanged() const
{
    return m_accessibleStateChanged;
}
