import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

Row {
    id: root

    property alias keySignature: infoModel.keySignature
    property alias timeSignature: infoModel.timeSignature
    property alias timeSignatureType: infoModel.timeSignatureType

    property alias withTempo: infoModel.withTempo
    property alias tempo: infoModel.tempo

    property alias withPickupMeasure: infoModel.withPickupMeasure
    property alias pickupTimeSignature: infoModel.pickupTimeSignature
    property alias measureCount: infoModel.measureCount

    spacing: 20

    QtObject {
        id: privatesProperties

        property real contentWidth: (width / 4) - 15
        readonly property real buttonHeight: 120
    }

    Component.onCompleted: {
        infoModel.init()
    }

    AdditionalInfoModel {
        id: infoModel
    }

    Column {
        id: keySignatureСolumn

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: privatesProperties.contentWidth

        spacing: 10

        StyledTextLabel {
            anchors.left: parent.left
            anchors.right: parent.right

            font.bold: true
            horizontalAlignment: Text.AlignLeft
            text: qsTrc("userscores", "Key Signature")
        }

        KeySignatureSettings {
            anchors.left: parent.left
            anchors.right: parent.right

            height: privatesProperties.buttonHeight

            model: infoModel
            arrowX: Boolean(oppened) ? popupPositionX + (width / 2) : 0
            popupPositionX: Boolean(oppened) ? mapToGlobal(root.x, root.y).x - mapToGlobal(x, y).x : 0
        }
    }

    Column {
        id: timeSignatureСolumn
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: privatesProperties.contentWidth

        spacing: 10

        StyledTextLabel {
            anchors.left: parent.left
            anchors.right: parent.right

            font.bold: true
            horizontalAlignment: Text.AlignLeft
            text: qsTrc("userscores", "Time Signature")
        }

        TimeSignatureSettings {
            anchors.left: parent.left
            anchors.right: parent.right

            height: privatesProperties.buttonHeight

            model: infoModel
            arrowX: Boolean(oppened) ? popupPositionX + (width / 2) : 0
            popupPositionX: Boolean(oppened) ? mapToGlobal(root.x, root.y).x - mapToGlobal(x, y).x : 0
        }
    }

    Column {
        id: tempoСolumn
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: privatesProperties.contentWidth

        spacing: 10

        StyledTextLabel {
            anchors.left: parent.left
            anchors.right: parent.right

            font.bold: true
            horizontalAlignment: Text.AlignLeft
            text: qsTrc("userscores", "Tempo")
        }

        TempoSettings {
            anchors.left: parent.left
            anchors.right: parent.right

            height: privatesProperties.buttonHeight

            model: infoModel
            arrowX: Boolean(oppened) ? popupPositionX + (width / 2) : 0
            popupPositionX: Boolean(oppened) ? mapToGlobal(root.x, root.y).x - mapToGlobal(x, y).x : 0
        }
    }

    Column {
        id: measuresColumn
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: privatesProperties.contentWidth

        spacing: 10

        StyledTextLabel {
            anchors.left: parent.left
            anchors.right: parent.right

            font.bold: true
            horizontalAlignment: Text.AlignLeft
            text: qsTrc("userscores", "Measures")
        }

        MeasuresSettings {
            anchors.left: parent.left
            anchors.right: parent.right

            height: privatesProperties.buttonHeight

            model: infoModel
            arrowX: Boolean(oppened) ? popupPositionX + (width / 2) : 0
            popupPositionX: Boolean(oppened) ? mapToGlobal(root.x, root.y).x - mapToGlobal(x, y).x : 0
        }
    }
}
