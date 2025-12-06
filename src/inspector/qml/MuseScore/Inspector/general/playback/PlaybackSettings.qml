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

Column {
    id: root

    required property PlaybackProxyModel proxyModel

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 0

    width: parent.width
    spacing: 12

    Repeater {
        id: repeater

        readonly property var fullModel: [
            AbstractInspectorModel.TYPE_NOTE,
            AbstractInspectorModel.TYPE_ARPEGGIO,
            AbstractInspectorModel.TYPE_FERMATA,
            AbstractInspectorModel.TYPE_BREATH,
            AbstractInspectorModel.TYPE_GLISSANDO,
            AbstractInspectorModel.TYPE_GRADUAL_TEMPO_CHANGE,
        ]

        function getVisibleModel() {
            if (!root.proxyModel) {
                return []
            }
            var visibleModel = []
            for (let type of fullModel) {
                var currentItemModel = root.proxyModel.modelByType(type)
                if (Boolean(currentItemModel) && !currentItemModel.isEmpty) {
                    visibleModel.push({ itemModel: currentItemModel })
                }
            }
            return visibleModel
        }

        model: getVisibleModel()

        delegate: Column {
            id: itemColumn

            required property AbstractInspectorModel itemModel
            required property int index

            readonly property int navigationRowStart: root.navigationRowStart + index * 1000

            width: parent.width
            spacing: root.spacing

            SeparatorLine {
                anchors.margins: -12
                visible: itemColumn.index > 0
            }

            Loader {
                id: expandableLoader
                width: parent.width
                sourceComponent: {
                    switch (itemColumn.itemModel.modelType) {
                        case AbstractInspectorModel.TYPE_NOTE: return noteSection
                        case AbstractInspectorModel.TYPE_ARPEGGIO: return arpeggioSection
                        case AbstractInspectorModel.TYPE_FERMATA: return fermataSection
                        case AbstractInspectorModel.TYPE_BREATH: return breathSection
                        case AbstractInspectorModel.TYPE_GLISSANDO: return glissandoSection
                        case AbstractInspectorModel.TYPE_GRADUAL_TEMPO_CHANGE: return tempoChangeSection
                        default: return null
                    }
                }

                Component {
                    id: noteSection

                    NoteExpandableBlank {
                        model: itemColumn.itemModel as NotePlaybackModel
                        navigation.panel: root.navigationPanel
                        navigation.row: itemColumn.navigationRowStart
                    }
                }

                Component {
                    id: arpeggioSection

                    ArpeggioExpandableBlank {
                        model: itemColumn.itemModel as ArpeggioPlaybackModel
                        navigation.panel: root.navigationPanel
                        navigation.row: itemColumn.navigationRowStart
                    }
                }

                Component {
                    id: fermataSection

                    FermataExpandableBlank {
                        model: itemColumn.itemModel as FermataPlaybackModel
                        navigation.panel: root.navigationPanel
                        navigation.row: itemColumn.navigationRowStart
                    }
                }

                Component {
                    id: breathSection

                    BreathExpandableBlank { 
                        model: itemColumn.itemModel as BreathPlaybackModel
                        navigation.panel: root.navigationPanel
                        navigation.row: itemColumn.navigationRowStart
                    }
                }

                Component {
                    id: glissandoSection

                    GlissandoExpandableBlank {
                        model: itemColumn.itemModel as GlissandoPlaybackModel
                        navigation.panel: root.navigationPanel
                        navigation.row: itemColumn.navigationRowStart
                    }
                }

                Component {
                    id: tempoChangeSection

                    GradualTempoChangeBlank {
                        model: itemColumn.itemModel as GradualTempoChangePlaybackModel
                        navigation.panel: root.navigationPanel
                        navigation.row: itemColumn.navigationRowStart
                    }
                }
            }
        }
    }
}
