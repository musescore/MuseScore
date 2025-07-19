/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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
import MuseScore.Playback 1.0

StyledDialogView {
    id: root

    title: qsTrc("playback", "Playback setup")

    contentWidth: 896
    contentHeight: 424

    Component.onCompleted: {
        profilesListModel.init()
    }

    SoundProfilesModel {
        id: profilesListModel
    }

    onNavigationActivateRequested: {
        profilesListView.itemAtIndex(0).navigation.requestActive()
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
                id: profilesColumn

                Layout.fillHeight: true
                Layout.preferredWidth: root.width * 0.45

                spacing: 20

                property NavigationPanel navigationPanel: NavigationPanel {
                    name: "ProfilesView"
                    section: root.navigationSection
                    direction: NavigationPanel.Vertical
                    order: 1
                    accessible.name: profilesTitleLabel.text
                }

                StyledTextLabel {
                    id: profilesTitleLabel

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

                    delegate: ListItemBlank {
                        id: profileButton

                        height: 52

                        readonly property bool isActive: profilesListModel.activeProfile === titleRole
                        readonly property string activeTitle: isActive ? qsTrc("playback", "Active")
                                                                       : qsTrc("playback", "Inactive")
                        isSelected: profilesListModel.currentlySelectedProfile === titleRole

                        enabled: isEnabledRole

                        navigation.panel: profilesColumn.navigationPanel
                        navigation.order: index
                        navigation.accessible.name: titleRole + "; " + activeTitle

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 2
                            anchors.rightMargin: 2
                            anchors.topMargin: 8
                            anchors.bottomMargin: 8

                            spacing: 12

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

                                    color: ui.theme.fontPrimaryColor
                                    opacity: enabled? 0.7 : 0.7 * ui.theme.itemOpacityDisabled

                                    text: profileButton.activeTitle
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
                id: settingsColumn

                Layout.fillHeight: true
                Layout.preferredWidth: root.width * 0.45
                Layout.alignment: Qt.AlignTop

                spacing: 20

                property NavigationPanel navigationPanel: NavigationPanel {
                    name: "SettingsView"
                    section: root.navigationSection
                    direction: NavigationPanel.Vertical
                    order: 2
                    accessible.name: profilesTitleLabel.text
                }

                StyledTextLabel {
                    id: settingsTitleLabel

                    Layout.fillWidth: true

                    font: ui.theme.bodyBoldFont
                    text: qsTrc("playback", "Settings")
                    horizontalAlignment: Text.AlignLeft
                }

                CheckBox {
                    id: defaultProfileCheckBox

                    checked: profilesListModel.currentlySelectedProfile == profilesListModel.defaultProjectsProfile
                    text: qsTrc("playback", "Set as default for new scores")

                    navigation.panel: settingsColumn.navigationPanel
                    navigation.order: 1

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
            id: buttons

            Layout.alignment: Qt.AlignRight | Qt.AlignBottom

            spacing: 12

            property NavigationPanel navigationPanel: NavigationPanel {
                name: "ButtonsPanel"
                section: root.navigationSection
                direction: NavigationPanel.Vertical
                order: 3
            }

            FlatButton {
                height: 30
                width: 160

                accentButton: true
                visible: profilesListModel.currentlySelectedProfile != profilesListModel.activeProfile
                text: qsTrc("playback", "Activate this profile")

                navigation.panel: buttons.navigationPanel
                navigation.order: 1

                onClicked: {
                    profilesListModel.activeProfile = profilesListModel.currentlySelectedProfile
                }
            }

            FlatButton {
                height: 30
                width: 160

                text: qsTrc("global", "OK")

                navigation.panel: buttons.navigationPanel
                navigation.order: 2

                onClicked: {
                    root.hide()
                }
            }
        }
    }
}
