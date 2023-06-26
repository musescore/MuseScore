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

#include "meiwriter.h"

#include "meiimporter.h"

#include "log.h"

#include "libmscore/masterscore.h"
#include "engraving/engravingerrors.h"

using namespace mu;
using namespace mu::iex::mei;
using namespace mu::notation;
using namespace mu::project;

std::vector<INotationWriter::UnitType> MeiWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART };
}

bool MeiWriter::supportsUnitType(UnitType unitType) const
{
    std::vector<UnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

mu::Ret MeiWriter::write(notation::INotationPtr notation, QIODevice& destinationDevice, const Options&)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    mu::engraving::Score* score = notation->elements()->msScore();
    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    MeiExporter exporter(score);
    return exporter.write(destinationDevice);
}

mu::Ret MeiWriter::writeList(const notation::INotationPtrList&, QIODevice&, const Options&)
{
    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}
