// SPDX-License-Identifier: GPL-3.0-only
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import Muse.UiComponents
import Muse.Ui
import MuseScore.NotationScene
import "drawing/ui"

Item {
    id: root
    property var navigationSection: null
    property var editor: null
    property string group: ""
    property string helpText: ""
    property bool panelOpen: false
    property bool sliderPointerPreview: false
    readonly property bool pointerPreview: sliderPointerPreview || (panelOpen && (group === "pencil" || group === "pen"))
    property bool eraserPreview: false
    property bool sliderBusy: false
    readonly property bool erase: group === "eraser"
    readonly property bool stroke: ["pencil", "pen", "shapes"].indexOf(group) >= 0 || !!(editor && editor.styleEditActive)
    readonly property bool music: ["select", "pencil", "pen", "shapes", "nodes"].indexOf(group) >= 0 && editor && editor.tool !== "unlink"
    implicitHeight: content.implicitHeight
    function tr(s) {
        return qsTrc("drawing-tools", s);
    }
    function adjusting(active, preview, eraser) {
        sliderBusy = active;
        sliderPointerPreview = active && preview;
        eraserPreview = eraser;
        if (active && editor && (group === "nodes" || group === "select"))
            editor.remember(true);
        if (!active && editor && !editor.textEditing)
            editor.forceActiveFocus();
    }
    Column {
        id: content
        ShapeControls {
            width: parent.width
            editor: root.editor
            visible: root.group === "shapes"
        }
        DrawingObjects {
            width: parent.width
            editor: root.editor
            visible: root.group === "objects"
        }
        width: parent.width
        spacing: 12
        Column {
            width: parent.width
            spacing: 6
            visible: root.group === "pencil" || root.group === "pen"
            StyledTextLabel {
                text: root.tr("Nib shape")
                horizontalAlignment: Text.AlignLeft
            }
            Row {
                width: parent.width
                spacing: 6
                FlatButton {
                    width: (parent.width - parent.spacing) / 2
                    text: root.tr("Round nib")
                    accentButton: root.editor && root.editor.nibShape === "round"
                    onClicked: if (root.editor)
                        root.editor.nibShape = "round"
                }
                FlatButton {
                    width: (parent.width - parent.spacing) / 2
                    text: root.tr("Square nib")
                    accentButton: root.editor && root.editor.nibShape === "square"
                    onClicked: if (root.editor)
                        root.editor.nibShape = "square"
                }
            }
            Column {
                width: parent.width
                spacing: 6
                visible: !!root.editor && root.editor.nibShape === "square"
                StyledTextLabel {
                    text: root.tr("Nib inclination") + ": " + Math.round(root.editor ? root.editor.nibTilt : 0) + "°"
                }
                StyledSlider {
                    width: parent.width
                    from: -60
                    to: 60
                    stepSize: 1
                    value: root.editor ? root.editor.nibTilt : 0
                    onMoved: if (root.editor)
                        root.editor.nibTilt = value
                }
            }
            StyledTextLabel {
                text: root.tr("Flattening") + ": " + (root.editor ? Math.round(root.editor.nibFlatten * 100) : 0) + "%"
            }
            StyledSlider {
                width: parent.width
                from: 0
                to: 0.95
                stepSize: 0.01
                value: root.editor ? root.editor.nibFlatten : 0
                onMoved: if (root.editor)
                    root.editor.nibFlatten = value
            }
            StyledTextLabel {
                text: root.tr("Nib angle") + ": " + (root.editor ? Math.round(root.editor.nibAngle) : 0) + "°"
            }
            StyledSlider {
                width: parent.width
                from: -180
                to: 180
                stepSize: 1
                value: root.editor ? root.editor.nibAngle : 0
                onMoved: if (root.editor)
                    root.editor.nibAngle = value
            }
        }
        Column {
            width: parent.width
            spacing: 8
            visible: root.group === "paint"
            StyledTextLabel {
                horizontalAlignment: Text.AlignLeft
                width: parent.width
                wrapMode: Text.WordWrap
                text: root.tr("Use the primary colour for painting. Gradients start with the primary and secondary colours.")
            }
            ColorPicker {
                visible: !root.editor || root.editor.tool.indexOf("gradient-") !== 0

                color: root.editor ? root.editor.strokeColor : "#161616"
                onNewColorSelected: function (colour) {
                    if (root.editor)
                        root.editor.strokeColor = colour.toString();
                }
            }
            FlatButton {
                width: parent.width
                text: root.tr("Apply paint to selection")
                enabled: !!(root.editor && root.editor.selectedShapes.length)
                onClicked: root.editor.paintSelection()
            }
            Column {
                width: parent.width
                spacing: 8
                visible: !!(root.editor && root.editor.currentGradient)
                FlatButton {
                    width: parent.width
                    text: root.tr("Add gradient colour")
                    onClicked: root.editor.addGradientStop()
                }
                FlatButton {
                    width: parent.width
                    text: root.tr("Remove gradient colour")
                    enabled: !!(root.editor && root.editor.currentGradient && root.editor.currentGradient.stops.length > 2)
                    onClicked: root.editor.removeGradientStop()
                }
            }
        }
        Column {
            width: parent.width
            spacing: 8
            visible: root.group === "colors"
            StyledTextLabel {
                horizontalAlignment: Text.AlignLeft
                text: root.tr("Primary colour")
            }
            ColorPicker {
                color: root.editor ? root.editor.primaryColor : "#161616"
                onNewColorSelected: function (color) {
                    if (root.editor) {
                        root.editor.selectPrimaryColor();
                        root.editor.strokeColor = color.toString();
                        root.editor.fillColor = color.toString();
                    }
                }
            }
            StyledTextLabel {
                horizontalAlignment: Text.AlignLeft
                text: root.tr("Secondary colour")
            }
            ColorPicker {
                color: root.editor ? root.editor.alternateColor : "#ffffff"
                onNewColorSelected: function (color) {
                    if (root.editor) {
                        root.editor.selectSecondaryColor();
                        root.editor.strokeColor = color.toString();
                        root.editor.fillColor = color.toString();
                    }
                }
            }
            FlatButton {
                width: parent.width
                text: root.tr("Swap colours") + " (X)"
                onClicked: root.editor.swapColors()
            }
            StyledTextLabel {
                horizontalAlignment: Text.AlignLeft
                text: root.tr("Opacity")
            }
            StyledSlider {
                width: parent.width
                from: 0.05
                to: 1
                stepSize: 0.01
                value: root.editor ? root.editor.strokeOpacity : 1
                onMoved: root.editor.strokeOpacity = value
            }
        }
        Column {
            width: parent.width
            spacing: 6
            visible: root.group === "select" && root.editor && root.editor.tool === "transform"
            StyledTextLabel {
                horizontalAlignment: Text.AlignLeft
                text: root.tr("Transform selection")
            }
            StyledDropdown {
                textRole: ""
                valueRole: ""
                width: parent.width
                model: [root.tr("Resize / stretch"), root.tr("Distort / skew")]
                currentIndex: root.editor && root.editor.transformMode === "skew" ? 1 : 0
                onActivated: function (index, value) {
                    if (root.editor)
                        root.editor.transformMode = index ? "skew" : "scale";
                }
            }
            StyledTextLabel {
                horizontalAlignment: Text.AlignLeft
                width: parent.width
                wrapMode: Text.WordWrap
                text: root.tr("Shift: mirrored sides. Ctrl: proportional size. Centre node: rotate.")
            }
            FlatButton {
                width: parent.width
                text: root.tr("Flip horizontally")
                onClicked: root.editor.mirrorSelected(true)
            }
            FlatButton {
                width: parent.width
                text: root.tr("Flip vertically")
                onClicked: root.editor.mirrorSelected(false)
            }
        }
        Column {
            width: parent.width
            spacing: 6
            visible: root.group === "pencil"
            StyledTextLabel {
                horizontalAlignment: Text.AlignLeft
                text: root.tr("Pencil smoothing")
            }
            StyledSlider {
                width: parent.width
                from: 0
                to: 1
                stepSize: 0
                value: root.editor ? root.editor.smoothing : 0.7
                onMoved: root.editor.setSmoothing(value)
            }
            CheckBox {
                text: root.tr("Rotate nib along stroke")
                checked: root.editor ? root.editor.nibFollowDirection : false
                onClicked: root.editor.nibFollowDirection = !checked
            }
            CheckBox {
                text: root.tr("Use pen pressure")
                checked: root.editor ? root.editor.pressureEnabled : false
                onClicked: root.editor.pressureEnabled = !checked
            }
            StyledTextLabel {
                horizontalAlignment: Text.AlignLeft
                text: root.tr("Pencil pointer")
            }
            StyledDropdown {
                textRole: ""
                valueRole: ""
                width: parent.width
                model: [root.tr("Dynamic (follows zoom)"), root.tr("Fixed (constant screen size)")]
                currentIndex: root.editor && root.editor.pointerMode === "fixed" ? 1 : 0
                onActivated: function (index, value) {
                    root.editor.pointerMode = index ? "fixed" : "dynamic";
                }
            }
        }
        Column {
            width: parent.width
            spacing: 6
            visible: root.stroke
            StyledTextLabel {
                horizontalAlignment: Text.AlignLeft
                text: root.tr(root.group === "pencil" ? "Pointer size: " : "Stroke width: ") + (root.editor ? Math.round(root.editor.strokeWidth) : 30)
            }
            IncrementalPropertyControl {
                width: parent.width
                minValue: root.editor && root.editor.styleEditActive ? 0.1 : 1
                maxValue: 1000
                step: root.editor && root.editor.styleEditActive ? 0.1 : 1
                decimals: root.editor && root.editor.styleEditActive ? 1 : 0
                currentValue: root.editor ? root.editor.strokeWidth : 30
                onValueEdited: function (value) {
                    if (root.editor)
                        root.editor.strokeWidth = value;
                }
            }
            StyledSlider {
                width: parent.width
                from: 1
                to: 1000
                stepSize: 1
                value: root.editor ? root.editor.strokeWidth : 30
                onPressedChanged: root.adjusting(pressed, true, false)
                onMoved: root.editor.strokeWidth = value
            }
            CheckBox {
                visible: root.group === "shapes" || root.group === "pen"
                text: root.tr("Fill shapes")
                checked: root.editor ? root.editor.filled : false
                onClicked: root.editor.filled = !checked
            }
            StyledTextLabel {
                horizontalAlignment: Text.AlignLeft
                text: root.tr("Opacity")
            }
            StyledSlider {
                width: parent.width
                from: 0.05
                to: 1
                stepSize: 0.01
                value: root.editor ? root.editor.strokeOpacity : 1
                onMoved: root.editor.strokeOpacity = value
            }
            StyledDropdown {
                visible: !!(root.editor && root.editor.styleEditActive)
                width: parent.width
                model: [
                    {
                        text: root.tr("Solid"),
                        value: "solid"
                    },
                    {
                        text: root.tr("Dashed"),
                        value: "dash"
                    },
                    {
                        text: root.tr("Dotted"),
                        value: "dot"
                    }
                ]
                currentIndex: root.editor ? Math.max(0, ["solid", "dash", "dot"].indexOf(root.editor.strokeLinePattern)) : 0
                onActivated: function (index, value) {
                    root.editor.strokeLinePattern = value;
                }
            }
            StyledDropdown {
                visible: !!(root.editor && root.editor.styleEditActive)
                width: parent.width
                model: [
                    {
                        text: root.tr("Round ends"),
                        value: "round"
                    },
                    {
                        text: root.tr("Flat ends"),
                        value: "butt"
                    },
                    {
                        text: root.tr("Square ends"),
                        value: "square"
                    }
                ]
                currentIndex: root.editor ? Math.max(0, ["round", "butt", "square"].indexOf(root.editor.strokeLineCap)) : 0
                onActivated: function (index, value) {
                    root.editor.strokeLineCap = value;
                }
            }
            FlatButton {
                width: parent.width
                visible: !root.editor || !root.editor.styleEditActive
                text: root.tr("Apply style to selection")
                enabled: root.editor && root.editor.selected >= 0
                onClicked: root.editor.updateStyle()
            }
        }
        Column {
            width: parent.width
            spacing: 6
            visible: root.erase && root.editor && root.editor.tool === "eraser"
            StyledTextLabel {
                horizontalAlignment: Text.AlignLeft
                text: root.tr("Eraser size: ") + (root.editor ? Math.round(root.editor.eraserSize) : 24)
            }
            IncrementalPropertyControl {
                width: parent.width
                minValue: 1
                maxValue: 1000
                step: 1
                decimals: 0
                currentValue: root.editor ? root.editor.eraserSize : 24
                onValueEdited: function (value) {
                    if (root.editor)
                        root.editor.eraserSize = value;
                }
            }
            StyledSlider {
                width: parent.width
                from: 1
                to: 1000
                stepSize: 1
                value: root.editor ? root.editor.eraserSize : 24
                onPressedChanged: root.adjusting(pressed, true, true)
                onMoved: root.editor.eraserSize = value
            }
        }
        Column {
            width: parent.width
            spacing: 6
            visible: root.group === "nodes" && root.editor && root.editor.tool === "nodes"
            StyledTextLabel {
                horizontalAlignment: Text.AlignLeft
                text: root.tr("Node curvature: ") + (root.editor ? Math.round(root.editor.pointCurvature * 100) : 50) + "%"
            }
            StyledSlider {
                width: parent.width
                from: 0
                to: 1
                stepSize: 0
                value: root.editor ? root.editor.pointCurvature : 0.5
                onPressedChanged: root.adjusting(pressed, false, false)
                onMoved: root.editor.setPointCurvature(value)
            }
            StyledTextLabel {
                horizontalAlignment: Text.AlignLeft
                text: root.tr("Drag resistance: ") + (root.editor ? Math.round(root.editor.dragResistance * 100) : 0) + "%"
            }
            StyledSlider {
                width: parent.width
                from: 0
                to: 1
                stepSize: 0
                value: root.editor ? root.editor.dragResistance : 0
                onMoved: root.editor.dragResistance = value
            }
        }
        Column {
            width: parent.width
            spacing: 8
            visible: root.group === "text"
            StyledTextLabel {
                width: parent.width
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignLeft
                text: root.tr("Drag a box on the score to type. Double-click existing text to edit. Ctrl+Enter finishes.")
            }
            FlatButton {
                width: parent.width
                text: root.tr("Special and musical characters")
                onClicked: if (DrawingSession.view)
                    DrawingSession.view.openDrawingCharacters()
            }
            FlatButton {
                width: parent.width
                text: root.tr("Edit selected text")
                enabled: !!(root.editor && root.editor.shapes[root.editor.selected] && root.editor.shapes[root.editor.selected].kind === "text")
                onClicked: root.editor.beginTextEditing(root.editor.selected)
            }
        }
        Column {
            width: parent.width
            spacing: 8
            visible: (root.group === "text" || root.group === "shapes") && !!(root.editor && root.editor.shapes[root.editor.selected] && root.editor.shapes[root.editor.selected].kind !== "text")
            StyledDropdown {
                width: parent.width
                textRole: ""
                valueRole: ""
                model: [root.tr("Above the line"), root.tr("After the line"), root.tr("Middle of the line")]
                currentIndex: root.editor ? ["above", "after", "middle"].indexOf(root.editor.labelPlacement) : 0
                onActivated: function (index, value) {
                    root.editor.labelPlacement = ["above", "after", "middle"][index];
                }
            }
            FlatButton {
                width: parent.width
                text: root.tr("Add text to selection")
                onClicked: root.editor.addLabelToSelection()
            }
        }
        Column {
            width: parent.width
            spacing: 6
            visible: root.music
            CheckBox {
                text: root.tr("Show anchors while moving")
                checked: root.editor ? root.editor.switchStaff : false
                onClicked: root.editor.switchStaff = !checked
            }
            StyledTextLabel {
                horizontalAlignment: Text.AlignLeft
                width: parent.width
                wrapMode: Text.WordWrap
                text: root.tr("Continuation after break: ") + (root.editor ? Math.round(root.editor.systemContinuation * 100) : 0) + "%"
            }
            StyledSlider {
                width: parent.width
                from: 0
                to: 1
                stepSize: 0.01
                value: root.editor ? root.editor.systemContinuation : 0
                onPressedChanged: root.adjusting(pressed, false, false)
                onMoved: root.editor.setSystemContinuation(value)
            }
            StyledTextLabel {
                horizontalAlignment: Text.AlignLeft
                width: parent.width
                wrapMode: Text.WordWrap
                text: root.tr("Start after break: ") + (root.editor ? Math.round(root.editor.systemStartContinuation * 100) : 0) + "%"
            }
            StyledSlider {
                width: parent.width
                from: 0
                to: 1
                stepSize: 0.01
                value: root.editor ? root.editor.systemStartContinuation : 0
                onPressedChanged: root.adjusting(pressed, false, false)
                onMoved: root.editor.setSystemStartContinuation(value)
            }
            StyledTextLabel {
                horizontalAlignment: Text.AlignLeft
                text: root.tr("Musical attachment")
            }
            StyledDropdown {
                textRole: ""
                valueRole: ""
                width: parent.width
                model: [root.tr("Note (without deformation)"), root.tr("Measures / rhythmic spacing")]
                currentIndex: root.editor && root.editor.anchorMode === "rhythm" ? 1 : 0
                onActivated: function (index, value) {
                    root.editor.anchorMode = index ? "rhythm" : "note";
                }
            }
            StyledTextLabel {
                horizontalAlignment: Text.AlignLeft
                text: root.tr("Rhythmic subdivision")
            }
            StyledDropdown {
                textRole: ""
                valueRole: ""
                width: parent.width
                model: ["1/4", "1/8", "1/16", "1/32"]
                currentIndex: root.editor ? [4, 8, 16, 32].indexOf(root.editor.subdivision) : 1
                onActivated: function (index, value) {
                    root.editor.subdivision = [4, 8, 16, 32][index];
                }
            }
            FlatButton {
                width: parent.width
                text: root.tr("Attach selection to rhythm")
                enabled: root.editor && root.editor.selected >= 0
                onClicked: root.editor.linkSelection()
            }
        }
    }
}
