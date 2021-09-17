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
import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0

Row {
    id: root

    property alias keySignature: infoModel.keySignature
    property alias timeSignature: infoModel.timeSignature
    property alias timeSignatureType: infoModel.timeSignatureType

    property alias withTempo: infoModel.withTempo
    property alias tempo: infoModel.tempo

    property alias withPickupMeasure: infoModel.withPickupMeasure
    property alias pickupTimeSignature: infoModel.pickupTimeSignature
    property alias measureCount: infoModel.measureCount

    property alias navigationPanel: navPanel
    property var popupsAnchorItem: null

    spacing: 20

    function focusOnFirst() {
        keySignatureSettings.navigation.requestActive()
    }

    QtObject {
        id: privatesProperties

        property real contentWidth: (width / 4) - 15
        readonly property real buttonHeight: 120
    }

    Component.onCompleted: {
        infoModel.init()
    }

    AdditionalInfoModel {
        id: infoModel
    }

    NavigationPanel {
        id: navPanel
        name: "NavPanel"
        direction: NavigationPanel.Horizontal
        enabled: root.visible
    }

    Column {
        id: keySignatureСolumn

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: privatesProperties.contentWidth

        spacing: 10

        StyledTextLabel {
            anchors.left: parent.left
            anchors.right: parent.right

            font: ui.theme.bodyBoldFont
            horizontalAlignment: Text.AlignLeft
            text: qsTrc("project", "Key signature")
        }

        KeySignatureSettings {
            id: keySignatureSettings

            anchors.left: parent.left
            anchors.right: parent.right

            height: privatesProperties.buttonHeight

            model: infoModel

            popupAnchorItem: root.popupsAnchorItem

            navigation.name: "KeySignatureButton"
            navigation.panel: navPanel
            navigation.column: 0
        }
    }

    Column {
        id: timeSignatureСolumn
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: privatesProperties.contentWidth

        spacing: 10

        StyledTextLabel {
            anchors.left: parent.left
            anchors.right: parent.right

            font: ui.theme.bodyBoldFont
            horizontalAlignment: Text.AlignLeft
            text: qsTrc("project", "Time signature")
        }

        TimeSignatureSettings {
            anchors.left: parent.left
            anchors.right: parent.right

            height: privatesProperties.buttonHeight

            model: infoModel

            popupAnchorItem: root.popupsAnchorItem

            navigation.name: "TimeSignatureButton"
            navigation.panel: navPanel
            navigation.column: 1
        }
    }

    Column {
        id: tempoСolumn
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: privatesProperties.contentWidth

        spacing: 10

        StyledTextLabel {
            anchors.left: parent.left
            anchors.right: parent.right

            font: ui.theme.bodyBoldFont
            horizontalAlignment: Text.AlignLeft
            text: qsTrc("project", "Tempo")
        }

        TempoSettings {
            anchors.left: parent.left
            anchors.right: parent.right

            height: privatesProperties.buttonHeight

            model: infoModel

            popupAnchorItem: root.popupsAnchorItem

            navigation.name: "TempoSignatureButton"
            navigation.panel: navPanel
            navigation.column: 2
        }
    }

    Column {
        id: measuresColumn
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: privatesProperties.contentWidth

        spacing: 10

        StyledTextLabel {
            anchors.left: parent.left
            anchors.right: parent.right

            font: ui.theme.bodyBoldFont
            horizontalAlignment: Text.AlignLeft
            text: qsTrc("project", "Measures")
        }

        MeasuresSettings {
            anchors.left: parent.left
            anchors.right: parent.right

            height: privatesProperties.buttonHeight

            model: infoModel

            popupAnchorItem: root.popupsAnchorItem

            navigation.name: "MeasuresSignatureButton"
            navigation.panel: navPanel
            navigation.column: 3
        }
    }
}
