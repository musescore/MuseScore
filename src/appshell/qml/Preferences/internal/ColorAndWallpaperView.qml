import QtQuick 2.15
import QtQuick.Layouts 1.12

import MuseScore.UiComponents 1.0

Column {
    id: root

    property alias title: titleLabel.text
    property alias color: colorPicker.color
    property alias wallpaperPath: wallpaperPicker.path

    property bool useColor: true
    property int firstColumnWidth: 0

    spacing: 18

    StyledTextLabel {
        id: titleLabel

        font: ui.theme.bodyBoldFont
    }

    GridLayout {
        rows: 2
        columns: 2

        rowSpacing: 8
        columnSpacing: 0

        RoundedRadioButton {
            implicitWidth: root.firstColumnWidth

            padding: 0
            spacing: 6

            checked: root.useColor

            StyledTextLabel {
                horizontalAlignment: Qt.AlignLeft
                text: qsTrc("preferences", "Colour:")
            }
        }

        ColorPicker {
            id: colorPicker

            width: 112
        }

        RoundedRadioButton {
            implicitWidth: root.firstColumnWidth

            padding: 0
            spacing: 6

            checked: !root.useColor

            StyledTextLabel {
                horizontalAlignment: Qt.AlignLeft
                text: qsTrc("preferences", "Wallpaper:")
            }
        }

        FilePicker {
            id: wallpaperPicker

            width: 208
        }
    }
}
