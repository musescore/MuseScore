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
#ifndef MU_LANGUAGES_ILANGUAGEUNPACKER_H
#define MU_LANGUAGES_ILANGUAGEUNPACKER_H

#include <QString>

#include "modularity/imoduleexport.h"
#include "types/ret.h"
#include "io/path.h"

namespace mu::languages {
class ILanguageUnpacker : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ILanguageUnpacker)

public:
    virtual ~ILanguageUnpacker() = default;

    virtual Ret unpack(const QString& languageCode, const QString& source, const io::path_t& destination) const = 0;
};
}

#endif // MU_LANGUAGES_ILANGUAGEUNPACKER_H
