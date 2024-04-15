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

#include "meiwriter.h"

#include "meiexporter.h"

#include "io/file.h"
#include "log.h"

#include "engraving/engravingerrors.h"
#include "engraving/dom/score.h"

using namespace mu;
using namespace muse;
using namespace mu::iex::mei;
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

Ret MeiWriter::write(notation::INotationPtr notation, io::IODevice& destinationDevice, const Options&)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    mu::engraving::Score* score = notation->elements()->msScore();
    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    MeiExporter exporter(score);
    std::string meiData;
    if (exporter.write(meiData)) {
        ByteArray data = ByteArray::fromRawData(meiData.c_str(), meiData.size());
        destinationDevice.write(data);
        return muse::make_ok();
    } else {
        return make_ret(Ret::Code::UnknownError);
    }
}

mu::engraving::Err MeiWriter::writeScore(mu::engraving::Score* score, const muse::io::path_t& path)
{
    MeiExporter exporter(score);
    // Force no layout option in this case
    exporter.configuration()->setMeiExportLayout(false);
    std::string meiData;
    if (exporter.write(meiData) && io::File::writeFile(path, ByteArray(meiData.c_str(), meiData.size()))) {
        return engraving::Err::NoError;
    } else {
        return engraving::Err::UnknownError;
    }
}

Ret MeiWriter::writeList(const notation::INotationPtrList&, io::IODevice&, const Options&)
{
    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}
