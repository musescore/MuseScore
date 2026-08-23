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
#include "dummyparent.h"

#include "rootitem.h"
#include "score.h"

#ifndef ENGRAVING_NO_ACCESSIBILITY
#include "../accessibility/accessibleitem.h"
#endif

using namespace mu::engraving;

DummyParent::DummyParent(EngravingObject* parent)
    : EngravingItem(ElementType::DUMMY, parent)
{
}

DummyParent::~DummyParent()
{
    delete m_root;
}

void DummyParent::init()
{
#ifndef ENGRAVING_NO_ACCESSIBILITY
    setupAccessible();
#endif

    m_root = new RootItem(score());
    m_root->setOwnershipParent(this);

#ifndef ENGRAVING_NO_ACCESSIBILITY
    m_root->setupAccessible();
#endif
}

RootItem* DummyParent::rootItem()
{
    return m_root;
}

EngravingItem* DummyParent::clone() const
{
    return nullptr;
}

#ifndef ENGRAVING_NO_ACCESSIBILITY
AccessibleItemPtr DummyParent::createAccessible()
{
    using namespace muse::accessibility;
    return std::make_shared<AccessibleItem>(this, IAccessible::Group);
}

#endif
