import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

Column {
    id: root

    property alias title: titleInfo.info
    property alias subtitle: subtitleInfo.info
    property alias composer: composerInfo.info
    property alias lyricist: lyricistInfo.info
    property alias copyright: copyrightInfo.info

    spacing: 20

    Row {
        anchors.left: parent.left
        anchors.right: parent.right

        property real childWidth: (width / 2) - 15
        height: 60

        spacing: 20

        GeneralInfoItem {
            id: titleInfo

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.childWidth

            title: qsTrc("userscores", "Title")
        }
        GeneralInfoItem {
            id: composerInfo

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.childWidth

            title: qsTrc("userscores", "Composer")
        }
    }

    Row {
        anchors.left: parent.left
        anchors.right: parent.right

        property real childWidth: (width / 3) - 15
        height: 60

        spacing: 20

        GeneralInfoItem {
            id: subtitleInfo

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.childWidth

            title: qsTrc("userscores", "Subtitle")
        }

        GeneralInfoItem {
            id: lyricistInfo

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.childWidth

            title: qsTrc("userscores", "Lyricist")
        }

        GeneralInfoItem {
            id: copyrightInfo

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.childWidth

            title: qsTrc("userscores", "Copyright")
        }
    }
}
