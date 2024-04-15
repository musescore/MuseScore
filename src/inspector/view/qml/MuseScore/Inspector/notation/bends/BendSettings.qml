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

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "BendSettings"

    spacing: 12

    function focusOnFirst() {
        bendTypeSection.focusOnFirst()
    }

    PlacementSection {
        id: placmentSection

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

    InspectorPropertyView {
        id: bend
        titleText: qsTrc("inspector", "Customize bend")

        enabled: root.model ? root.model.isBendCurveEnabled : false
        visible: true

        navigationPanel: root.navigationPanel
        navigationRowStart: showHoldSection.navigationRowEnd + 1

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
                height: 200

                enabled: bend.enabled
                focus: true

                pointList: root.model ? root.model.bendCurve : null

                rowCount: 13
                columnCount: 13
                rowSpacing: 4
                columnSpacing: 3

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
