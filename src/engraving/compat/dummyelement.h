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

namespace Ms {
enum class Pid : int;
}

namespace mu::engraving::compat {
class DummyElement : public Ms::EngravingItem
{
public:
    DummyElement(Ms::Score* s);
    ~DummyElement();

    Ms::Page* page();
    Ms::System* system();
    Ms::Measure* measure();
    Ms::Segment* segment();
    Ms::Chord* chord();
    Ms::Note* note();

    Ms::EngravingItem* clone() const override;

    QVariant getProperty(Ms::Pid) const override { return QVariant(); }
    bool setProperty(Ms::Pid, const QVariant&) override { return false; }

private:
    Ms::Score* score();

    Ms::Page* m_page = nullptr;
    Ms::System* m_system = nullptr;
    Ms::Measure* m_measure = nullptr;
    Ms::Segment* m_segment = nullptr;
    Ms::Chord* m_chord = nullptr;
    Ms::Note* m_note = nullptr;
    std::list<Ms::EngravingObject*> m_elements;
};
}

#endif // MU_ENGRAVING_DUMMYELEMENT_H
