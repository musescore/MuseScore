import QtQuick 2.7
import QtQuick.Controls 2.0
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    width: radioButtonList.contentWidth
    height: radioButtonList.contentHeight

    property alias keynav: keynavSub

    property var currentUri: "musescore://home"
    property var items: [
        {
            title: qsTrc("appshell", "Home"),
            uri: "musescore://home"
        },
        {
            title: qsTrc("appshell", "Score"),
            uri: "musescore://notation"
        },
        {
            title: qsTrc("appshell", "Publish"),
            uri: "musescore://publish"
        },
        {
            title: qsTrc("appshell", "DevTools"),
            uri: "musescore://devtools"
        }
    ]

    signal selected(string uri)

    function select(uri) {
        root.selected(uri)
    }

    KeyNavigationSubSection {
        id: keynavSub
        name: "MainToolBar"
    }

    RadioButtonGroup {
        id: radioButtonList

        spacing: 0

        model: root.items

        delegate: GradientTabButton {
            id: radioButtonDelegate

            ButtonGroup.group: radioButtonList.radioButtonGroup

            spacing: 0
            leftPadding: 12

            keynav.name: modelData["title"]
            keynav.subsection: keynavSub
            keynav.order: model.index

            checked: modelData["uri"] === root.currentUri
            title: modelData["title"]

            onToggled: {
                root.selected(modelData["uri"])
            }
        }
    }
}
