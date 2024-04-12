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
#include "playbackstubmodule.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "playbackcontrollerstub.h"
#include "playbackconfigurationstub.h"

using namespace mu::playback;
using namespace muse::modularity;

static void playback_init_qrc()
{
    Q_INIT_RESOURCE(playback);
}

std::string PlaybackModule::moduleName() const
{
    return "playback_stub";
}

void PlaybackModule::registerExports()
{
    ioc()->registerExport<IPlaybackController>(moduleName(), new PlaybackControllerStub());
    ioc()->registerExport<IPlaybackConfiguration>(moduleName(), new PlaybackConfigurationStub());
}

void PlaybackModule::registerResources()
{
    playback_init_qrc();
}

void PlaybackModule::registerUiTypes()
{
    std::shared_ptr<muse::ui::IUiEngine> ui = ioc()->resolve<muse::ui::IUiEngine>(moduleName());
    if (ui) {
        ui->addSourceImportPath(playback_QML_IMPORT);
    }
}
