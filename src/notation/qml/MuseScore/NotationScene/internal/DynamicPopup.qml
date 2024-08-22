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

    takeFocusOnClick: false

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
            direction: NavigationPanel.Horizontal
            section: root.notationViewNavigationSection
            order: root.navigationOrderStart
            accessible.name: qsTrc("notation", "Dynamics Popup")

            onNavigationEvent: function(event) {
                if (event.type === NavigationEvent.Escape) {
                    root.close()
                }
            }
        }

        function goToPreviousPage() {
            if (currentPage > 0) {
                currentPage--
            } else {
                currentPage = dynamicModel.pages.length - 1
            }

            Qt.callLater(requestNavigationActive, dynamicRepeater.count - 1)
        }

        function goToNextPage() {
            if (currentPage < dynamicModel.pages.length - 1) {
                currentPage++
            } else {
                currentPage = 0
            }

            Qt.callLater(requestNavigationActive, 0)
        }

        function requestNavigationActive(index) {
            dynamicRepeater.itemAt(index).navigation.requestActive()
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
                content.goToPreviousPage()
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

                navigation.panel: dynamicsNavPanel
                navigation.order: index
                accessible.name: modelData.accessibleName
                navigation.onNavigationEvent: function(event) {
                    switch (event.type) {
                    case NavigationEvent.Up:
                    case NavigationEvent.Left: {
                        if (index == 0) {
                            content.goToPreviousPage()

                            event.accepted = true
                        }

                        break
                    }

                    case NavigationEvent.Right:
                    case NavigationEvent.Down: {
                        if (index == dynamicRepeater.count - 1) {
                            content.goToNextPage()

                            event.accepted = true
                        }

                        break
                    }
                    }
                }

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

                mouseArea.onContainsMouseChanged: {
                    if (modelData.type === DynamicPopupModel.Dynamic) {
                        mouseArea.containsMouse ? dynamicModel.showPreview(currentPage, index) : dynamicModel.hidePreview()
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
                content.goToNextPage()
            }
        }
    }
}
