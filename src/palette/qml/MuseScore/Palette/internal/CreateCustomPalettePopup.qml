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

        width: parent.width

        spacing: 14

        StyledTextLabel {
            text: qsTrc("palette", "Name your custom palette")
        }

        TextInputField {
            id: paletteNameField

            width: parent.width

            property string name: ""

            onCurrentTextEdited: {
                name = newTextValue
            }
        }

        Row {
            width: parent.width
            height: childrenRect.height

            spacing: 4

            function close() {
                paletteNameField.clear()
                root.close()
            }

            FlatButton {
                text: qsTrc("global", "Cancel")

                width: parent.width / 2

                onClicked: {
                    parent.close()
                }
            }

            FlatButton {
                text: qsTrc("pallette", "Create")

                width: parent.width / 2

                enabled: Boolean(paletteNameField.name)

                onClicked: {
                    root.addCustomPaletteRequested(paletteNameField.name)
                    parent.close()
                }
            }
        }
    }
}
