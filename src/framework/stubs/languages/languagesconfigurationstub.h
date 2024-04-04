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
#ifndef MU_LANGUAGES_LANGUAGESCONFIGURATIONSTUB_H
#define MU_LANGUAGES_LANGUAGESCONFIGURATIONSTUB_H

#include "languages/ilanguagesconfiguration.h"

namespace muse::languages {
class LanguagesConfigurationStub : public ILanguagesConfiguration
{
public:
    ValCh<QString> currentLanguageCode() const override;
    void setCurrentLanguageCode(const QString& languageCode) const override;

    QUrl languagesUpdateUrl() const override;
    QUrl languageFileServerUrl(const QString& languageCode) const override;

    io::path_t languagesAppDataPath() const override;
    io::path_t languagesUserAppDataPath() const override;

    io::path_t builtinLanguagesJsonPath() const override;
    io::path_t builtinLanguageFilePath(const QString& resourceName, const QString& languageCode) const override;
    io::path_t userLanguageFilePath(const QString& resourceName, const QString& languageCode) const override;
};
}

#endif // MU_LANGUAGES_LANGUAGESCONFIGURATIONSTUB_H
