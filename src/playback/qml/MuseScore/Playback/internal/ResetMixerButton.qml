/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

Item {
    id: root

    property NavigationPanel navigationPanel: null

    signal resetMixerRequested()

    anchors.fill: parent

    FlatButton {
        id: resetMixerButton

        anchors.right: parent.right
        anchors.rightMargin: 6
        anchors.verticalCenter: parent.verticalCenter

        width: 90

        text: qsTrc("playback", "Reset Mixer")
        transparent: false
        accentButton: false

        navigation.name: "Reset mixer"
        navigation.panel: root.navigationPanel
        navigation.row: 0

        onClicked: {
            resetVolumesConfirmPopup.toggleOpened()
        }
    }

    StyledPopupView {
        id: resetVolumesConfirmPopup

        anchorItem: root

        contentWidth: 360
        contentHeight: popupContentColumn.implicitHeight

        placementPolicies: PopupView.PreferBelow | PopupView.PreferAbove
        closePolicies: PopupView.CloseOnEscape | PopupView.CloseOnPressOutsideParent

        ColumnLayout {
            id: popupContentColumn

            width: 360
            spacing: 12

            RowLayout {
                Layout.fillWidth: true

                StyledTextLabel {
                    Layout.fillWidth: true
                    font: ui.theme.bodyBoldFont
                    text: qsTrc("playback", "Confirm mixer reset")
                    horizontalAlignment: Text.AlignLeft
                }

                FlatButton {
                    icon: IconCode.CLOSE_X_ROUNDED
                    transparent: true
                    accentButton: false

                    navigation.name: qsTrc("global", "Close")
                    navigation.panel: root.navigationPanel
                    navigation.row: 1

                    onClicked: {
                        resetVolumesConfirmPopup.close()
                    }
                }
            }

            StyledTextLabel {
                Layout.fillWidth: true
                text: qsTrc("playback", "This will reset the mixer to default settings. Are you sure you want to continue?")
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignLeft
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Item {
                    Layout.fillWidth: true
                }

                FlatButton {
                    text: qsTrc("global", "Cancel")
                    transparent: true
                    accentButton: false

                    navigation.panel: root.navigationPanel
                    navigation.row: 2

                    onClicked: {
                        resetVolumesConfirmPopup.close()
                    }
                }

                FlatButton {
                    text: qsTrc("global", "Confirm")
                    transparent: false
                    accentButton: true

                    navigation.panel: root.navigationPanel
                    navigation.row: 3

                    onClicked: {
                        resetVolumesConfirmPopup.close()
                        root.resetMixerRequested()
                    }
                }
            }
        }
    }
}

