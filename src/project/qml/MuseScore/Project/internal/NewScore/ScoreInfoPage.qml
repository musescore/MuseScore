/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
import MuseScore.Project 1.0

Item {
    id: root

    property NavigationSection navigationSection: null
    property var popupsAnchorItem: null

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

    function focusOnFirst() {
        additionalInfo.focusOnFirst()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8

        spacing: 30

        StyledTextLabel {
            id: title

            Layout.fillWidth: true

            font: ui.theme.largeBodyBoldFont
            text: qsTrc("project/newscore", "Additional score information")
        }

        AdditionalInfoView {
            id: additionalInfo

            Layout.preferredHeight: 150
            Layout.fillWidth: true

            popupsAnchorItem: root.popupsAnchorItem

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 1
        }

        GeneralInfoView {
            id: generalInfo

            Layout.fillWidth: true

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2
        }
    }
}
