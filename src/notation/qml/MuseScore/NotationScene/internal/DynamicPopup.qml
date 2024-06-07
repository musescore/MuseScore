import QtQuick 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0

StyledPopupView {
    id: root

    property NavigationSection notationViewNavigationSection: null
    property int navigationOrderStart: 0
    property int navigationOrderEnd: dynamicsNavPanel.order

    property color normalColor: "transparent"
    property color hoverHitColor: ui.theme.buttonColor
    property color accentColor: ui.theme.accentColor

    property int rectHeight: 30
    property real rectRadius: 3

    property int currentPage: 0

    property var widthOffsetList: [
        // [Width, Offset] of each dynamic in that Page
        [[30, 1.5], [21, 1.0], [29, 0.5], [30, 0.5], [23, 2.5], [30, 2.5]], // Page 1
        [[38, 2.5], [45, 2.5], [53, 2.5]                                 ], // Page 2
        [[30, 2.5], [32 ,2.0], [25, 0.5], [29, 0.5], [38 ,0.5]           ], // Page 3
        [[37, 0.5], [33, 0,5], [30, 0.5], [26, 0.5], [26, 2.5]           ], // Page 4
        [[74, 2.0], [60, 2.5]                                            ], // Page 5
        [[64, 2.0], [52, 2.0], [44, 2.0]                                 ], // Page 6
        [[62, 0.0], [62, 0.0]                                            ]  // Page 7
    ]

    margins: 4

    contentWidth: content.width
    contentHeight: content.height

    showArrow: false

    signal elementRectChanged(var elementRect)

    function updatePosition() {
        var h = root.contentHeight
        root.x = root.parent.width / 2 - root.contentWidth / 2
        root.y = root.parent.height / 2 + root.margins * 2
    }

    // Child components are inheriting "opacity" from Parent components.
    // StyledIconLabel and StyledTextLabel inheriting opacity from Rectangle.
    // Instead use "color" with Qt.rgba()
    function hexToRgba(hex, opacity) {
        if (hex === "#00000000") {
            return Qt.rgba(0, 0, 0, 0)
        }

        hex = hex.slice(1); // Remove '#'

        var bigint = parseInt(hex, 16);
        var r = (bigint >> 16) & 255;
        var g = (bigint >> 8) & 255;
        var b = bigint & 255;

        r /= 255;
        g /= 255;
        b /= 255;

        return Qt.rgba(r, g, b, opacity);
    }

    RowLayout {
        id: content

        width: 208
        spacing: 1

        DynamicPopupModel {
            id: dynamicModel

            onItemRectChanged: function(rect) {
                root.elementRectChanged(rect)
            }
        }

        Component.onCompleted: {
            dynamicModel.init()
        }

        NavigationPanel {
            id: dynamicsNavPanel
            name: "DynamicsPopup"
            direction: NavigationPanel.Vertical
            section: root.notationViewNavigationSection
            order: root.navigationOrderStart
            accessible.name: qsTrc("notation", "Dynamics Popup")
        }

        // -------------------------------- Custom Rects Approach --------------------------------
        Rectangle {
            id: leftArrowRect

            width: leftArrowLabel.contentWidth
            height: root.rectHeight
            radius: root.rectRadius

            border.width: ui.theme.borderWidth
            border.color: ui.theme.strokeColor

            color: hexToRgba(root.normalColor.toString(), ui.theme.buttonOpacityNormal)

            StyledIconLabel {
                id: leftArrowLabel
                iconCode: IconCode.CHEVRON_LEFT
                font.pixelSize: 16
                anchors.centerIn: parent
            }

            states: [
                State {
                    name: "PRESSED"
                    when: mouseAreaLeft.pressed

                    PropertyChanges {
                        target: leftArrowRect
                        color: hexToRgba(root.hoverHitColor.toString(), ui.theme.buttonOpacityHit)
                    }
                },

                State {
                    name: "HOVERED"
                    when: mouseAreaLeft.containsMouse && !mouseAreaLeft.pressed

                    PropertyChanges {
                        target: leftArrowRect
                        color: hexToRgba(root.hoverHitColor.toString(), ui.theme.buttonOpacityHover)
                    }
                }
            ]

            MouseArea {
                id: mouseAreaLeft
                anchors.fill: parent

                hoverEnabled: true

                onClicked: {
                    if (currentPage > 0) {
                        currentPage--
                    } else {
                        currentPage = dynamicModel.pages.length - 1
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
        }

        Repeater {
            model: dynamicModel.pages[currentPage]

            delegate: Rectangle {
                id: dynamicRect

                width: widthOffsetList[currentPage][index][0]
                height: root.rectHeight
                radius: root.rectRadius

                border.width: ui.theme.borderWidth
                border.color: ui.theme.strokeColor

                color: hexToRgba(root.normalColor.toString(), ui.theme.buttonOpacityNormal)

                StyledTextLabel {
                    id: dynamicLabel
                    text: modelData
                    font.pixelSize: 30

                    anchors.centerIn: parent
                    anchors.horizontalCenterOffset: widthOffsetList[currentPage][index][1]
                    anchors.verticalCenterOffset: (currentPage === 6) ? 7 : 5 // More offset for Hairpins

                    transform: Scale {
                        origin.x: dynamicLabel.width / 2
                        origin.y: dynamicLabel.height / 2
                        xScale: (currentPage === 6) ? (dynamicRect.width - 8) / dynamicLabel.width : 1
                        yScale: 1
                    }
                }

                states: [
                    State {
                        name: "PRESSED"
                        when: mouseAreaDynamic.pressed

                        PropertyChanges {
                            target: dynamicRect
                            color: hexToRgba(root.hoverHitColor.toString(), ui.theme.buttonOpacityHit)
                        }
                    },

                    State {
                        name: "HOVERED"
                        when: mouseAreaDynamic.containsMouse && !mouseAreaDynamic.pressed

                        PropertyChanges {
                            target: dynamicRect
                            color: hexToRgba(root.hoverHitColor.toString(), ui.theme.buttonOpacityHover)
                        }
                    }
                ]

                MouseArea {
                    id: mouseAreaDynamic
                    anchors.fill: parent

                    hoverEnabled: true

                    onClicked: {
                        console.log(content.width)
                        // TO-DO
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
        }

        Rectangle {
            id: rightArrowRect

            width: rightArrowLabel.contentWidth
            height: root.rectHeight
            radius: root.rectRadius

            border.width: ui.theme.borderWidth
            border.color: ui.theme.strokeColor

            color: hexToRgba(root.normalColor.toString(), ui.theme.buttonOpacityNormal)

            StyledIconLabel {
                id: rightArrowLabel
                iconCode: IconCode.CHEVRON_RIGHT
                font.pixelSize: 16
                anchors.centerIn: parent
            }

            states: [
                State {
                    name: "PRESSED"
                    when: mouseAreaRight.pressed

                    PropertyChanges {
                        target: rightArrowRect
                        color: hexToRgba(root.hoverHitColor.toString(), ui.theme.buttonOpacityHit)
                    }
                },

                State {
                    name: "HOVERED"
                    when: mouseAreaRight.containsMouse && !mouseAreaRight.pressed

                    PropertyChanges {
                        target: rightArrowRect
                        color: hexToRgba(root.hoverHitColor.toString(), ui.theme.buttonOpacityHover)
                    }
                }
            ]

            MouseArea {
                id: mouseAreaRight
                anchors.fill: parent

                hoverEnabled: true

                onClicked: {
                    if (currentPage < dynamicModel.pages.length - 1) {
                        currentPage++
                    } else {
                        currentPage = 0
                    }
                }
            }
        }


        // -------------------------------- FlatButtons Approach --------------------------------
        // FlatButton {
        //     implicitWidth: 16
        //     isNarrow: true
        //     transparent: true

        //     contentItem: Rectangle {
        //         id: leftArrowRect

        //         width: leftArrowLabel.contentWidth
        //         height: 24
        //         color: "transparent"

        //         StyledIconLabel {
        //             id: leftArrowLabel
        //             iconCode: IconCode.CHEVRON_LEFT
        //             font.pixelSize: 12
        //             anchors.centerIn: parent
        //         }
        //     }

        //     onClicked: {
        //         if (currentPage > 0) {
        //             currentPage--
        //         } else {
        //             currentPage = dynamicModel.pages.length - 1
        //         }
        //     }
        // }

        // Item {
        //     Layout.fillWidth: true
        // }

        // Repeater {
        //     model: dynamicModel.pages[currentPage]

        //     delegate: FlatButton {
        //         isNarrow: true
        //         transparent: true

        //         contentItem: Rectangle {
        //             id: dynamicRect

        //             width: dynamicLabel.contentWidth
        //             height: 24
        //             color: "transparent"

        //             StyledTextLabel {
        //                 id: dynamicLabel
        //                 text: modelData
        //                 font.pixelSize: 24
        //                 anchors.centerIn: parent
        //                 anchors.verticalCenterOffset: 4
        //             }
        //         }

        //         onClicked: {
        //             // TO-DO
        //         }
        //     }
        // }

        // Item {
        //     Layout.fillWidth: true
        // }

        // FlatButton {
        //     implicitWidth: 16
        //     isNarrow: true
        //     transparent: true

        //     contentItem: Rectangle {
        //         id: rightArrowRect

        //         width: rightArrowLabel.contentWidth
        //         height: 24
        //         color: "transparent"

        //         StyledIconLabel {
        //             id: rightArrowLabel
        //             iconCode: IconCode.CHEVRON_RIGHT
        //             font.pixelSize: 12
        //             anchors.centerIn: parent
        //         }
        //     }
        //     onClicked: {
        //         if (currentPage < dynamicModel.pages.length - 1) {
        //             currentPage++
        //         } else {
        //             currentPage = 0
        //         }
        //     }
        // }
    }
}

