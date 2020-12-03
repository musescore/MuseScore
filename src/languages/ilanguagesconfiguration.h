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
#ifndef MU_LANGUAGES_ILANGUAGESCONFIGURATION_H
#define MU_LANGUAGES_ILANGUAGESCONFIGURATION_H

#include "modularity/imoduleexport.h"
#include "retval.h"
#include "io/path.h"
#include "languagestypes.h"

namespace mu {
namespace languages {
class ILanguagesConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ILanguagesConfiguration)

public:
    virtual ~ILanguagesConfiguration() = default;

    virtual QString currentLanguageCode() const = 0;
    virtual Ret setCurrentLanguageCode(const QString& languageCode) const = 0;

    virtual QUrl languagesUpdateUrl() const = 0;
    virtual QUrl languageFileServerUrl(const QString& languageCode) const = 0;

    virtual ValCh<LanguagesHash> languages() const = 0;
    virtual Ret setLanguages(const LanguagesHash& languages) const = 0;

    virtual io::path languagesSharePath() const = 0;
    virtual io::path languagesDataPath() const = 0;

    virtual io::paths languageFilePaths(const QString& languageCode) const = 0;
    virtual io::path languageArchivePath(const QString& languageCode) const = 0;
};
}
}

#endif // MU_LANGUAGES_ILANGUAGESCONFIGURATION_H
