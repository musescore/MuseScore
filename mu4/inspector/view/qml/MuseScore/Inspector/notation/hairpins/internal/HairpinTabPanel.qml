import QtQuick 2.9
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import MuseScore.UiComponents 1.0
import "../../../common"

TabPanel {
    id: root

    property QtObject model: undefined

    implicitHeight: Math.max(styleTab.visible ? styleTab.implicitHeight : 0,
                             textTab.visible ? textTab.implicitHeight : 0) + tabBarHeight + 24
    width: parent ? parent.width : 0

    Tab {
        id: styleTab

        title: qsTr("Style")

        HairpinStyleSettingsTab {
            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            model: root.model
        }
    }

    Tab {
        id: textTab

        title: qsTr("Text")

        HairpinTextSettingsTab {
            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            model: root.model
        }
    }
}
