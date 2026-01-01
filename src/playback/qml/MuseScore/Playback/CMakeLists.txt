# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

muse_create_qml_module(playback_qml FOR playback)

qt_add_qml_module(playback_qml
    URI MuseScore.Playback
    VERSION 1.0
    SOURCES
        abstractaudioresourceitem.cpp
        abstractaudioresourceitem.h
        auxsenditem.cpp
        auxsenditem.h
        inputresourceitem.cpp
        inputresourceitem.h
        mixerchannelitem.cpp
        mixerchannelitem.h
        mixerpanelcontextmenumodel.cpp
        mixerpanelcontextmenumodel.h
        mixerpanelmodel.cpp
        mixerpanelmodel.h
        msbasicpresetscategories.h
        notationregionsbeingprocessedmodel.cpp
        notationregionsbeingprocessedmodel.h
        onlinesoundsstatusmodel.cpp
        onlinesoundsstatusmodel.h
        outputresourceitem.cpp
        outputresourceitem.h
        playbackloadingmodel.cpp
        playbackloadingmodel.h
        playbacktoolbarmodel.cpp
        playbacktoolbarmodel.h
        soundflagsettingsmodel.cpp
        soundflagsettingsmodel.h
        soundprofilesmodel.cpp
        soundprofilesmodel.h
    QML_FILES
        internal/AudioProcessingProgressBar.qml
        internal/AudioResourceControl.qml
        internal/AuxSendControl.qml
        internal/KnobControl.qml
        internal/MeasureAndBeatFields.qml
        internal/MixerAuxSendsSection.qml
        internal/MixerBalanceSection.qml
        internal/MixerFaderSection.qml
        internal/MixerFxSection.qml
        internal/MixerMuteAndSoloSection.qml
        internal/MixerPanelSection.qml
        internal/MixerPanelToolbar.qml
        internal/MixerSoundSection.qml
        internal/MixerTitleSection.qml
        internal/MixerVolumeSection.qml
        internal/PlaybackSpeedPopup.qml
        internal/PlaybackSpeedSlider.qml
        internal/PlaybackToolBarActions.qml
        internal/SoundFlag/MuseSoundsParams.qml
        internal/SoundFlag/ParamsGridView.qml
        internal/VolumePressureMeter.qml
        internal/VolumeSlider.qml
        MixerPanel.qml
        NotationRegionsBeingProcessedView.qml
        OnlineSoundsStatusView.qml
        PlaybackLoadingInfo.qml
        PlaybackToolBar.qml
        SoundFlagPopup.qml
        SoundProfilesDialog.qml
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
        TARGET notationscene_qml
)

fixup_qml_module_dependencies(playback_qml)
