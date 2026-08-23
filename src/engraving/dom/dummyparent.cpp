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

#include "barline.h"
#include "bracketitem.h"
#include "chord.h"
#include "factory.h"
#include "measure.h"
#include "note.h"
#include "page.h"
#include "rootitem.h"
#include "score.h"
#include "segment.h"
#include "system.h"

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
    delete m_bracketItem;
    delete m_note;
    delete m_chord;
    delete m_segment;
    delete m_measure;
    delete m_system;
    delete m_page;
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

    // the dummy's chain is a fake hierarchy, so it deliberately bypasses the typed setters
    m_page = Factory::createPage(score());
    static_cast<EngravingItem*>(m_page)->setOwnershipParent(m_root);

    m_system = Factory::createSystem(score());
    static_cast<EngravingItem*>(m_system)->setOwnershipParent(m_page);
    m_system->setPage(m_page);

    m_measure = Factory::createMeasure(score());
    static_cast<EngravingItem*>(m_measure)->setOwnershipParent(m_system);
    m_measure->setSystem(m_system);

    m_segment = Factory::createSegment(m_measure);
    m_segment->setOwnershipParent(m_measure);

    m_chord = Factory::createChord(m_segment);
    m_chord->setOwnershipParent(m_segment);

    m_note = Factory::createNote(m_chord);
    m_note->setOwnershipParent(m_chord);

    m_bracketItem = Factory::createBracketItem(m_system);
    m_bracketItem->setOwnershipParent(m_system);
}

RootItem* DummyParent::rootItem()
{
    return m_root;
}

Page* DummyParent::page()
{
    return m_page;
}

System* DummyParent::system()
{
    return m_system;
}

Measure* DummyParent::measure()
{
    return m_measure;
}

Segment* DummyParent::segment()
{
    return m_segment;
}

Chord* DummyParent::chord()
{
    return m_chord;
}

Note* DummyParent::note()
{
    return m_note;
}

BracketItem* DummyParent::bracketItem()
{
    return m_bracketItem;
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
