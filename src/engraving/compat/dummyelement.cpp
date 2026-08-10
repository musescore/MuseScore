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
#include "dummyelement.h"

#include "dom/barline.h"
#include "dom/factory.h"
#include "dom/score.h"
#include "dom/page.h"
#include "dom/system.h"
#include "dom/measure.h"
#include "dom/segment.h"
#include "dom/chord.h"
#include "dom/note.h"
#include "dom/bracketitem.h"

#ifndef ENGRAVING_NO_ACCESSIBILITY
#include "accessibility/accessibleitem.h"
#endif

using namespace mu::engraving;
using namespace mu::engraving::compat;

DummyElement::DummyElement(EngravingObject* parent)
    : EngravingItem(ElementType::DUMMY, parent)
{
}

DummyElement::~DummyElement()
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

void DummyElement::init()
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

RootItem* DummyElement::rootItem()
{
    return m_root;
}

Page* DummyElement::page()
{
    return m_page;
}

System* DummyElement::system()
{
    return m_system;
}

Measure* DummyElement::measure()
{
    return m_measure;
}

Segment* DummyElement::segment()
{
    return m_segment;
}

Chord* DummyElement::chord()
{
    return m_chord;
}

Note* DummyElement::note()
{
    return m_note;
}

BracketItem* DummyElement::bracketItem()
{
    return m_bracketItem;
}

EngravingItem* DummyElement::clone() const
{
    return nullptr;
}

#ifndef ENGRAVING_NO_ACCESSIBILITY
AccessibleItemPtr DummyElement::createAccessible()
{
    using namespace muse::accessibility;
    return std::make_shared<AccessibleItem>(this, IAccessible::Group);
}

#endif
