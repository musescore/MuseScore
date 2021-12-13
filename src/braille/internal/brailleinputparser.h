/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#ifndef MU_BRAILLE_BRAILLEINPUTPARSER_H
#define MU_BRAILLE_BRAILLEINPUTPARSER_H

#include <map>
#include <string>
#include <vector>

namespace mu::engraving {
class braille_code;

struct BiePattern {
    std::string name;
    std::vector<braille_code*> codes;
    bool mandatory = false;
};

enum class BieSequencePatternType {
    Undefined,
    Note,
    Interval,
    Rest,
    Tuplet3,
    Tuplet,
    Dot,
    Tie,
    NoteSlur,
    LongSlurStop
};

class BieSequencePattern
{
public:
    BieSequencePattern(BieSequencePatternType t, std::string sequence);
    ~BieSequencePattern();

    BieSequencePatternType type() const;
    bool recognize(std::string braille);
    const std::map<std::string, braille_code*>& res() const;
    braille_code* res(std::string key);
    bool valid() const;
private:
    BieSequencePatternType _type;
    std::vector<BiePattern> patterns;
    bool _valid;
    std::map<std::string, braille_code*> _res;
    int max_cell_length;
    int _mandatories = 0;
};

BieSequencePattern* BieRecognize(std::string braille, bool tuplet_indicator);
}

#endif // MU_BRAILLE_BRAILLEINPUTPARSER_H
