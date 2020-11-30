import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

Item {
    id: root

    function result() {
        var result = {
            title: generalInfo.title,
            subtitle: generalInfo.subtitle,
            composer: generalInfo.composer,
            lyricist: generalInfo.lyricist,
            copyright: generalInfo.copyright,
            keySignature: additionalInfo.keySignature,
            timeSignature: additionalInfo.timeSignature,
            timeSignatureType: additionalInfo.timeSignatureType,
            withTempo: additionalInfo.withTempo,
            tempo: additionalInfo.tempo,
            withPickupMeasure: additionalInfo.withPickupMeasure,
            pickupTimeSignature: additionalInfo.pickupTimeSignature,
            measureCount: additionalInfo.measureCount
        }

        return result
    }

    StyledTextLabel {
        id: title

        anchors.top: parent.top
        anchors.topMargin: 24
        anchors.horizontalCenter: parent.horizontalCenter

        font: ui.theme.bodyBoldFont
        text: qsTrc("userscores", "Additional Score Information")
    }

    ColumnLayout {
        anchors.top: title.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20

        spacing: 30

        AdditionalInfoView {
            id: additionalInfo

            Layout.preferredHeight: 150
            Layout.fillWidth: true
        }

        SeparatorLine { }

        GeneralInfoView {
            id: generalInfo

            Layout.fillWidth: true
        }
    }
}
