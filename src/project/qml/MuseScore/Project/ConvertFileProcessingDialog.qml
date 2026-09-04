/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

StyledDialogView {
    id: root

    objectName: "ConvertFileProcessingDialog"

    contentWidth: 580
    contentHeight: buttonBox.y + buttonBox.height + 16
    margins: 0

    modal: true

    function bodyText(accessible) {
        var homeScores = qsTrc("project/convert", "Home > Scores")
        if (!accessible) {
            homeScores = "<b>" + homeScores + "</b>"
        }

        return qsTrc("project/convert", "We’ll notify you once the score is ready to open. You can check the status of the score in %1.")
               .arg(homeScores)
    }

    function done(action) {
        root.ret = { errcode: 0, value: { action: action, showAgain: !dontShowAgainCheckBox.checked } }
        root.hide()
    }

    onNavigationActivateRequested: {
        buttonBox.firstFocusBtn.navigation.requestActive()
    }

    onAccessibilityActivateRequested: {
        accessibleInfo.readInfo()
    }

    NavigationPanel {
        id: buttonsPanel

        name: "ConvertFileProcessingDialogButtons"
        order: 1
        section: root.navigationSection
        direction: NavigationPanel.Horizontal
        accessible.role: MUAccessible.Dialog

        onNavigationEvent: function(event) {
            if (event.type === NavigationEvent.AboutActive) {
                var btn = buttonBox.firstFocusBtn
                if (Boolean(btn) && btn.enabled) {
                    event.setData("controlIndex", [ btn.navigation.row, btn.navigation.column ])
                }
            } else {
                buttonBox.restoreAccessibility()
                accessibleInfo.resetFocus()
            }
        }
    }

    AccessibleItem {
        id: accessibleInfo

        accessibleParent: buttonsPanel.accessible
        visualItem: textColumn
        role: MUAccessible.Button
        name: titleLabel.text + "; " + root.bodyText(/*accessible*/ true)
              + "; " + dontShowAgainCheckBox.text + "; " + buttonBox.firstFocusBtn.text

        function readInfo() {
            accessibleInfo.ignored = false
            accessibleInfo.focused = true
        }

        function resetFocus() {
            accessibleInfo.ignored = true
            accessibleInfo.focused = false
        }
    }

    Image {
        id: processingImage

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 24
        anchors.leftMargin: 24

        width: 86
        height: 111

        fillMode: Image.PreserveAspectFit
        source: "resources/ScoreProcessing.png"
    }

    Item {
        id: textColumn

        anchors.top: parent.top
        anchors.topMargin: 24
        anchors.left: parent.left
        anchors.leftMargin: processingImage.x + processingImage.width + 27
        anchors.right: parent.right
        anchors.rightMargin: 24

        //! NOTE: matches the icon's height so the checkbox can be pinned to its bottom,
        //! but grows if the localized text needs more room
        height: Math.max(processingImage.height, textBlock.implicitHeight + 16 + dontShowAgainCheckBox.implicitHeight)

        Column {
            id: textBlock

            anchors.top: parent.top
            width: parent.width

            spacing: 16

            StyledTextLabel {
                id: titleLabel

                width: parent.width
                text: qsTrc("project/convert", "Your score is being processed")
                font: ui.theme.largeBodyBoldFont
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.Wrap
            }

            StyledTextLabel {
                width: parent.width
                text: root.bodyText(/*accessible*/ false)
                textFormat: Text.RichText
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.Wrap
            }
        }

        CheckBox {
            id: dontShowAgainCheckBox

            anchors.bottom: parent.bottom
            width: parent.width

            text: qsTrc("global", "Don’t show again")
            checked: false

            navigation.panel: NavigationPanel {
                name: "ConvertFileProcessingDontShowAgain"
                section: root.navigationSection
                order: 2
            }

            onClicked: {
                dontShowAgainCheckBox.checked = !dontShowAgainCheckBox.checked
            }
        }
    }

    ButtonBox {
        id: buttonBox

        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.right: parent.right
        anchors.rightMargin: 24
        anchors.top: textColumn.bottom
        anchors.topMargin: 24

        navigationPanel: buttonsPanel
        isAccessibilityDisabledWhenInit: true

        FlatButton {
            text: qsTrc("project/convert", "Convert more")

            buttonRole: ButtonBoxModel.CustomRole
            buttonId: ButtonBoxModel.CustomButton + 1

            onClicked: root.done("convertMore")
        }

        FlatButton {
            text: qsTrc("project/convert", "Go to scores")

            buttonRole: ButtonBoxModel.CustomRole
            buttonId: ButtonBoxModel.CustomButton + 2

            onClicked: root.done("goToScores")
        }

        FlatButton {
            text: qsTrc("global", "OK")
            accentButton: true

            buttonRole: ButtonBoxModel.AcceptRole
            buttonId: ButtonBoxModel.Ok

            onClicked: root.done("")
        }
    }
}
