/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited and others
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
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import Muse.GraphicalEffects
import MuseScore.Project

FocusScope {
    id: root

    property string name: ""
    property string path: ""
    property string suffix: ""
    property alias timeSinceModified: timeSinceModified.text
    property string thumbnailUrl: ""
    property bool isCreateNew: false
    property bool isNoResultsFound: false
    property var processingStatus: undefined
    readonly property bool isProcessing: root.processingStatus !== undefined
    property int convertType: 0
    property int convertId: 0
    property bool isCloud: false
    property int cloudScoreId: 0
    property bool showRemoveFromRecentFiles: false

    property alias navigation: navCtrl

    signal clicked()
    signal revealInFileBrowserRequested(string scorePath)
    signal viewOnlineRequested(int scoreId)
    signal removeFromRecentFilesRequested(string scorePath)
    signal retryRequested()
    signal cancelRequested(int convertType, int convertId)

    NavigationControl {
        id: navCtrl
        name: root.name
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.Button
        accessible.name: {
            if (root.isProcessing) {
                if (root.processingStatus === ScoreProcessingPlaceholder.Failed) {
                    //: %1 is the name of the score whose conversion failed
                    return qsTrc("project/convert", "Processing failed: %1").arg(root.name)
                }

                //: %1 is the name of the score being converted
                return qsTrc("project", "Processing %1").arg(root.name)
            }

            return root.name
        }

        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }

        onTriggered: root.clicked()
    }

    MouseArea {
        id: rootMouseArea
        anchors.fill: parent

        enabled: root.enabled && !root.isProcessing
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: function(mouse) {
            navCtrl.requestActiveByInteraction()

            if (mouse.button === Qt.RightButton) {
                if (contextMenu.menuModel.length > 0) {
                    contextMenu.show(Qt.point(mouse.x, mouse.y), root)
                }
                return
            }

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
                            return createNewComp
                        }

                        if (root.isProcessing) {
                            return processingComp
                        }

                        if (root.isNoResultsFound) {
                            return noResultFoundComp
                        }

                        return scoreItemComp
                    }

                    layer.enabled: ui.isEffectsAllowed
                    layer.effect: RoundedCornersEffect {
                        radius: thumbnail.radius
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
                        when: !rootMouseArea.containsMouse && !rootMouseArea.pressed

                        PropertyChanges {
                            target: thumbnail
                            borderWidth: ui.theme.borderWidth
                        }
                    },

                    State {
                        name: "HOVERED"
                        when: rootMouseArea.containsMouse && !rootMouseArea.pressed

                        PropertyChanges {
                            target: thumbnail
                            opacity: 1
                            borderWidth: 1
                        }
                    },

                    State {
                        name: "PRESSED"
                        when: rootMouseArea.pressed

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

            ScoreItemMenuButton {
                id: contextMenu

                anchors.top: parent.top
                anchors.topMargin: 8
                anchors.right: parent.right
                anchors.rightMargin: 8
                visible: menuModel.length > 0
                         && (rootMouseArea.containsMouse
                             || mouseArea.containsMouse
                             || root.navigation.active
                             || navigation.active
                             || isMenuOpenedByButton)
                transparent: !isMenuOpenedByButton && !rootMouseArea.containsMouse

                isCreateNew: root.isCreateNew
                isNoResultsFound: root.isNoResultsFound
                isCloud: root.isCloud
                showRemoveFromRecentFiles: root.showRemoveFromRecentFiles

                navigation.panel: root.navigation.panel
                navigation.row: root.navigation.row
                navigation.column: root.navigation.column + 1

                onOpenRequested: root.clicked()
                onViewOnlineRequested: root.viewOnlineRequested(root.cloudScoreId)
                onRevealInFileBrowserRequested: root.revealInFileBrowserRequested(root.path)
                onRemoveFromRecentFilesRequested: root.removeFromRecentFilesRequested(root.path)
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
                        navigation.column: root.navigation.column + 2
                    }

                    CloudScoreIndicatorButton {
                        Layout.alignment: Qt.AlignTrailing | Qt.AlignVCenter

                        isProgress: cloudScoreStatusWatcher.isProgress
                        isDownloadedAndUpToDate: cloudScoreStatusWatcher.isDownloadedAndUpToDate

                        navigation.panel: root.navigation.panel
                        navigation.row: root.navigation.row
                        navigation.column: root.navigation.column + 3

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

                visible: !root.isCreateNew && !root.isNoResultsFound && !root.isProcessing
            }
        }
    }

    Component {
        id: createNewComp

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
        id: processingComp

        ScoreProcessingPlaceholder {
            status: root.processingStatus
            iconSize: 24

            navigationPanel: root.navigation.panel
            navigationRow: root.navigation.row
            navigationColumn: root.navigation.column + 2

            onRetryRequested: root.retryRequested()
            onCancelRequested: root.cancelRequested(root.convertType, root.convertId)
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
