/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
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
pragma ComponentBehavior: Bound

import QtQuick

import MuseScore.PropertiesPanel
import Muse.UiComponents

import "../../../common"

ExpandableBlank {
    id: root

    required property GlissandoPlaybackModel model

    property int navigationRowEnd: contentItem.navigationRowEnd

    readonly property bool isHarpGliss: model ? model.isHarpGliss : false

    enabled: model ? !model.isEmpty : false

    title: model ? model.title : ""

    width: parent.width

    contentItemComponent: DropdownPropertyView {
        width: root.width

        navigationName: "Glissando Style"
        navigationPanel: root.navigation.panel
        navigationRowStart: root.navigation.row + 1

        titleText: qsTrc("propertiespanel", "Style")
        propertyItem: root.model ? root.model.styleType : null

        model: root.isHarpGliss
               ? [
                     { text: qsTrc("propertiespanel", "Default (Diatonic)"), value: Glissando.STYLE_DIATONIC },
                     { text: qsTrc("propertiespanel", "Chromatic"), value: Glissando.STYLE_CHROMATIC }
                 ]
               : [
                     { text: qsTrc("propertiespanel", "Chromatic"), value: Glissando.STYLE_CHROMATIC },
                     { text: qsTrc("propertiespanel", "White keys"), value: Glissando.STYLE_WHITE_KEYS },
                     { text: qsTrc("propertiespanel", "Black keys"), value: Glissando.STYLE_BLACK_KEYS },
                     { text: qsTrc("propertiespanel", "Diatonic"), value: Glissando.STYLE_DIATONIC },
                     { text: qsTrc("propertiespanel", "Portamento"), value: Glissando.STYLE_PORTAMENTO }
                 ]
    }
}
