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

#ifndef MU_NOTATION_MSCNOTATIONWRITER_H
#define MU_NOTATION_MSCNOTATIONWRITER_H

#include "project/inotationwriter.h"

#include "engraving/infrastructure/mscio.h"

namespace mu::notation {
class MscNotationWriter : public project::INotationWriter
{
public:

    explicit MscNotationWriter(engraving::MscIoMode mode);

    std::vector<UnitType> supportedUnitTypes() const override;
    bool supportsUnitType(UnitType unitType) const override;

    muse::Ret write(notation::INotationPtr notation, muse::io::IODevice& device, const Options& options = Options()) override;
    muse::Ret writeList(const INotationPtrList& notations, muse::io::IODevice& device, const Options& options = Options()) override;

private:
    engraving::MscIoMode m_mode;
};
}

#endif // MU_NOTATION_MSCNOTATIONWRITER_H
