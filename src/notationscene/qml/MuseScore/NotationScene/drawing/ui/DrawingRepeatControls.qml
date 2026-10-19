// SPDX-License-Identifier: GPL-3.0-only
import QtQuick
import Muse.UiComponents

Column {
    id: root
    property var editor
    spacing: 8
    width: parent.width
    FlatButton {
        width: parent.width
        text: qsTr("Select path")
        accentButton: root.editor && root.editor.repeatPick === "path"
        onClicked: {
            root.editor.beginRepeatEdit();
            root.editor.repeatPick = "path";
        }
    }
    FlatButton {
        width: parent.width
        text: qsTr("Select source SVG")
        enabled: !!(root.editor && root.editor.repeatSession && root.editor.repeatSession.pathId)
        accentButton: root.editor && root.editor.repeatPick === "source"
        onClicked: root.editor.repeatPick = "source"
    }
    StyledTextLabel {
        width: parent.width
        wrapMode: Text.WordWrap
        text: qsTr("Copies are equally spaced along the path. Changing the count resets their individual positions.")
    }
    IncrementalPropertyControl {
        width: parent.width
        minValue: 1
        maxValue: 500
        step: 1
        decimals: 0
        currentValue: root.editor ? root.editor.repeatCount : 5
        onValueEdited: function (value) {
            root.editor.repeatCount = Math.round(value);
        }
    }
    StyledSlider {
        width: parent.width
        from: 1
        to: 500
        stepSize: 1
        value: root.editor ? root.editor.repeatCount : 5
        onMoved: root.editor.repeatCount = Math.round(value)
    }
    CheckBox {
        text: qsTr("Follow path direction")
        checked: root.editor ? root.editor.repeatFollow : true
        onClicked: root.editor.repeatFollow = !checked
    }
    StyledTextLabel {
        text: qsTr("Rotation") + ": " + (root.editor ? Math.round(root.editor.repeatRotation) : 0) + "°"
    }
    Item {
        width: parent.width
        height: 32
        Rectangle {
            x: parent.width / 2
            y: 4
            width: 1
            height: 24
            color: "#808080"
        }
        StyledSlider {
            anchors.fill: parent
            from: -180
            to: 180
            stepSize: 1
            value: root.editor ? root.editor.repeatRotation : 0
            onMoved: root.editor.repeatRotation = value
        }
    }
    StyledTextLabel {
        text: qsTr("Path opacity")
    }
    StyledSlider {
        width: parent.width
        from: 0
        to: 1
        stepSize: 0.01
        value: root.editor ? root.editor.repeatOpacity : 1
        onMoved: root.editor.repeatOpacity = value
    }
}
