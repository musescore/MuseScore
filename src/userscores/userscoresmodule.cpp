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

#include "view/newscoremodel.h"
#include "view/additionalinfomodel.h"
#include "view/scorethumbnail.h"
#include "view/templatesmodel.h"
#include "view/templatepaintview.h"

#include "internal/templatesrepository.h"

#include "ui/iinteractiveuriregister.h"

using namespace mu::userscores;
using namespace mu::modularity;
using namespace mu::ui;

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
    ioc()->registerExport<ITemplatesRepository>(moduleName(), new TemplatesRepository());
}

void UserScoresModule::resolveImports()
{
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
    qmlRegisterType<NewScoreModel>("MuseScore.UserScores", 1, 0, "NewScoreModel");
    qmlRegisterType<AdditionalInfoModel>("MuseScore.UserScores", 1, 0, "AdditionalInfoModel");

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
}
