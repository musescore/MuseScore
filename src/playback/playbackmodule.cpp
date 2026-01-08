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

#include "playbackmodule.h"

#include "modularity/ioc.h"

#include "ui/iuiactionsregister.h"
#include "ui/iinteractiveuriregister.h"

#include "internal/playbackcontroller.h"
#include "internal/playbackuiactions.h"
#include "internal/playbackconfiguration.h"
#include "internal/soundprofilesrepository.h"

using namespace mu::playback;
using namespace muse;
using namespace muse::modularity;
using namespace muse::ui;
using namespace muse::actions;

std::string PlaybackModule::moduleName() const
{
    return "playback";
}

void PlaybackModule::registerExports()
{
    m_configuration = std::make_shared<PlaybackConfiguration>(iocContext());
    m_playbackController = std::make_shared<PlaybackController>(iocContext());
    m_playbackUiActions = std::make_shared<PlaybackUiActions>(m_playbackController, iocContext());
    m_soundProfileRepo = std::make_shared<SoundProfilesRepository>(iocContext());

    ioc()->registerExport<IPlaybackController>(moduleName(), m_playbackController);
    ioc()->registerExport<IPlaybackConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<ISoundProfilesRepository>(moduleName(), m_soundProfileRepo);
}

void PlaybackModule::resolveImports()
{
    auto ar = ioc()->resolve<IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(m_playbackUiActions);
    }

    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("musescore://playback/soundprofilesdialog"), "MuseScore.Playback", "SoundProfilesDialog");
    }
}

void PlaybackModule::onInit(const IApplication::RunMode& mode)
{
    m_configuration->init();
    m_soundProfileRepo->init();
    m_playbackController->init();

    if (mode != IApplication::RunMode::GuiApp) {
        return;
    }

    m_playbackUiActions->init();
}
