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
#ifndef MU_ENGRAVING_READ302_H
#define MU_ENGRAVING_READ302_H

#include "libmscore/masterscore.h"

namespace Ms {
class XmlReader;
}

namespace mu::engraving::compat {
class Read302
{
public:

    static Ms::Score::FileError read302(Ms::MasterScore* masterScore, Ms::XmlReader& e, ReadContext& ctx);

private:
    static bool readScore302(Ms::Score* score, Ms::XmlReader& e, ReadContext& ctx);
};
}

#endif // MU_ENGRAVING_READ302_H
