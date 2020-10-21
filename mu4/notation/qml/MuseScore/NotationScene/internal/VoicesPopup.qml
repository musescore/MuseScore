import QtQuick 2.9
import QtQuick.Controls 2.2

import MuseScore.UiComponents 1.0

StyledPopup {
    id: root

    property var voicesVisibility: [ true, true, true, true ]
    property int visibleVoicesCount: voicesVisibility.length

    property string visibleVoicesTitle: {
        if (visibleVoicesCount === voicesVisibility.length) {
            return qsTrc("notation", "All")
        } else if (visibleVoicesCount === 0) {
            return qsTrc("notation", "None")
        }

        var visibleVoicesIndexes = []
        for (var i = 0; i < voicesVisibility.length; ++i) {
            if (voicesVisibility[i]) {
                visibleVoicesIndexes.push(i + 1)
            }
        }

        return visibleVoicesIndexes.join(", ")
    }

    height: contentColumn.implicitHeight + bottomPadding + topPadding
    width: contentColumn.implicitWidth + leftPadding + rightPadding

    Column {
        id: contentColumn

        spacing: 18

        StyledTextLabel {
            text: qsTrc("notation", "Voices visible on this score")
        }

        ListView {
            spacing: 8

            height: contentHeight
            width: parent.width

            model: root.voicesVisibility

            delegate: CheckBox {
                checked: modelData
                text: qsTrc("notation", "Voice ") + (model.index + 1)

                onClicked: {
                    checked = !checked
                    root.voicesVisibility[model.index] = checked
                    root.visibleVoicesCount += checked ? 1 : -1
                }
            }
        }
    }
}
