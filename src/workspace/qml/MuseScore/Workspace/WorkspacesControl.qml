import QtQuick 2.12

import MuseScore.UiComponents 1.0
import MuseScore.Workspace 1.0

Item {
    width: childrenRect.width
    height: childrenRect.height

    CurrentWorkspaceModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }

    FlatButton {
        text: qsTrc("workspace", "Workspace: ") + model.currentWorkspaceName

        normalStateColor: "transparent"

        onClicked: {
            Qt.callLater(model.selectWorkspace)
        }
    }
}
