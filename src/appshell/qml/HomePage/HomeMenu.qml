import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

RadioButtonGroup {
    id: radioButtonList

    orientation: ListView.Vertical
    spacing: 0

    signal selected(string name)

    model: [
        { "name": qsTrc("appshell", "scores"), "title": qsTrc("appshell", "Scores"), "icon": IconCode.MUSIC_NOTES },
        { "name": qsTrc("appshell", "add-ons"), "title": qsTrc("appshell", "Add-ons"), "icon":  IconCode.PLUS },
        { "name": qsTrc("appshell", "audio"), "title": qsTrc("appshell", "Audio"), "icon":  IconCode.AUDIO },
        { "name": qsTrc("appshell", "feautured"), "title": qsTrc("appshell", "Featured"), "icon":  IconCode.STAR },
        { "name": qsTrc("appshell", "learn"), "title": qsTrc("appshell", "Learn"), "icon":  IconCode.MORTAR_BOARD },
        { "name": qsTrc("appshell", "support"), "title": qsTrc("appshell", "Support"), "icon": IconCode.FEEDBACK },
    ]

    currentIndex: 0

    delegate: GradientTabButton {
        id: radioButtonDelegate

        width: parent.width

        ButtonGroup.group: radioButtonList.radioButtonGroup
        orientation: Qt.Horizontal
        checked: index === radioButtonList.currentIndex

        title: modelData["title"]

        iconComponent: StyledIconLabel {
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width * 0.3

            iconCode: modelData["icon"]
        }

        onToggled: {
            radioButtonList.currentIndex = index
            radioButtonList.selected(modelData["name"])
        }
    }
}
