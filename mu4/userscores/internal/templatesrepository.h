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

#ifndef MU_USERSCORES_TEMPLATESREPOSITORY_H
#define MU_USERSCORES_TEMPLATESREPOSITORY_H

#include "modularity/ioc.h"

#include "itemplatesrepository.h"
#include "userscores/iuserscoresconfiguration.h"
#include "domain/notation/imsczmetareader.h"
#include "system/ifsoperations.h"

namespace mu {
namespace userscores {
class TemplatesRepository : public ITemplatesRepository
{
    INJECT(userscores, IUserScoresConfiguration, configuration)
    INJECT(userscores, domain::notation::IMsczMetaReader, msczReader)
    INJECT(userscores, framework::IFsOperations, fsOperations)

public:
    RetVal<TemplateCategoryList> categories() const override;
    RetVal<domain::notation::MetaList> templatesMeta(const QString& categoryCode) const override;

private:
    bool isEmpty(const QString& dirPath) const;
    QString correctedTitle(const QString& title) const;
    QStringList templatesPaths(const QString& dirPath) const;
};
}
}

#endif // MU_USERSCORES_TEMPLATESREPOSITORY_H
