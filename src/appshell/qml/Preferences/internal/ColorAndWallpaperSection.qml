import QtQuick 2.15
import QtQuick.Layouts 1.12

import MuseScore.UiComponents 1.0

Column {
    id: root

    property alias title: titleLabel.text
    property alias wallpaperDialogTitle: wallpaperPicker.dialogTitle

    property bool useColor: true
    property alias color: colorPicker.color

    property alias wallpaperPath: wallpaperPicker.path
    property alias wallpapersDir: wallpaperPicker.dir
    property alias wallpaperFilter: wallpaperPicker.filter

    property int firstColumnWidth: 0

    signal useColorChangeRequested(var newValue)
    signal colorChangeRequested(var newColor)
    signal wallpaperPathChangeRequested(var newWallpaperPath)

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

            onClicked: {
                root.useColorChangeRequested(true)
            }

            StyledTextLabel {
                horizontalAlignment: Qt.AlignLeft
                text: qsTrc("appshell", "Colour:")
            }
        }

        ColorPicker {
            id: colorPicker

            width: 112

            enabled: root.useColor

            onNewColorSelected: {
                root.colorChangeRequested(newColor)
            }
        }

        RoundedRadioButton {
            implicitWidth: root.firstColumnWidth

            padding: 0
            spacing: 6

            checked: !root.useColor

            onClicked: {
                root.useColorChangeRequested(false)
            }

            StyledTextLabel {
                horizontalAlignment: Qt.AlignLeft
                text: qsTrc("appshell", "Wallpaper:")
            }
        }

        FilePicker {
            id: wallpaperPicker

            width: 208

            enabled: !root.useColor

            onPathEdited: {
                root.wallpaperPathChangeRequested(newPath)
            }
        }
    }
}
