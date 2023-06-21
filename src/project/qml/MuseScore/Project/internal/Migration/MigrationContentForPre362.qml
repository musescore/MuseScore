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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0

Item {
    id: root

    property string appVersion: ""
    property bool isAskAgain: false

    property bool isApplyLeland: true
    property bool isApplyEdwin: true

    property NavigationSection navigationSection: null

    function activateNavigation() {
        applyBtn.navigation.requestActive()
        accessibleInfo.readInfo()
    }

    signal isAskAgainChangeRequested(bool askAgain)
    signal isApplyLelandChangeRequested(bool applyLeland)
    signal isApplyEdwinChangeRequested(bool applyEdwin)

    signal watchVideoRequested()
    signal acceptRequested()

    AccessibleItem {
        id: accessibleInfo

        accessibleParent: footer.navigationPanel.accessible
        visualItem: root
        role: MUAccessible.Button
        name: headerTitle.text + ". " + headerSubtitle.text + " " + applyBtn.text

        function readInfo() {
            accessibleInfo.ignored = false
            accessibleInfo.focused = true
        }

        function resetFocus() {
            accessibleInfo.ignored = true
            accessibleInfo.focused = false
        }
    }

    Column {
        id: mainContent
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 16
        height: childrenRect.height

        property NavigationPanel navigationPanel: NavigationPanel {
            name: "MigrationContentPanel"
            section: root.navigationSection
            accessible.role: MUAccessible.Dialog
            order: 2
        }

        StyledTextLabel {
            id: headerTitle
            anchors.left: parent.left
            anchors.right: parent.right
            height: 32

            font: ui.theme.tabBoldFont

            horizontalAlignment: Qt.AlignLeft
            verticalAlignment: Qt.AlignVCenter

            text: qsTrc("project/migration", "This file was last saved in MuseScore %1").arg(root.appVersion)
        }

        StyledTextLabel {
            id: headerSubtitle
            anchors.left: parent.left
            anchors.right: parent.right
            height: 24

            horizontalAlignment: Qt.AlignLeft
            verticalAlignment: Qt.AlignVCenter

            text: qsTrc("project/migration", "Select the engraving improvements you would like to apply to your score")
        }

        Item {
            id: imageItem
            anchors.left: parent.left
            anchors.right: parent.right
            height: 264

            Image {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                height: 200
                fillMode: Image.PreserveAspectFit
                source: "migration.png"
            }
        }

        CheckBox {
            id: lelandOption
            anchors.left: parent.left
            height: 32
            text: qsTrc("project/migration", "Our new notation font, Leland")
            checked: root.isApplyLeland

            navigation.panel: mainContent.navigationPanel
            navigation.row: 1

            onClicked: {
                root.isApplyLelandChangeRequested(!checked)
            }
        }

        CheckBox {
            id: edwinOption
            anchors.left: parent.left
            height: 32
            text: qsTrc("project/migration", "Our new text font, Edwin")
            checked: root.isApplyEdwin

            navigation.panel: mainContent.navigationPanel
            navigation.row: 2

            onClicked: {
                root.isApplyEdwinChangeRequested(!checked)
            }
        }

        Item {
            width: 1
            height: 16
        }

        StyledTextLabel {
            id: noteLabel
            anchors.left: parent.left
            anchors.right: parent.right
            height: 32

            horizontalAlignment: Qt.AlignLeft
            verticalAlignment: Qt.AlignVCenter

            text: qsTrc("project/migration", "Please note: score layouts will be affected by improvements to MuseScore 4")
        }

        FlatButton {
            id: watchVideo

            text: qsTrc("project/migration", "Watch video")

            navigation.panel: mainContent.navigationPanel
            navigation.row: 4

            onClicked: {
                root.watchVideoRequested()
            }
        }
    }

    Item {
        id: footer

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        height: 56

        property NavigationPanel navigationPanel: NavigationPanel {
            name: "MigrationFooterPanel"
            section: root.navigationSection
            accessible.role: MUAccessible.Dialog
            order: 1
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: -16
            height: 2
            color: ui.theme.buttonColor
        }

        CheckBox {
            id: askAgain
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            text: qsTrc("global", "Don't ask again")
            checked: !root.isAskAgain

            navigation.panel: footer.navigationPanel
            navigation.row: 2

            onClicked: {
                root.isAskAgainChangeRequested(checked) // not `!checked` because the negation is in `checked: !root.isAskAgain`
            }
        }

        FlatButton {
            id: applyBtn
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: qsTrc("global", "OK")

            navigation.panel: footer.navigationPanel
            navigation.row: 1
            navigation.accessible.ignored: true
            navigation.onActiveChanged: {
                if (!navigation.active) {
                    accessible.ignored = false
                    accessible.focused = true
                    accessibleInfo.resetFocus()
                }
            }

            onClicked: {
                root.acceptRequested()
            }
        }
    }
}
