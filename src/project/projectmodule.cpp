/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "projectmodule.h"

#include "modularity/ioc.h"
#include "internal/projectcreator.h"
#include "internal/projectautosaver.h"
#include "internal/projectactionscontroller.h"
#include "internal/engravingpluginapihelper.h"
#include "internal/projectuiactions.h"
#include "internal/projectconfiguration.h"
#include "internal/openprojectscenario.h"
#include "internal/saveprojectscenario.h"
#include "internal/closeprojectscenario.h"
#include "internal/exportprojectscenario.h"
#include "internal/mscmetareader.h"
#include "internal/templatesrepository.h"
#include "internal/projectmigrator.h"
#include "internal/projectautosaver.h"
#include "internal/convertfiletoscoreservice.h"
#include "internal/convertfiletoscorescenario.h"

#include "internal/notationreadersregister.h"
#include "internal/notationwritersregister.h"
#include "internal/projectcommandsregister.h"
#include "internal/projectcommandsstate.h"

#ifdef Q_OS_MAC
#include "internal/platform/macos/macosrecentfilescontroller.h"
#elif defined (Q_OS_WIN)
#include "internal/platform/windows/windowsrecentfilescontroller.h"
#else
#include "internal/recentfilescontroller.h"
#endif

#include "rcommand/icommandsstate.h"
#include "ui/iuiactionsregister.h"
#include "interactive/iinteractiveuriregister.h"

using namespace mu::project;
using namespace muse;
using namespace muse::modularity;

static const std::string mname("project");

std::string ProjectModule::moduleName() const
{
    return mname;
}

void ProjectModule::registerExports()
{
    m_configuration = std::make_shared<ProjectConfiguration>(globalCtx());

    globalIoc()->registerExport<IProjectConfiguration>(mname, m_configuration);
    globalIoc()->registerExport<IProjectCreator>(mname, new ProjectCreator());
    globalIoc()->registerExport<IMscMetaReader>(mname, new MscMetaReader());

    globalIoc()->registerExport<INotationReadersRegister>(mname, new NotationReadersRegister());
    globalIoc()->registerExport<INotationWritersRegister>(mname, new NotationWritersRegister());
}

void ProjectModule::resolveImports()
{
    auto ir = globalIoc()->resolve<muse::interactive::IInteractiveUriRegister>(mname);
    if (ir) {
        ir->registerQmlUri(Uri("musescore://project/newscore"), "MuseScore.Project", "NewScoreDialog");
        ir->registerQmlUri(Uri("musescore://project/asksavelocationtype"), "MuseScore.Project", "AskSaveLocationTypeDialog");
        ir->registerQmlUri(Uri("musescore://project/savetocloud"), "MuseScore.Project", "SaveToCloudDialog");
        ir->registerQmlUri(Uri("musescore://project/alsoshareaudiocom"), "MuseScore.Project", "AlsoShareAudioComDialog");
        ir->registerQmlUri(Uri("musescore://project/export"), "MuseScore.Project", "ExportDialog");
        ir->registerQmlUri(Uri("musescore://project/migration"), "MuseScore.Project", "MigrationDialog");
        ir->registerQmlUri(Uri("musescore://project/properties"), "MuseScore.Project", "ProjectPropertiesDialog");
        ir->registerQmlUri(Uri("musescore://project/upload/progress"), "MuseScore.Project", "UploadProgressDialog");
        ir->registerQmlUri(Uri("musescore://project/upload/success"), "MuseScore.Project", "ProjectUploadedDialog");
        ir->registerQmlUri(Uri("musescore://project/audiogenerationsettings"), "MuseScore.Project", "AudioGenerationSettingsDialog");
        ir->registerQmlUri(Uri("musescore://project/convert/selectfiles"), "MuseScore.Project", "ConvertFileToScoreDialog");
        ir->registerQmlUri(Uri("musescore://project/convert/processing"), "MuseScore.Project", "ConvertFileProcessingDialog");
    }

    auto cr = globalIoc()->resolve<muse::rcommand::ICommandsRegister>(mname);
    if (cr) {
        cr->reg(std::make_shared<ProjectCommandsRegister>());
    }
}

void ProjectModule::onInit(const IApplication::RunMode& mode)
{
    if (IApplication::RunMode::GuiApp != mode) {
        return;
    }

    m_configuration->init();
}

IContextSetup* ProjectModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new ProjectContext(ctx);
}

void ProjectContext::registerExports()
{
    m_actionsController = std::make_shared<ProjectActionsController>(iocContext());
    m_projectAutoSaver = std::make_shared<ProjectAutoSaver>(iocContext());
    m_engravingPluginAPIHelper = std::make_shared<EngravingPluginAPIHelper>(iocContext());
    m_convertFileToScoreService = std::make_shared<ConvertFileToScoreService>(iocContext());
    m_convertFileToScoreScenario = std::make_shared<ConvertFileToScoreScenario>(iocContext());

#ifdef Q_OS_MAC
    m_recentFilesController = std::make_shared<MacOSRecentFilesController>();
#elif defined(Q_OS_WIN)
    m_recentFilesController = std::make_shared<WindowsRecentFilesController>();
#else
    m_recentFilesController = std::make_shared<RecentFilesController>();
#endif

    ioc()->registerExport<IProjectCommandsController>(mname, m_actionsController);
    ioc()->registerExport<mi::IProjectProvider>(mname, m_actionsController);
    ioc()->registerExport<IOpenProjectScenario>(mname, new OpenProjectScenario(iocContext()));
    ioc()->registerExport<ISaveProjectScenario>(mname, new SaveProjectScenario(iocContext()));
    ioc()->registerExport<ICloseProjectScenario>(mname, new CloseProjectScenario(iocContext()));
    ioc()->registerExport<IExportProjectScenario>(mname, new ExportProjectScenario(iocContext()));
    ioc()->registerExport<IRecentFilesController>(mname, m_recentFilesController);
    ioc()->registerExport<ITemplatesRepository>(mname, new TemplatesRepository());
    ioc()->registerExport<IProjectMigrator>(mname, new ProjectMigrator(iocContext()));
    ioc()->registerExport<IProjectAutoSaver>(mname, m_projectAutoSaver);
    ioc()->registerExport<mu::engraving::IEngravingPluginAPIHelper>(mname, m_engravingPluginAPIHelper);
    ioc()->registerExport<IConvertFileToScoreService>(mname, m_convertFileToScoreService);
    ioc()->registerExport<IConvertFileToScoreScenario>(mname, m_convertFileToScoreScenario);
}

void ProjectContext::resolveImports()
{
    auto cs = ioc()->resolve<muse::rcommand::ICommandsState>(mname);
    if (cs) {
        cs->reg(std::make_shared<ProjectCommandsState>(iocContext()));
    }

    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(mname);
    if (ar) {
        ar->reg(std::make_shared<ProjectUiActions>(m_actionsController, iocContext()));
    }
}

void ProjectContext::onInit(const IApplication::RunMode& mode)
{
    if (IApplication::RunMode::GuiApp != mode) {
        return;
    }

    m_actionsController->init();
    m_recentFilesController->init();
    m_projectAutoSaver->init();
    m_convertFileToScoreService->init();
    m_convertFileToScoreScenario->init();
}

void ProjectContext::onAllInited(const IApplication::RunMode& mode)
{
    if (IApplication::RunMode::GuiApp != mode) {
        return;
    }

    //! NOTE: resuming polling can show dialogs (errors, review prompts), so it must wait
    //! until the main window is up rather than running during onInit()
    m_convertFileToScoreService->resumeConvert();
}
