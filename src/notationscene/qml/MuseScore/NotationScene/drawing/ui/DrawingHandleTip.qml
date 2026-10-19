// SPDX-License-Identifier: GPL-3.0-only
import QtQuick
import QtQuick.Window
import Muse.Ui
import Muse.UiComponents

Window {
    id: root
    property Item anchorItem: null
    property string title: ""
    visible: false
    color: "transparent"
    flags: Qt.ToolTip | Qt.WindowTransparentForInput | Qt.WindowDoesNotAcceptFocus
    transientParent: anchorItem ? anchorItem.Window.window : null
    width: Math.min(320, label.implicitWidth + 20)
    height: label.implicitHeight + 16
    function schedule(item, text) {
        hideTip();
        anchorItem = item;
        title = text;
        delay.restart();
    }
    function hideTip() {
        delay.stop();
        visible = false;
    }
    Timer {
        id: delay
        // Matches the pinned MuseScore UiConfiguration::tooltipDelay().
        interval: 500
        onTriggered: {
            if (!root.anchorItem)
                return;
            var point = root.anchorItem.mapToGlobal(0, root.anchorItem.height), win = root.anchorItem.Window.window;
            root.x = Math.max(root.Screen.virtualX, Math.min(point.x, root.Screen.virtualX + root.Screen.desktopAvailableWidth - root.width));
            root.y = point.y + 8;
            if (root.y + root.height > root.Screen.virtualY + root.Screen.desktopAvailableHeight)
                root.y = point.y - root.anchorItem.height - root.height - 8;
            root.y = Math.max(root.Screen.virtualY, root.y);
            root.visible = true;
        }
    }
    Rectangle {
        anchors.fill: parent
        color: ui.theme.popupBackgroundColor
        radius: 4
        border.width: 1
        border.color: ui.theme.strokeColor
        StyledTextLabel {
            id: label
            anchors.centerIn: parent
            width: Math.min(300, implicitWidth)
            text: root.title
            wrapMode: Text.WordWrap
            font: ui.theme.bodyBoldFont
        }
    }
}
