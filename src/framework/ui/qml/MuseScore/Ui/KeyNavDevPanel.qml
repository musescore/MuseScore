import QtQuick 2.15
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Rectangle {

    id: root

    color: ui.theme.backgroundPrimaryColor

    Component.onCompleted: {
        keynavModel.reload()
    }

    KeyNavDevModel {
        id: keynavModel
    }

    Row {
        id: tools
        anchors.left: parent.left
        anchors.right: parent.right
        height: 48

        FlatButton {
            anchors.verticalCenter: parent.verticalCenter
            text: "Refresh"
            onClicked: keynavModel.reload()
        }
    }

    function formatIndex(idx) {
        if (idx.row === -1) {
            return "order: " + idx.column
        }
        return "index: [" + idx.row + "," + idx.column + "]"
    }

    ListView {
        anchors.top: tools.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        clip: true
        spacing: 8

        model: keynavModel.sections
        delegate: Item {

            id: item

            property var section: modelData

            width: parent ? parent.width : 0
            height: 48 + subView.height

            Rectangle {
                anchors.fill: parent
                color: item.section.enabled ? "#fcaf3e" : "#eeeeec"
                border.width: item.section.active ? 2 : 0
                border.color: ui.theme.focusColor
            }

            StyledTextLabel {
                id: secLabel
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 8
                text: "section: " + item.section.name + ", " + root.formatIndex(item.section.index) + ", enabled: " + item.section.enabled
            }

            ListView {
                id: subView
                anchors.top: secLabel.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 16
                height: contentHeight
                clip: true
                interactive: false
                spacing: 8

                model: item.section.subsections
                delegate: Item {

                    id: subitem

                    property var subsection: modelData

                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 48 + ctrlView.height

                    Rectangle {
                        anchors.fill: parent
                        color: subitem.subsection.enabled ? "#729fcf" : "#eeeeec"
                        border.width: subitem.subsection.active ? 2 : 0
                        border.color: ui.theme.focusColor
                    }

                    StyledTextLabel {
                        id: subLabel
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.margins: 8
                        text: "subsection: " + subitem.subsection.name
                              + ", " + root.formatIndex(subitem.subsection.index)
                              + ", enabled: " + subitem.subsection.enabled
                    }

                    GridView {
                        id: ctrlView
                        anchors.top: subLabel.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 16
                        height: contentHeight
                        clip: true
                        interactive: false

                        cellHeight: 32
                        cellWidth: 68

                        model: subitem.subsection.controls
                        delegate: Rectangle {

                            property var control: modelData

                            height: 28
                            width: 64
                            radius: 4
                            color: control.enabled ? "#73d216" : "#eeeeec"
                            enabled: control.enabled
                            border.width: control.active ? 2 : 1
                            border.color: control.active ? ui.theme.focusColor : "#2e3436"

                            StyledTextLabel {
                                anchors.top: parent.top
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.margins: 8
                                text: control.name
                            }

                            MouseArea {
                                anchors.fill: parent

                                hoverEnabled: true
                                onContainsMouseChanged: {
                                    if (containsMouse) {
                                        var info = control.name + "\n"
                                                + root.formatIndex(control.index) + "\n"
                                                + "enabled: " + control.enabled

                                        ui.tooltip.show(this, info)
                                    } else {
                                        ui.tooltip.hide(this)
                                    }
                                }

                                 onClicked: {
                                     control.forceActive()
                                     control.trigger()
                                 }
                            }
                        }
                    }
                }
            }
        }
    }
}
