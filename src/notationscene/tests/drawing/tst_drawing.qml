// SPDX-License-Identifier: GPL-3.0-only
import QtQuick
import QtTest
import "../../qml/MuseScore/NotationScene/drawing/ui"
import "../../qml/MuseScore/NotationScene/drawing/core/Vector.js" as Vector
import "../../qml/MuseScore/NotationScene/drawing/core/Painting.js" as Painting

Item {
    width: 800
    height: 500
    DrawingCanvas { id: editor; anchors.fill: parent }
    TestCase {
        name: "NativeDrawingEditor"
        when: windowShown
        function init() {
            editor.shapes = []; editor.bindings = []
            editor.history = []; editor.future = []
            editor.selected = -1; editor.selectedShapes = []
            editor.preview = null; editor.penNodes = []
            editor.rhythmMarkers = []; editor.nativeHistory = false
            editor.drawingEnabled = true; editor.tool = "select"
            editor.strokeColor = "#161616"; editor.strokeWidth = 3
            editor.strokeOpacity = 1; editor.eraserMode = "whole"
        }
        function addLine() {
            editor.append(Vector.shape("line", {x:80,y:90}, {x:240,y:90}, editor.currentStyle()))
        }
        function test_undo_preserves_stroke_defaults() {
            editor.strokeWidth = 42; addLine()
            compare(editor.shapes.length, 1)
            editor.undo(); compare(editor.shapes.length, 0)
            compare(editor.strokeWidth, 42)
            editor.redo(); compare(editor.shapes.length, 1)
        }
        function test_duplicate_has_independent_identity() {
            addLine(); editor.duplicateSelected()
            compare(editor.shapes.length, 2)
            verify(editor.shapes[0].drawId !== editor.shapes[1].drawId)
            verify(editor.shapes[0].commands[0][1] !== editor.shapes[1].commands[0][1])
            editor.undo(); compare(editor.shapes.length, 1)
            editor.redo(); compare(editor.shapes.length, 2)
        }
        function test_erase_and_undo() {
            addLine(); editor.tool = "eraser"
            mouseClick(editor, 160, 90, Qt.LeftButton)
            compare(editor.shapes.length, 0)
            editor.undo(); compare(editor.shapes.length, 1)
            compare(editor.shapes[0].commands[0][1], 80)
        }
        function test_mouse_line_and_undo() {
            editor.tool = "line"
            mouseDrag(editor, 80, 90, 160, 20, Qt.LeftButton)
            compare(editor.shapes.length, 1)
            editor.undo(); compare(editor.shapes.length, 0)
        }
        function test_selection_moves_without_scaling() {
            addLine()
            mouseDrag(editor, 160, 90, 40, 25, Qt.LeftButton)
            compare(editor.shapes[0].commands[0][1], 120)
            compare(editor.shapes[0].commands[1][1], 280)
        }
        function test_group_keeps_member_ids_and_geometry() {
            addLine(); addLine(); editor.setSelection([0,1])
            var before = JSON.stringify(editor.shapes.map(function(s) { return s.commands }))
            var ids = editor.shapes.map(function(s) { return s.drawId })
            editor.groupSelected()
            compare(editor.shapes[0].selectionGroup, editor.shapes[1].selectionGroup)
            compare(JSON.stringify(editor.shapes.map(function(s) { return s.commands })), before)
            compare(editor.shapes[0].drawId, ids[0]); compare(editor.shapes[1].drawId, ids[1])
            editor.ungroupSelected(); verify(!editor.shapes[0].selectionGroup)
        }
        function test_gradient_applies_to_selection() {
            addLine(); addLine(); editor.tool = "gradient-stroke"
            var gradient = {from:{x:0,y:0},to:{x:1,y:0},stops:[
                {id:"start",offset:0,color:"#ff0000"},
                {id:"middle",offset:0.4,color:"#00ff00"},
                {id:"end",offset:1,color:"#0000ff"}]}
            var output = Painting.applySet(editor,editor.shapes,[0,1],editor.tool,"#ff0000",gradient,editor.shapes[0])
            compare(output[0].style.strokeGradient.stops.length, 3)
            compare(output[1].style.strokeGradient.stops[1].offset, 0.4)
            verify(!editor.shapes[0].style.strokeGradient)
            verify(Vector.svg(output,Vector.bounds(output)).indexOf("linearGradient") >= 0)
        }
        function test_inactive_drawing_does_not_create_objects() {
            editor.drawingEnabled = false; editor.tool = "line"
            mouseDrag(editor,80,90,160,20,Qt.LeftButton)
            compare(editor.shapes.length,0)
        }
    }
}
