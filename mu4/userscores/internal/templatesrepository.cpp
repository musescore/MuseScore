//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "templatesrepository.h"

#include "log.h"

using namespace mu;
using namespace mu::notation;
using namespace mu::userscores;
using namespace mu::framework;

RetVal<TemplateCategoryList> TemplatesRepository::categories() const
{
    TemplateCategoryList result;

    for (const io::path& dirPath : configuration()->templatesDirPaths()) {
        if (isEmpty(dirPath)) {
            continue;
        }

        TemplateCategory category;

        category.codeKey = dirPath.toQString();
        category.title = correctedTitle(io::dirname(dirPath).toQString());

        result << category;
    }

    return RetVal<TemplateCategoryList>::make_ok(result);
}

RetVal<MetaList> TemplatesRepository::templatesMeta(const QString& categoryCode) const
{
    MetaList result;
    io::paths templates = templatesPaths(categoryCode);

    for (const io::path& path : templates) {
        RetVal<Meta> meta = msczReader()->readMeta(path);

        if (!meta.ret) {
            LOGE() << meta.ret.toString();
            continue;
        }

        result << meta.val;
    }

    return RetVal<MetaList>::make_ok(result);
}

bool TemplatesRepository::isEmpty(const io::path& dirPath) const
{
    return templatesPaths(dirPath).empty();
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

io::paths TemplatesRepository::templatesPaths(const io::path& dirPath) const
{
    QStringList filters { "*.mscz", "*.mscx" };
    RetVal<io::paths> result = fileSystem()->scanFiles(dirPath, filters, IFileSystem::ScanMode::IncludeSubdirs);

    if (!result.ret) {
        LOGE() << result.ret.toString();
    }

    return result.val;
}
