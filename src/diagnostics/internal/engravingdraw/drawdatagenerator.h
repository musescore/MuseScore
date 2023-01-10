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
#ifndef MU_DIAGNOSTICS_DRAWDATAGENERATOR_H
#define MU_DIAGNOSTICS_DRAWDATAGENERATOR_H

#include "global/types/ret.h"
#include "global/io/path.h"

namespace mu::engraving {
class MasterScore;
}

namespace mu::diagnostics {
class DrawDataGenerator
{
public:
    DrawDataGenerator() = default;

    Ret processDir(const io::path_t& scoreDir, const io::path_t& outDir, const io::path_t& ignoreFile);

private:
    void processFile(const io::path_t& scorePath, const io::path_t& outDir);

    std::vector<std::string> loadIgnore(const mu::io::path_t& ignoreFile) const;
    bool loadScore(engraving::MasterScore* score, const io::path_t& path);
};
}

#endif // MU_DIAGNOSTICS_DRAWDATAGENERATOR_H
