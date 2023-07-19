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

#ifndef MU_IMPORTEXPORT_MEIIMPORTER_H
#define MU_IMPORTEXPORT_MEIIMPORTER_H

#include "engraving/types/fraction.h"

#include "modularity/ioc.h"
#include "imeiconfiguration.h"

namespace mu::engraving {
class Fraction;
class Measure;
class Score;
}

namespace mu::iex::mei {
enum GraceReading {
    GraceNone = 0,
    GraceAsGrp,
    GraceAsNote
};

/**
 * Class of importing Music Encoding Initiative (MEI) files
 */

class MeiImporter
{
    INJECT_STATIC(mu::iex::mei::IMeiConfiguration, configuration)

public:
    MeiImporter(engraving::Score* s) { m_score = s; }
    bool read(const QString&);
    void convert();

    engraving::Score* m_score = nullptr;

private:
    //
};
} // namespace

#endif // MU_IMPORTEXPORT_MEIIMPORTER_H
