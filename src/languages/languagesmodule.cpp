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

#include "modularity/ioc.h"

#include "internal/languagesconfiguration.h"
#include "internal/languagesservice.h"
#include "internal/languageunpacker.h"

#include "diagnostics/idiagnosticspathsregister.h"

using namespace mu::languages;
using namespace mu::modularity;

static std::shared_ptr<LanguagesConfiguration> s_languagesConfiguration = std::make_shared<LanguagesConfiguration>();
static std::shared_ptr<LanguagesService> s_languagesService = std::make_shared<LanguagesService>();

std::string LanguagesModule::moduleName() const
{
    return "languages";
}

void LanguagesModule::registerExports()
{
    ioc()->registerExport<ILanguagesConfiguration>(moduleName(), s_languagesConfiguration);
    ioc()->registerExport<ILanguagesService>(moduleName(), s_languagesService);
    ioc()->registerExport<ILanguageUnpacker>(moduleName(), std::make_shared<LanguageUnpacker>());
}

void LanguagesModule::onInit(const framework::IApplication::RunMode& mode)
{
    //! NOTE: configurator must be initialized before any service that uses it
    s_languagesConfiguration->init();

    if (framework::IApplication::RunMode::Converter == mode) {
        return;
    }

    s_languagesService->init();

    auto pr = modularity::ioc()->resolve<diagnostics::IDiagnosticsPathsRegister>(moduleName());
    if (pr) {
        pr->reg("languagesAppDataPath", s_languagesConfiguration->languagesAppDataPath());
        pr->reg("languagesUserAppDataPath", s_languagesConfiguration->languagesUserAppDataPath());
    }
}

void LanguagesModule::onDelayedInit()
{
    s_languagesService->refreshLanguages();
}
