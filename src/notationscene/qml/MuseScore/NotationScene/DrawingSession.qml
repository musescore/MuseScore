// SPDX-License-Identifier: GPL-3.0-only
pragma Singleton
import QtQuick

QtObject {
    property var editor: null
    property var view: null
    property var panel: null
    // Session memory: a fresh MuseScore process starts in selection mode.
    property string lastTool: "select"
    property string lastEraserMode: "whole"
    readonly property bool modeActive: editor !== null && editor.drawingEnabled
    function setDrawingMode(active) {
        if (!editor)
            return;
        if (!active && (editor.styleEditActive || editor.repeatEditActive)) {
            editor.status = qsTr("Confirm or cancel the style edit first.");
            return;
        }
        if (!active) {
            editor.finishTextEditing();
            editor.finishPen();
            editor.selected = -1;
            editor.selectedShapes = [];
        }
        editor.drawingEnabled = active;
        if (active)
            editor.forceActiveFocus();
    }
}
