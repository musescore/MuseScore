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

    property int buttonHeight: 30

    property int currentPage: 0
    property bool isHairpinPage: currentPage == 6

    property var widthOffsetList: [
        // [Width, Offset] of each dynamic in that Page
        [[30, 1.5], [21, 1.0], [29, 0.5], [30, 0.5], [23, 2.5], [30, 2.5]], // Page 1
        [[38, 2.5], [45, 2.5], [53, 2.5]                                 ], // Page 2
        [[30, 2.5], [32 ,2.0], [25, 0.5], [29, 0.5], [38 ,0.5]           ], // Page 3
        [[37, 0.5], [33, 0,5], [30, 0.5], [26, 0.5], [26, 2.5]           ], // Page 4
        [[74, 2.0], [60, 2.5]                                            ], // Page 5
        [[64, 2.0], [52, 2.0], [44, 2.0]                                 ], // Page 6
        [[62, 0.0], [62, 0.0]                                            ]  // Page 7 - Hairpins
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

        FlatButton {
            id: leftButton

            implicitWidth: 16
            transparent: true

            contentItem: StyledIconLabel {
                id: leftArrowLabel
                iconCode: IconCode.CHEVRON_LEFT
                font.pixelSize: 16
            }

            onClicked: {
                if (currentPage > 0) {
                    currentPage--
                } else {
                    currentPage = dynamicModel.pages.length - 1
                }
            }
        }

        Item {
            Layout.fillWidth: true
        }

        Repeater {
            id: dynamicRepeater
            model: dynamicModel.pages[currentPage]

            delegate: FlatButton {
                id: dynamicButton

                implicitWidth: widthOffsetList[currentPage][index][0]
                implicitHeight: root.buttonHeight
                transparent: true

                contentItem: isHairpinPage ? index == 0 ? crescHairpinComp : dimHairpinComp : dynamicComp

                Component {
                    id: dynamicComp

                    StyledTextLabel {
                        id: dynamicLabel
                        text: modelData
                        font.pixelSize: 30

                        anchors.centerIn: parent
                        anchors.horizontalCenterOffset: widthOffsetList[currentPage][index][1]
                        anchors.verticalCenterOffset: 5
                    }
                }

                Component {
                    id: crescHairpinComp

                    Canvas {
                        width: widthOffsetList[currentPage][index][0]
                        height: root.buttonHeight

                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.clearRect(0, 0, width, height);

                            ctx.beginPath();
                            ctx.moveTo(4, root.buttonHeight / 2);
                            ctx.lineTo(width - 4, root.buttonHeight / 2 - 4);
                            ctx.strokeStyle = ui.theme.fontPrimaryColor;
                            ctx.lineWidth = 1;
                            ctx.stroke();

                            ctx.beginPath();
                            ctx.moveTo(4, root.buttonHeight / 2);
                            ctx.lineTo(width - 4, root.buttonHeight / 2 + 4);
                            ctx.strokeStyle = ui.theme.fontPrimary;
                            ctx.lineWidth = 1;
                            ctx.stroke();
                        }
                    }
                }

                Component {
                    id: dimHairpinComp

                    Canvas {
                        width: widthOffsetList[currentPage][index][0]
                        height: root.buttonHeight

                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.clearRect(0, 0, width, height);

                            ctx.beginPath();
                            ctx.moveTo(width - 4, root.buttonHeight / 2);
                            ctx.lineTo(4, root.buttonHeight / 2 - 4);
                            ctx.strokeStyle = ui.theme.fontPrimaryColor;
                            ctx.lineWidth = 1;
                            ctx.stroke();

                            ctx.beginPath();
                            ctx.moveTo(width - 4, root.buttonHeight / 2);
                            ctx.lineTo(4, root.buttonHeight / 2 + 4);
                            ctx.strokeStyle = ui.theme.fontPrimary;
                            ctx.lineWidth = 1;
                            ctx.stroke();
                        }
                    }
                }

                onClicked: {
                    // TO-DO
                }
            }
        }

        Item {
            Layout.fillWidth: true
        }

        FlatButton {
            id: rightButton

            implicitWidth: 16
            transparent: true

            contentItem: StyledIconLabel {
                id: rightArrowLabel
                iconCode: IconCode.CHEVRON_RIGHT
                font.pixelSize: 16
            }

            onClicked: {
                if (currentPage < dynamicModel.pages.length - 1) {
                    currentPage++
                } else {
                    currentPage = 0
                }
            }
        }
    }
}

