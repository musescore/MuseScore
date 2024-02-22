/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#include <string>

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

    BieSequencePatternType type();
    bool recognize(std::string braille);
    std::map<std::string, braille_code*> res();
    braille_code* res(std::string key);
    bool valid();
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
