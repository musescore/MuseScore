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
import QtQuick.Layouts 1.3

import Muse.Ui 1.0

Rectangle {
    id: root

    property int orientation: prv.parentIsHorizontal ? Qt.Vertical : Qt.Horizontal
    property int thickness: 1

    color: ui.theme.strokeColor

    QtObject {
        id: prv

        readonly property bool parentIsHorizontal: root.parent instanceof Row || root.parent instanceof RowLayout
        readonly property bool parentIsLayout: root.parent instanceof ColumnLayout || root.parent instanceof RowLayout || root.parent instanceof GridLayout
        readonly property bool parentIsLoader: root.parent instanceof Loader
    }

    states: [
        State {
            name: "HORIZONTAL"
            when: orientation == Qt.Horizontal

            PropertyChanges {
                target: root
                height: root.thickness
                Layout.fillWidth: true
            }

            StateChangeScript {
                script: {
                    if (prv.parentIsLayout) {
                        root.Layout.fillWidth = true
                    } else {
                        if (prv.parentIsLoader && root.parent.status !== Loader.Ready) {
                            return
                        }

                        root.anchors.left = root.parent.left
                        root.anchors.right = root.parent.right
                    }
                }
            }
        },

        State {
            name: "VERTICAL"
            when: orientation == Qt.Vertical

            PropertyChanges {
                target: root
                width: root.thickness
                Layout.fillHeight: true
            }

            StateChangeScript {
                script: {
                    if (prv.parentIsLayout) {
                        root.Layout.fillHeight = true
                    } else {
                        if (prv.parentIsLoader && root.parent.status !== Loader.Ready) {
                            return
                        }

                        root.anchors.top = root.parent.top
                        root.anchors.bottom = root.parent.bottom
                    }
                }
            }
        }
    ]
}
