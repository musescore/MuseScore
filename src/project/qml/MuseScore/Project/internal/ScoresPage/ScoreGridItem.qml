/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.GraphicalEffects 1.0
import MuseScore.Project 1.0

FocusScope {
    id: root

    property string name: ""
    property string path: ""
    property string suffix: ""
    property alias timeSinceModified: timeSinceModified.text
    property string thumbnailUrl: ""
    property bool isCreateNew: false
    property bool isNoResultsFound: false
    property bool isCloud: false
    property int cloudScoreId: 0

    property alias navigation: navCtrl

    signal clicked()

    NavigationControl {
        id: navCtrl
        name: root.name
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.Button
        accessible.name: root.name

        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }

        onTriggered: root.clicked()
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        hoverEnabled: true

        onClicked: {
            root.clicked()
        }
    }

    Column {
        anchors.fill: parent

        spacing: 16

        Item {
            height: 224
            width: 172

            Item {
                id: thumbnail
                anchors.fill: parent

                opacity: 0.9

                property int borderWidth: 0
                readonly property int radius: 3

                Loader {
                    id: loader

                    anchors.fill: parent

                    sourceComponent: {
                        if (root.isCreateNew) {
                            return addComp
                        }

                        if (root.isNoResultsFound) {
                            return noResultFoundComp
                        }

                        return scoreItemComp
                    }

                    layer.enabled: ui.isEffectsAllowed
                    layer.effect: EffectOpacityMask {
                        maskSource: Rectangle {
                            width: thumbnail.width
                            height: thumbnail.height
                            radius: thumbnail.radius
                        }
                    }
                }

                Rectangle {
                    anchors.fill: parent

                    color: "transparent"
                    radius: parent.radius

                    NavigationFocusBorder {
                        navigationCtrl: navCtrl

                        padding: 2
                    }

                    border.color: ui.theme.strokeColor
                    border.width: parent.borderWidth
                }

                states: [
                    State {
                        name: "NORMAL"
                        when: !mouseArea.containsMouse && !mouseArea.pressed

                        PropertyChanges {
                            target: thumbnail
                            borderWidth: ui.theme.borderWidth
                        }
                    },

                    State {
                        name: "HOVERED"
                        when: mouseArea.containsMouse && !mouseArea.pressed

                        PropertyChanges {
                            target: thumbnail
                            opacity: 1
                            borderWidth: 1
                        }
                    },

                    State {
                        name: "PRESSED"
                        when: mouseArea.pressed

                        PropertyChanges {
                            target: thumbnail
                            opacity: 0.5
                        }
                    }
                ]

                EffectRectangularGlow {
                    anchors.fill: thumbnail
                    z: -1

                    glowRadius: 20
                    color: "#08000000"
                    cornerRadius: thumbnail.radius + glowRadius
                }
            }

            Loader {
                active: root.isCloud

                anchors.left: parent.left
                anchors.leftMargin: 8
                anchors.right: parent.right
                anchors.rightMargin: 8
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 8

                sourceComponent: RowLayout {
                    visible: root.isCloud

                    spacing: 8

                    CloudScoreStatusWatcher {
                        id: cloudScoreStatusWatcher
                    }

                    Component.onCompleted: {
                        cloudScoreStatusWatcher.load(root.cloudScoreId)
                    }

                    ProgressBar {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 16

                        visible: cloudScoreStatusWatcher.isProgress

                        from: 0
                        to: cloudScoreStatusWatcher.progressTotal
                        value: cloudScoreStatusWatcher.progressCurrent

                        navigation.panel: root.navigation.panel
                        navigation.row: root.navigation.row
                        navigation.column: root.navigation.column + 1
                    }

                    CloudScoreIndicatorButton {
                        Layout.alignment: Qt.AlignTrailing | Qt.AlignVCenter

                        isProgress: cloudScoreStatusWatcher.isProgress
                        isDownloadedAndUpToDate: cloudScoreStatusWatcher.isDownloadedAndUpToDate

                        navigation.panel: root.navigation.panel
                        navigation.row: root.navigation.row
                        navigation.column: root.navigation.column + 2

                        onClicked: {
                            if (isProgress) {
                                cloudScoreStatusWatcher.cancel()
                            } else {
                                root.clicked()
                            }
                        }
                    }
                }
            }
        }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right

            spacing: 4

            StyledTextLabel {
                anchors.horizontalCenter: parent.horizontalCenter

                text: root.name

                wrapMode: Text.WrapAnywhere
                maximumLineCount: 1
                width: parent.width

                font: ui.theme.largeBodyFont
            }

            StyledTextLabel {
                id: timeSinceModified

                anchors.horizontalCenter: parent.horizontalCenter

                font.capitalization: Font.AllUppercase

                visible: !root.isCreateNew && !root.isNoResultsFound
            }
        }
    }

    Component {
        id: addComp

        Rectangle {
            anchors.fill: parent
            color: "white"

            StyledIconLabel {
                anchors.centerIn: parent

                iconCode: IconCode.PLUS

                font.pixelSize: 50
                color: "black"
            }
        }
    }

    Component {
        id: noResultFoundComp

        Rectangle {
            anchors.fill: parent
            color: ui.theme.backgroundPrimaryColor

            StyledTextLabel {
                anchors.fill: parent
                text: qsTrc("global", "No results found")
            }
        }
    }

    Component {
        id: scoreItemComp

        ScoreThumbnail {
            path: root.path
            suffix: root.suffix
            thumbnailUrl: root.thumbnailUrl
        }
    }
}
