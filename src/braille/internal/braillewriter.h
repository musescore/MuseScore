/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

#ifndef MU_BRAILLE_BRAILLEWRITER_H
#define MU_BRAILLE_BRAILLEWRITER_H

#include "project/iprojectwriter.h"

namespace mu::engraving {
class BrailleWriter : public project::IProjectWriter
{
public:
    std::vector<project::WriteUnitType> supportedUnitTypes() const override;
    bool supportsUnitType(project::WriteUnitType unitType) const override;

    muse::Ret write(project::INotationProjectPtr project, muse::io::IODevice& dstDevice,
                    const project::WriteOptions& options = project::WriteOptions()) override;
    muse::Ret write(project::INotationProjectPtr project, const muse::io::path_t& filePath,
                    const project::WriteOptions& options = project::WriteOptions()) override;
};
}

#endif // MU_BRAILLE_BRAILLEWRITER_H
