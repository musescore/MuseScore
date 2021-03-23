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

#include "io/path.h"

using namespace mu;
using namespace mu::notation;
using namespace mu::userscores;
using namespace mu::framework;

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
