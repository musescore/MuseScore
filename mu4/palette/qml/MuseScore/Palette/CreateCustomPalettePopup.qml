import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

StyledPopup {
    id: root

    signal addCustomPaletteRequested(var paletteName)

    height: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        anchors.fill: parent
        anchors.margins: 12

        spacing: 14

        StyledTextLabel {
            text: qsTrc("palette", "Name your custom palette")
        }

        TextInputField {
            id: paletteNameField

            property string name: ""

            onCurrentTextEdited: {
                name = newTextValue
            }
        }

        Row {
            width: parent.width
            height: childrenRect.height + 20

            spacing: 4

            function close() {
                paletteNameField.clear()
                root.close()
            }

            FlatButton {
                text: qsTrc("palette", "Cancel")

                width: parent.width / 2

                onClicked: {
                    parent.close()
                }
            }

            FlatButton {
                text: qsTrc("pallette", "Create")

                width: parent.width / 2

                onClicked: {
                    root.addCustomPaletteRequested(paletteNameField.name)
                    parent.close()
                }
            }
        }
    }
}
