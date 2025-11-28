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
pragma ComponentBehavior: Bound

import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.Inspector

import "../../common"
import "internal"

Item {
    id: root

    property PlaybackProxyModel proxyModel: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width
        height: childrenRect.height

        spacing: 12

        readonly property var fullModel: [
            { typeRole: AbstractInspectorModel.TYPE_NOTE, componentRole: noteSection },
            { typeRole: AbstractInspectorModel.TYPE_ARPEGGIO, componentRole: arpeggioSection },
            { typeRole: AbstractInspectorModel.TYPE_FERMATA, componentRole: fermataSection },
            { typeRole: AbstractInspectorModel.TYPE_BREATH, componentRole: breathSection },
            { typeRole: AbstractInspectorModel.TYPE_GLISSANDO, componentRole: glissandoSection },
            { typeRole: AbstractInspectorModel.TYPE_GRADUAL_TEMPO_CHANGE, componentRole: tempoChangeSection }
        ]

        function getVisibleModel() {
            var visibleModel = []
            for (let i = 0; i < fullModel.length; i++) {
                var currentItemModel = root.proxyModel ? root.proxyModel.modelByType(fullModel[i].typeRole) : null
                if (Boolean(currentItemModel) && !currentItemModel.isEmpty) {
                     visibleModel.push({ componentRole: fullModel[i].componentRole, itemModel: currentItemModel })
                }
            }
            return visibleModel
        }

        Repeater {
            id: repeater

            model: contentColumn.getVisibleModel()

            delegate: Column {
                id: itemColumn

                required property var modelData
                required property int index

                width: parent.width
                spacing: contentColumn.spacing

                Loader {
                    id: expandableLoader

                    width: parent.width

                    property var model: itemColumn.modelData.itemModel

                    sourceComponent: itemColumn.modelData.componentRole

                    onLoaded: {
                        item.navigation.panel = Qt.binding(() => root.navigationPanel)
                        item.navigation.row = Qt.binding(() => root.navigationRowStart + itemColumn.index * 1000)
                    }
                }

                SeparatorLine {
                    anchors.margins: -12
                    visible: itemColumn.index < (repeater.count - 1)
                }
            }
        }
    }

    Component {
        id: noteSection

        NoteExpandableBlank {}
    }

    Component {
        id: arpeggioSection

        ArpeggioExpandableBlank {}
    }

    Component {
        id: fermataSection

        FermataExpandableBlank {}
    }

    Component {
        id: breathSection

        BreathExpandableBlank {}
    }

    Component {
        id: glissandoSection

        GlissandoExpandableBlank {}
    }

    Component {
        id: tempoChangeSection

        GradualTempoChangeBlank {}
    }
}
