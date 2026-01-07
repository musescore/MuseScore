/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#pragma once

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
    GlobalInject<ILanguagesConfiguration> configuration;
    GlobalInject<network::INetworkManagerCreator> networkManagerCreator;
    GlobalInject<io::IFileSystem> fileSystem;
    GlobalInject<mi::IMultiInstancesProvider> multiInstancesProvider;

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

    void checkUpdateAvailable(const QString& languageCode, std::function<void(const RetVal<bool>&)> finished);
    void updateLanguage(const QString& languageCode, std::function<void(const Ret&)> finished);

    Ret unpackAndWriteLanguage(const QByteArray& zipData);

    RetVal<QString> fileHash(const io::path_t& path) const;

private:
    LanguagesHash m_languagesHash;
    Language m_currentLanguage;
    async::Notification m_currentLanguageChanged;
    Language m_placeholderLanguage;

    std::vector<QTranslator*> m_translators;
    QHash<QString, Progress> m_updateOperationsHash;

    bool m_inited = false;
    bool m_restartRequiredToApplyLanguage = false;
    async::Channel<bool> m_restartRequiredToApplyLanguageChanged;

    network::INetworkManagerPtr m_networkManager;
};
}
