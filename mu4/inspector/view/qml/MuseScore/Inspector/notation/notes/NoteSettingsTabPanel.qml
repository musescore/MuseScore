import QtQuick 2.9
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
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

        property QtObject beamModel: proxyModel ? proxyModel.modelByType(Inspector.TYPE_BEAM) : null

        title: beamModel ? beamModel.title : ""

        BeamSettings {
            id: beamSettings

            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            model: beamModel
        }
    }

    Tab {
        id: headTab

        property QtObject headModel: proxyModel ? proxyModel.modelByType(Inspector.TYPE_NOTEHEAD) : null

        title: headModel ? headModel.title : ""

        HeadSettings {
            id: headSettings

            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            model: headModel
        }
    }

    Tab {
        id: stemTab

        property QtObject stemModel: proxyModel ? proxyModel.modelByType(Inspector.TYPE_STEM) : null
        property QtObject hookModel: proxyModel ? proxyModel.modelByType(Inspector.TYPE_HOOK) : null
        property QtObject beamModel: proxyModel ? proxyModel.modelByType(Inspector.TYPE_BEAM) : null

        height: implicitHeight
        width: root.width

        title: stemModel ? stemModel.title : ""

        StemSettings {
            id: stemSettings

            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            stemModel: stemTab.stemModel
            hookModel: stemTab.hookModel
            beamModel: stemTab.beamModel
        }
    }
}
