// SPDX-License-Identifier: GPL-3.0-only
import QtQuick
import Muse.Ui
import Muse.UiComponents

// A window-space overlay: never inherits the score's zoom or PopupView's parent-height offset.
Item {
    id: root
    property bool finishing: false
    property var editor
    signal finishRequested(bool apply)
    visible: !!editor && editor.drawingEnabled && editor.editSessionActive && !editor.repeatEditActive
    z: 10000
    width: buttons.width + 16
    height: buttons.height + 12
    readonly property rect selection: {
        if (!editor || editor.selected < 0)
            return Qt.rect(8, 60, 1, 1);
        var dependency = editor.paintRegion, scale = editor.viewScale;
        var box = editor.selectionBounds();
        var a = editor.mapToItem(parent, box.x, box.y), b = editor.mapToItem(parent, box.x + box.width, box.y + box.height);
        return Qt.rect(Math.min(a.x, b.x), Math.min(a.y, b.y), Math.abs(b.x - a.x), Math.abs(b.y - a.y));
    }
    x: Math.max(8, Math.min(selection.x + selection.width / 2 - width / 2, parent.width - width - 8))
    y: Math.max(8, selection.y - height - 8)
    function finish(apply) {
        finishRequested(apply);
    }
    Rectangle {
        anchors.fill: parent
        color: ui.theme.popupBackgroundColor
        border.color: ui.theme.strokeColor
        radius: 4
        // Prevent clicks on the small toolbar background reaching the score.
        MouseArea {
            anchors.fill: parent
        }
    }
    Row {
        id: buttons
        x: 8
        y: 6
        spacing: 8
        FlatButton {
            icon: IconCode.TICK_RIGHT_ANGLE
            toolTipTitle: qsTrc("drawing-tools", "Confirm (Enter)")
            onClicked: root.finish(true)
        }
        FlatButton {
            icon: IconCode.CROSS
            toolTipTitle: qsTrc("drawing-tools", "Cancel (Esc)")
            onClicked: root.finish(false)
        }
        FlatButton {
            visible: !!root.editor && root.editor.shapeEditActive
            icon: IconCode.SETTINGS_COG
            toolTipTitle: qsTrc("drawing-tools", "Shape adjustment")
            onClicked: parameters.open()
        }
    }
    StyledPopupView {
        id: parameters
        parent: root
        contentWidth: 290
        contentHeight: shapeControls.implicitHeight
        ShapeControls {
            id: shapeControls
            width: parent.width
            editor: root.editor
        }
    }
    onVisibleChanged: if (!visible)
        parameters.close()
    Shortcut {
        sequence: "Return"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: root.finish(true)
    }
    Shortcut {
        sequence: "Enter"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: root.finish(true)
    }
    Shortcut {
        sequence: "Escape"
        context: Qt.ApplicationShortcut
        enabled: root.visible
        onActivated: root.finish(false)
    }
}
