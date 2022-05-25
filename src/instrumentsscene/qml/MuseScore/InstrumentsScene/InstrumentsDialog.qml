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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.InstrumentsScene 1.0

StyledDialogView {
    id: root

    property bool canSelectMultipleInstruments: true
    property string currentInstrumentId: ""

    contentHeight: 500
    contentWidth: root.canSelectMultipleInstruments ? 900 : 600
    margins: 12

    title: canSelectMultipleInstruments ? qsTrc("instruments", "Add or remove instruments") :
                                          qsTrc("instruments", "Select instrument")

    onNavigationActivateRequested: {
        instrumentsPage.focusOnFirst()
    }

    function submit() {
        var result = {}
        result["instruments"] = instrumentsPage.instruments()
        result["scoreOrder"] = instrumentsPage.currentOrder()

        root.ret = { errcode: 0, value: result }
        root.hide()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 20

        ChooseInstrumentsPage {
            id: instrumentsPage

            Layout.fillWidth: true
            Layout.fillHeight: true

            canSelectMultipleInstruments: root.canSelectMultipleInstruments
            currentInstrumentId: root.currentInstrumentId

            navigationSection: root.navigationSection

            onSubmitRequested: {
                root.submit()
            }
        }

        RowLayout {
            spacing: 12

            StyledTextLabel {
                id: descriptionLabel
                text: instrumentsPage.description

                Layout.fillWidth: true
                Layout.maximumHeight: buttonBox.height

                font: ui.theme.bodyFont
                opacity: 0.7
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.Wrap
            }

            ButtonBox {
                id: buttonBox

                buttons: ButtonBoxModel.Cancel
                separationGap: false

                ButtonBoxItem {
                    text: qsTrc("global", "OK")
                    enabled: instrumentsPage.hasSelectedInstruments
                    isAccent: true

                    onClicked: {
                        root.submit()
                    }
                }

                navigationPanel: NavigationPanel {
                    name: "BottomPanel"
                    section: root.navigationSection
                    order: 100
                    direction: NavigationPanel.Horizontal
                }

                onStandardButtonClicked: function(type) {
                    if (type === ButtonBoxModel.Cancel) {
                        root.reject()
                    }
                }
            }
        }
    }
}
