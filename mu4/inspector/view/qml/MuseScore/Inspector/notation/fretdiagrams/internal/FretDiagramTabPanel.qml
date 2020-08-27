import QtQuick 2.9
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import MuseScore.UiComponents 1.0
import "../../../common"

TabPanel {
    id: root

    property QtObject model: undefined

    implicitHeight: Math.max(generalTab.visible ? generalTab.implicitHeight : 0,
                             advancedTab.visible ? advancedTab.implicitHeight : 0) + tabBarHeight + 24
    width: parent ? parent.width : 0

    Tab {
        id: generalTab

        title: qsTr("General")

        FretGeneralSettingsTab {
            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            model: root.model
        }
    }

    Tab {
        id: advancedTab

        title: qsTr("Settings")

        enabled: root.model ? root.model.areSettingsAvailable : false

        FretAdvancedSettingsTab {
            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            model: root.model
        }
    }
}
