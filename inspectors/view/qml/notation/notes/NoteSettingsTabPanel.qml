import QtQuick 2.9
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import "../../common"

TabPanel {
    id: root

    property QtObject proxyModel: null

    implicitHeight: Math.max(beamTab.visible ? beamTab.implicitHeight : 0,
                             headTab.visible ? headTab.implicitHeight : 0,
                             stemTab.visible ? stemTab.implicitHeight : 0) + tabBarHeight + 24
    width: parent ? parent.width : 0

    Tab {
        id: beamTab

        title: proxyModel && proxyModel.beamSettingsModel ? proxyModel.beamSettingsModel.title : ""

        BeamSettings {
            id: beamSettings

            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            model: proxyModel ? proxyModel.beamSettingsModel : null
        }
    }

    Tab {
        id: headTab

        title: proxyModel && proxyModel.headSettingsModel ? proxyModel.headSettingsModel.title : ""

        HeadSettings {
            id: headSettings

            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            model: proxyModel ? proxyModel.headSettingsModel : null
        }
    }

    Tab {
        id: stemTab

        height: implicitHeight
        width: root.width

        title: proxyModel && proxyModel.stemSettingsModel ? proxyModel.stemSettingsModel.title : ""

        StemSettings {
            id: stemSettings

            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            stemModel: proxyModel ? proxyModel.stemSettingsModel : null
            hookModel: proxyModel ? proxyModel.hookSettingsModel : null
            beamModel: proxyModel ? proxyModel.beamSettingsModel : null
        }
    }
}
