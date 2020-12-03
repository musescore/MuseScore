import QtQuick 2.7
import MuseScore.Shortcuts 1.0

QtObject {

    id: root

    Component.onCompleted: {
        shortcutsModel.load()
    }

    property var objects: []

    property ShortcutsInstanceModel model: ShortcutsInstanceModel {
        id: shortcutsModel

        onShortcutsChanged: {
            root.objects = [];
            for (var i = 0; i < shortcutsModel.shortcuts.length; ++i) {
                var sh = shortcutsModel.shortcuts[i]
                var obj = shortcutComponent.createObject(root, {sequence: sh})
                root.objects.push(obj)
            }
        }
    }

    property Component component: Component {
        id: shortcutComponent
        Shortcut {
            context: Qt.ApplicationShortcut
            onActivated: shortcutsModel.activate(sequence)
        }
    }
}
