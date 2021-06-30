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

    ValCh<Language> currentLanguage() const override;

    RetCh<Language> languageChanged() override;
};
}

#endif // MU_LANGUAGES_LANGUAGESSERVICESTUB_H
