/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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
import QtQuick 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents

PopupPanel {
    id: root

    property string title: ""
    property string description: ""
    property var additionalInfoModel: undefined

    property Component buttonsPanel: null
    property var mainButton: null

    property NavigationPanel contentNavigation: NavigationPanel {
        name: root.objectName != "" ? root.objectName : "InfoPanel"

        enabled: root.visible
        section: root.navigationSection
        order: 1

        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }
    }

    implicitWidth: content.implicitWidth + content.anchors.leftMargin + content.anchors.rightMargin
    implicitHeight: content.implicitHeight + content.anchors.topMargin + content.anchors.bottomMargin

    height: Math.max(implicitHeight, 360)

    visible: false

    accessible.name: root.title

    ColumnLayout {
        id: content
        anchors.fill: parent
        anchors.topMargin: 44
        anchors.leftMargin: 68
        anchors.rightMargin: 68
        anchors.bottomMargin: 44

        spacing: 36

        property bool opened: root.visible
        onOpenedChanged: {
            if (opened) {
                Qt.callLater(focusOnOpened)
            } else {
                Qt.callLater(resetFocusOnInfo)
            }
        }

        function focusOnOpened() {
            if (Boolean(root.mainButton)) {
                root.mainButton.navigation.requestActive()
            }

            readInfo()
        }

        function readInfo() {
            accessibleInfo.ignored = false
            accessibleInfo.focused = true
        }

        function resetFocusOnInfo() {
            accessibleInfo.ignored = true
            accessibleInfo.focused = false
        }

        AccessibleItem {
            id: accessibleInfo
            accessibleParent: root.accessible
            visualItem: root
            role: MUAccessible.Button
            name: {
                var text = root.title + "."

                if (Boolean(root.additionalInfoModel)) {
                    for (var i = 0; i < root.additionalInfoModel.length; i++) {
                        text += " " + root.additionalInfoModel[i].title + " " + root.additionalInfoModel[i].value + ". "
                    }
                }

                text += root.description + ". " + root.mainButton.text

                return text
            }
        }

        Column {
            Layout.fillWidth: true
            spacing: 8

            StyledTextLabel {
                id: titleLabel
                width: parent.width
                horizontalAlignment: Text.AlignLeft
                text: root.title ?? ""
                font: ui.theme.headerBoldFont
            }

            StyledTextLabel {
                visible: Boolean(root.additionalInfoModel)
                width: parent.width
                horizontalAlignment: Text.AlignLeft
                font: ui.theme.largeBodyFont

                text: root.additionalInfoModel
                      ?.map(pair => `${pair.title} <b>${pair.value}</b>`)
                      ?.join(" | ")
            }
        }

        StyledTextLabel {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumWidth: 600

            opacity: 0.75
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignTop
            horizontalAlignment: Text.AlignLeft

            text: root.description ?? ""
        }

        Loader {
            id: buttonsPanelLoader

            Layout.fillWidth: true

            sourceComponent: root.buttonsPanel
        }
    }
}
