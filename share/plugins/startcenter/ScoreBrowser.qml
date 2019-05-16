import QtQuick 2.12
import QtQuick.Controls 2.5
import "style.js" as Style

GridView {
    id: scoreBrowser
    clip: true

    signal scoreClicked(url url)

    // values are taken from mscore/scoreBrowser.h
    cellWidth:  140 + Style.layoutMargin
    cellHeight: 228

    delegate: ItemDelegate {
        background: null
        contentItem: Column {
//             property bool isCurrentItem: GridView.isCurrentItem
            Image {
                asynchronous: true
                width: scoreBrowser.cellWidth - Style.layoutMargin
                height: scoreBrowser.cellHeight - 30 // TODO: from scoreBrowser.cpp
                source: "image://score-thumbnail/" + modelData
            }

            Text {
                function basename(str) {
                    return str.slice(str.lastIndexOf('/') + 1, str.lastIndexOf('.'));
                }

                text: basename(modelData).replace(/_/g, ' ').trim()
                width: scoreBrowser.cellWidth
                wrapMode: Text.Wrap
                maximumLineCount: 2
                horizontalAlignment: Text.AlignHCenter
                color: Style.textColor
//                 color: parent.isCurrentItem ? Style.textColor : "green"
            }
        }

        onClicked: scoreBrowser.scoreClicked(modelData)
    }
}
