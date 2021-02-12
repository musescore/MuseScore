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
#include "playbackmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "actions/iactionsregister.h"
#include "ui/iinteractiveuriregister.h"

#include "internal/playbackcontroller.h"
#include "internal/playbackactions.h"
#include "internal/playbackconfiguration.h"

#include "view/playbacktoolbarmodel.h"
#include "view/internal/playbacksettingsmodel.h"

using namespace mu::playback;
using namespace mu::framework;
using namespace mu::ui;
using namespace mu::actions;

static std::shared_ptr<PlaybackController> s_playbackController = std::make_shared<PlaybackController>();
static std::shared_ptr<PlaybackConfiguration> s_configuration = std::make_shared<PlaybackConfiguration>();

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
    auto ar = ioc()->resolve<IActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<PlaybackActions>());
    }
}

void PlaybackModule::registerResources()
{
    playback_init_qrc();
}

void PlaybackModule::registerUiTypes()
{
    qmlRegisterType<PlaybackToolBarModel>("MuseScore.Playback", 1, 0, "PlaybackToolBarModel");
    qmlRegisterType<PlaybackSettingsModel>("MuseScore.Playback", 1, 0, "PlaybackSettingsModel");

    ioc()->resolve<IUiEngine>(moduleName())->addSourceImportPath(playback_QML_IMPORT);
}

void PlaybackModule::onInit(const IApplication::RunMode& mode)
{
    if (IApplication::RunMode::Editor != mode) {
        return;
    }

    s_configuration->init();
    s_playbackController->init();
}
