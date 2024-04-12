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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../common"
import "internal"

Item {
    id: root

    property QtObject proxyModel: null

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
            { typeRole: Inspector.TYPE_NOTE, componentRole: noteSection },
            { typeRole: Inspector.TYPE_ARPEGGIO, componentRole: arpeggioSection },
            { typeRole: Inspector.TYPE_FERMATA, componentRole: fermataSection },
            { typeRole: Inspector.TYPE_BREATH, componentRole: pausesSection },
            { typeRole: Inspector.TYPE_GLISSANDO, componentRole: glissandoSection },
            { typeRole: Inspector.TYPE_GRADUAL_TEMPO_CHANGE, componentRole: tempoChangeSection }
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

                width: parent.width

                spacing: contentColumn.spacing

                Loader {
                    id: expandableLoader

                    width: parent.width

                    sourceComponent: modelData.componentRole

                    onLoaded: {
                        expandableLoader.item.model = modelData.itemModel
                        expandableLoader.item.navigation.panel = root.navigationPanel
                        expandableLoader.item.navigation.row = root.navigationRowStart + model.index * 1000
                    }
                }

                SeparatorLine {
                    anchors.margins: -12
                    visible: model.index < (repeater.count - 1)
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
        id: pausesSection

        PausesExpandableBlank {}
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
