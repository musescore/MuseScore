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
#ifndef MUSE_LANGUAGES_LANGUAGESSERVICE_H
#define MUSE_LANGUAGES_LANGUAGESSERVICE_H

#include "ilanguagesservice.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "ilanguagesconfiguration.h"
#include "framework/network/inetworkmanagercreator.h"
#include "io/ifilesystem.h"
#include "multiinstances/imultiinstancesprovider.h"

class QTranslator;

namespace muse::languages {
class LanguagesService : public ILanguagesService, public Injectable, public async::Asyncable
{
    Inject<ILanguagesConfiguration> configuration = { this };
    Inject<network::INetworkManagerCreator> networkManagerCreator = { this };
    Inject<io::IFileSystem> fileSystem = { this };
    Inject<mi::IMultiInstancesProvider> multiInstancesProvider = { this };

public:
    LanguagesService(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

    const LanguagesHash& languages() const override;
    Language language(const QString& languageCode) const override;
    const Language& currentLanguage() const override;
    async::Notification currentLanguageChanged() const override;

    bool hasPlaceholderLanguage() const override;
    const Language& placeholderLanguage() const override;

    Progress update(const QString& languageCode) override;

    bool restartRequiredToApplyLanguage() const override;
    async::Channel<bool> restartRequiredToApplyLanguageChanged() const override;

private:
    void loadLanguages();

    void setCurrentLanguage(const QString& languageCode);
    QString effectiveLanguageCode(QString languageCode) const;
    Ret loadLanguage(Language& lang);

    void th_update(const QString& languageCode, Progress progress);
    bool canUpdate(const QString& languageCode);
    Ret downloadLanguage(const QString& languageCode, Progress progress) const;
    RetVal<QString> fileHash(const io::path_t& path);

private:
    LanguagesHash m_languagesHash;
    Language m_currentLanguage;
    async::Notification m_currentLanguageChanged;
    Language m_placeholderLanguage;

    QSet<QTranslator*> m_translators;
    mutable QHash<QString, Progress> m_updateOperationsHash;

    bool m_inited = false;
    bool m_restartRequiredToApplyLanguage = false;
    async::Channel<bool> m_restartRequiredToApplyLanguageChanged;
};
}

#endif // MUSE_LANGUAGES_LANGUAGESSERVICE_H
