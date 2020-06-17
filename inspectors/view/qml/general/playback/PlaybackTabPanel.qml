import QtQuick 2.9
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import MuseScore.UiComponents 1.0
import "../../common"

TabPanel {
    id: root

    property QtObject proxyModel: null

    implicitHeight: Math.max(generalTab.visible ? generalTab.implicitHeight : 0,
                             dynamicsTab.visible ? dynamicsTab.implicitHeight : 0) + tabBarHeight + 24
    width: parent.width

    Tab {
        id: generalTab

        height: implicitHeight
        width: root.width

        title: qsTr("General")

        GeneralSettingsTab {
            id: generalSettings

            proxyModel: root.proxyModel

            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width
        }
    }

    Tab {
        id: dynamicsTab

        height: implicitHeight
        width: root.width

        title: qsTr("Dynamics")

        DynamicsSettingsTab {
            id: dynamicsSettings

            proxyModel: root.proxyModel

            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width
        }
    }
}
