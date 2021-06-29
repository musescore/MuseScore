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

#include "templatesrepository.h"

#include "log.h"

#include "io/path.h"

using namespace mu;
using namespace mu::notation;
using namespace mu::userscores;

RetVal<Templates> TemplatesRepository::templates() const
{
    Templates result;

    for (const io::path& dirPath: configuration()->availableTemplatesPaths()) {
        QStringList filters { "*.mscz", "*.mscx" };
        RetVal<io::paths> files = fileSystem()->scanFiles(dirPath, filters);

        if (!files.ret) {
            LOGE() << files.ret.toString();
            continue;
        }

        result << loadTemplates(files.val);
    }

    return RetVal<Templates>::make_ok(result);
}

Templates TemplatesRepository::loadTemplates(const io::paths& filePaths) const
{
    Templates result;
    MetaList metaList = msczReader()->readMetaList(filePaths);

    for (const Meta& meta: metaList) {
        Template templ(meta);
        templ.categoryTitle = correctedTitle(io::dirname(meta.filePath).toQString());

        result << templ;
    }

    return result;
}

QString TemplatesRepository::correctedTitle(const QString& title) const
{
    QString corrected = title;

    if (!corrected.isEmpty() && corrected[0].isNumber()) {
        constexpr int NUMBER_LENGTH = 3;
        corrected = corrected.mid(NUMBER_LENGTH);
    }

    return corrected.replace('_', ' ');
}
