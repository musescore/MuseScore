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
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0

Item {
    id: root

    function load(templatePath) {
        templateView.load(templatePath)
    }

    function zoomIn() {
        templateView.zoomIn()
    }

    function zoomOut() {
        templateView.zoomOut()
    }

    StyledTextLabel {
        id: title

        anchors.top: parent.top

        text: qsTrc("project", "Preview")
        font: ui.theme.bodyBoldFont
    }

    TemplatePaintView {
        id: templateView

        anchors.top: title.bottom
        anchors.topMargin: 16
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        onHorizontalScrollChanged: {
            if (!horizontalScrollBar.pressed) {
                horizontalScrollBar.setPosition(templateView.startHorizontalScrollPosition)
            }
        }

        onVerticalScrollChanged: {
            if (!verticalScrollBar.pressed) {
                verticalScrollBar.setPosition(templateView.startVerticalScrollPosition)
            }
        }

        StyledScrollBar {
            id: verticalScrollBar

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            orientation: Qt.Vertical

            position: templateView.startVerticalScrollPosition
            size: templateView.verticalScrollSize

            onPositionChanged: {
                if (pressed) {
                    templateView.scrollVertical(position)
                }
            }
        }

        StyledScrollBar {
            id: horizontalScrollBar

            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            orientation: Qt.Horizontal

            position: templateView.startHorizontalScrollPosition
            size: templateView.horizontalScrollSize

            onPositionChanged: {
                if (pressed) {
                    templateView.scrollHorizontal(position)
                }
            }
        }
    }

    Shortcut {
        sequences: [templateView.zoomInSequence()]

        onActivated: {
            templateView.zoomIn()
        }
    }

    Shortcut {
        sequences: [templateView.zoomOutSequence()]

        onActivated: {
            templateView.zoomOut()
        }
    }
}
