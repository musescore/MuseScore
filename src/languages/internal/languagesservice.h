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
#include "multiinstances/imultiinstancesprovider.h"

class QTranslator;

namespace mu::languages {
class LanguagesService : public ILanguagesService, public async::Asyncable
{
    INJECT(ILanguagesConfiguration, configuration)
    INJECT(network::INetworkManagerCreator, networkManagerCreator)
    INJECT(io::IFileSystem, fileSystem)
    INJECT(mi::IMultiInstancesProvider, multiInstancesProvider)

public:
    void init();

    const LanguagesHash& languages() const override;
    Language language(const QString& languageCode) const override;
    const Language& currentLanguage() const override;
    async::Notification currentLanguageChanged() const override;

    bool hasPlaceholderLanguage() const override;
    const Language& placeholderLanguage() const override;

    framework::Progress update(const QString& languageCode) override;

    bool needRestartToApplyLanguageChange() const override;
    async::Channel<bool> needRestartToApplyLanguageChangeChanged() const override;

private:
    void loadLanguages();

    void setCurrentLanguage(const QString& languageCode);
    QString effectiveLanguageCode(const QString& languageCode) const;
    Ret loadLanguage(Language& lang);

    void th_update(const QString& languageCode, framework::Progress progress);
    bool canUpdate(const QString& languageCode);
    Ret downloadLanguage(const QString& languageCode, framework::Progress progress) const;
    RetVal<QString> fileHash(const io::path_t& path);

private:
    LanguagesHash m_languagesHash;
    Language m_currentLanguage;
    async::Notification m_currentLanguageChanged;
    Language m_placeholderLanguage;

    QSet<QTranslator*> m_translators;
    mutable QHash<QString, framework::Progress> m_updateOperationsHash;

    bool m_inited = false;
    bool m_needRestartToApplyLanguageChange = false;
    async::Channel<bool> m_needRestartToApplyLanguageChangeChanged;
};
}

#endif // MU_LANGUAGES_LANGUAGESSERVICE_H
