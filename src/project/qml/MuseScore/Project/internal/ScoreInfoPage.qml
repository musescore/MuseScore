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

Item {
    id: root

    function result() {
        var result = {
            title: generalInfo.title,
            subtitle: generalInfo.subtitle,
            composer: generalInfo.composer,
            lyricist: generalInfo.lyricist,
            copyright: generalInfo.copyright,
            keySignature: additionalInfo.keySignature,
            timeSignature: additionalInfo.timeSignature,
            timeSignatureType: additionalInfo.timeSignatureType,
            withTempo: additionalInfo.withTempo,
            tempo: additionalInfo.tempo,
            withPickupMeasure: additionalInfo.withPickupMeasure,
            pickupTimeSignature: additionalInfo.pickupTimeSignature,
            measureCount: additionalInfo.measureCount
        }

        return result
    }

    StyledTextLabel {
        id: title

        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter

        font: ui.theme.largeBodyBoldFont
        text: qsTrc("project", "Additional Score Information")
    }

    ColumnLayout {
        anchors.top: title.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20

        spacing: 30

        AdditionalInfoView {
            id: additionalInfo

            Layout.preferredHeight: 150
            Layout.fillWidth: true
        }

        SeparatorLine {}

        GeneralInfoView {
            id: generalInfo

            Layout.fillWidth: true
        }
    }
}
