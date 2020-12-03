import QtQuick 2.9

import MuseScore.Ui 1.0

TextInputField {
    id: root

    property string searchText: currentText

    onCurrentTextEdited: {
        searchText = newTextValue
    }

    width: 220
    opacity: 0.8

    hint: qsTrc("uicomponents", "Search")
    hintIcon: IconCode.SEARCH

    clearTextButtonVisible: Boolean(searchText)
}
