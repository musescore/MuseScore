import QtQuick 2.0

Text {
    id: root

    color: globalStyle.buttonText

    elide: Text.ElideRight
    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter

    font {
        family: globalStyle.font.family
        pixelSize: globalStyle.font.pixelSize
    }
}
