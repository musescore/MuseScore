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
import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.Inspector

import "../../common"

Column {
    id: root

    required property BendSettingsModel model

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "BendSettings"

    spacing: 12

    function focusOnFirst() {
        bendTypeSection.focusOnFirst()
    }

    PlacementSection {
        id: placmentSection

        visible: root.model ? !root.model.isTabStaff && !root.model.isDip : true

        propertyItem: root.model ? root.model.bendDirection : null

        //! NOTE: Bend uses the direction property,
        // but for convenience we will display it in the placement section
        model: [
            { text: qsTrc("inspector", "Auto"), value: DirectionTypes.VERTICAL_AUTO },
            { text: qsTrc("inspector", "Above"), value: DirectionTypes.VERTICAL_UP },
            { text: qsTrc("inspector", "Below"), value: DirectionTypes.VERTICAL_DOWN }
        ]

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1
    }

    FlatRadioButtonGroupPropertyView {
        id: showHoldSection
        visible: root.model ? root.model.isShowHoldLineAvailable : false
        titleText: qsTrc("inspector", "Hold line")
        propertyItem: root.model ? root.model.showHoldLine : null

        navigationName: "HoldLine"
        navigationPanel: root.navigationPanel
        navigationRowStart: placmentSection.navigationRowEnd + 1

        model: [
            { text: qsTrc("inspector", "Auto"), value: BendTypes.SHOW_HOLD_AUTO},
            { text: qsTrc("inspector", "Show"), value: BendTypes.SHOW_HOLD_SHOW},
            { text: qsTrc("inspector", "Hide"), value: BendTypes.SHOW_HOLD_HIDE},
        ]
    }

    FlatRadioButtonGroupPropertyView {
        id: dipVibratoTypeSection
        visible: root.model ? root.model.isDip : false
        titleText: qsTrc("inspector", "Tremolo line")
        propertyItem: root.model ? root.model.dipVibratoType : null

        navigationName: "TremoloLine"
        navigationPanel: root.navigationPanel
        navigationRowStart: showHoldSection.navigationRowEnd + 1

        model: [
            { text: qsTrc("inspector", "None"), value: BendTypes.VIBRATO_NONE },
            { text: qsTrc("inspector", "Normal"), value: BendTypes.VIBRATO_VIBRATO },
            { text: qsTrc("inspector", "Wide"), value: BendTypes.VIBRATO_VIBRATO_WIDE },
        ]
    }

    FlatRadioButtonGroupPropertyView {
        id: tabStaffLayout
        visible: root.model ? root.model.isDiveTabPosAvailable : false
        titleText: qsTrc("inspector", "Tablature staff layout")
        propertyItem: root.model ? root.model.diveTabPos : null

        navigationName: "HoldLine"
        navigationPanel: root.navigationPanel
        navigationRowStart: dipVibratoTypeSection.navigationRowEnd + 1

        model: [
            { text: qsTrc("inspector", "Auto"), value: DirectionTypes.VERTICAL_AUTO},
            { text: qsTrc("inspector", "On staff"), value: DirectionTypes.VERTICAL_DOWN},
            { text: qsTrc("inspector", "Above staff"), value: DirectionTypes.VERTICAL_UP},
        ]
    }

    FlatRadioButtonGroupPropertyView {
        id: holdLineStyle
        visible: root.model && root.model.isHoldLine
        titleText: qsTrc("inspector", "Line style")
        propertyItem: root.model ? root.model.lineStyle : null

        navigationName: "HoldLineStyle"
        navigationPanel: root.navigationPanel
        navigationRowStart: tabStaffLayout.navigationRowEnd + 1

        model: [
            { iconCode: IconCode.LINE_NORMAL, value: LineTypes.LINE_STYLE_SOLID, title: qsTrc("inspector", "Normal", "line type") },
            { iconCode: IconCode.LINE_DASHED, value: LineTypes.LINE_STYLE_DASHED, title: qsTrc("inspector", "Dashed", "line type") },
            { iconCode: IconCode.LINE_DOTTED, value: LineTypes.LINE_STYLE_DOTTED, title: qsTrc("inspector", "Dotted", "line type") },
        ]
    }

    InspectorPropertyView {
        id: bend
        titleText: root.model && root.model.isDive ? qsTrc("inspector", "Customize dive") : qsTrc("inspector", "Customize bend")

        enabled: root.model ? root.model.isBendCurveEnabled : false
        visible: root.model ? !root.model.isHoldLine : false

        navigationPanel: root.navigationPanel
        navigationRowStart: holdLineStyle.navigationRowEnd + 1

        spacing: 0

        Item {
            width: parent.width
            height: bendCanvas.height

            NavigationControl {
                id: navCtrl
                name: "BendCanvas"
                enabled: bendCanvas.enabled && bendCanvas.visible

                panel: root.navigationPanel
                row: bend.navigationRowEnd + 1

                accessible.role: MUAccessible.Information
                accessible.name: bend.titleText + "; " + qsTrc("inspector", "Press Enter to start editing")

                onActiveChanged: {
                    if (navCtrl.active) {
                        bendCanvas.forceActiveFocus()
                    } else {
                        bendCanvas.resetFocus()
                    }
                }

                onNavigationEvent: function(event) {
                    if (!root.model) {
                        return
                    }

                    var accepted = false
                    switch(event.type) {
                    case NavigationEvent.Trigger:
                        accepted = bendCanvas.focusOnFirstPoint()

                        if (accepted) {
                            navCtrl.accessible.focused = false
                        }

                        break
                    case NavigationEvent.Escape:
                        accepted = bendCanvas.resetFocus()

                        if (accepted) {
                            navCtrl.accessible.focused = true
                        }

                        break
                    case NavigationEvent.Left:
                        accepted = bendCanvas.moveFocusedPointToLeft()
                        break
                    case NavigationEvent.Right:
                        accepted = bendCanvas.moveFocusedPointToRight()
                        break
                    case NavigationEvent.Up:
                        accepted = bendCanvas.moveFocusedPointToUp()
                        break
                    case NavigationEvent.Down:
                        accepted = bendCanvas.moveFocusedPointToDown()
                        break
                    }

                    event.accepted = accepted
                }
            }

            NavigationFocusBorder {
                navigationCtrl: navCtrl
            }

            BendGridCanvas {
                id: bendCanvas

                width: parent.width
                height: root.model && root.model.isDive ? 400 : 200

                enabled: bend.enabled
                focus: true

                pointList: root.model ? root.model.bendCurve : null

                rowCount: root.model && root.model.isDive ? 50 : 13
                columnCount: 13
                rowSpacing: 4
                columnSpacing: 3

                topLineValue: root.model && root.model.isDive ? 4 : 3
                showHalfs: !(root.model && root.model.isDive)
                straightLines: root.model && root.model.isDive

                accessibleParent: navCtrl.accessible

                onCanvasChanged: {
                    if (root.model) {
                        root.model.bendCurve = pointList
                    }
                }
            }
        }
    }
}
