import QtQuick 2.9
import MuseScore.Inspectors 3.3

import "../../common"
import "internal"

StyledPopup {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        BracketPopupSection {
             intBracketProperty: model ? model.bracketColumnPosition : nul
             title: qsTr("Column")
        }

        BracketPopupSection {
             intBracketProperty: model ? model.bracketSpanStaves : nul
             title: qsTr("Span")
        }
    }
}
