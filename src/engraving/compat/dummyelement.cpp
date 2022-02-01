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
#include "dummyelement.h"

#include "libmscore/factory.h"
#include "libmscore/score.h"
#include "libmscore/page.h"
#include "libmscore/system.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"

#include "accessibility/accessibleitem.h"

using namespace mu::engraving;
using namespace mu::engraving::compat;

DummyElement::DummyElement(EngravingObject* parent)
    : Ms::EngravingItem(Ms::ElementType::DUMMY, parent)
{
}

DummyElement::~DummyElement()
{
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
    setupAccessible();
    m_root = new RootItem(score());
    m_root->setParent(explicitParent());
    m_root->setupAccessible();

    m_page = Factory::createPage(m_root);
    m_page->setParent(explicitParent());

    m_system = Factory::createSystem(m_page);
    m_system->setParent(m_page);

    m_measure = Factory::createMeasure(m_system);
    m_measure->setParent(m_system);

    m_segment = Factory::createSegment(m_measure);
    m_segment->setParent(m_measure);

    m_chord = Factory::createChord(m_segment);
    m_chord->setParent(m_segment);

    m_note = Factory::createNote(m_chord);
    m_note->setParent(m_chord);
}

RootItem* DummyElement::rootItem()
{
    return m_root;
}

Ms::Page* DummyElement::page()
{
    return m_page;
}

Ms::System* DummyElement::system()
{
    return m_system;
}

Ms::Measure* DummyElement::measure()
{
    return m_measure;
}

Ms::Segment* DummyElement::segment()
{
    return m_segment;
}

Ms::Chord* DummyElement::chord()
{
    return m_chord;
}

Ms::Note* DummyElement::note()
{
    return m_note;
}

Ms::EngravingItem* DummyElement::clone() const
{
    return nullptr;
}
