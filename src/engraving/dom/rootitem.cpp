/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "rootitem.h"

#include "page.h"
#include "score.h"

#ifndef ENGRAVING_NO_ACCESSIBILITY
#include "../accessibility/accessibleroot.h"
#endif

using namespace mu::engraving;

RootItem::RootItem(Score* score)
    : EngravingItem(ElementType::ROOT_ITEM, score)
{
    m_dummy = new compat::DummyElement(this);
}

RootItem::~RootItem()
{
    compat::DummyElement* d = m_dummy;
    m_dummy = nullptr;
    delete d;
}

compat::DummyElement* RootItem::dummy() const
{
    return m_dummy;
}

EngravingItemList RootItem::accessibleChildren() const
{
    EngravingItemList children = EngravingItem::accessibleChildren();

    if (this == score()->rootItem()) {
        // The pages are owned by the score, but this is the top of the accessibility
        // tree, so this is where they are listed. (The dummy has a root item of its
        // own, which heads the tree of the items parked on it and has no pages.)
        children.insert(children.end(), score()->pages().begin(), score()->pages().end());
    }

    return children;
}

void RootItem::init()
{
#ifndef ENGRAVING_NO_ACCESSIBILITY
    setupAccessible();
#endif

    m_dummy->setOwnershipParent(this);
    m_dummy->init();
}

#ifndef ENGRAVING_NO_ACCESSIBILITY
AccessibleItemPtr RootItem::createAccessible()
{
    return std::make_shared<AccessibleRoot>(this, AccessibleItem::Group);
}

#endif
