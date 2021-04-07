import QtQuick 2.15
import MuseScore.Ui 1.0

Rectangle {

    property alias keynavSection: keynavsec.section
    property alias subsectionName: keynavsec.name
    property alias subsectionOrder: keynavsec.order

    height: 40
    width: 40
    opacity: 0.8

    border.color: "#75507b"
    border.width: keynavsec.active ? 4 : 0

    KeyNavigationSubSection {
        id: keynavsec
    }
}
