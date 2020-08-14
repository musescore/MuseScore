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
#ifndef MU_LANGUAGES_LANGUAGESCONFIGURATION_H
#define MU_LANGUAGES_LANGUAGESCONFIGURATION_H

#include "modularity/ioc.h"
#include "ilanguagesconfiguration.h"
#include "iglobalconfiguration.h"
#include "framework/system/ifsoperations.h"

namespace mu {
namespace languages {
class LanguagesConfiguration : public ILanguagesConfiguration
{
    INJECT(languages, framework::IGlobalConfiguration, globalConfiguration)
    INJECT(languages, framework::IFsOperations, fsOperations)

public:
    void init();

    QString currentLanguageCode() const override;
    Ret setCurrentLanguageCode(const QString& languageCode) const override;

    QUrl languagesUpdateUrl() const override;
    QUrl languageFileServerUrl(const QString& languageCode) const override;

    ValCh<LanguagesHash> languages() const override;
    Ret setLanguages(const LanguagesHash& languages) const override;

    QString languagesSharePath() const override;
    QString languagesDataPath() const override;

    QStringList languageFilePaths(const QString& languageCode) const override;
    QString languageArchivePath(const QString& languageCode) const override;

private:
    LanguagesHash parseLanguagesConfig(const QByteArray& json) const;
    QString languageFileName(const QString& languageCode) const;

    async::Channel<LanguagesHash> m_languagesHashChanged;
};
}
}

#endif // MU_LANGUAGES_LANGUAGESCONFIGURATION_H
