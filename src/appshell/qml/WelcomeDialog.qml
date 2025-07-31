/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
import Muse.GraphicalEffects 1.0

import MuseScore.AppShell 1.0

StyledDialogView {
    id: root

    title: qsTrc("appshell/welcome", "Welcome")

    contentHeight: 560
    contentWidth: 612

    WelcomeDialogModel {
        id: model

        Component.onCompleted: {
            model.init()
        }
    }

    QtObject {
        id: prv
        readonly property int imageWidth: 480
        readonly property string titleText: model.currentItem ? model.currentItem.title : ""
        readonly property string descText: model.currentItem ? model.currentItem.description : ""
    }

    function openCurrent() {
        if (!model.currentItem) {
            return
        }
        api.launcher.openUrl(model.currentItem.destinationUrl)
    }

    Column {
        id: contentColumn

        spacing: 16

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            topMargin: contentColumn.spacing
        }

        PageIndicator {
            anchors {
                horizontalCenter: parent.horizontalCenter
            }
            count: model.count
            currentIndex: model.currentIndex
        }

        Row {
            id: imageAndArrowsRow

            height: 270
            anchors {
                left: contentColumn.left
                right: contentColumn.right
            }

            NavigationPanel {
                id: arrowButtonsPanel
                name: "ArrowButtonsPanel"
                order: 2
                section: root.navigationSection
                direction: NavigationPanel.Horizontal
            }

            Item {
                id: prevButtonArea

                height: imageAndArrowsRow.height
                width: (imageAndArrowsRow.width - image.width) / 2

                FlatButton {
                    id: prevButton

                    height: 30
                    width: prevButton.height
                    anchors.centerIn: prevButtonArea

                    enabled: model.hasPrev
                    icon: IconCode.CHEVRON_LEFT

                    navigation.panel: arrowButtonsPanel
                    navigation.column: 0
                    navigation.accessible.description: qsTrc("appshell/welcome", "Previous item")

                    onClicked: {
                        model.prevItem()
                    }
                }
            }

            Image {
                id: image

                width: prv.imageWidth
                height: imageAndArrowsRow.height

                fillMode: Image.PreserveAspectCrop
                source: model.currentItem ? model.currentItem.imageUrl : ""

                layer.enabled: ui.isEffectsAllowed
                layer.effect: EffectOpacityMask {
                    maskSource: Rectangle {
                        width: image.width
                        height: image.height
                        radius: 8
                    }
                }

                MouseArea {
                    anchors.fill: image
                    onClicked: {
                        root.openCurrent()
                    }
                }
            }

            Item {
                id: nextButtonArea

                width: (imageAndArrowsRow.width - image.width) / 2
                height: imageAndArrowsRow.height

                FlatButton {
                    id: nextButton

                    height: 30
                    width: nextButton.height
                    anchors.centerIn: nextButtonArea

                    enabled: model.hasNext
                    icon: IconCode.CHEVRON_RIGHT

                    navigation.panel: arrowButtonsPanel
                    navigation.column: 1
                    navigation.accessible.description: qsTrc("appshell/welcome", "Next item")

                    onClicked: {
                        model.nextItem()
                    }
                }
            }
        }

        Column {
            id: titleAndDescriptionColumn

            spacing: 6
            anchors {
                left: contentColumn.left
                right: contentColumn.right
            }

            StyledTextLabel {
                id: titleLabel

                height: 52
                width: prv.imageWidth
                anchors.horizontalCenter: titleAndDescriptionColumn.horizontalCenter

                text: prv.titleText
                font: ui.theme.headerBoldFont
                wrapMode: Text.WordWrap

                maximumLineCount: 2
            }

            StyledTextLabel {
                id: descriptionLabel

                height: 52
                width: prv.imageWidth
                anchors.horizontalCenter: titleAndDescriptionColumn.horizontalCenter

                text: prv.descText

                font: ui.theme.bodyFont
                wrapMode: Text.WordWrap
                maximumLineCount: 3
            }
        }

        FlatButton {
            id: contentButton

            height: 32
            anchors.horizontalCenter: contentColumn.horizontalCenter

            text: model.currentItem ? model.currentItem.buttonText : ""
            accentButton: true

            navigation.panel: NavigationPanel {
                name: "ContentButton"
                order: 0
                section: root.navigationSection
            }
            navigation.accessible.description: prv.titleText + "; " + prv.descText

            onClicked: {
                root.openCurrent()
            }
        }
    }

    Rectangle {
        id: footerArea

        height: 60
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        color: ui.theme.backgroundSecondaryColor

        NavigationPanel {
            id: footerPanel
            name: "FooterPanel"
            order: 1
            section: root.navigationSection
            direction: NavigationPanel.Horizontal
        }

        CheckBox {
            id: showOnStartup

            anchors {
                margins: 24
                left: footerArea.left
                verticalCenter: footerArea.verticalCenter
            }

            text: qsTrc("appshell/welcome", "Don’t show welcome dialog on startup")
            checked: !model.showOnStartup

            navigation.panel: footerPanel
            navigation.column: 1
            navigation.accessible.description: showOnStartup.text

            onClicked: {
                model.showOnStartup = !model.showOnStartup
            }
        }

        FlatButton {
            id: okButton

            width: 154
            height: 30
            anchors {
                margins: 24
                right: footerArea.right
                verticalCenter: footerArea.verticalCenter
            }

            text: qsTrc("global", "OK")

            navigation.panel: footerPanel
            navigation.column: 0
            navigation.accessible.description: okButton.text

            onClicked: {
                root.accept()
            }
        }
    }
}
