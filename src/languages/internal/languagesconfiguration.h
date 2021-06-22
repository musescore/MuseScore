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
#ifndef MU_LANGUAGES_LANGUAGESCONFIGURATION_H
#define MU_LANGUAGES_LANGUAGESCONFIGURATION_H

#include "modularity/ioc.h"
#include "ilanguagesconfiguration.h"
#include "iglobalconfiguration.h"
#include "framework/system/ifilesystem.h"
#include "multiinstances/imultiinstancesprovider.h"

namespace mu::languages {
class LanguagesConfiguration : public ILanguagesConfiguration
{
    INJECT(languages, framework::IGlobalConfiguration, globalConfiguration)
    INJECT(languages, system::IFileSystem, fileSystem)
    INJECT(languages, mi::IMultiInstancesProvider, multiInstancesProvider)

public:
    void init();

    ValCh<QString> currentLanguageCode() const override;
    void setCurrentLanguageCode(const QString& languageCode) const override;

    QUrl languagesUpdateUrl() const override;
    QUrl languageFileServerUrl(const QString& languageCode) const override;

    ValCh<LanguagesHash> languages() const override;
    Ret setLanguages(const LanguagesHash& languages) override;

    io::path languagesAppDataPath() const;
    io::path languagesUserAppDataPath() const override;

    io::paths languageFilePaths(const QString& languageCode) const override;
    io::path languageArchivePath(const QString& languageCode) const override;

private:
    LanguagesHash parseLanguagesConfig(const QByteArray& json) const;
    io::path languageFileName(const QString& languageCode) const;

    RetVal<QByteArray> readLanguagesState() const;
    Ret writeLanguagesState(const QByteArray& data);

    async::Channel<QString> m_currentLanguageCodeChanged;
    async::Channel<LanguagesHash> m_languagesHashChanged;
};
}

#endif // MU_LANGUAGES_LANGUAGESCONFIGURATION_H
