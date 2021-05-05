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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

StyledDialogView {
    id: root

    property bool canSelectMultipleInstruments: true
    property string currentInstrumentId: ""
    property string initiallySelectedPartIds: ""

    contentHeight: 500
    contentWidth: root.canSelectMultipleInstruments ? 900 : 600

    title: canSelectMultipleInstruments ? qsTrc("instruments", "Instruments") :
                                          qsTrc("instruments", "Select instrument")

    Rectangle {
        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor

        ChooseInstrumentsPage {
            id: instrumentsPage

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: buttons.top
            anchors.margins: 10

            initiallySelectedPartIds: root.initiallySelectedPartIds
            canSelectMultipleInstruments: root.canSelectMultipleInstruments
            currentInstrumentId: root.currentInstrumentId
        }

        Row {
            id: buttons

            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10

            spacing: 8

            readonly property int buttonWidth: 134

            FlatButton {
                width: buttons.buttonWidth

                text: qsTrc("global", "Cancel")

                onClicked: {
                    root.reject()
                }
            }

            FlatButton {
                width: buttons.buttonWidth

                text: qsTrc("global", "OK")
                enabled: instrumentsPage.hasSelectedInstruments

                onClicked: {
                    var selectedInstruments = instrumentsPage.selectedInstruments()

                    root.ret = { errcode: 0, value: selectedInstruments }
                    root.hide()
                }
            }
        }
    }
}
