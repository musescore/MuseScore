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
#include "interactive/iinteractiveuriregister.h"

#include "internal/playbackcontroller.h"
#include "internal/playbackuiactions.h"
#include "internal/playbackconfiguration.h"
#include "internal/soundprofilesrepository.h"

using namespace mu::playback;
using namespace muse;
using namespace muse::modularity;
using namespace muse::actions;

static const std::string mname("playback");

std::string PlaybackModule::moduleName() const
{
    return mname;
}

void PlaybackModule::registerExports()
{
    m_configuration = std::make_shared<PlaybackConfiguration>();

    globalIoc()->registerExport<IPlaybackConfiguration>(mname, m_configuration);
}

void PlaybackModule::resolveImports()
{
    auto ir = globalIoc()->resolve<muse::interactive::IInteractiveUriRegister>(mname);
    if (ir) {
        ir->registerQmlUri(Uri("musescore://playback/soundprofilesdialog"), "MuseScore.Playback", "SoundProfilesDialog");
    }
}

void PlaybackModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();
}

IContextSetup* PlaybackModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new PlaybackContext(ctx);
}

void PlaybackContext::registerExports()
{
    m_playbackController = std::make_shared<PlaybackController>(iocContext());
    m_playbackUiActions = std::make_shared<PlaybackUiActions>(m_playbackController, iocContext());
    m_soundProfileRepo = std::make_shared<SoundProfilesRepository>(iocContext());

    ioc()->registerExport<IPlaybackController>(mname, m_playbackController);
    ioc()->registerExport<ISoundProfilesRepository>(mname, m_soundProfileRepo);
}

void PlaybackContext::resolveImports()
{
    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(mname);
    if (ar) {
        ar->reg(m_playbackUiActions);
    }
}

void PlaybackContext::onInit(const IApplication::RunMode& mode)
{
    m_soundProfileRepo->init();
    m_playbackController->init();

    if (mode != IApplication::RunMode::GuiApp) {
        return;
    }

    m_playbackUiActions->init();
}
