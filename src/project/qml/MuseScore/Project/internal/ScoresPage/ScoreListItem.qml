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

    implicitHeight: 64

    navigation.accessible.name: root.score.name ?? ""
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

                sourceComponent: ScoreThumbnail {
                    path: root.score.path ?? ""
                    suffix: root.score.suffix ?? ""
                    thumbnailUrl: root.score.thumbnailUrl ?? ""
                }

                layer.enabled: ui.isEffectsAllowed
                layer.effect: RoundedCornersEffect {
                    radius: 2
                }
            }

            StyledTextLabel {
                Layout.fillWidth: true

                text: root.score.name ?? ""
                font: ui.theme.largeBodyFont
                horizontalAlignment: Text.AlignLeft
            }

            ScoreItemMenuButton {
                id: contextMenu

                isCreateNew: root.score.isCreateNew ?? false
                isNoResultsFound: root.score.isNoResultsFound ?? false
                isCloud: root.score.isCloud ?? false
                showRemoveFromRecentFiles: root.showRemoveFromRecentFiles

                Layout.alignment: Qt.AlignTrailing | Qt.AlignVCenter
                Layout.preferredWidth: 20
                Layout.preferredHeight: 20
                visible: menuModel.length > 0
                         && (root.mouseArea.containsMouse 
                             || mouseArea.containsMouse
                             || root.navigation.active 
                             || navigation.active 
                             || isMenuOpenedByButton)

                navigation.panel: root.navigation.panel
                navigation.row: root.navigation.row
                navigation.column: 1

                onOpenRequested: root.clicked(null)
                onViewOnlineRequested: root.viewOnlineRequested(root.score.scoreId ?? 0)
                onRevealInFileBrowserRequested: root.revealInFileBrowserRequested(root.score.path ?? "")
                onRemoveFromRecentFilesRequested: root.removeFromRecentFilesRequested(root.score.path ?? "")
            }

            Loader {
                active: root.score.isCloud ?? false

                sourceComponent: RowLayout {
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

    SeparatorLine {
        id: bottomBorder
        anchors.bottom: parent.bottom
    }
}
