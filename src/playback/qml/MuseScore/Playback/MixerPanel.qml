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
import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Audio 1.0
import MuseScore.Playback 1.0

import "internal"

Rectangle {
    id: root

    property alias contextMenuModel: contextMenuModel

    property NavigationSection navigationSection: null

    color: ui.theme.backgroundPrimaryColor

    Flickable {
        id: flickable

        function positionViewAtEnd() {
            if (flickable.contentY == flickable.contentHeight) {
                return
            }

            flickable.contentY = flickable.contentHeight - flickable.height
        }

        onContentHeightChanged: {
            flickable.positionViewAtEnd()
        }

        anchors.fill: parent

        clip: true
        boundsBehavior: Flickable.StopAtBounds

        contentWidth: contentColumn.width
        contentHeight: contentColumn.height
        interactive: height < contentHeight || width < contentWidth

        ScrollBar.vertical: StyledScrollBar { policy: ScrollBar.AlwaysOn }
        ScrollBar.horizontal: StyledScrollBar {}

        MixerPanelModel {
            id: mixerPanelModel

            Component.onCompleted: {
                mixerPanelModel.load(root.navigationSection)
            }
        }

        MixerPanelContextMenuModel {
            id: contextMenuModel

            Component.onCompleted: {
                contextMenuModel.load()
            }
        }

        Column {
            id: contentColumn

            width: childrenRect.width

            spacing: 8

            MixerSoundSection {
                id: soundSection

                visible: contextMenuModel.soundSectionVisible
                headerVisible: contextMenuModel.labelsSectionVisible
                rootPanel: root

                model: mixerPanelModel

                navigationRowStart: 1
            }

            MixerFxSection {
                id: fxSection

                visible: contextMenuModel.audioFxSectionVisible
                headerVisible: contextMenuModel.labelsSectionVisible
                rootPanel: root

                model: mixerPanelModel

                navigationRowStart: soundSection.navigationRowEnd + 1
            }

            MixerBalanceSection {
                id: balanceSection

                visible: contextMenuModel.balanceSectionVisible
                headerVisible: contextMenuModel.labelsSectionVisible
                rootPanel: root

                model: mixerPanelModel

                navigationRowStart: fxSection.navigationRowEnd + 1
            }

            MixerVolumeSection {
                id: volumeSection

                visible: contextMenuModel.volumeSectionVisible
                headerVisible: contextMenuModel.labelsSectionVisible
                rootPanel: root

                model: mixerPanelModel

                navigationRowStart: balanceSection.navigationRowEnd + 1
            }

            MixerFaderSection {
                id: faderSection

                visible: contextMenuModel.faderSectionVisible
                headerVisible: contextMenuModel.labelsSectionVisible
                rootPanel: root

                model: mixerPanelModel

                navigationRowStart: volumeSection.navigationRowEnd + 1
            }

            MixerMuteAndSoloSection {
                id: muteAndSoloSection

                visible: contextMenuModel.muteAndSoloSectionVisible
                headerVisible: contextMenuModel.labelsSectionVisible
                rootPanel: root

                model: mixerPanelModel

                navigationRowStart: muteAndSoloSection.navigationRowEnd + 1
            }

            MixerTitleSection {
                id: titleSection

                visible: contextMenuModel.titleSectionVisible
                headerVisible: contextMenuModel.labelsSectionVisible
                rootPanel: root

                model: mixerPanelModel

                navigationRowStart: muteAndSoloSection.navigationRowEnd + 1
            }
        }
    }

    onHeightChanged: {
        flickable.positionViewAtEnd()
    }
}
