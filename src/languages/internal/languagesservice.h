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
#ifndef MU_LANGUAGES_LANGUAGESSERVICE_H
#define MU_LANGUAGES_LANGUAGESSERVICE_H

#include "ilanguagesservice.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "ilanguagesconfiguration.h"
#include "framework/network/inetworkmanagercreator.h"
#include "io/ifilesystem.h"
#include "ui/iuiengine.h"

class QTranslator;

namespace mu::languages {
class LanguagesService : public ILanguagesService, public async::Asyncable
{
    INJECT(languages, ILanguagesConfiguration, configuration)
    INJECT(languages, network::INetworkManagerCreator, networkManagerCreator)
    INJECT(languages, io::IFileSystem, fileSystem)
    INJECT(languages, ui::IUiEngine, uiEngine)

public:
    void init();

    const LanguagesHash& languages() const override;
    ValCh<Language> currentLanguage() const override;

    bool hasPlaceholderLanguage() const override;
    const Language& placeholderLanguage() const override;

    LanguageProgressChannel update(const QString& languageCode) override;

    ValCh<bool> needRestartToApplyLanguageChange() const override;

private:
    void loadLanguages();

    void setCurrentLanguage(const QString& languageCode);
    QString effectiveLanguageCode(const QString& languageCode) const;
    Ret loadLanguage(Language& lang);

    void th_update(const QString& languageCode, LanguageProgressChannel progressChannel, async::Channel<Ret> finishChannel);
    bool canUpdate(const QString& languageCode);
    Ret downloadLanguage(const QString& languageCode, LanguageProgressChannel progressChannel) const;
    RetVal<QString> fileHash(const io::path_t& path);

private:
    LanguagesHash m_languagesHash;
    Language m_placeholderLanguage;
    ValCh<Language> m_currentLanguage;

    QSet<QTranslator*> m_translators;
    mutable QHash<QString, LanguageProgressChannel > m_updateOperationsHash;

    bool m_inited = false;
    ValCh<bool> m_needRestartToApplyLanguageChange;
};
}

#endif // MU_LANGUAGES_LANGUAGESSERVICE_H
