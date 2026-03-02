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
#include "engravingpluginapihelper.h"

#include "types/projecttypes.h"
#include "types/projecttypes.h"

#include "log.h"

using namespace mu::project;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;

bool EngravingPluginAPIHelper::writeScore(const QString& name, const QString& ext)
{
    const INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        LOGW() << "No notation found";
        return false;
    }

    const auto unitType = determineWriterUnitType(ext.toStdString());
    if (!unitType) {
        LOGW() << "'" << ext << "' format is not supported";
        return false;
    }

    const QString outPath = name.endsWith(ext) ? name : (name + '.' + ext);
    return exportProjectScenario()->exportScores({ notation }, outPath, *unitType, /* openDestinationFolderOnExport */ false);
}

Score* EngravingPluginAPIHelper::readScore(const QString& name)
{
    const muse::io::path_t path(name);
    const ProjectFile file(path);
    const Ret ret = projectFilesController()->openProject(file);

    if (ret.success() && globalContext()->currentNotation()) {
        return globalContext()->currentNotation()->elements()->msScore();
    }
    return nullptr;
}

void EngravingPluginAPIHelper::closeScore()
{
    projectFilesController()->closeOpenedProject();
}

std::optional<WriteUnitType> EngravingPluginAPIHelper::determineWriterUnitType(const std::string& ext) const
{
    auto projectWriter = projectRW()->writer(ext);
    if (!projectWriter) {
        return std::nullopt;
    }

    if (projectWriter->supportsUnitType(WriteUnitType::PER_PAGE)) {
        return WriteUnitType::PER_PAGE;
    } else if (projectWriter->supportsUnitType(WriteUnitType::PER_PART)) {
        return WriteUnitType::PER_PART;
    } else {
        return WriteUnitType::MULTI_PART;
    }
}
