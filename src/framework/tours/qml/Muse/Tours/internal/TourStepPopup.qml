/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
import Muse.UiComponents 1.0

StyledPopupView {
    id: root

    property alias title: titleLabel.text
    property alias description: descriptionLabel.text
    property string videoExplanationUrl: ""

    property int index: 0
    property int total: 0

    padding: 8
    margins: 8

    signal hideRequested()
    signal nextRequested()

    function calculateSize() {
        contentWidth = Math.min(content.implicitWidth, 300 - margins * 2)
        contentHeight = content.implicitHeight

        x = root.parent.width / 2 - (contentWidth + padding * 2 + margins * 2) / 2
        y = root.parent.height
    }

    ColumnLayout {
        id: content

        anchors.fill: parent
        spacing: 8

        RowLayout {
            id: row

            spacing: 6

            StyledTextLabel {
                id: titleLabel

                font: ui.theme.largeBodyBoldFont
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.Wrap
                maximumLineCount: 3
            }

            Rectangle {
                Layout.preferredWidth: newLabel.implicitWidth + 4
                Layout.preferredHeight: newLabel.implicitHeight + 4

                color: ui.theme.fontPrimaryColor
                radius: 2

                StyledTextLabel {
                    id: newLabel

                    anchors.centerIn: parent

                    text: qsTrc("tours", "New")
                    font: ui.theme.bodyBoldFont
                    color: ui.theme.backgroundPrimaryColor
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            StyledTextLabel {
                text: (root.index) + "/" + root.total

                visible: root.total > 1
            }
        }

        StyledTextLabel {
            id: descriptionLabel
            Layout.fillWidth: true

            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.Wrap

            visible: Boolean(root.description)
        }

        ButtonBox {
            id: box

            Layout.fillWidth: true
            //! hack: it looks like ButtonBox doesn't work well in ColumnLayout
            Layout.leftMargin: -4

            spacing: 4

            FlatButton {
                id: watchVideoBtn

                Layout.preferredWidth: (content.width - box.spacing) / 2

                text: qsTrc("tours", "Watch video")
                icon: IconCode.OPEN_LINK
                orientation: Qt.Horizontal

                buttonRole: ButtonBoxModel.CustomRole
                buttonId: ButtonBoxModel.CustomButton + 1

                visible: root.videoExplanationUrl !== ""

                onClicked: {
                    api.launcher.openUrl(root.videoExplanationUrl)
                }
            }

            FlatButton {
                Layout.preferredWidth: watchVideoBtn.visible ? (content.width - box.spacing) / 2 : 54
                Layout.alignment: Qt.AlignRight

                property bool isLastStep: root.index == root.total

                text: isLastStep ? qsTrc("tours", "Got it") : qsTrc("tours", "Next")

                buttonRole: isLastStep ? ButtonBoxModel.Apply : ButtonBoxModel.Next
                buttonId:  isLastStep ? ButtonBoxModel.ApplyRole : ButtonBoxModel.ContinueRole
                accentButton: true
                isNarrow: !watchVideoBtn.visible

                onClicked: {
                    if (isLastStep) {
                        root.hideRequested()
                    } else {
                        root.nextRequested()
                    }
                }
            }
        }
    }
}
