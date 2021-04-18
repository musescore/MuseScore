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
#ifndef MU_LANGUAGES_LANGUAGESSERVICE_H
#define MU_LANGUAGES_LANGUAGESSERVICE_H

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "ilanguagesservice.h"
#include "ilanguagesconfiguration.h"
#include "ilanguageunpacker.h"
#include "iglobalconfiguration.h"
#include "framework/network/inetworkmanagercreator.h"
#include "framework/system/ifilesystem.h"

class QTranslator;

namespace mu::languages {
class LanguagesService : public ILanguagesService, public async::Asyncable
{
    INJECT(languages, ILanguagesConfiguration, configuration)
    INJECT(languages, ILanguageUnpacker, languageUnpacker)
    INJECT(languages, framework::IGlobalConfiguration, globalConfiguration)
    INJECT(languages, network::INetworkManagerCreator, networkManagerCreator)
    INJECT(languages, system::IFileSystem, fileSystem)

public:
    void init();

    ValCh<LanguagesHash> languages() const override;
    RetCh<LanguageProgress> install(const QString& languageCode) override;
    RetCh<LanguageProgress> update(const QString& languageCode) override;
    Ret uninstall(const QString& languageCode) override;

    ValCh<Language> currentLanguage() const override;

    RetCh<Language> languageChanged() override;

private:
    RetVal<LanguagesHash> parseLanguagesConfig(const QByteArray& json) const;
    LanguageFiles parseLanguageFiles(const QJsonObject& languageObject) const;

    void setCurrentLanguage(const QString& languageCode);

    bool isLanguageExists(const QString& languageCode) const;
    bool checkLanguageFilesHash(const QString& languageCode, const LanguageFiles& languageFiles) const;

    Language language(const QString& languageCode) const;

    RetVal<LanguagesHash> correctLanguagesStates(LanguagesHash& languages) const;
    LanguageStatus::Status languageStatus(const Language& language) const;

    RetVal<QString> downloadLanguage(const QString& languageCode, async::Channel<LanguageProgress>* progressChannel) const;
    Ret removeLanguage(const QString& languageCode) const;

    Ret loadLanguage(const QString& languageCode);

    void resetLanguageToDefault();

    void th_refreshLanguages();
    void th_install(const QString& languageCode, async::Channel<LanguageProgress>* progressChannel, async::Channel<Ret>* finishChannel);
    void th_update(const QString& languageCode, async::Channel<LanguageProgress>* progressChannel, async::Channel<Ret>* finishChannel);

    void closeOperation(const QString& languageCode, async::Channel<LanguageProgress>* progressChannel);

    enum OperationType
    {
        None,
        Install,
        Update
    };

    struct Operation
    {
        OperationType type = OperationType::None;
        async::Channel<LanguageProgress>* progressChannel = nullptr;

        Operation() = default;
        Operation(const OperationType& type, async::Channel<LanguageProgress>* progressChannel)
            : type(type), progressChannel(progressChannel) {}
    };

private:
    async::Channel<Language> m_languageChanged;
    async::Channel<Language> m_currentLanguageChanged;
    QList<QTranslator*> m_translatorList;

    mutable QHash<QString, Operation> m_operationsHash;
};
}

#endif // MU_LANGUAGES_LANGUAGESSERVICE_H
