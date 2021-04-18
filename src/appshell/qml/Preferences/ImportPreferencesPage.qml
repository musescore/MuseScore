import QtQuick 2.15

import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

import "internal"

PreferencesPage {
    id: root

    contentHeight: content.height

    Component.onCompleted: {
        importPreferencesModel.load()
    }

    ImportPreferencesModel {
        id: importPreferencesModel
    }

    Column {
        id: content

        width: parent.width
        height: childrenRect.height

        spacing: 24

        ImportStyleSection {
            anchors.left: parent.left
            anchors.right: parent.right

            preferencesModel: importPreferencesModel
        }

        SeparatorLine { }

        CharsetsSection {
            anchors.left: parent.left
            anchors.right: parent.right

            preferencesModel: importPreferencesModel
        }

        SeparatorLine { }

        MusicXmlSection {
            anchors.left: parent.left
            anchors.right: parent.right

            preferencesModel: importPreferencesModel
        }

        SeparatorLine { }

        MidiSection {
            anchors.left: parent.left
            anchors.right: parent.right

            preferencesModel: importPreferencesModel
        }
    }
}
