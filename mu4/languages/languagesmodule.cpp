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
#include "languagesmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "internal/languagesconfiguration.h"
#include "internal/languagescontroller.h"
#include "internal/languageunpacker.h"
#include "view/languagelistmodel.h"

using namespace mu::languages;

static LanguagesConfiguration* m_languagesConfiguration = new LanguagesConfiguration();
static LanguagesController* m_languagesController = new LanguagesController();

static void languages_init_qrc()
{
    Q_INIT_RESOURCE(languages);
}

std::string LanguagesModule::moduleName() const
{
    return "languages";
}

void LanguagesModule::registerExports()
{
    framework::ioc()->registerExport<ILanguagesConfiguration>(moduleName(), m_languagesConfiguration);
    framework::ioc()->registerExport<ILanguagesController>(moduleName(), m_languagesController);
    framework::ioc()->registerExport<ILanguageUnpacker>(moduleName(), new LanguageUnpacker());
}

void LanguagesModule::registerResources()
{
    languages_init_qrc();
}

void LanguagesModule::registerUiTypes()
{
    qmlRegisterType<LanguageListModel>("MuseScore.Languages", 1, 0, "LanguageListModel");
    qmlRegisterUncreatableType<LanguageStatus>("MuseScore.Languages", 1, 0, "LanguageStatus", "Cannot create an LanguageStatus");

    framework::ioc()->resolve<framework::IUiEngine>(moduleName())->addSourceImportPath(languages_QML_IMPORT);
}

void LanguagesModule::onInit()
{
    m_languagesController->init();
    m_languagesConfiguration->init();
}
