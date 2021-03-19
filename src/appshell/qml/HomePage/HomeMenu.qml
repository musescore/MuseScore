import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

RadioButtonGroup {
    id: radioButtonList

    property string currentPageName: ""

    orientation: ListView.Vertical
    spacing: 0

    signal selected(string name)

    model: [
        { "name": "scores", "title": qsTrc("appshell", "Scores"), "icon": IconCode.MUSIC_NOTES },
        { "name": "add-ons", "title": qsTrc("appshell", "Add-ons"), "icon":  IconCode.PLUS },
        { "name": "audio", "title": qsTrc("appshell", "Audio"), "icon":  IconCode.AUDIO },
        { "name": "feautured", "title": qsTrc("appshell", "Featured"), "icon":  IconCode.STAR },
        { "name": "learn", "title": qsTrc("appshell", "Learn"), "icon":  IconCode.MORTAR_BOARD },
        { "name": "support", "title": qsTrc("appshell", "Support"), "icon": IconCode.FEEDBACK },
    ]

    currentIndex: 0

    delegate: GradientTabButton {
        id: radioButtonDelegate

        width: parent.width

        ButtonGroup.group: radioButtonList.radioButtonGroup
        orientation: Qt.Horizontal
        checked: modelData["name"] === radioButtonList.currentPageName

        title: modelData["title"]

        iconComponent: StyledIconLabel {
            iconCode: modelData["icon"]
        }

        onToggled: {
            radioButtonList.currentIndex = index
            radioButtonList.selected(modelData["name"])
        }
    }
}
