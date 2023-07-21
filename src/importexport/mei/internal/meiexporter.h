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

#ifndef MU_IMPORTEXPORT_MEIEXPORTER_H
#define MU_IMPORTEXPORT_MEIEXPORTER_H

#include "engraving/types/fraction.h"

#include "modularity/ioc.h"
#include "imeiconfiguration.h"

#include "thirdparty/pugixml.hpp"

namespace mu::engraving {
class Fraction;
class Measure;
class Score;
class Staff;
}

namespace mu::iex::mei {
/**
 * Class of exporting Music Encoding Initiative (MEI) files
 */

class MeiExporter
{
    INJECT_STATIC(mu::iex::mei::IMeiConfiguration, configuration)

public:
    MeiExporter(engraving::Score* s) { m_score = s; }
    bool write(QIODevice& destinationDevice);

private:
    /** The Score pointer */
    engraving::Score* m_score = nullptr;

    /** MEI xml element */
    pugi::xml_node m_mei;
};
} // namespace

#endif // MU_IMPORTEXPORT_MEIEXPORTER_H
