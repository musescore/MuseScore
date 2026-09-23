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

ListItemBlank {
    id: root

    required property var score
    property alias columns: columnsRepeater.model

    property alias thumbnailComponent: thumbnailLoader.sourceComponent

    property real itemInset: 12
    property real columnSpacing: 44
    property alias showBottomBorder: bottomBorder.visible
    property bool showRemoveFromRecentFiles: false

    signal revealInFileBrowserRequested(string scorePath)
    signal viewOnlineRequested(int scoreId)
    signal removeFromRecentFilesRequested(string scorePath)
    signal retryRequested()
    signal cancelRequested(int convertType, int convertId)

    readonly property bool isProcessing: root.score.processingStatus !== undefined

    implicitHeight: 64

    navigation.accessible.name: {
        const name = root.score.name ?? ""

        if (root.isProcessing) {
            if (root.score.processingStatus === ScoreProcessingPlaceholder.Failed) {
                //: %1 is the name of the score whose conversion failed
                return qsTrc("project/convert", "Processing failed: %1").arg(name)
            }

            //: %1 is the name of the score being converted
            return qsTrc("project", "Processing %1").arg(name)
        }

        return name
    }
    navigation.onActiveChanged: {
        if (navigation.active) {
            root.scrollIntoView()
        }
    }

    focusBorder.anchors.bottomMargin: bottomBorder.visible ? bottomBorder.height : 0

    MouseArea {
        anchors.fill: parent
        enabled: root.visible && root.enabled
        acceptedButtons: Qt.RightButton
        onClicked: function(mouse) {
            if (contextMenu.menuModel.length > 0) {
                contextMenu.show(Qt.point(mouse.x, mouse.y), root)
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: root.itemInset
        anchors.rightMargin: root.itemInset

        spacing: root.columnSpacing

        RowLayout {
            spacing: 24

            Loader {
                id: thumbnailLoader

                Layout.preferredWidth: 30
                Layout.preferredHeight: 40

                sourceComponent: root.isProcessing ? processingComp : scoreThumbnailComp

                layer.enabled: ui.isEffectsAllowed
                layer.effect: RoundedCornersEffect {
                    radius: 2
                }
            }

            Component {
                id: scoreThumbnailComp

                ScoreThumbnail {
                    path: root.score.path ?? ""
                    suffix: root.score.suffix ?? ""
                    thumbnailUrl: root.score.thumbnailUrl ?? ""
                }
            }

            Component {
                id: processingComp

                ScoreProcessingPlaceholder {
                    status: root.score.processingStatus
                    compact: true
                    iconSize: 16

                    onRetryRequested: root.retryRequested()
                    onCancelRequested: root.cancelRequested(root.score.convertType, root.score.convertId)
                }
            }

            StyledTextLabel {
                Layout.fillWidth: true

                text: root.score.name ?? ""
                font: ui.theme.largeBodyFont
                horizontalAlignment: Text.AlignLeft
            }

            Loader {
                active: root.isProcessing || (root.score.isCloud ?? false)

                sourceComponent: root.isProcessing ? processingStatusComp : cloudStatusComp
            }

            Component {
                id: processingStatusComp

                RowLayout {
                    spacing: 12

                    StyledTextLabel {
                        text: root.score.processingStatus === ScoreProcessingPlaceholder.Failed
                              ? qsTrc("project/convert", "Processing failed")
                              : qsTrc("global", "Processing…")
                        font: ui.theme.bodyBoldFont
                        horizontalAlignment: Text.AlignLeft
                    }

                    FlatButton {
                        minWidth: 60

                        text: root.score.processingStatus === ScoreProcessingPlaceholder.Failed
                              ? qsTrc("global", "Retry")
                              : qsTrc("global", "Cancel")

                        navigation.panel: root.navigation.panel
                        navigation.row: root.navigation.row
                        navigation.column: 2
                        navigation.onActiveChanged: {
                            if (navigation.active) {
                                root.scrollIntoView()
                            }
                        }

                        onClicked: {
                            if (root.score.processingStatus === ScoreProcessingPlaceholder.Failed) {
                                root.retryRequested()
                            } else {
                                root.cancelRequested(root.score.convertType, root.score.convertId)
                            }
                        }
                    }
                }
            }

            Component {
                id: cloudStatusComp

                RowLayout {
                    visible: root.score.isCloud

                    spacing: 24

                    CloudScoreStatusWatcher {
                        id: cloudScoreStatusWatcher
                    }

                    Component.onCompleted: {
                        cloudScoreStatusWatcher.load(root.score.scoreId)
                    }

                    ProgressBar {
                        Layout.preferredWidth: 118
                        Layout.preferredHeight: 16

                        visible: cloudScoreStatusWatcher.isProgress

                        from: 0
                        to: cloudScoreStatusWatcher.progressTotal
                        value: cloudScoreStatusWatcher.progressCurrent

                        navigation.panel: root.navigation.panel
                        navigation.row: root.navigation.row
                        navigation.column: 2
                        navigation.onActiveChanged: {
                            if (navigation.active) {
                                root.scrollIntoView()
                            }
                        }
                    }

                    CloudScoreIndicatorButton {
                        Layout.alignment: Qt.AlignTrailing | Qt.AlignVCenter

                        isProgress: cloudScoreStatusWatcher.isProgress
                        isDownloadedAndUpToDate: cloudScoreStatusWatcher.isDownloadedAndUpToDate

                        navigation.panel: root.navigation.panel
                        navigation.row: root.navigation.row
                        navigation.column: 3
                        navigation.onActiveChanged: {
                            if (navigation.active) {
                                root.scrollIntoView()
                            }
                        }

                        onClicked: {
                            if (isProgress) {
                                cloudScoreStatusWatcher.cancel()
                            } else {
                                root.clicked(null)
                            }
                        }
                    }
                }
            }
        }

        Repeater {
            id: columnsRepeater

            delegate: Loader {
                Layout.preferredWidth: modelData.width(parent.width)

                // These properties are here to give the delegate access to them
                readonly property ScoreListItem listItem: root
                readonly property var score: root.score
                readonly property NavigationPanel navigationPanel: root.navigation.panel
                readonly property int navigationRow: root.navigation.row
                readonly property int navigationColumnStart: 100 * (model.index + 1)

                sourceComponent: modelData.delegate
            }
        }
    }

    ScoreItemMenuButton {
        id: contextMenu

        // Drawn on top of the row instead of taking a spot in the RowLayout, so hover never shifts the columns
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: root.itemInset
        width: 20
        height: 20
        z: 1

        isCreateNew: root.score.isCreateNew ?? false
        isNoResultsFound: root.score.isNoResultsFound ?? false
        isCloud: root.score.isCloud ?? false
        showRemoveFromRecentFiles: root.showRemoveFromRecentFiles

        visible: root.mouseArea.containsMouse
                 || mouseArea.containsMouse
                 || root.navigation.active
                 || navigation.active
                 || isMenuOpenedByButton

        navigation.panel: root.navigation.panel
        navigation.row: root.navigation.row
        navigation.column: 100 * (root.columns.length + 1) + 1

        onOpenRequested: root.clicked(null)
        onViewOnlineRequested: root.viewOnlineRequested(root.score.scoreId ?? 0)
        onRevealInFileBrowserRequested: root.revealInFileBrowserRequested(root.score.path ?? "")
        onRemoveFromRecentFilesRequested: root.removeFromRecentFilesRequested(root.score.path ?? "")
    }

    SeparatorLine {
        id: bottomBorder
        anchors.bottom: parent.bottom
    }
}
