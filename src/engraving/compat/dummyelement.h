/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_ENGRAVING_DUMMYELEMENT_H
#define MU_ENGRAVING_DUMMYELEMENT_H

#include "../dom/engravingitem.h"

namespace mu::engraving {
enum class Pid : int;

class RootItem;
}

namespace mu::engraving::compat {
class DummyElement : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, DummyElement)
public:
    DummyElement(EngravingObject* parent);
    ~DummyElement();

    void init();

    RootItem* rootItem();
    Page* page();
    System* system();
    Measure* measure();
    Segment* segment();
    Chord* chord();
    Note* note();
    BracketItem* bracketItem();

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
    BracketItem* m_bracketItem = nullptr;
};
}

#endif // MU_ENGRAVING_DUMMYELEMENT_H
