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
#ifndef MU_BRAILLE_IBRAILLECONVERTER_H
#define MU_BRAILLE_IBRAILLECONVERTER_H

#include <QIODevice>

#include "modularity/imoduleinterface.h"

namespace mu::engraving {
class Score;
}

namespace mu::braille {
class IBrailleConverter : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IBrailleConverter)

public:
    virtual ~IBrailleConverter() = default;

    virtual bool write(engraving::Score* score, QIODevice& device) = 0;
};
}

#endif // MU_BRAILLE_IBRAILLECONVERTER_H
