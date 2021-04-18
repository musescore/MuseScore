import QtQuick 2.15

import MuseScore.UiComponents 1.0

Column {
    id: root

    property alias currentThemeIndex: view.currentIndex
    property alias themes: view.model

    signal themeChangeRequested(var newThemeIndex)

    spacing: 18

    StyledTextLabel {
        text: qsTrc("appshell", "Themes")
        font: ui.theme.bodyBoldFont
    }

    ListView {
        id: view

        width: parent.width
        height: contentHeight
        contentHeight: 120

        orientation: Qt.Horizontal
        interactive: false

        spacing: 106

        delegate: Column {
            width: 112
            height: 120

            spacing: 16

            ThemeSample {
                strokeColor: modelData.strokeColor
                backgroundPrimaryColor: modelData.backgroundPrimaryColor
                backgroundSecondaryColor: modelData.backgroundSecondaryColor
                fontPrimaryColor: modelData.fontPrimaryColor
                buttonColor: modelData.buttonColor
                accentColor: modelData.accentColor

                onClicked: {
                    root.themeChangeRequested(model.index)
                }
            }

            RoundedRadioButton {
                width: parent.width

                checked: view.currentIndex === model.index
                text: modelData.title

                onClicked: {
                    root.themeChangeRequested(model.index)
                }
            }
        }
    }
}
