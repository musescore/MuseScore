import QtQuick 2
import QtQuick.Layouts 1

import MuseScore.UiComponents 1

RowLayout {
    id: root

    property alias text: label.text
    property int firstColumnWidth

    spacing: 12

    StyledTextLabel {
        id: label
        Layout.preferredWidth: root.firstColumnWidth
        horizontalAlignment: Text.AlignLeft
    }
}
