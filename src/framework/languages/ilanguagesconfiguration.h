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
#ifndef MUSE_LANGUAGES_ILANGUAGESCONFIGURATION_H
#define MUSE_LANGUAGES_ILANGUAGESCONFIGURATION_H

#include "modularity/imoduleinterface.h"
#include "types/retval.h"
#include "io/path.h"

namespace muse::languages {
class ILanguagesConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ILanguagesConfiguration)

public:
    virtual ~ILanguagesConfiguration() = default;

    virtual ValCh<QString> currentLanguageCode() const = 0;
    virtual void setCurrentLanguageCode(const QString& languageCode) const = 0;

    virtual QUrl languagesUpdateUrl() const = 0;
    virtual QUrl languageFileServerUrl(const QString& languageCode) const = 0;

    virtual io::path_t languagesAppDataPath() const = 0;
    virtual io::path_t languagesUserAppDataPath() const = 0;

    virtual io::path_t builtinLanguagesJsonPath() const = 0;
    virtual io::path_t builtinLanguageFilePath(const QString& resourceName, const QString& languageCode) const = 0;
    virtual io::path_t userLanguageFilePath(const QString& resourceName, const QString& languageCode) const = 0;
};
}

#endif // MUSE_LANGUAGES_ILANGUAGESCONFIGURATION_H
