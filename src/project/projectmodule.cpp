/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "internal/projectcreator.h"
#include "internal/projectautosaver.h"
#include "internal/projectactionscontroller.h"
#include "internal/projectuiactions.h"
#include "internal/projectconfiguration.h"
#include "internal/opensaveprojectscenario.h"
#include "internal/exportprojectscenario.h"
#include "internal/mscmetareader.h"
#include "internal/templatesrepository.h"
#include "internal/projectmigrator.h"
#include "internal/projectautosaver.h"

#include "internal/notationreadersregister.h"
#include "internal/notationwritersregister.h"
#include "internal/projectrwregister.h"

#include "view/exportdialogmodel.h"
#include "view/scorespagemodel.h"
#include "view/recentscoresmodel.h"
#include "view/cloudscoresmodel.h"
#include "view/cloudscorestatuswatcher.h"
#include "view/scorethumbnailloader.h"
#include "view/pixmapscorethumbnailview.h"
#include "view/templatesmodel.h"
#include "view/templatepaintview.h"
#include "view/newscoremodel.h"
#include "view/additionalinfomodel.h"
#include "view/projectpropertiesmodel.h"
#include "view/audiogenerationsettingsmodel.h"

#ifdef Q_OS_MAC
#include "internal/platform/macos/macosrecentfilescontroller.h"
#elif defined (Q_OS_WIN)
#include "internal/platform/windows/windowsrecentfilescontroller.h"
#else
#include "internal/recentfilescontroller.h"
#endif

#include "ui/iuiactionsregister.h"
#include "ui/iinteractiveuriregister.h"
#include "extensions/iextensionsexecpointsregister.h"
#include "projectextensionpoints.h"

using namespace mu::project;
using namespace muse;
using namespace muse::modularity;

static void project_init_qrc()
{
    Q_INIT_RESOURCE(project);
}

std::string ProjectModule::moduleName() const
{
    return "project";
}

void ProjectModule::registerExports()
{
    m_configuration = std::make_shared<ProjectConfiguration>();
    m_actionsController = std::make_shared<ProjectActionsController>(iocContext());
    m_projectAutoSaver = std::make_shared<ProjectAutoSaver>();

#ifdef Q_OS_MAC
    m_recentFilesController = std::make_shared<MacOSRecentFilesController>();
#elif defined(Q_OS_WIN)
    m_recentFilesController = std::make_shared<WindowsRecentFilesController>();
#else
    m_recentFilesController = std::make_shared<RecentFilesController>();
#endif

    ioc()->registerExport<IProjectConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IProjectCreator>(moduleName(), new ProjectCreator());
    ioc()->registerExport<IProjectFilesController>(moduleName(), m_actionsController);
    ioc()->registerExport<mi::IProjectProvider>(moduleName(), m_actionsController);
    ioc()->registerExport<IOpenSaveProjectScenario>(moduleName(), new OpenSaveProjectScenario());
    ioc()->registerExport<IExportProjectScenario>(moduleName(), new ExportProjectScenario());
    ioc()->registerExport<IRecentFilesController>(moduleName(), m_recentFilesController);
    ioc()->registerExport<IMscMetaReader>(moduleName(), new MscMetaReader());
    ioc()->registerExport<ITemplatesRepository>(moduleName(), new TemplatesRepository());
    ioc()->registerExport<IProjectMigrator>(moduleName(), new ProjectMigrator());
    ioc()->registerExport<IProjectAutoSaver>(moduleName(), m_projectAutoSaver);

    //! TODO Should be replace INotationReaders/WritersRegister with IProjectRWRegister
    ioc()->registerExport<INotationReadersRegister>(moduleName(), new NotationReadersRegister());
    ioc()->registerExport<INotationWritersRegister>(moduleName(), new NotationWritersRegister());
    ioc()->registerExport<IProjectRWRegister>(moduleName(), new ProjectRWRegister());
}

