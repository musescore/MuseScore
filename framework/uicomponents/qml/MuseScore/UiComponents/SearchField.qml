import MuseScore.Ui 1.0

TextInputField {
    property string searchText: currentText

    onCurrentTextEdited: {
        searchText = newTextValue
    }

    width: 220

    color: ui.theme.backgroundPrimaryColor
    border.color: ui.theme.buttonColor

    hint: qsTrc("uicomponents", "Search")
    hintIcon: IconCode.SEARCH
}
