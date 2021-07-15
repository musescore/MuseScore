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
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import "../../common"

TabPanel {
    id: root

    property QtObject model: null

    objectName: "NoteSettings"

    implicitHeight: Math.max(beamTab.visible ? beamTab.implicitHeight : 0,
                             headTab.visible ? headTab.implicitHeight : 0,
                             stemTab.visible ? stemTab.implicitHeight : 0) + tabBarHeight + 24
    width: parent ? parent.width : 0

    currentIndex: root.model ? indexByType(root.model.preferedSubModelType) : 0

    function indexByType(modelType) {
        switch (modelType) {
        case Inspector.TYPE_BEAM: return 0
        case Inspector.TYPE_NOTE: return 1
        case Inspector.TYPE_NOTEHEAD: return 1
        case Inspector.TYPE_STEM: return 2
        case Inspector.TYPE_HOOK: return 2
        }

        return 1 // note
    }

    Tab {
        id: beamTab

        property QtObject beamModel: root.model ? root.model.modelByType(Inspector.TYPE_BEAM) : null

        title: beamModel ? beamModel.title : ""
        width: root.width

        BeamSettings {
            id: beamSettings

            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            model: beamTab.beamModel
        }
    }

    Tab {
        id: headTab

        property QtObject headModel: root.model ? root.model.modelByType(Inspector.TYPE_NOTEHEAD) : null

        title: headModel ? headModel.title : ""
        width: root.width

        HeadSettings {
            id: headSettings

            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            model: headTab.headModel
        }
    }

    Tab {
        id: stemTab

        property QtObject stemModel: root.model ? root.model.modelByType(Inspector.TYPE_STEM) : null
        property QtObject hookModel: root.model ? root.model.modelByType(Inspector.TYPE_HOOK) : null
        property QtObject beamModel: root.model ? root.model.modelByType(Inspector.TYPE_BEAM) : null

        height: implicitHeight
        width: root.width

        title: stemModel ? stemModel.title : ""

        StemSettings {
            id: stemSettings

            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            stemModel: stemTab.stemModel
            hookModel: stemTab.hookModel
            beamModel: stemTab.beamModel
        }
    }
}
