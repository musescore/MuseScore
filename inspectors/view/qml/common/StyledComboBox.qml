import QtQuick 2.9
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.0

ComboBox {
    id: root

    property bool isExpanded: false
    property bool isIndeterminate: false
    property string valueRole: "valueRole"
    property var currentValue

    function indexOfValue(value) {
        for (var i = 0; i < count; ++i) {
            if (model[i][valueRole] === value)
                return i
        }

        return -1
    }

    onCurrentIndexChanged: {
        if (currentIndex === -1)
            return

        root.currentValue = model[currentIndex][valueRole]
    }

    displayText: currentIndex === -1 ? "--" : currentText

    implicitHeight: 32

    padding: 0

    delegate: ItemDelegate {
        height: root.implicitHeight
        width: root.width
        contentItem: Text {
            text: modelData[textRole]
            color: "#000000"
            font: root.font
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        background: Rectangle {
            anchors.fill: parent

            radius: 4
            color: highlighted ? globalStyle.highlight : globalStyle.button

            Rectangle {
                id: roundedCornersOverlay

                anchors.left: parent.left
                anchors.right: parent.right

                height: parent.radius

                color: parent.color

                states: [
                    State {
                        name: "TOP_CORNERS_ROUNDED"
                        when: index === 0

                        AnchorChanges { target: roundedCornersOverlay; anchors.top: undefined
                            anchors.bottom: parent.bottom }
                    },

                    State {
                        name: "NO_ROUNDED_CORNERS"
                        when: index !== 0 && index !== count - 1

                        AnchorChanges { target: roundedCornersOverlay; anchors.top: parent.top
                            anchors.bottom: parent.bottom }
                    },

                    State {
                        name: "BOTTOM_CORNERS_ROUNDED"
                        when: index === count - 1

                        AnchorChanges { target: roundedCornersOverlay; anchors.top: parent.top
                            anchors.bottom: undefined }
                    }
                ]
            }
        }

        highlighted: root.highlightedIndex === index
    }

    contentItem: Text {
        leftPadding: 12
        rightPadding: root.indicator.width + root.spacing

        text: root.displayText
        font: root.font
        color: "#000000"
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        anchors.fill: parent

        color: globalStyle.button

        radius: 4
    }

    indicator: Rectangle {
        implicitHeight: 32
        implicitWidth: 32

        x: root.width - width
        y: root.topPadding + (root.availableHeight - height) / 2

        radius: 4

        color: root.pressed || root.isExpanded ? globalStyle.highlight : globalStyle.button

        StyledIcon {
            anchors.centerIn: parent

            icon: "qrc:/resources/icons/arrow_down.svg"

            sourceSize.height: 16
            sourceSize.width: 16
        }

        Rectangle {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left

            width: parent.radius

            color: parent.color
        }
    }

    popup: Popup {
        id: popup

        y: 0

        padding: 0
        margins: 0

        implicitHeight: contentItem.implicitHeight
        width: root.width

        contentItem: ListView {
            id: contentListView

            implicitHeight: contentHeight
            clip: true
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex
            interactive: false

            populate: Transition {
                NumberAnimation { property: "opacity"; from: 0.5; to: 1; duration: 200; easing.type: Easing.OutCubic }
            }
        }

        background: DropShadow {
            anchors.fill: parent
            verticalOffset: 4
            radius: 12.0
            samples: 30
            color: "#75000000"
            source: popup.contentItem
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
