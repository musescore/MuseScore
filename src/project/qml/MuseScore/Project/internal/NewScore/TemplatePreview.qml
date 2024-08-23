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
import QtQuick.Controls 2.15

import Muse.UiComponents 1.0
import MuseScore.Project 1.0
import MuseScore.NotationScene 1.0

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

        text: qsTrc("project/newscore", "Preview")
        font: ui.theme.bodyBoldFont
    }

    StyledViewScrollAndZoomArea {
        anchors.top: title.bottom
        anchors.topMargin: 16
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        horizontalScrollbarSize: templateView.horizontalScrollbarSize
        startHorizontalScrollPosition: templateView.startHorizontalScrollPosition

        verticalScrollbarSize: templateView.verticalScrollbarSize
        startVerticalScrollPosition: templateView.startVerticalScrollPosition

        TemplatePaintView {
            id: templateView
            objectName: "TemplatePreview"
            anchors.fill: parent
        }

        onPinchToZoom: function(scale, pos) {
            templateView.pinchToZoom(scale, pos)
        }

        onScrollHorizontal: function(newPos) {
            templateView.scrollHorizontal(newPos)
        }

        onScrollVertical: function(newPos) {
            templateView.scrollVertical(newPos)
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
