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

using namespace mu::engraving::compat;

DummyElement::DummyElement(Ms::Score* s)
    : Ms::EngravingItem(Ms::ElementType::INVALID, s)
{
    m_page = Factory::createPage(this);
    m_system = Factory::createSystem(m_page);
    m_measure = new Ms::Measure(m_system);
    m_segment = Factory::createSegment(m_measure);
    m_chord = new Ms::Chord(m_segment);
    m_note = Factory::createNote(m_chord);

    setIsDummy(true);
    m_page->setIsDummy(true);
    m_system->setIsDummy(true);
    m_measure->setIsDummy(true);
    m_segment->setIsDummy(true);
    m_chord->setIsDummy(true);
    m_note->setIsDummy(true);
}

DummyElement::~DummyElement()
{
}

Ms::Score* DummyElement::score()
{
    return static_cast<Ms::Score*>(parent());
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
