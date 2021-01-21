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
#ifndef MU_LANGUAGES_LANGUAGESSERVICESTUB_H
#define MU_LANGUAGES_LANGUAGESSERVICESTUB_H

#include "languages/ilanguagesservice.h"

namespace mu::languages {
class LanguagesServiceStub : public ILanguagesService
{
public:
    ValCh<LanguagesHash> languages() const override;
    RetCh<LanguageProgress> install(const QString& languageCode) override;
    RetCh<LanguageProgress> update(const QString& languageCode) override;
    Ret uninstall(const QString& languageCode) override;

    RetVal<Language> currentLanguage() const override;
    Ret setCurrentLanguage(const QString& languageCode) override;

    RetCh<Language> languageChanged() override;
};
}

#endif // MU_LANGUAGES_LANGUAGESSERVICESTUB_H
