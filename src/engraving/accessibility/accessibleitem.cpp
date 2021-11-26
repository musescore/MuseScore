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

#include "accessibleroot.h"
#include "../libmscore/score.h"

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
    AccessibleRoot* root = accessibleRoot();
    if (root && root->focusedElement() == this) {
        root->setFocusedElement(nullptr);
    }

    if (m_registred && accessibilityController()) {
        accessibilityController()->unreg(this);
        m_registred = false;
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

AccessibleRoot* AccessibleItem::accessibleRoot() const
{
    if (!m_element) {
        return nullptr;
    }

    Ms::Score* score = m_element->score();
    if (!score) {
        return nullptr;
    }

    RootItem* rootItem = m_element->explicitParent() ? score->rootItem() : score->dummy()->rootItem();
    return dynamic_cast<AccessibleRoot*>(rootItem->accessible());
}

const Ms::EngravingItem* AccessibleItem::element() const
{
    return m_element;
}

bool AccessibleItem::registered() const
{
    return m_registred;
}

void AccessibleItem::notifyAboutFocus(bool focused)
{
    m_accessibleStateChanged.send(IAccessible::State::Focused, focused);
}

const IAccessible* AccessibleItem::accessibleParent() const
{
    Ms::EngravingObject* p = m_element->parent();
    if (!p || !p->isEngravingItem()) {
        return nullptr;
    }

    return static_cast<EngravingItem*>(p)->accessible();
}

size_t AccessibleItem::accessibleChildCount() const
{
    TRACEFUNC;
    size_t count = 0;
    for (const EngravingObject* obj : m_element->children()) {
        if (obj->isEngravingItem()) {
            AccessibleItem* access = Ms::toEngravingItem(obj)->accessible();
            if (access && access->registered()) {
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

QVariant AccessibleItem::accesibleValue() const
{
    return QVariant();
}

QVariant AccessibleItem::accesibleMaximumValue() const
{
    return QVariant();
}

QVariant AccessibleItem::accesibleMinimumValue() const
{
    return QVariant();
}

QVariant AccessibleItem::accesibleValueStepSize() const
{
    return QVariant();
}

bool AccessibleItem::accessibleState(State st) const
{
    if (!registered()) {
        return false;
    }

    switch (st) {
    case IAccessible::State::Enabled: return true;
    case IAccessible::State::Active: return true;
    case IAccessible::State::Focused: {
        auto root = accessibleRoot();
        return root ? root->focusedElement() == this : false;
    }
    case IAccessible::State::Selected: return m_element->selected();
    default:
        break;
    }
    return false;
}

QRect AccessibleItem::accessibleRect() const
{
    if (!registered()) {
        return QRect();
    }

    RectF bbox = m_element->bbox();
    PointF canvasPos = m_element->canvasPos();
    QRect canvasRect = QRectF(canvasPos.toQPointF(), bbox.size().toQSizeF()).toRect();

    return accessibleRoot()->toScreenRect(canvasRect);
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
