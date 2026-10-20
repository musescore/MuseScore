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

using namespace mu::engraving;

DummyParent::DummyParent(Score* score)
    : EngravingObject(ElementType::DUMMY, score)
{
}

DummyParent::~DummyParent()
{
    delete m_root;
}

void DummyParent::init()
{
    m_root = new RootItem(score(), RootItem::Kind::Dummy);

#ifndef ENGRAVING_NO_ACCESSIBILITY
    m_root->setupAccessible();
#endif
}
