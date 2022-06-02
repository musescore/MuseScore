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
#ifndef MU_ENGRAVING_DUMMYELEMENT_H
#define MU_ENGRAVING_DUMMYELEMENT_H

#include <list>
#include <QVariant>
#include "libmscore/engravingitem.h"

namespace mu::engraving {
enum class Pid : int;
}

namespace mu::engraving {
class RootItem;
}

namespace mu::engraving::compat {
class DummyElement : public mu::engraving::EngravingItem
{
public:
    DummyElement(EngravingObject* parent);
    ~DummyElement();

    void init();

    RootItem* rootItem();
    mu::engraving::Page* page();
    mu::engraving::System* system();
    mu::engraving::Measure* measure();
    mu::engraving::Segment* segment();
    mu::engraving::Chord* chord();
    mu::engraving::Note* note();

    mu::engraving::EngravingItem* clone() const override;

    mu::engraving::PropertyValue getProperty(mu::engraving::Pid) const override { return mu::engraving::PropertyValue(); }
    bool setProperty(mu::engraving::Pid, const mu::engraving::PropertyValue&) override { return false; }

private:
    mu::engraving::AccessibleItem* createAccessible() override;

    RootItem* m_root = nullptr;
    mu::engraving::Page* m_page = nullptr;
    mu::engraving::System* m_system = nullptr;
    mu::engraving::Measure* m_measure = nullptr;
    mu::engraving::Segment* m_segment = nullptr;
    mu::engraving::Chord* m_chord = nullptr;
    mu::engraving::Note* m_note = nullptr;
};
}

#endif // MU_ENGRAVING_DUMMYELEMENT_H
