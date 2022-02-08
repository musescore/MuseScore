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
    virtual ValCh<Language> currentLanguage() const = 0;

    virtual RetCh<LanguageProgress> install(const QString& languageCode) = 0;
    virtual RetCh<LanguageProgress> update(const QString& languageCode) = 0;
    virtual Ret uninstall(const QString& languageCode) = 0;
};
}

#endif // MU_LANGUAGES_ILANGUAGESSERVICE_H
