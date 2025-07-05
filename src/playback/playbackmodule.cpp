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

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "ui/iuiactionsregister.h"
#include "ui/iinteractiveuriregister.h"

#include "internal/playbackcontroller.h"
#include "internal/playbackuiactions.h"
#include "internal/playbackconfiguration.h"
#include "internal/soundprofilesrepository.h"

#include "view/playbacktoolbarmodel.h"
#include "view/playbackloadingmodel.h"
#include "view/mixerpanelmodel.h"
#include "view/mixerpanelcontextmenumodel.h"
#include "view/soundprofilesmodel.h"
#include "view/internal/soundflag/soundflagsettingsmodel.h"
#include "view/internal/onlinesoundsstatusmodel.h"
#include "view/internal/notationregionsbeingprocessedmodel.h"

using namespace mu::playback;
using namespace muse;
using namespace muse::modularity;
using namespace muse::ui;
using namespace muse::actions;

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
    m_configuration = std::make_shared<PlaybackConfiguration>();
    m_playbackController = std::make_shared<PlaybackController>();
    m_playbackUiActions = std::make_shared<PlaybackUiActions>(m_playbackController);
    m_soundProfileRepo = std::make_shared<SoundProfilesRepository>();

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
        ir->registerUri(Uri("musescore://playback/soundprofilesdialog"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/Playback/SoundProfilesDialog.qml"));
    }
}

void PlaybackModule::registerResources()
{
    playback_init_qrc();
}

void PlaybackModule::registerUiTypes()
{
    qmlRegisterType<PlaybackToolBarModel>("MuseScore.Playback", 1, 0, "PlaybackToolBarModel");
    qmlRegisterType<PlaybackLoadingModel>("MuseScore.Playback", 1, 0, "PlaybackLoadingModel");
    qmlRegisterType<MixerPanelModel>("MuseScore.Playback", 1, 0, "MixerPanelModel");
    qmlRegisterType<MixerPanelContextMenuModel>("MuseScore.Playback", 1, 0, "MixerPanelContextMenuModel");
    qmlRegisterType<SoundProfilesModel>("MuseScore.Playback", 1, 0, "SoundProfilesModel");

    qmlRegisterType<SoundFlagSettingsModel>("MuseScore.Playback", 1, 0, "SoundFlagSettingsModel");
    qmlRegisterType<OnlineSoundsStatusModel>("MuseScore.Playback", 1, 0, "OnlineSoundsStatusModel");

    qmlRegisterType<NotationRegionsBeingProcessedModel>("MuseScore.Playback", 1, 0, "NotationRegionsBeingProcessedModel");

    qmlRegisterUncreatableType<MixerChannelItem>("MuseScore.Playback", 1, 0, "MixerChannelItem", "Cannot create a MixerChannelItem");

    ioc()->resolve<IUiEngine>(moduleName())->addSourceImportPath(playback_QML_IMPORT);
}

void PlaybackModule::onInit(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    m_configuration->init();
    m_soundProfileRepo->init();
    m_playbackController->init();

    if (mode != IApplication::RunMode::GuiApp) {
        return;
    }

    m_playbackUiActions->init();
}

void PlaybackModule::onAllInited(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    m_soundProfileRepo->refresh();
}
