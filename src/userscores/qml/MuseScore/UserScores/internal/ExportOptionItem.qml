import QtQuick 2.0
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0

RowLayout {
    property alias text: label.text
    property int firstColumnWidth
    spacing: 12

    StyledTextLabel {
        id: label
        Layout.preferredWidth: firstColumnWidth
        horizontalAlignment: Text.AlignLeft
    }
}
