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

#ifndef MU_ENGRAVING_PAUSEMAP_H
#define MU_ENGRAVING_PAUSEMAP_H

#include <map>
#include <memory>

/**
 \file
 Definition of class PauseMap.
*/

namespace mu::engraving {
class Score;
class TempoMap;

//---------------------------------------------------
//   PauseMap
//    MIDI files cannot contain pauses so need to insert
//    extra ticks extra ticks and tempo changes instead.
//---------------------------------------------------
class PauseMap : std::map<int, int>
{
public:
    void calculate(const engraving::Score* s);
    inline int tickWithPauses(int utick) const { return utick + offsetAtUTick(utick); }
    const engraving::TempoMap* tempomapWithPauses() const { return m_tempomapWithPauses.get(); }

private:
    int offsetAtUTick(int utick) const;

    std::shared_ptr<engraving::TempoMap> m_tempomapWithPauses;
};
} // namespace mu::engraving
#endif
