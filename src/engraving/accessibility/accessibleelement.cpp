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
#include "accessibleelement.h"

#include "async/async.h"
#include "accessiblescore.h"
#include "log.h"

using namespace mu::engraving;
using namespace mu::accessibility;

AccessibleElement::AccessibleElement(Ms::Element* e)
{
    setElement(e);
}

AccessibleElement::~AccessibleElement()
{
    AccessibleScore* ascore = accessibleScore();
    m_element = nullptr;

    if (!ascore) {
        return;
    }

    if (m_registred) {
        ascore->removeChild(this);
        m_registred = false;
    }

    if (ascore->focusedElement() == this) {
        ascore->setFocusedElement(nullptr);
    }
}

AccessibleElement* AccessibleElement::clone(Ms::Element* e) const
{
    return new AccessibleElement(e);
}

bool AccessibleElement::isAvalaible() const
{
    if (!m_element) {
        return false;
    }

    Ms::Score* s = m_element->score();
    if (!s) {
        return false;
    }

    //! NOTE Disabled for not score elements and palettes
    if (s->isPaletteScore()) {
        return false;
    }

    if (!s->accessible()) {
        return false;
    }
    return true;
}

AccessibleScore* AccessibleElement::accessibleScore() const
{
    if (!m_element) {
        return nullptr;
    }

    if (!m_element->score()) {
        return nullptr;
    }

    return m_element->score()->accessible();
}

void AccessibleElement::setElement(Ms::Element* e)
{
    AccessibleScore* ascore = accessibleScore();
    if (ascore && m_element) {
        ascore->removeChild(this);
    }

    m_element = e;

    if (isAvalaible()) {
        async::Async::call(this, [this]() {
            if (!m_element->isNote()) {
                return;
            }

            accessibleScore()->addChild(this);
        });
    }
}

const Ms::Element* AccessibleElement::element() const
{
    return m_element;
}

void AccessibleElement::focused()
{
    LOGI() << accessibleName();
    AccessibleScore* ascore = accessibleScore();
    if (!ascore) {
        return;
    }

    ascore->setActive(true);
    ascore->setFocusedElement(this);
    m_accessibleStateChanged.send(IAccessible::State::Focused, true);
}

const IAccessible* AccessibleElement::accessibleParent() const
{
    return accessibleScore();

    //! TODO Not completed, please don't remove (igor.korsukov@gmail.com)
    //    Ms::ScoreElement* treeParent = m_element->treeParent();
    //    if (!treeParent) {
    //        return nullptr;
    //    }

    //    if (treeParent == s || !treeParent->isElement()) {
    //        //! TODO Add Accessible Score
    //        return accessibilityController()->accessibleRoot();
    //    }

    //    Ms::Element* p = static_cast<Ms::Element*>(treeParent);
    //    return p->accessible();
}

size_t AccessibleElement::accessibleChildCount() const
{
    return 0;
    //! TODO Not completed, please don't remove (igor.korsukov@gmail.com)
    //    size_t count = static_cast<size_t>(m_element->treeChildCount());
    //    LOGI() << "count: " << count;
    //    return count;
}

const IAccessible* AccessibleElement::accessibleChild(size_t) const
{
    return nullptr;
    //! TODO Not completed, please don't remove (igor.korsukov@gmail.com)
    //    Ms::ScoreElement* se = m_element->treeChild(static_cast<int>(i));
    //    if (!se || !se->isElement()) {
    //        return nullptr;
    //    }
    //    Ms::Element* p = static_cast<Ms::Element*>(se);
    //    return p->accessible();
}

IAccessible::Role AccessibleElement::accessibleRole() const
{
    return IAccessible::Role::ElementOnScore;
}

QString AccessibleElement::accessibleName() const
{
    return m_element->accessibleInfo();
}

QString AccessibleElement::accessibleDescription() const
{
    return m_element->accessibleExtraInfo();
}

bool AccessibleElement::accessibleState(State st) const
{
    if (!isAvalaible()) {
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

QRect AccessibleElement::accessibleRect() const
{
    if (!isAvalaible()) {
        return QRect();
    }

    RectF bbox = m_element->bbox();
    PointF canvasPos = m_element->canvasPos();
    QRect canvasRect = QRectF(canvasPos.toQPointF(), bbox.size().toQSizeF()).toRect();

    return accessibleScore()->toScreenRect(canvasRect);
}

mu::async::Channel<IAccessible::Property> AccessibleElement::accessiblePropertyChanged() const
{
    static async::Channel<IAccessible::Property> ch;
    return ch;
}

mu::async::Channel<IAccessible::State, bool> AccessibleElement::accessibleStateChanged() const
{
    return m_accessibleStateChanged;
}
