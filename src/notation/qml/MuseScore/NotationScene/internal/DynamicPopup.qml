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

    margins: 4

    contentWidth: content.width
    contentHeight: content.height

    showArrow: false

    signal elementRectChanged(var elementRect)

    function updatePosition() {
        root.x = root.parent.width / 2 - root.contentWidth / 2
        root.y = root.parent.height
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

                implicitWidth: modelData.width
                implicitHeight: root.buttonHeight
                transparent: true

                contentItem: modelData.type === DynamicPopupModel.Dynamic ? dynamicComp :
                                modelData.type === DynamicPopupModel.Crescendo ? crescHairpinComp :
                                modelData.type === DynamicPopupModel.Decrescendo ? dimHairpinComp : null

                Component {
                    id: dynamicComp

                    StyledTextLabel {
                        id: dynamicLabel
                        text: modelData.text
                        font.family: dynamicModel.fontFamily
                        font.pixelSize: 30

                        anchors.centerIn: parent
                        anchors.horizontalCenterOffset: modelData.offset
                        anchors.verticalCenterOffset: 5
                    }
                }

                Component {
                    id: crescHairpinComp

                    Canvas {
                        width: modelData.width
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
                            ctx.strokeStyle = ui.theme.fontPrimaryColor;
                            ctx.lineWidth = 1;
                            ctx.stroke();
                        }
                    }
                }

                Component {
                    id: dimHairpinComp

                    Canvas {
                        width: modelData.width
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
                            ctx.strokeStyle = ui.theme.fontPrimaryColor;
                            ctx.lineWidth = 1;
                            ctx.stroke();
                        }
                    }
                }

                onClicked: {
                    modelData.type === DynamicPopupModel.Dynamic ? dynamicModel.addOrChangeDynamic(currentPage, index) : dynamicModel.addHairpinToDynamic(modelData.type);
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
