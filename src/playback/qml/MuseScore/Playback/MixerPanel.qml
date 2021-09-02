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

    property alias navigation: flickable.navigation
    property alias contextMenuModel: flickable.contextMenuModel

    color: ui.theme.backgroundPrimaryColor

    Flickable {
        id: flickable

        property alias navigation: navPanel
        property alias contextMenuModel: contextMenuModel

        anchors.fill: parent

        clip: true

        contentWidth: width
        contentHeight: contentColumn.height
        interactive: height < contentHeight

        ScrollBar.vertical: StyledScrollBar {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.rightMargin: 16
        }

        NavigationPanel {
            id: navPanel
            name: "MixerPanel"
            enabled: root.enabled && root.visible
        }

        MixerPanelModel {
            id: mixerPanelModel

            Component.onCompleted: {
                mixerPanelModel.load()
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

            anchors.left: parent.left
            anchors.right: parent.right

            spacing: 8

            MixerSoundSection {
                id: soundSection

                active: contextMenuModel.soundSectionVisible
                headerVisible: contextMenuModel.labelsSectionVisible

                model: mixerPanelModel
            }

            MixerFxSection {
                id: fxSection

                active: contextMenuModel.audioFxSectionVisible
                headerVisible: contextMenuModel.labelsSectionVisible

                model: mixerPanelModel
            }

            MixerBalanceSection {
                id: balanceSection

                active: contextMenuModel.balanceSectionVisible
                headerVisible: contextMenuModel.labelsSectionVisible

                model: mixerPanelModel
            }

            MixerVolumeSection {
                id: volumeSection

                active: contextMenuModel.volumeSectionVisible
                headerVisible: contextMenuModel.labelsSectionVisible

                model: mixerPanelModel
            }

            MixerTitleSection {
                id: titleSection

                active: contextMenuModel.titleSectionVisible
                headerVisible: contextMenuModel.labelsSectionVisible

                model: mixerPanelModel
            }
        }
    }
}
