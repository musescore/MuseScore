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
#pragma once

#include "engravingitem.h"

namespace mu::engraving {
enum class Pid : short;

class RootItem;

//! The parent an object has while it is not attached to anything. Nothing is laid
//! out in it and it takes no part in the score; it only keeps unattached objects
//! reachable, and heads their accessibility tree.
class DummyParent : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, DummyParent)
public:
    DummyParent(EngravingObject* parent);
    ~DummyParent();

    void init();

    RootItem* rootItem();
    Page* page();
    System* system();
    Measure* measure();
    Segment* segment();
    Chord* chord();
    Note* note();

    EngravingItem* clone() const override;

    PropertyValue getProperty(Pid) const override { return PropertyValue(); }
    bool setProperty(Pid, const PropertyValue&) override { return false; }

private:
#ifndef ENGRAVING_NO_ACCESSIBILITY
    AccessibleItemPtr createAccessible() override;
#endif

    RootItem* m_root = nullptr;
    Page* m_page = nullptr;
    System* m_system = nullptr;
    Measure* m_measure = nullptr;
    Segment* m_segment = nullptr;
    Chord* m_chord = nullptr;
    Note* m_note = nullptr;
};
}
