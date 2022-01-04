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
#include "languagesmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "internal/languagesconfiguration.h"
#include "internal/languagesservice.h"
#include "internal/languageunpacker.h"
#include "view/languagelistmodel.h"

#include "diagnostics/idiagnosticspathsregister.h"

using namespace mu::languages;

static LanguagesConfiguration* m_languagesConfiguration = new LanguagesConfiguration();
static LanguagesService* m_languagesService = new LanguagesService();

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
    modularity::ioc()->registerExport<ILanguagesConfiguration>(moduleName(), m_languagesConfiguration);
    modularity::ioc()->registerExport<ILanguagesService>(moduleName(), m_languagesService);
    modularity::ioc()->registerExport<ILanguageUnpacker>(moduleName(), new LanguageUnpacker());
}

void LanguagesModule::registerResources()
{
    languages_init_qrc();
}

void LanguagesModule::registerUiTypes()
{
    qmlRegisterType<LanguageListModel>("MuseScore.Languages", 1, 0, "LanguageListModel");
    qmlRegisterUncreatableType<LanguageStatus>("MuseScore.Languages", 1, 0, "LanguageStatus", "Cannot create an LanguageStatus");

    modularity::ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(languages_QML_IMPORT);
}

void LanguagesModule::onInit(const framework::IApplication::RunMode& mode)
{
    //! NOTE: configurator must be initialized before any service that uses it
    m_languagesConfiguration->init();

    if (framework::IApplication::RunMode::Converter == mode) {
        return;
    }

    m_languagesService->init();

    auto pr = modularity::ioc()->resolve<diagnostics::IDiagnosticsPathsRegister>(moduleName());
    if (pr) {
        pr->reg("languagesAppDataPath", m_languagesConfiguration->languagesAppDataPath());
        pr->reg("languagesUserAppDataPath", m_languagesConfiguration->languagesUserAppDataPath());
    }
}

void LanguagesModule::onDelayedInit()
{
    m_languagesService->refreshLanguages();
}
