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

#include "global/io/file.h"

#include "export/mnxexporter.h"
#include "notationmnxwriter.h"

#include "engraving/dom/score.h"

#include "log.h"

using namespace mu::iex::mnxio;
using namespace mu::project;
using namespace muse;
using namespace muse::io;

std::vector<WriteUnitType> NotationMnxWriter::supportedUnitTypes() const
{
    return { WriteUnitType::PER_PART };
}

bool NotationMnxWriter::supportsUnitType(WriteUnitType unitType) const
{
    std::vector<WriteUnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

Ret NotationMnxWriter::write(INotationProjectPtr project, muse::io::IODevice& destinationDevice, const WriteOptions& /*options*/)
{
    IF_ASSERT_FAILED(project) {
        return make_ret(Ret::Code::UnknownError);
    }

    mu::engraving::Score* score = project->masterNotation()->notation()->elements()->msScore();
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
}

Ret NotationMnxWriter::write(INotationProjectPtr project, const muse::io::path_t& filePath, const WriteOptions& options)
{
    muse::io::File file(filePath);
    if (!file.open(IODevice::WriteOnly)) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ret ret = write(project, file, options);
    file.close();
    return ret;
}
