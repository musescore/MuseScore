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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Workspace 1.0

import "internal"

StyledDialogView {
    id: root

    property alias notes: view.notes
    property alias features: featuresViewRepeater.model
    property alias imageUrl: image.source
    property alias actionTitle: buttons.defaultButtonName
    property alias cancelTitle: buttons.cancelButtonName

    contentWidth: 530
    contentHeight: 510

    onNavigationActivateRequested: {
        buttons.focusOnFirst()
    }

    onAccessibilityActivateRequested: {
        accessibleInfo.readInfo()
    }

    ColumnLayout {
        id: content

        anchors.fill: parent

        AccessibleItem {
            id: accessibleInfo

            accessibleParent: buttons.navigationPanel.accessible
            visualItem: content
            role: MUAccessible.Button

            name: "%1; %2; %3; %4; %5".arg(view.notes)
                                      .arg(featuresViewRepeater.contentText())
                                      .arg(buttons.defaultButtonName)

            function readInfo() {
                accessibleInfo.ignored = false
                accessibleInfo.focused = true
            }

            function resetFocus() {
                accessibleInfo.ignored = true
                accessibleInfo.focused = false
            }
        }

        Image {
            id: image

            Layout.fillWidth: true
            Layout.preferredHeight: 186
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Layout.margins: 24

            ReleaseNotesView {
                id: view

                width: parent.width

                anchors.top: parent.top
                anchors.bottom: featuresFrame.top
            }

            Item {
                id: featuresFrame

                width: parent.width
                height: gridView.height + gridView.anchors.margins * 2

                anchors.bottom: buttons.top
                anchors.bottomMargin: 20

                Rectangle {
                    anchors.fill: parent

                    color: ui.theme.buttonColor
                    opacity: 0.4
                    radius: 3
                }

                GridLayout {
                    id: gridView

                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 20

                    readonly property int cellRadius: 2

                    flow: GridLayout.LeftToRight
                    columns: 2
                    rows: Math.ceil(featuresViewRepeater.model.length / 2)
                    columnSpacing: 8
                    rowSpacing: 4

                    Repeater {
                        id: featuresViewRepeater

                        width: parent.width

                        function contentText() {
                            var result = ""
                            for (var i = 0; i < featuresViewRepeater.count; ++i) {
                                var item = itemAt(i)
                                result += item.title + "; "
                            }

                            return result
                        }

                        delegate: RowLayout {
                            Layout.preferredHeight: titleLabel.height
                            Layout.preferredWidth: (content.width / 2 - gridView.columnSpacing / gridView.columns)

                            spacing: 10

                            property string title: modelData

                            Rectangle {
                                Layout.preferredWidth: 9
                                Layout.preferredHeight: width
                                radius: width / 2

                                color: ui.theme.accentColor
                            }

                            StyledTextLabel {
                                id: titleLabel

                                Layout.fillWidth: true
                                Layout.preferredHeight: implicitHeight

                                text: title
                                horizontalAlignment: Text.AlignLeft
                                wrapMode: Text.WordWrap
                                maximumLineCount: 2
                            }
                        }
                    }
                }
            }

            MuseSoundsReleaseInfoBottomPanel {
                id: buttons

                width: parent.width
                height: 30
                implicitWidth: parent.width

                anchors.bottom: parent.bottom

                navigationPanel.section: root.navigationSection
                navigationPanel.order: 1

                onOpenMuseHubRequested: {
                    root.ret = { errcode: 0, value: "openMuseHub" }
                    root.hide()
                }

                onSkipRequested: {
                    root.ret = { errcode: 0, value: "skip" }
                    root.hide()
                }
            }
        }
    }
}
