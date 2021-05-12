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
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.0
import MuseScore.Ui 1.0

ComboBox {
    id: root

    property bool isExpanded: false
    property bool isIndeterminate: false
    property bool opensUpward: false
    property alias textRoleName: root.textRole
    property string valueRoleName: "valueRole"
    property var value
    property var maxVisibleItemCount: 6

    property alias navigation: navCtrl

    opacity: root.enabled ? 1 : ui.theme.itemOpacityDisabled

    Keys.onReleased: {
        //! TODO Conflicted with navigation, needs research
        if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
            event.accepted = true
        }
    }

    onPressedChanged: {
        if (root.pressed) {
            root.ensureActiveFocus()
        }
    }

    function ensureActiveFocus() {
        if (!root.activeFocus) {
            root.forceActiveFocus()
        }
    }

    function valueFromModel(index, roleName) {

        // Simple models (like JS array) with single predefined role name - modelData
        if (model[index] !== undefined) {
            if (model[index][roleName] === undefined) {
                return model[index]
            }

            return model[index][roleName]
        }

        // Complex models (like QAbstractItemModel) with multiple role names
        var item = delegateModel.items.get(index)

        return item.model[roleName]
    }

    function indexOfValue(value) {
        if (!model) {
            return -1
        }

        for (var i = 0; i < count; ++i) {
            if (valueFromModel(i, valueRoleName) === value) {
                return i
            }
        }

        return -1
    }

    onCurrentIndexChanged: {
        if (currentIndex === -1)
            return

        root.value = valueFromModel(currentIndex, valueRoleName)
    }

    displayText: currentIndex === -1 ? "--" : currentText

    implicitHeight: 30

    padding: 0

    NavigationControl {
        id: navCtrl
        name: root.objectName != "" ? root.objectName : "StyledComboBox"
        enabled: root.enabled
        onActiveChanged: {
            if (!root.activeFocus) {
                root.forceActiveFocus()
            }
        }
        onTriggered: {
            if (root.popup.opened) {
                root.popup.close()
            } else {
                root.popup.open()
            }
        }
    }

    QtObject {
        id: privateProperties

        readonly property int textPadding: 12
    }

    delegate: ItemDelegate {
        height: root.implicitHeight
        width: root.width

        contentItem: StyledTextLabel {
            leftPadding: privateProperties.textPadding
            rightPadding: leftPadding

            text: valueFromModel(index, textRoleName)
            elide: Text.ElideRight
            horizontalAlignment: Qt.AlignLeft
        }

        background: RoundedRectangle {
            id: delegateBackgroundRect

            anchors.fill: parent

            color: ui.theme.backgroundPrimaryColor

            RoundedRectangle {
                id: selectionOverlay

                anchors.fill: parent

                topLeftRadius: delegateBackgroundRect.topLeftRadius
                topRightRadius: delegateBackgroundRect.topRightRadius
                bottomLeftRadius: delegateBackgroundRect.bottomLeftRadius
                bottomRightRadius: delegateBackgroundRect.bottomRightRadius

                color: root.currentIndex === index ? ui.theme.accentColor : ui.theme.buttonColor
                opacity: highlighted ? ui.theme.accentOpacityNormal : ui.theme.buttonOpacityNormal
            }

            states: [
                State {
                    name: "TOP_CORNERS_ROUNDED"
                    when: index === 0

                    PropertyChanges {
                        target: delegateBackgroundRect;
                        topLeftRadius: 4
                        topRightRadius: 4
                        bottomLeftRadius: 0
                        bottomRightRadius: 0
                    }
                },

                State {
                    name: "NO_ROUNDED_CORNERS"
                    when: index !== 0 && index !== count - 1

                    PropertyChanges {
                        target: delegateBackgroundRect;
                        topLeftRadius: 0
                        topRightRadius: 0
                        bottomLeftRadius: 0
                        bottomRightRadius: 0
                    }
                },

                State {
                    name: "BOTTOM_CORNERS_ROUNDED"
                    when: index === count - 1

                    PropertyChanges {
                        target: delegateBackgroundRect;
                        topLeftRadius: 0
                        topRightRadius: 0
                        bottomLeftRadius: 4
                        bottomRightRadius: 4
                    }
                }
            ]
        }

        highlighted: root.highlightedIndex === index
    }

    contentItem: StyledTextLabel {
        leftPadding: privateProperties.textPadding
        rightPadding: leftPadding

        text: root.displayText
        elide: Text.ElideRight

        horizontalAlignment: Qt.AlignLeft
    }

    background: Rectangle {
        height: root.height
        width: root.width
        color: ui.theme.buttonColor
        opacity: ui.theme.buttonOpacityNormal

        radius: 4

        border.width: navCtrl.active ? 2 : 0
        border.color: ui.theme.focusColor
    }

    indicator: Item {
        id: indicatorCanvas

        height: root.implicitHeight
        width: height

        x: root.width - width
        y: root.topPadding + (root.availableHeight - height) / 2

        opacity: ui.theme.buttonOpacityNormal

        StyledIconLabel {
            anchors.fill: parent
            iconCode: IconCode.SMALL_ARROW_DOWN
            opacity: 1
        }
    }

    popup: Popup {
        id: popup

        y: root.opensUpward ? -(contentListView.height - contentListView.contentHeight) : 0

        padding: 0
        margins: 0

        implicitHeight: contentItem.implicitHeight
        width: root.width

        contentItem: ListView {
            id: contentListView

            implicitHeight: root.maxVisibleItemCount * (contentHeight / count)
            clip: true
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex
            interactive: root.maxVisibleItemCount < count

            boundsBehavior: Flickable.StopAtBounds
            highlightMoveDuration: 250

            ScrollBar.vertical: StyledScrollBar { }

            populate: Transition {
                NumberAnimation { property: "opacity"; from: 0.5; to: 1; duration: 200; easing.type: Easing.OutCubic }
            }
        }

        background: StyledDropShadow {
            anchors.fill: parent
            source: popup.contentItem
            radius: 12.0
        }

        Behavior on implicitHeight {
            NumberAnimation {
                duration: 200
                easing.type: Easing.OutCubic
            }
        }

        onOpened: {
            root.isExpanded = true
        }

        onClosed: {
            root.isExpanded = false
        }
    }
}
