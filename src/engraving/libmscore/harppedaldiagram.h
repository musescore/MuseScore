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

#ifndef __HARPPEDALDIAGRAM_H__
#define __HARPPEDALDIAGRAM_H__

#include "textbase.h"

namespace mu::engraving {
class HarpPedalDiagram final : public TextBase
{
    enum HarpString {
        D, C, B, E, F, G, A
    };

    enum PedalPosition {
        FLAT,
        NATURAL,
        SHARP
    };

    std::map<HarpString, PedalPosition> _pedalState;

    bool _isDiagram = true;

public:
    // Constructors - too many?
    HarpPedalDiagram();
    HarpPedalDiagram(std::map<HarpString, PedalPosition>);
    HarpPedalDiagram(PedalPosition d, PedalPosition c, PedalPosition b, PedalPosition e, PedalPosition f, PedalPosition g, PedalPosition a);

    EngravingItem* clone() const override { return new HarpPedalDiagram(*this); }
    bool isEditable() const override { return false; }

    void setIsDiagram(bool diagram) { _isDiagram = diagram; }
    bool isDiagram() { return _isDiagram; }

    std::map<HarpString, PedalPosition> getPedalState() { return _pedalState; }
    void setPedal(HarpString string, PedalPosition pedal);//TODO

private:
    friend class Factory;
};
} // namespace mu::engraving

#endif // __HARPPEDALDIAGRAM_H__
