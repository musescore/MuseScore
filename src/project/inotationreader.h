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
#ifndef MU_PROJECT_INOTATIONREADER_H
#define MU_PROJECT_INOTATIONREADER_H

#include <memory>
#include <map>

#include "types/ret.h"
#include "types/val.h"
#include "io/path.h"

namespace mu::engraving {
class MasterScore;
}

namespace mu::project {
class INotationReader
{
public:
    enum class OptionKey {
        ForceMode // Ignore lower priority errors (like version errors)
    };

    using Options = std::map<OptionKey, muse::Val>;

    virtual ~INotationReader() = default;

    virtual muse::Ret read(mu::engraving::MasterScore* score, const muse::io::path_t& path, const Options& options = Options()) = 0;
};

using INotationReaderPtr = std::shared_ptr<INotationReader>;
}

#endif // MU_PROJECT_INOTATIONREADER_H
