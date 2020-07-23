import QtQuick 2.7

import MuseScore.UiComponents 1.0
import MuseScore.Extensions 1.0

Rectangle {
    color: ui.theme.backgroundColor

    Component.onCompleted: {
        extensiomListModel.load()
    }

    ExtensionListModel {
        id: extensiomListModel
    }

    FlatButton {
        text: qsTrc("extensions", "Update")

        onClicked: {
            extensiomListModel.updateList()
        }
    }

    GridView {
        id: view

        anchors.fill: parent
        anchors.topMargin: 50

        model: extensiomListModel

        clip: true

        cellHeight: 150
        cellWidth: 200

        boundsBehavior: Flickable.StopAtBounds

        delegate: Item {
            height: view.cellHeight
            width: view.cellWidth

            Rectangle {

                anchors.centerIn: parent

                height: 130
                width: 180
                color: ui.theme.popupBackgroundColor

                Column {
                    anchors.fill: parent
                    spacing: 10

                    StyledTextLabel {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        text: name
                    }

                    Row {
                        anchors.left: parent.left
                        anchors.right: parent.right

                        FlatButton {
                            text: qsTrc("extensions", "Install")
                            width: 60

                            visible: status === ExtensionStatus.NoInstalled

                            onClicked: {
                                extensiomListModel.install(index)
                            }
                        }
                        FlatButton {
                            text: qsTrc("extensions", "Update")
                            width: 60

                            visible: status === ExtensionStatus.NeedUpdate

                            onClicked: {
                                extensiomListModel.update(index)
                            }
                        }
                        FlatButton {
                            text: qsTrc("extensions", "Uninstall")
                            width: 60

                            visible: status === ExtensionStatus.Installed || status === ExtensionStatus.NeedUpdate

                            onClicked: {
                                extensiomListModel.uninstall(index)
                            }
                        }
                    }
                }
            }
        }
    }
}
