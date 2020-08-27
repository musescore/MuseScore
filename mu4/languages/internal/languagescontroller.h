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
#ifndef MU_LANGUAGES_LANGUAGESCONTROLLER_H
#define MU_LANGUAGES_LANGUAGESCONTROLLER_H

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "ilanguagescontroller.h"
#include "ilanguagesconfiguration.h"
#include "ilanguageunpacker.h"
#include "iglobalconfiguration.h"
#include "framework/network/inetworkmanagercreator.h"
#include "framework/system/ifilesystem.h"

class QTranslator;

namespace mu {
namespace languages {
class LanguagesController : public ILanguagesController, public async::Asyncable
{
    INJECT(languages, ILanguagesConfiguration, configuration)
    INJECT(languages, ILanguageUnpacker, languageUnpacker)
    INJECT(languages, framework::IGlobalConfiguration, globalConfiguration)
    INJECT(languages, framework::INetworkManagerCreator, networkManagerCreator)
    INJECT(languages, framework::IFileSystem, fileSystem)

public:
    void init();

    Ret refreshLanguages() override;
    ValCh<LanguagesHash> languages() const override;
    RetCh<LanguageProgress> install(const QString& languageCode) override;
    Ret uninstall(const QString& languageCode) override;

    RetVal<Language> currentLanguage() const override;
    Ret setCurrentLanguage(const QString& languageCode) override;

    RetCh<Language> languageChanged() override;

private:
    RetVal<LanguagesHash> parseLanguagesConfig(const QByteArray& json) const;
    bool isLanguageExists(const QString& languageCode) const;

    RetVal<LanguagesHash> correctLanguagesStates(LanguagesHash& languages) const;

    RetVal<QString> downloadLanguage(const QString& languageCode, async::Channel<LanguageProgress>& progressChannel) const;
    Ret removeLanguage(const QString& languageCode) const;

    Ret loadLanguage(const QString& languageCode);

    void resetLanguageByDefault();

    void th_install(const QString& languageCode, async::Channel<LanguageProgress> progressChannel,async::Channel<Ret> finishChannel);

private:
    async::Channel<Language> m_languageChanged;
    async::Channel<LanguageProgress> m_languageProgressStatus;
    async::Channel<Ret> m_languageFinishCh;
    QList<QTranslator*> m_translatorList;
};
}
}

#endif // MU_LANGUAGES_LANGUAGESCONTROLLER_H
