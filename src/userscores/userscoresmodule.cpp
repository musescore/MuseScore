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
#include "userscoresmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "view/recentscoresmodel.h"
#include "view/newscoremodel.h"
#include "view/additionalinfomodel.h"
#include "view/scorethumbnail.h"
#include "view/templatesmodel.h"
#include "view/templatepaintview.h"
#include "view/exportdialogmodel.h"
#include "internal/filescorecontroller.h"
#include "internal/userscoresconfiguration.h"
#include "internal/userscoresservice.h"
#include "internal/exportscorescenario.h"
#include "internal/templatesrepository.h"
#include "internal/userscoresuiactions.h"

#ifdef Q_OS_MAC
#include "internal/platform/macos/macosrecentfilescontroller.h"
#elif defined (Q_OS_WIN)
#include "internal/platform/windows/windowsrecentfilescontroller.h"
#else
#include "internal/platform/stub/stubrecentfilescontroller.h"
#endif

#include "ui/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"

using namespace mu::userscores;
using namespace mu::modularity;
using namespace mu::ui;

static std::shared_ptr<FileScoreController> s_fileController = std::make_shared<FileScoreController>();
static std::shared_ptr<UserScoresConfiguration> s_userScoresConfiguration = std::make_shared<UserScoresConfiguration>();
static std::shared_ptr<UserScoresService> s_userScoresService = std::make_shared<UserScoresService>();
static std::shared_ptr<ExportScoreScenario> s_exportScoreScenario = std::make_shared<ExportScoreScenario>();

static void userscores_init_qrc()
{
    Q_INIT_RESOURCE(userscores);
}

std::string UserScoresModule::moduleName() const
{
    return "userscores";
}

void UserScoresModule::registerExports()
{
    ioc()->registerExport<IUserScoresConfiguration>(moduleName(), s_userScoresConfiguration);
    ioc()->registerExport<IUserScoresService>(moduleName(), s_userScoresService);
    ioc()->registerExport<ITemplatesRepository>(moduleName(), new TemplatesRepository());
    ioc()->registerExport<IFileScoreController>(moduleName(), s_fileController);
    ioc()->registerExport<IExportScoreScenario>(moduleName(), s_exportScoreScenario);

#ifdef Q_OS_MAC
    ioc()->registerExport<IPlatformRecentFilesController>(moduleName(), new MacOSRecentFilesController());
#elif defined (Q_OS_WIN)
    ioc()->registerExport<IPlatformRecentFilesController>(moduleName(), new WindowsRecentFilesController());
#else
    ioc()->registerExport<IPlatformRecentFilesController>(moduleName(), new StubRecentFilesController());
#endif
}

void UserScoresModule::resolveImports()
{
    auto ar = ioc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<UserScoresUiActions>(s_fileController));
    }

    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://userscores/newscore"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/UserScores/NewScoreDialog.qml"));

        ir->registerUri(Uri("musescore://userscores/export"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/UserScores/ExportDialog.qml"));
    }
}

void UserScoresModule::registerResources()
{
    userscores_init_qrc();
}

void UserScoresModule::registerUiTypes()
{
    qmlRegisterType<RecentScoresModel>("MuseScore.UserScores", 1, 0, "RecentScoresModel");
    qmlRegisterType<NewScoreModel>("MuseScore.UserScores", 1, 0, "NewScoreModel");
    qmlRegisterType<AdditionalInfoModel>("MuseScore.UserScores", 1, 0, "AdditionalInfoModel");
    qmlRegisterType<ExportDialogModel>("MuseScore.UserScores", 1, 0, "ExportDialogModel");

    qmlRegisterType<ScoreThumbnail>("MuseScore.UserScores", 1, 0, "ScoreThumbnail");
    qmlRegisterType<TemplatesModel>("MuseScore.UserScores", 1, 0, "TemplatesModel");
    qmlRegisterType<TemplatePaintView>("MuseScore.UserScores", 1, 0, "TemplatePaintView");

    ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(userscores_QML_IMPORT);
}

void UserScoresModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (framework::IApplication::RunMode::Converter == mode) {
        return;
    }
    s_userScoresConfiguration->init();
    s_userScoresService->init();
    s_fileController->init();
}
