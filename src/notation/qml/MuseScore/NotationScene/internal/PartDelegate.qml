import QtQuick 2.9
import QtQuick.Controls 2.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

ListItemBlank {
    id: root

    property string title: ""
    property int maxTitleWidth: 0
    property bool isMain: false
    property int currentPartIndex: -1
    property alias voicesVisibility: voicesPopup.voicesVisibility
    property alias voicesTitle: voicesLabel.text

    property int sidePadding: 0

    signal copyPartRequested()
    signal removePartRequested()
    signal voicesVisibilityChangeRequested(var voiceIndex, var voiceVisible)
    signal partClicked()

    function startEditTitle() {
        if (titleLoader.sourceComponent !== editPartTitleField) {
            titleLoader.sourceComponent = editPartTitleField
        }
    }

    function endEditTitle() {
        if (titleLoader.sourceComponent !== partTitle) {
            titleLoader.sourceComponent = partTitle
        }
    }

    height: 42

    onClicked: {
        voicesPopup.close()
        root.partClicked()
    }

    onDoubleClicked: {
        root.startEditTitle()
    }

    StyledIconLabel {
        id: partIcon

        anchors.left: parent.left
        anchors.leftMargin: root.sidePadding

        height: parent.height
        width: height

        iconCode: root.isMain ? IconCode.PAGE : IconCode.NEW_FILE
    }

    Component {
        id: partTitle

        StyledTextLabel {
            text: root.title

            horizontalAlignment: Qt.AlignLeft
            font: ui.theme.bodyBoldFont
        }
    }

    Component {
        id: editPartTitleField

        TextInputField {
            Component.onCompleted: {
                forceActiveFocus()
            }

            currentText: root.title

            onCurrentTextEdited: {
                root.title = newTextValue
            }
        }
    }

    Loader {
        id: titleLoader

        anchors.left: partIcon.right
        anchors.verticalCenter: parent.verticalCenter

        width: root.maxTitleWidth - partIcon.width

        sourceComponent: partTitle

        Connections {
            target: root

            function onCurrentPartIndexChanged(currentPartIndex) {
                root.endEditTitle()
            }
        } 
    }

    FlatButton {
        id: showVoicesPopupButton

        anchors.left: titleLoader.right
        anchors.verticalCenter: parent.verticalCenter

        normalStateColor: "transparent"
        icon: IconCode.SMALL_ARROW_DOWN

        onClicked: {
            if (voicesPopup.opened) {
                voicesPopup.close()
                return
            }

            voicesPopup.open()
        }
    }

    StyledTextLabel {
        id: voicesLabel

        anchors.left: showVoicesPopupButton.right
        anchors.leftMargin: 8
        height: parent.height

        horizontalAlignment: Qt.AlignLeft
    }

    FlatButton {
        anchors.right: parent.right
        anchors.rightMargin: root.sidePadding
        anchors.verticalCenter: parent.verticalCenter

        normalStateColor: "transparent"
        icon: IconCode.MENU_THREE_DOTS

        onClicked: {
            contextMenu.popup()
        }
    }

    VoicesPopup {
        id: voicesPopup

        x: showVoicesPopupButton.x + showVoicesPopupButton.width / 2 - width / 2
        y: showVoicesPopupButton.y + showVoicesPopupButton.height

        onVoiceVisibilityChangeRequested: {
            root.voicesVisibilityChangeRequested(voiceIndex, voiceVisible)
        }
    }

    ContextMenu {
        id: contextMenu

        StyledContextMenuItem {
            id: duplicateItem

            text: qsTrc("notation", "Duplicate")

            onTriggered: {
                root.copyPartRequested()
            }
        }

        StyledContextMenuItem {
            id: deleteItem

            text: qsTrc("notation", "Delete score")

            onTriggered: {
                root.removePartRequested()
            }
        }

        StyledContextMenuItem {
            id: renameItem

            text: qsTrc("notation", "Rename")

            onTriggered: {
                root.startEditTitle()
            }
        }

        Component.onCompleted: {
            if (root.isMain) {
                removeItem(deleteItem)
            }
        }
    }

    SeparatorLine {
        anchors.leftMargin: -root.anchors.leftMargin
        anchors.rightMargin: -root.anchors.rightMargin
        anchors.bottom: parent.bottom
    }
}
