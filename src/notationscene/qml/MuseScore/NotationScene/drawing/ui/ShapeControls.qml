// SPDX-License-Identifier: GPL-3.0-only
import QtQuick
import Muse.UiComponents

Column {
    id: root
    property var editor
    readonly property string kind: editor ? (editor.shapeEditActive ? editor.shapes[editor.selected].kind : editor.tool) : ""
    width: parent.width
    spacing: 8
    function setOption(key, value) {
        if (editor)
            editor.setShapeParameter(key, value);
    }
    StyledTextLabel {
        visible: ["polygon", "star"].indexOf(root.kind) >= 0
        text: qsTrc("drawing-tools", "Number of nodes / sides")
    }
    IncrementalPropertyControl {
        width: parent.width
        visible: ["polygon", "star"].indexOf(root.kind) >= 0
        minValue: 3
        maxValue: 24
        step: 1
        currentValue: root.editor ? root.editor.shapeParameters.sides : 5
        onValueEdited: function (v) {
            root.setOption("sides", Math.round(v));
        }
    }
    StyledTextLabel {
        visible: root.kind === "star"
        text: qsTrc("drawing-tools", "Inner radius")
    }
    StyledSlider {
        width: parent.width
        visible: root.kind === "star"
        from: 0.05
        to: 0.95
        stepSize: 0.01
        value: root.editor ? root.editor.shapeParameters.innerRatio : 0.4
        onMoved: root.setOption("innerRatio", value)
    }
    StyledTextLabel {
        visible: root.kind === "rounded-rectangle"
        text: qsTrc("drawing-tools", "Corner radius")
    }
    StyledSlider {
        width: parent.width
        visible: root.kind === "rounded-rectangle"
        from: 0
        to: 0.5
        stepSize: 0.01
        value: root.editor ? root.editor.shapeParameters.roundness : 0.2
        onMoved: root.setOption("roundness", value)
    }
    Column {
        visible: root.kind === "wave"
        width: parent.width
        spacing: 8
        StyledTextLabel {
            text: qsTrc("drawing-tools", "Amplitude")
        }
        IncrementalPropertyControl {
            width: parent.width
            minValue: 0
            maxValue: 1000
            step: 1
            currentValue: root.editor ? root.editor.shapeParameters.amplitude : 20
            onValueEdited: function (v) {
                root.setOption("amplitude", v);
            }
        }
        StyledTextLabel {
            text: qsTrc("drawing-tools", "Wavelength")
        }
        IncrementalPropertyControl {
            width: parent.width
            minValue: 2
            maxValue: 2000
            step: 1
            currentValue: root.editor ? root.editor.shapeParameters.period : 40
            onValueEdited: function (v) {
                root.setOption("period", v);
            }
        }
        StyledDropdown {
            width: parent.width
            model: [
                {
                    text: qsTrc("drawing-tools", "Sine wave"),
                    value: "sine"
                },
                {
                    text: qsTrc("drawing-tools", "Zigzag"),
                    value: "zigzag"
                }
            ]
            onActivated: function (index, value) {
                root.setOption("waveform", value);
            }
        }
        StyledDropdown {
            width: parent.width
            model: [
                {
                    text: qsTrc("drawing-tools", "Continue the pattern"),
                    value: "continue"
                },
                {
                    text: qsTrc("drawing-tools", "Deform with the layout"),
                    value: "deform"
                }
            ]
            onActivated: function (index, value) {
                root.setOption("layoutMode", value);
            }
        }
        StyledTextLabel {
            width: parent.width
            wrapMode: Text.WordWrap
            text: qsTrc("drawing-tools", "Purple handles adjust the envelope; green handles adjust the wavelength.")
        }
    }
}
