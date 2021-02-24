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
#ifndef MU_LANGUAGES_ILANGUAGESSERVICE_H
#define MU_LANGUAGES_ILANGUAGESSERVICE_H

#include "modularity/imoduleexport.h"
#include "retval.h"

#include "languagestypes.h"

namespace mu::languages {
class ILanguagesService : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ILanguagesService)

public:
    virtual ~ILanguagesService() = default;

    virtual ValCh<LanguagesHash> languages() const = 0;
    virtual RetCh<LanguageProgress> install(const QString& languageCode) = 0;
    virtual RetCh<LanguageProgress> update(const QString& languageCode) = 0;
    virtual Ret uninstall(const QString& languageCode) = 0;

    virtual ValCh<Language> currentLanguage() const = 0;
    virtual Ret setCurrentLanguage(const QString& languageCode) = 0;

    virtual RetCh<Language> languageChanged() = 0;
};
}

#endif // MU_LANGUAGES_ILANGUAGESSERVICE_H
