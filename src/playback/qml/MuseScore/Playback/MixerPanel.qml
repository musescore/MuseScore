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

    QtObject {
        id: prv

        property var currentNavigateControlIndex: []
        property bool isPanelActivated: false

        function setNavigateControlIndex(index) {
            if (index[0] !== prv.currentNavigateControlIndex[0] ||
                    index[1] !== prv.currentNavigateControlIndex[1]) {
                prv.isPanelActivated = false
            }

            prv.currentNavigateControlIndex = index
        }
    }

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

            onModelReset: {
                Qt.callLater(setupConnections)
            }

            function setupConnections() {
                for (var i = 0; i < mixerPanelModel.rowCount(); i++) {
                    var item = mixerPanelModel.get(i)
                    item.item.panel.navigationEvent.connect(function(event){
                        if (event.type === NavigationEvent.AboutActive) {
                            event.setData("controlIndex", prv.currentNavigateControlIndex)
                            event.setData("controlOptional", true)
                            prv.isPanelActivated = true
                        }
                    })
                }
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
                needReadChannelName: prv.isPanelActivated

                onNavigateControlIndexChanged: {
                    prv.setNavigateControlIndex(index)
                }
            }

            MixerFxSection {
                id: fxSection

                visible: contextMenuModel.audioFxSectionVisible
                headerVisible: contextMenuModel.labelsSectionVisible
                rootPanel: root

                model: mixerPanelModel

                navigationRowStart: 100
                needReadChannelName: prv.isPanelActivated

                onNavigateControlIndexChanged: {
                    prv.setNavigateControlIndex(index)
                }
            }

            MixerBalanceSection {
                id: balanceSection

                visible: contextMenuModel.balanceSectionVisible
                headerVisible: contextMenuModel.labelsSectionVisible
                rootPanel: root

                model: mixerPanelModel

                navigationRowStart: 200
                needReadChannelName: prv.isPanelActivated

                onNavigateControlIndexChanged: {
                    prv.setNavigateControlIndex(index)
                }
            }

            MixerVolumeSection {
                id: volumeSection

                visible: contextMenuModel.volumeSectionVisible
                headerVisible: contextMenuModel.labelsSectionVisible
                rootPanel: root

                model: mixerPanelModel

                navigationRowStart: 300
                needReadChannelName: prv.isPanelActivated

                onNavigateControlIndexChanged: {
                    prv.setNavigateControlIndex(index)
                }
            }

            MixerFaderSection {
                id: faderSection

                visible: contextMenuModel.faderSectionVisible
                headerVisible: contextMenuModel.labelsSectionVisible
                rootPanel: root

                model: mixerPanelModel

                navigationRowStart: 400
                needReadChannelName: prv.isPanelActivated

                onNavigateControlIndexChanged: {
                    prv.setNavigateControlIndex(index)
                }
            }

            MixerMuteAndSoloSection {
                id: muteAndSoloSection

                visible: contextMenuModel.muteAndSoloSectionVisible
                headerVisible: contextMenuModel.labelsSectionVisible
                rootPanel: root

                model: mixerPanelModel

                navigationRowStart: 500
                needReadChannelName: prv.isPanelActivated

                onNavigateControlIndexChanged: {
                    prv.setNavigateControlIndex(index)
                }
            }

            MixerTitleSection {
                id: titleSection

                visible: contextMenuModel.titleSectionVisible
                headerVisible: contextMenuModel.labelsSectionVisible
                rootPanel: root

                model: mixerPanelModel

                navigationRowStart: 600
                needReadChannelName: prv.isPanelActivated

                onNavigateControlIndexChanged: {
                    prv.setNavigateControlIndex(index)
                }
            }
        }
    }

    onHeightChanged: {
        flickable.positionViewAtEnd()
    }
}
