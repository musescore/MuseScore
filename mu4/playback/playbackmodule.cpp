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

#include "internal/playbackcontroller.h"
#include "internal/playbackactions.h"
#include "internal/playbackconfiguration.h"

#include "view/playbacktoolbarmodel.h"

using namespace mu::playback;

static std::shared_ptr<PlaybackController> pcontroller = std::make_shared<PlaybackController>();

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
    framework::ioc()->registerExport<IPlaybackController>(moduleName(), pcontroller);
    framework::ioc()->registerExport<IPlaybackConfiguration>(moduleName(), new PlaybackConfiguration());
}

void PlaybackModule::resolveImports()
{
    auto ar = framework::ioc()->resolve<actions::IActionsRegister>(moduleName());
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

    framework::ioc()->resolve<framework::IUiEngine>(moduleName())->addSourceImportPath(playback_QML_IMPORT);
}

void PlaybackModule::onInit()
{
    pcontroller->init();
}
