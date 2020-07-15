import QtQuick 2.7

import MuseScore.UiComponents 1.0
import MuseScore.Languages 1.0

Rectangle {
    color: ui.theme.backgroundColor

    Component.onCompleted: {
        languageListModel.load()
    }

    LanguageListModel {
        id: languageListModel
    }

    // TODO
    FlatButton {
        text: qsTrc("languages", "Update")

        onClicked: {
            languageListModel.updateList()
        }
    }

    GridView {
        id: view

        anchors.fill: parent
        anchors.topMargin: 50

        model: languageListModel

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

                        spacing: 4

                        FlatButton {
                            text: qsTrc("languages", "Install")
                            width: 60

                            visible: status === LanguageStatus.NoInstalled

                            onClicked: {
                                languageListModel.install(index)
                            }
                        }
                        FlatButton {
                            text: qsTrc("languages", "Set as language")
                            width: 60

                            visible: status === LanguageStatus.Installed && !isCurrent

                            onClicked: {
                                languageListModel.setLanguage(index)
                            }
                        }
                        FlatButton {
                            text: qsTrc("languages", "Uninstall")
                            width: 60

                            visible: status === LanguageStatus.Installed || status === LanguageStatus.NeedUpdate

                            onClicked: {
                                languageListModel.uninstall(index)
                            }
                        }
                    }
                }
            }
        }
    }
}
