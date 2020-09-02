import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

Item {
    id: root

    property bool held: false
    property var attachedControl: undefined
    property var index: styleData.index
    property string filterKey
    property bool isSelected: false
    property bool isDragAvailable: false
    property var type: InstrumentTreeItemType.UNDEFINED

    signal clicked(var mouse)

    QtObject {
        id: privateProperties

        property var currentSettingsPopup: null
    }

    anchors {
        verticalCenter: parent ? parent.verticalCenter : undefined
        horizontalCenter: parent ? parent.horizontalCenter : undefined
    }

    height: parent ? parent.height : implicitHeight
    width: parent ? parent.width : implicitWidth

    implicitHeight: 38
    implicitWidth: 248

    Drag.keys: [ root.filterKey ]
    Drag.active: held && isDragAvailable
    Drag.source: root
    Drag.hotSpot.x: width / 2
    Drag.hotSpot.y: height / 2

    Rectangle {
        id: background

        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor
        opacity: 1

        states: [
            State {
                name: "HOVERED"
                when: mouseArea.containsMouse && !mouseArea.containsPress && !root.isSelected

                PropertyChanges {
                    target: background
                    color: ui.theme.buttonColor
                    opacity: ui.theme.buttonOpacityHover
                }
            },

            State {
                name: "PRESSED"
                when: mouseArea.containsPress && !root.isSelected

                PropertyChanges {
                    target: background
                    color: ui.theme.buttonColor
                    opacity: ui.theme.buttonOpacityHit
                }
            },

            State {
                name: "SELECTED"
                when: root.isSelected

                PropertyChanges {
                    target: background
                    color: ui.theme.accentColor
                    opacity: 0.5
                }
            }
        ]
    }

    DropShadow {
        id: shadow

        anchors.fill: parent
        source: background
        color: "#75000000"
        verticalOffset: 4
        samples: 30
        visible: false
    }

    MouseArea {
        id: mouseArea

        anchors.fill: root

        propagateComposedEvents: true
        preventStealing: true

        hoverEnabled: true

        drag.target: root
        drag.axis: Drag.YAxis

        onPressAndHold: root.held = true
        onReleased: root.held = false

        onClicked: {
            root.clicked(mouse)

            if (Boolean(privateProperties.currentSettingsPopup)) {
                privateProperties.currentSettingsPopup.close()
            }
        }
    }

    InstrumentSettingsPopup {
        id: instrumentSettings

        y: settingsButton.y + settingsButton.height + 4
        arrowX: width - settingsButton.width / 2
    }

    StaffSettingsPopup {
        id: staffSettings

        y: settingsButton.y + settingsButton.height + 4
        arrowX: width - settingsButton.width / 2
    }

    RowLayout {
        id: rowLayout

        anchors.fill: root
        anchors.margins: 4

        spacing: 2

        FlatButton {
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth: implicitWidth

            normalStateColor: ui.theme.backgroundPrimaryColor
            hoveredStateColor: ui.theme.buttonColor
            pressedStateColor: ui.theme.accentColor

            icon: model && model.itemRole.isVisible ? IconCode.VISIBILITY_ON : IconCode.VISIBILITY_OFF
            enabled: model && model.itemRole.canChangeVisibility

            onClicked: {
                if (!model) {
                    return
                }

                model.itemRole.isVisible = !model.itemRole.isVisible
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.leftMargin: 20 * styleData.depth
            height: childrenRect.height

            FlatButton {
                id: expandButton

                anchors.left: parent.left

                normalStateColor: ui.theme.backgroundPrimaryColor
                hoveredStateColor: ui.theme.buttonColor
                pressedStateColor: ui.theme.accentColor
                icon: styleData.isExpanded ? IconCode.SMALL_ARROW_DOWN : IconCode.SMALL_ARROW_RIGHT

                visible: styleData.hasChildren

                onClicked: {
                    if (!styleData.isExpanded) {
                        attachedControl.expand(styleData.index)
                    } else {
                        attachedControl.collapse(styleData.index)
                    }
                }
            }

            StyledTextLabel {
                anchors {
                    left: expandButton.right
                    leftMargin: 8
                    right: parent.right
                    rightMargin: 8
                    verticalCenter: expandButton.verticalCenter
                }
                horizontalAlignment: Text.AlignLeft

                text: model ? model.itemRole.title : ""
                font.bold: model ? delegateType === InstrumentTreeItemType.PART && model.itemRole.isVisible : false
                font.pixelSize: 12
            }
        }

        FlatButton {
            id: settingsButton

            Layout.alignment: Qt.AlignRight
            Layout.preferredWidth: implicitWidth

            pressedStateColor: ui.theme.accentColor
            normalStateColor: ui.theme.backgroundPrimaryColor
            hoveredStateColor: ui.theme.buttonColor

            visible: model ? delegateType === InstrumentTreeItemType.INSTRUMENT ||
                             delegateType === InstrumentTreeItemType.STAFF : false

            icon: IconCode.SETTINGS_COG

            onClicked: {
                if (Boolean(privateProperties.currentSettingsPopup)) {
                    privateProperties.currentSettingsPopup.close()
                    privateProperties.currentSettingsPopup = null
                    return
                }

                var item = {}

                if (root.type == InstrumentTreeItemType.INSTRUMENT) {
                    privateProperties.currentSettingsPopup = instrumentSettings

                    item["instrumentId"] = model.itemRole.id()
                    item["instrumentName"] = model.itemRole.title
                    item["partId"] = model.itemRole.partId()
                    item["partName"] = model.itemRole.partName()
                    item["abbreviature"] = model.itemRole.abbreviature()
                } else if (root.type == InstrumentTreeItemType.STAFF) {
                    privateProperties.currentSettingsPopup = staffSettings

                    item["staffIndex"] = model.itemRole.staffIndex()
                    item["isSmall"] = model.itemRole.isSmall()
                    item["cutawayEnabled"] = model.itemRole.cutawayEnabled()
                    item["type"] = model.itemRole.staffType()
                    item["voicesVisibility"] = model.itemRole.voicesVisibility()
                }

                privateProperties.currentSettingsPopup.open()
                privateProperties.currentSettingsPopup.load(item)
            }

            Behavior on opacity {
                NumberAnimation { duration: 150 }
            }
        }
    }

    onVisibleChanged: {
        if (visible) {
            opacity = 1.0
        } else {
            opacity = 0.0
        }
    }

    Behavior on opacity {
        enabled: styleData.depth !== 0
        NumberAnimation { duration: 150 }
    }

    states: [
        State {
            when: root.held
            ParentChange {
                target: root
                parent: attachedControl.contentItem
            }

            PropertyChanges {
                target: shadow
                visible: true
            }

            PropertyChanges {
                target: root
                height: implicitHeight
                width: attachedControl.contentItem.width
            }

            AnchorChanges {
                target: root
                anchors {
                    verticalCenter: undefined
                    horizontalCenter: undefined
                }
            }
        }
    ]
}
