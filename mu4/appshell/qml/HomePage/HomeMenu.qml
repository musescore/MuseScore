import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Rectangle {
    id: root

    signal selected(string name)

    color: ui.theme.backgroundColor

    RadioButtonGroup {
        id: radioButtonList

        anchors.fill: parent

        orientation: ListView.Vertical
        spacing: 0

        model: [
            { "name": "scores", "title": "Scores", "icon": IconCode.MUSIC_NOTES },
            { "name": "extensions", "title": "Add-ons", "icon":  IconCode.LIST_ADD },
            { "name": "audio", "title": "Audio & VST", "icon":  IconCode.AUDIO },
            { "name": "feautured", "title": "Featured", "icon":  IconCode.STAR },
            { "name": "learn", "title": "Learn", "icon":  IconCode.GRADUATION_CAP },
            { "name": "support", "title": "Support", "icon": IconCode.FEEDBACK },
            { "name": "account", "title": "Account", "icon":  IconCode.ACCOUNT }
        ]

        currentIndex: 0

        delegate: FlatRadioButton {
            id: radioButtonDelegate

            ButtonGroup.group: radioButtonList.radioButtonGroup

            height: 56
            width: parent.width

            checked: index === radioButtonList.currentIndex
            onToggled: {
                radioButtonList.currentIndex = index
                root.selected(modelData["name"])
            }

            RowLayout {
                id: contentRow

                anchors.fill: parent

                StyledIconLabel {
                    Layout.fillWidth: true
                    Layout.preferredWidth: parent.width * 0.30

                    iconCode: modelData["icon"]
                }

                StyledTextLabel {
                    id: label

                    Layout.fillWidth: true
                    Layout.preferredWidth: parent.width * 0.70

                    text: modelData["title"]

                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                }
            }
        }
    }
}
