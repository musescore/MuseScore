/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Playback 1.0

StyledDialogView {
    id: root

    title: qsTrc("playback", "Playback Setup")

    contentWidth: 896
    contentHeight: 424

    SoundProfilesModel {
        id: profilesListModel
    }

    ColumnLayout {
        id: contentColumn

        anchors.fill: parent
        anchors.margins: 24

        RowLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            spacing: 24

            ColumnLayout {
                Layout.fillHeight: true
                Layout.preferredWidth: root.width * 0.45

                spacing: 20

                StyledTextLabel {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignLeft

                    font: ui.theme.bodyBoldFont
                    text: qsTrc("playback", "Profiles")
                    horizontalAlignment: Text.AlignLeft
                }

                StyledListView {
                    id: profilesListView

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    spacing: 8

                    model: profilesListModel

                    delegate: FlatButton {
                        id: profileButton

                        readonly property bool isActive: profilesListModel.activeProfile === titleRole
                        readonly property bool isSelected: profilesListModel.currentlySelectedProfile === titleRole

                        enabled: isEnabledRole

                        backgroundItem: Rectangle {
                            id: backgroundRect

                            color: profileButton.isSelected ? Utils.colorWithAlpha(ui.theme.accentColor, ui.theme.accentOpacityHit)
                                                            : Utils.colorWithAlpha(ui.theme.buttonColor, 0.0)

                            states: [
                                State {
                                    name: "PRESSED"
                                    when: profileButton.mouseArea.pressed

                                    PropertyChanges {
                                        target: backgroundRect
                                        color: profileButton.isSelected ? Utils.colorWithAlpha(ui.theme.accentColor, 1.0)
                                                                        : Utils.colorWithAlpha(ui.theme.buttonColor, 1.0)
                                    }
                                },

                                State {
                                    name: "HOVERED"
                                    when: !profileButton.mouseArea.pressed && profileButton.mouseArea.containsMouse

                                    PropertyChanges {
                                        target: backgroundRect
                                        color: profileButton.isSelected ? Utils.colorWithAlpha(ui.theme.accentColor, 0.5)
                                                                        : Utils.colorWithAlpha(ui.theme.buttonColor, 0.5)
                                    }
                                }
                            ]
                        }

                        contentItem: RowLayout {

                            height: 52
                            width: 320
                            spacing: 12

                            Layout.leftMargin: 2
                            Layout.rightMargin: 2
                            Layout.topMargin: 8
                            Layout.bottomMargin: 8

                            Item {
                                Layout.minimumHeight: 30
                                Layout.minimumWidth: 30

                                StyledIconLabel {
                                    anchors.fill: parent

                                    iconCode: IconCode.TICK_RIGHT_ANGLE
                                    visible: profileButton.isActive
                                }
                            }

                            Column {
                                id: titleColumn

                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                                spacing: 2

                                StyledTextLabel {
                                    Layout.preferredWidth: titleColumn.width

                                    font: ui.theme.tabBoldFont
                                    text: titleRole
                                }

                                StyledTextLabel {
                                    Layout.preferredWidth: titleColumn.width

                                    color: ui.theme.fontSecondaryColor

                                    text: profileButton.isActive ? qsTrc("playback", "Active")
                                                                 : qsTrc("playback", 'Inactive')
                                }
                            }
                        }

                        onClicked: {
                            profilesListModel.currentlySelectedProfile = titleRole
                        }
                    }
                }
            }

            SeparatorLine {
                Layout.fillHeight: true

                orientation: Qt.Vertical
            }

            ColumnLayout {
                Layout.fillHeight: true
                Layout.preferredWidth: root.width * 0.45
                Layout.alignment: Qt.AlignTop

                spacing: 20

                StyledTextLabel {
                    Layout.fillWidth: true

                    font: ui.theme.bodyBoldFont
                    text: qsTrc("playback", "Settings")
                    horizontalAlignment: Text.AlignLeft
                }

                CheckBox {
                    id: defaultProfileCheckBox

                    checked: profilesListModel.currentlySelectedProfile == profilesListModel.defaultProjectsProfile
                    text: qsTrc("playback", "Set as default for new scores")

                    onClicked: {
                        if (defaultProfileCheckBox.checked) {
                            profilesListModel.defaultProjectsProfile = ""
                        } else {
                            profilesListModel.defaultProjectsProfile = profilesListModel.currentlySelectedProfile
                        }
                    }
                }
            }
        }

        Row {
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom

            spacing: 12

            FlatButton {
                height: 30
                width: 160

                accentButton: true
                visible: profilesListModel.currentlySelectedProfile != profilesListModel.activeProfile
                text: qsTrc("playback", "Activate this profile")

                onClicked: {
                    profilesListModel.activeProfile = profilesListModel.currentlySelectedProfile
                }
            }

            FlatButton {
                height: 30
                width: 160

                text: qsTrc("playback", "OK")

                onClicked: {
                    root.hide()
                }
            }
        }
    }
}
