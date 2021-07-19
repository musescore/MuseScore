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
#include "playbackmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "ui/iuiactionsregister.h"
#include "ui/iinteractiveuriregister.h"

#include "internal/playbackcontroller.h"
#include "internal/playbackuiactions.h"
#include "internal/playbackconfiguration.h"

#include "view/playbacktoolbarmodel.h"
#include "view/mixerpanelmodel.h"

using namespace mu::playback;
using namespace mu::modularity;
using namespace mu::ui;
using namespace mu::actions;

static std::shared_ptr<PlaybackConfiguration> s_configuration = std::make_shared<PlaybackConfiguration>();
static std::shared_ptr<PlaybackController> s_playbackController = std::make_shared<PlaybackController>();
static std::shared_ptr<PlaybackUiActions> s_playbackUiActions = std::make_shared<PlaybackUiActions>(s_playbackController);

static void playback_init_qrc()
{
    Q_INIT_RESOURCE(playback);
}

std::string PlaybackModule::moduleName() const
{
    return "playback";
}

void PlaybackModule::registerExports()
{
    ioc()->registerExport<IPlaybackController>(moduleName(), s_playbackController);
    ioc()->registerExport<IPlaybackConfiguration>(moduleName(), s_configuration);
}

void PlaybackModule::resolveImports()
{
    auto ar = ioc()->resolve<IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(s_playbackUiActions);
    }
}

void PlaybackModule::registerResources()
{
    playback_init_qrc();
}

void PlaybackModule::registerUiTypes()
{
    qmlRegisterType<PlaybackToolBarModel>("MuseScore.Playback", 1, 0, "PlaybackToolBarModel");
    qmlRegisterType<MixerPanelModel>("MuseScore.Playback", 1, 0, "MixerPanelModel");

    ioc()->resolve<IUiEngine>(moduleName())->addSourceImportPath(playback_QML_IMPORT);
}

void PlaybackModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (framework::IApplication::RunMode::Editor != mode) {
        return;
    }

    s_configuration->init();
    s_playbackController->init();
    s_playbackUiActions->init();
}
