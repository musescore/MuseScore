/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include <algorithm>

#include "export/mnxexporter.h"
#include "notationmnxwriter.h"

#include "engraving/dom/score.h"
#include "log.h"

using namespace mu::iex::mnxio;
using namespace mu::project;
using namespace muse;
using namespace muse::io;

std::vector<INotationWriter::UnitType> NotationMnxWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART };
}

bool NotationMnxWriter::supportsUnitType(UnitType unitType) const
{
    std::vector<UnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

Ret NotationMnxWriter::write(notation::INotationPtr notation, io::IODevice& destinationDevice, const Options&)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }
    mu::engraving::Score* score = notation->elements()->msScore();
    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    const bool exportBeams = mnxConfiguration()->mnxExportBeams();
    const bool exportRestPositions = mnxConfiguration()->mnxExportRestPositions();
    MnxExporter exporter(score, exportBeams, exportRestPositions);

    try {
        Ret exportResult = exporter.exportMnx();
        if (!exportResult) {
            return exportResult;
        }
        const int indentSpaces = mnxConfiguration()->mnxIndentSpaces();
        std::string json = indentSpaces >= 0
                           ? exporter.mnxDocument().root()->dump(indentSpaces)
                           : exporter.mnxDocument().root()->dump();
        ByteArray data = ByteArray::fromRawData(json.data(), json.size());
        destinationDevice.write(data);
        return muse::make_ok();
    } catch (const std::exception& ex) {
        LOGE() << String::fromStdString(ex.what());
        return make_ret(Ret::Code::InternalError);
    }

    return make_ret(Ret::Code::UnknownError);
}

Ret NotationMnxWriter::writeList(const notation::INotationPtrList&, io::IODevice&, const Options&)
{
    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}