void ProjectModule::resolveImports()
{
    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<ProjectUiActions>(m_actionsController));
    }

    auto ir = ioc()->resolve<muse::ui::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("musescore://project/newscore"), "MuseScore/Project/NewScoreDialog.qml");
        ir->registerQmlUri(Uri("musescore://project/asksavelocationtype"), "MuseScore/Project/AskSaveLocationTypeDialog.qml");
        ir->registerQmlUri(Uri("musescore://project/savetocloud"), "MuseScore/Project/SaveToCloudDialog.qml");
        ir->registerQmlUri(Uri("musescore://project/alsoshareaudiocom"), "MuseScore/Project/AlsoShareAudioComDialog.qml");
        ir->registerQmlUri(Uri("musescore://project/export"), "MuseScore/Project/ExportDialog.qml");
        ir->registerQmlUri(Uri("musescore://project/migration"), "MuseScore/Project/MigrationDialog.qml");
        ir->registerQmlUri(Uri("musescore://project/properties"), "MuseScore/Project/ProjectPropertiesDialog.qml");
        ir->registerQmlUri(Uri("musescore://project/upload/progress"), "MuseScore/Project/UploadProgressDialog.qml");
        ir->registerQmlUri(Uri("musescore://project/upload/success"), "MuseScore/Project/ProjectUploadedDialog.qml");
        ir->registerQmlUri(Uri("musescore://project/audiogenerationsettings"), "MuseScore/Project/AudioGenerationSettingsDialog.qml");
    }

    auto er = ioc()->resolve<muse::extensions::IExtensionsExecPointsRegister>(moduleName());
    if (er) {
        er->reg(moduleName(), { EXEC_ONPOST_PROJECT_CREATED,
                                TranslatableString::untranslatable("On post project created") });
        er->reg(moduleName(), { EXEC_ONPOST_PROJECT_OPENED,
                                TranslatableString::untranslatable("On post project opened") });
        er->reg(moduleName(), { EXEC_ONPRE_PROJECT_SAVE,
                                TranslatableString::untranslatable("On pre project save") });
        er->reg(moduleName(), { EXEC_ONPOST_PROJECT_SAVED,
                                TranslatableString::untranslatable("On post project saved") });
    }
}

void ProjectModule::registerResources()
{
    project_init_qrc();
}

void ProjectModule::registerUiTypes()
{
    qmlRegisterType<ExportDialogModel>("MuseScore.Project", 1, 0, "ExportDialogModel");

    qmlRegisterType<ScoresPageModel>("MuseScore.Project", 1, 0, "ScoresPageModel");
    qmlRegisterUncreatableType<AbstractScoresModel>("MuseScore.Project", 1, 0, "AbstractScoresModel",
                                                    "Not creatable as it is an abstract type");
    qmlRegisterType<RecentScoresModel>("MuseScore.Project", 1, 0, "RecentScoresModel");
    qmlRegisterType<CloudScoresModel>("MuseScore.Project", 1, 0, "CloudScoresModel");
    qmlRegisterType<CloudScoreStatusWatcher>("MuseScore.Project", 1, 0, "CloudScoreStatusWatcher");
    qmlRegisterType<NewScoreModel>("MuseScore.Project", 1, 0, "NewScoreModel");
    qmlRegisterType<AdditionalInfoModel>("MuseScore.Project", 1, 0, "AdditionalInfoModel");
    qmlRegisterType<ProjectPropertiesModel>("MuseScore.Project", 1, 0, "ProjectPropertiesModel");
    qmlRegisterType<AudioGenerationSettingsModel>("MuseScore.Project", 1, 0, "AudioGenerationSettingsModel");

    qmlRegisterType<ScoreThumbnailLoader>("MuseScore.Project", 1, 0, "ScoreThumbnailLoader");
    qmlRegisterType<PixmapScoreThumbnailView>("MuseScore.Project", 1, 0, "PixmapScoreThumbnailView");
    qmlRegisterType<TemplatesModel>("MuseScore.Project", 1, 0, "TemplatesModel");
    qmlRegisterType<TemplatePaintView>("MuseScore.Project", 1, 0, "TemplatePaintView");

    qmlRegisterUncreatableType<QMLSaveLocationType>("MuseScore.Project", 1, 0, "SaveLocationType",
                                                    "Not creatable as it is an enum type");

    qmlRegisterUncreatableType<GenerateAudioTimePeriod>("MuseScore.Project", 1, 0, "GenerateAudioTimePeriodType",
                                                        "Not creatable as it is an enum type");

    qmlRegisterUncreatableType<Migration>("MuseScore.Project", 1, 0, "MigrationType",
                                          "Not creatable as it is an enum type");
}

void ProjectModule::onInit(const IApplication::RunMode& mode)
{
    if (IApplication::RunMode::GuiApp != mode) {
        return;
    }

    m_configuration->init();
    m_actionsController->init();
    m_recentFilesController->init();
    m_projectAutoSaver->init();
}
