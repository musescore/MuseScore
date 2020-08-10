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
using namespace mu::domain::notation;
using namespace mu::userscores;
using namespace mu::framework;

RetVal<TemplateCategoryList> TemplatesRepository::categories() const
{
    TemplateCategoryList result;

    for (const QString& dirPath: configuration()->templatesDirPaths()) {
        if (isEmpty(dirPath)) {
            continue;
        }

        TemplateCategory category;

        category.codeKey = dirPath;
        category.title = correctedTitle(fsOperations()->dirName(dirPath));

        result << category;
    }

    return RetVal<TemplateCategoryList>::make_ok(result);
}

RetVal<MetaList> TemplatesRepository::templatesMeta(const QString& categoryCode) const
{
    MetaList result;
    QStringList templates = templatesPaths(categoryCode);

    for (const QString& path: templates) {
        RetVal<Meta> meta = msczReader()->readMeta(io::pathFromQString(path));

        if (!meta.ret) {
            LOGE() << meta.ret.toString();
            continue;
        }

        result << meta.val;
    }

    return RetVal<MetaList>::make_ok(result);
}

bool TemplatesRepository::isEmpty(const QString& dirPath) const
{
    return templatesPaths(dirPath).isEmpty();
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

QStringList TemplatesRepository::templatesPaths(const QString& dirPath) const
{
    QStringList filters { "*.mscz", "*.mscx" };
    RetVal<QStringList> result = fsOperations()->scanFiles(dirPath, filters, IFsOperations::ScanMode::IncludeSubdirs);

    if (!result.ret) {
        LOGE() << result.ret.toString();
    }

    return result.val;
}
