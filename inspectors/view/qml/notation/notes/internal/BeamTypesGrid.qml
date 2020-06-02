import QtQuick 2.9
import MuseScore.Inspectors 3.3
import "../../../common"

FocusableItem {
    id: root

    property QtObject beamTypesModel: null
    property bool isIndeterminate: false

    function iconByBeamType(beamMode) {
        switch (beamMode) {
        case Beam.MODE_AUTO: return "qrc:/resources/icons/beams/beam_auto.svg"
        case Beam.MODE_BEGIN: return "qrc:/resources/icons/beams/beam_start.svg"
        case Beam.MODE_MID: return "qrc:/resources/icons/beams/beam_middle.svg"
        case Beam.MODE_NONE: return "qrc:/resources/icons/beams/beam_none.svg"
        case Beam.MODE_BEGIN32: return "qrc:/resources/icons/beams/beam_16_sub.svg"
        case Beam.MODE_BEGIN64: return "qrc:/resources/icons/beams/beam_32_sub.svg"

        default: return "qrc:/resources/icons/beams/beam_auto.svg"
        }
    }

    implicitHeight: gridView.height + 16
    width: parent.width

    Rectangle {
        height: root.implicitHeight
        width: parent.width

        color: "#FFFFFF"

        radius: 3
    }

    GridView {
        id: gridView

        height: contentHeight
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8

        cellHeight: 40
        cellWidth: 40

        model: beamTypesModel

        interactive: false

        delegate: FocusableItem {

            implicitHeight: gridView.cellHeight
            implicitWidth: gridView.cellWidth

            Image {
                anchors.centerIn: parent

                source: iconByBeamType(beamModeRole)
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    beamTypesModel.selectedTypeIndex = index
                }
            }
        }

        highlight: Rectangle {
            color: Qt.rgba(globalStyle.highlight.r, globalStyle.highlight.g, globalStyle.highlight.b, 0.3)
            radius: 2
        }

        currentIndex: beamTypesModel ? beamTypesModel.selectedTypeIndex : -1
    }
}
