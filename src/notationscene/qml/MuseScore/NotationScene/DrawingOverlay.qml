// SPDX-License-Identifier: GPL-3.0-only
import QtQuick
import Muse.UiComponents
import QtQuick.Controls as Controls
import MuseScore.NotationScene
import "drawing/ui"

Item {
    id: root
    property var scoreView: parent
    readonly property var drawingEditor: editor
    // Hidden editors must not subscribe to every pan/zoom of the score view.
    property var frame: editor.drawingEnabled ? scoreView.drawingViewTransform : ({
            x: 0,
            y: 0,
            width: 1,
            height: 1,
            left: 0,
            top: 0,
            scale: 1,
            visibleWidth: 1,
            visibleHeight: 1
        })
    anchors.fill: parent
    clip: true
    property bool loading: false
    property bool syncing: false
    property bool pending: false
    property bool geometryPending: false
    property bool restoreNeeded: true
    property bool defaultsInitialized: false
    property int layoutRevision: 0
    property int anchorsRevision: -1
    property string anchorsMode: ""
    Shortcut {
        sequence: "Escape"
        context: Qt.WindowShortcut
        enabled: DrawingSession.modeActive
        onActivated: editor.returnToSelection()
    }
    property bool anchorsRequested: false
    property int anchorRefreshAttempts: 0
    function pointTool() {
        return ["nodes", "anchors", "addnode", "deletenode", "join"].indexOf(editor.tool) >= 0;
    }
    function requestAnchors() {
        anchorsRequested = true;
        anchorRefreshAttempts = 0;
        refreshAnchors();
        anchorTimer.restart();
    }
    function ensureEditableAnchors() {
        if (!anchorsRequested || !editor.drawingEnabled)
            return;
        if (loading || syncing || busy()) {
            anchorTimer.restart();
            return;
        }
        if (!editor.rhythmMarkers.length) {
            if (++anchorRefreshAttempts < 8)
                anchorTimer.restart();
            return;
        }
        anchorsRequested = false;
        var updated = false;
        for (var i = 0; i < editor.shapes.length; i++) {
            if (editor.shapes[i].svgSource || editor.shapes[i].anchorMode === "note" || editor.shapes[i].suppressAutoAnchors || editor.bindings.some(function (b) {
                return b.shape === i && b.editable === true;
            }))
                continue;
            // Older documents may contain only layout control points. Recover
            // their editable rhythmic intersections once the score has a grid.
            editor.autoLink(i);
            if (editor.bindings.some(function (b) {
                return b.shape === i && b.editable === true;
            }))
                updated = true;
        }
        if (updated)
            editor.changed();
        console.info("[MuseDraw anchors] grid=" + editor.rhythmMarkers.length + " editable=" + editor.bindings.filter(function (b) {
            return b.editable === true;
        }).length + " visible=" + editor.rhythmPresentation.points.length);
    }
    function refreshFragments() {
        var fingerprints = {}, rendered = {};
        editor.nativeFragments = scoreView.nativeDrawingFragments();
        editor.nativeFragments.forEach(function (fragment) {
            if (fragment.svg && fragment.width > 0 && fragment.height > 0)
                rendered[fragment.id] = true;
        });
        editor.shapes.forEach(function (shape) {
            if (rendered[shape.drawId])
                fingerprints[shape.drawId] = shape;
        });
        editor.nativeFragmentFingerprints = fingerprints;
    }
    function previewFragments() {
        editor.nativeFragments = scoreView.previewNativeDrawingFragments(changeDocument());
        var fingerprints = {}, rendered = {};
        editor.nativeFragments.forEach(function (fragment) {
            if (fragment.svg && fragment.width > 0 && fragment.height > 0)
                rendered[fragment.id] = true;
        });
        editor.shapes.forEach(function (shape) {
            if (rendered[shape.drawId])
                fingerprints[shape.drawId] = shape;
        });
        editor.nativeFragmentFingerprints = fingerprints;
    }
    function changeDocument() {
        if (!geometryPending && editor.liveContinuationPreview)
            return JSON.stringify({
                format: "musescore-drawing",
                version: 1,
                nativeOperation: "continuation",
                systemContinuation: editor.systemContinuation,
                systemStartContinuation: editor.systemStartContinuation
            });
        if (editor.liveFragmentEdit && editor.shapes[editor.selected])
            return JSON.stringify({
                format: "musescore-drawing",
                version: 1,
                nativeOperation: "fragment",
                drawId: editor.shapes[editor.selected].drawId,
                fragmentOverrides: editor.shapes[editor.selected].fragmentOverrides || {}
            });
        return editor.exportDocument();
    }
    function busy() {
        if (editor.gradientOpacityEditing)
            return true;
        return editor.textEditing || editor.interactionActive || editor.penNodes.length > 0 || (DrawingSession.panel && DrawingSession.panel.sliderBusy);
    }
    property bool historyRedo: false
    function requestHistory(redo) {
        historyRedo = redo;
        editor.finishTextEditing();
        editor.finishPen();
        editor.finishInteractionForHistory();
        if (busy()) {
            historyTimer.restart();
            return;
        }
        commit();
        if (pending)
            return;
        scoreView.drawingUndo(redo);
        restoreNeeded = true;
        restore();
        restoreTimer.restart();
    }
    function commit() {
        if (loading || syncing || !pending)
            return;
        if (busy()) {
            commitTimer.restart();
            return;
        }
        syncing = true;
        try {
            var error = scoreView.syncNativeDrawingDocument(changeDocument());
            if (error) {
                editor.status = error;
                return;
            }
            pending = false;
            geometryPending = false;
            previewTimer.stop();
            editor.liveFragmentEdit = false;
            editor.liveContinuationPreview = false;
            refreshFragments();
            refreshAnchors();
        } finally {
            syncing = false;
        }
    }
    function restore() {
        if (!editor.drawingEnabled) {
            restoreNeeded = true;
            return;
        }
        if (syncing || pending || busy()) {
            restoreTimer.restart();
            return;
        }
        var text = scoreView.nativeDrawingDocument();
        if (!text)
            return;
        var selectedId = editor.shapes[editor.selected] ? editor.shapes[editor.selected].drawId : null;
        var selectedIds = editor.selectionIndices().map(function (index) {
            return editor.shapes[index].drawId;
        });
        var defaults = defaultsInitialized ? {
            strokeWidth: editor.strokeWidth,
            strokeColor: editor.strokeColor,
            secondaryColor: editor.secondaryColor,
            secondaryActive: editor.secondaryActive,
            fillColor: editor.fillColor,
            filled: editor.filled,
            strokeOpacity: editor.strokeOpacity,
            smoothing: editor.smoothing,
            pointerMode: editor.pointerMode,
            eraserSize: editor.eraserSize,
            pressureEnabled: editor.pressureEnabled
        } : null;
        loading = true;
        try {
            editor.importDocument(text);
            editor.history = [];
            editor.future = [];
            if (defaults)
                Object.keys(defaults).forEach(function (key) {
                    editor[key] = defaults[key];
                });
            defaultsInitialized = true;
            var selection = [];
            editor.shapes.forEach(function (shape, index) {
                if (selectedIds.indexOf(shape.drawId) >= 0)
                    selection.push(index);
            });
            editor.setSelection(selection, selectedId ? editor.shapes.findIndex(function (shape) {
                return shape.drawId === selectedId;
            }) : -1);
            refreshAnchors();
            refreshFragments();
            restoreNeeded = false;
        } catch (error) {
            editor.status = error.message;
        } finally {
            loading = false;
        }
        if (editor.drawingEnabled && pointTool())
            requestAnchors();
    }
    Timer {
        id: commitTimer
        interval: 16
        onTriggered: root.commit()
    }
    Timer {
        id: restoreTimer
        interval: 80
        onTriggered: root.restore()
    }
    Timer {
        id: anchorTimer
        interval: 80
        onTriggered: root.refreshAnchors()
    }
    Timer {
        id: previewTimer
        interval: 33
        onTriggered: root.previewFragments()
    }
    Timer {
        id: historyTimer
        interval: 16
        onTriggered: root.requestHistory(root.historyRedo)
    }
    function refreshAnchors() {
        if (!editor.drawingEnabled)
            return;
        var mode = editor.anchorGridMode + ":" + editor.subdivision;
        if (anchorsRevision === layoutRevision && anchorsMode === mode && editor.rhythmMarkers.length) {
            ensureEditableAnchors();
            return;
        }
        var markers = scoreView.drawingAnchorPoints(editor.anchorGridMode === "note" ? 0 : editor.subdivision);
        editor.rhythmMarkers = markers;
        anchorsRevision = layoutRevision;
        anchorsMode = mode;
        ensureEditableAnchors();
    }
    Connections {
        target: root.scoreView
        function onDrawingCommitRequested() {
            editor.finishTextEditing();
            editor.finishPen();
            editor.finishInteractionForHistory();
            root.commit();
        }
        function onDrawingCharacterReady(text, family) {
            if (DrawingSession.view !== root.scoreView)
                return;
            editor.insertCharacter(text, family, (root.frame.left || 0) + root.frame.visibleWidth / 2, (root.frame.top || 0) + root.frame.visibleHeight / 2);
        }
        function onDrawingScoreAboutToChange() {
            editor.finishTextEditing();
            editor.finishPen();
            editor.finishInteractionForHistory();
            root.commit();
        }
        function onDrawingScoreChanged() {
            commitTimer.stop();
            anchorTimer.stop();
            previewTimer.stop();
            restoreTimer.stop();
            historyTimer.stop();
            root.layoutRevision++;
            root.anchorsRevision = -1;
            root.anchorsMode = "";
            root.anchorsRequested = false;
            root.anchorRefreshAttempts = 0;
            root.pending = false;
            root.geometryPending = false;
            root.loading = true;
            root.defaultsInitialized = false;
            editor.editingTextId = "";
            editor.editingTextOriginal = null;
            editor.liveFragmentEdit = false;
            editor.liveContinuationPreview = false;
            editor.deformationSession = null;
            editor.preview = null;
            editor.penNodes = [];
            editor.selectedHandle = null;
            editor.pendingBinding = null;
            editor.snapTarget = null;
            editor.targetStaffMarker = null;
            editor.selectionBox = null;
            editor.rhythmMarkers = [];
            editor.shapes = [];
            editor.bindings = [];
            editor.selected = -1;
            editor.history = [];
            editor.future = [];
            editor.nativeFragments = [];
            editor.nativeFragmentFingerprints = {};
            root.loading = false;
            root.restoreNeeded = true;
            if (editor.drawingEnabled)
                restoreTimer.restart();
        }
        function onDrawingLayoutChanged() {
            root.layoutRevision++;
            if (root.syncing || root.pending)
                return;
            root.restoreNeeded = true;
            if (editor.drawingEnabled) {
                anchorTimer.restart();
                restoreTimer.restart();
            }
        }
        function onDrawingModeExitRequested() {
            DrawingSession.setDrawingMode(false);
        }
    }
    Connections {
        target: editor
        function onToolChanged() {
            if (DrawingSession.editor === editor)
                DrawingSession.lastTool = editor.tool;
        }
        function onEraserModeChanged() {
            if (DrawingSession.editor === editor)
                DrawingSession.lastEraserMode = editor.eraserMode;
        }
    }

    property string gradientDialogDrawing: ""
    property string gradientDialogStop: ""
    property string gradientDialogTool: ""
    ColorPickerModel {
        id: gradientColourDialog
        onColorSelected: function (colour) {
            var shape = editor.shapes[editor.selected];
            if (!shape || shape.drawId !== root.gradientDialogDrawing || editor.tool !== root.gradientDialogTool)
                return;
            editor.gradientStopId = root.gradientDialogStop;
            editor.setGradientStopColor(colour.toString());
        }
    }
    Item {
        id: gradientOpacityAnchor
        property point scorePoint: Qt.point(0, 0)
        readonly property point screenPoint: editor.mapToItem(root, scorePoint.x, scorePoint.y)
        x: screenPoint.x
        y: screenPoint.y
        width: 1
        height: 1
    }
    StyledPopupView {
        id: gradientOpacityPopup
        parent: gradientOpacityAnchor
        showArrow: true
        placementPolicies: PopupView.PreferAbove
        contentWidth: 200
        contentHeight: 66
        padding: 12
        property bool remembered: false
        function finishAdjustment() {
            editor.gradientOpacityEditing = false;
            if (remembered) {
                remembered = false;
                editor.gestureFinished();
            }
        }
        onClosed: finishAdjustment()
        Column {
            width: 200
            spacing: 8
            StyledTextLabel {
                width: parent.width
                text: qsTrc("drawing-tools", "Opacity") + ": " + Math.round(editor.gradientStopOpacity * 100) + "%"
            }
            StyledSlider {
                width: parent.width
                from: 0
                to: 100
                stepSize: 1
                value: Math.round(editor.gradientStopOpacity * 100)
                onPressedChanged: {
                    if (pressed)
                        editor.gradientOpacityEditing = true;
                    else
                        gradientOpacityPopup.finishAdjustment();
                }
                onMoved: {
                    if (!gradientOpacityPopup.remembered) {
                        editor.gestureStarting();
                        editor.remember();
                        gradientOpacityPopup.remembered = true;
                    }
                    editor.setGradientStopOpacity(value / 100);
                }
            }
        }
    }
    Connections {
        target: editor
        function onGradientColorRequested() {
            gradientOpacityPopup.close();
            var shape = editor.shapes[editor.selected];
            if (!shape || !editor.currentGradient)
                return;
            root.gradientDialogDrawing = shape.drawId;
            root.gradientDialogStop = editor.gradientStopId;
            root.gradientDialogTool = editor.tool;
            gradientColourDialog.selectColor(editor.gradientStopColor, false);
        }
        function onGradientOpacityRequested(anchorX, anchorY) {
            gradientOpacityPopup.finishAdjustment();
            gradientOpacityAnchor.scorePoint = Qt.point(anchorX, anchorY);
            gradientOpacityPopup.open();
        }
        function onToolChanged() {
            gradientOpacityPopup.close();
        }
        function onSelectedChanged() {
            gradientOpacityPopup.close();
        }
        function onCurrentGradientChanged() {
            if (!editor.currentGradient)
                gradientOpacityPopup.close();
        }
        function onDrawingEnabledChanged() {
            if (!editor.drawingEnabled)
                gradientOpacityPopup.close();
        }
    }

    DrawingCanvas {
        id: editor
        tool: "select"
        width: Math.max(1, root.frame.width || 1)
        height: Math.max(1, root.frame.height || 1)
        viewScale: Math.max(0.000001, root.frame.scale || 1)
        transform: Matrix4x4 {
            matrix: Qt.matrix4x4(editor.viewScale, 0, 0, root.frame.x || 0, 0, editor.viewScale, 0, root.frame.y || 0, 0, 0, 1, 0, 0, 0, 0, 1)
        }
        paintRegion: Qt.rect(root.frame.left || 0, root.frame.top || 0, Math.max(1, root.frame.visibleWidth || root.width), Math.max(1, root.frame.visibleHeight || root.height))
        contextualRhythmMarkers: true
        coordinateSpace: "score"
        visible: drawingEnabled
        nativeHistory: true
        svgVectorize: function (svg, matrix) {
            return root.scoreView.drawingSvgPaths(svg, matrix);
        }
        svgHitTest: function (shape, x, y, tolerance) {
            return root.scoreView.drawingSvgContains(shape.svgSource, shape.svgMatrix, Qt.point(x, y), tolerance);
        }
        svgRegionHitTest: function (shape, polygon) {
            return root.scoreView.drawingSvgIntersects(shape.svgSource, shape.svgMatrix, polygon);
        }
        nativeShapeHitTest: function (shape, x, y, tolerance) {
            if (!hasNativeFragments(shape))
                return null;
            return nativeFragments.some(function (fragment) {
                if (fragment.id !== shape.drawId || x < fragment.x - tolerance || x > fragment.x + fragment.width + tolerance || y < fragment.y - tolerance || y > fragment.y + fragment.height + tolerance)
                    return false;
                return root.scoreView.drawingSvgContains(fragment.svg, [fragment.width, 0, 0, fragment.height, fragment.x, fragment.y], Qt.point(x, y), tolerance);
            });
        }
        onNativeUndoRequested: function (redo) {
            root.requestHistory(redo);
        }
        onGestureStarting: root.commit()
        onGestureFinished: root.commit()
        onExitRequested: DrawingSession.setDrawingMode(false)
        onCharacterPickerRequested: root.scoreView.openDrawingCharacters()
        onDocumentChanged: if (!root.loading) {
            root.geometryPending = true;
            root.pending = true;
            if ((liveFragmentEdit || liveContinuationPreview) && !previewTimer.running)
                previewTimer.start();
            commitTimer.restart();
        }
        onContinuationChanged: if (!root.loading) {
            root.pending = true;
            if (!previewTimer.running)
                previewTimer.start();
            commitTimer.restart();
        }
        onRhythmAnchorsRequested: root.requestAnchors()
        drawingEnabled: false
        anchorMode: "rhythm"
        onSubdivisionChanged: root.refreshAnchors()
        onAnchorGridModeChanged: {
            root.requestAnchors();
            if (!root.loading)
                changed();
        }
        onDrawingEnabledChanged: {
            if (drawingEnabled) {
                if (root.restoreNeeded)
                    root.restore();
                if (root.pointTool())
                    root.requestAnchors();
                else {
                    root.refreshAnchors();
                    anchorTimer.restart();
                }
            } else {
                root.commit();
                anchorTimer.stop();
                restoreTimer.stop();
                previewTimer.stop();
            }
            root.scoreView.setDrawingMode(drawingEnabled);
        }
        Component.onCompleted: {
            tool = DrawingSession.lastTool;
            eraserMode = DrawingSession.lastEraserMode;
            DrawingSession.editor = editor;
            DrawingSession.view = root.scoreView;
            if (drawingEnabled) {
                root.refreshAnchors();
                restoreTimer.restart();
            }
            status = qsTrc("drawing-tools", "Activate Draw on score to create or edit drawings.");
        }
        Component.onDestruction: {
            if (DrawingSession.editor === editor) {
                DrawingSession.editor = null;
                DrawingSession.view = null;
            }
        }
    }
    Item {
        id: textPopupAnchor
        readonly property var entry: editor.shapeLookup[editor.editingTextId]
        readonly property var box: entry ? entry.bounds : Qt.rect(0, 0, 1, 1)
        x: Math.max(0, Math.min(root.width - 1, (root.frame.x || 0) + box.x * editor.viewScale))
        y: Math.max(0, Math.min(root.height - 1, (root.frame.y || 0) + box.y * editor.viewScale))
        width: Math.max(1, Math.min(root.width - x, box.width * editor.viewScale))
        height: Math.max(1, Math.min(24, box.height * editor.viewScale))
    }
    Loader {
        active: editor.textEditing
        sourceComponent: Component {
            NativeDrawingTextPopup {
                parent: textPopupAnchor
                editor: root.drawingEditor
                availableWidth: root.width
                boundsProvider: function (item) {
                    return root.scoreView.drawingPopupBounds(item);
                }
                Component.onCompleted: Qt.callLater(begin)
            }
        }
    }
    Item {
        id: panelNibPreview
        property var panel: DrawingSession.panel
        visible: editor.drawingEnabled && panel && panel.pointerPreview
        anchors.centerIn: parent
        width: panel && panel.eraserPreview ? editor.eraserSize * editor.viewScale : editor.effectiveStrokeWidth * editor.viewScale
        height: width
        rotation: panel && !panel.eraserPreview ? editor.nibAngle : 0
        z: 30
        Rectangle {
            id: panelNibBody
            anchors.fill: parent
            readonly property real squeeze: panelNibPreview.panel && !panelNibPreview.panel.eraserPreview ? 1 - editor.nibFlatten : 1
            readonly property real shear: panelNibPreview.panel && !panelNibPreview.panel.eraserPreview && editor.nibShape === "square" ? Math.tan(editor.nibTilt * Math.PI / 180) : 0
            transform: Matrix4x4 {
                matrix: Qt.matrix4x4(1, panelNibBody.shear, 0, -panelNibBody.shear * panelNibBody.height / 2, 0, panelNibBody.squeeze, 0, (1 - panelNibBody.squeeze) * panelNibBody.height / 2, 0, 0, 1, 0, 0, 0, 0, 1)
            }
            radius: panelNibPreview.panel && !panelNibPreview.panel.eraserPreview && editor.nibShape === "square" ? 0 : width / 2
            color: panelNibPreview.panel && !panelNibPreview.panel.eraserPreview ? editor.strokeColor : "transparent"
            border.color: "white"
            border.width: 2
            Rectangle {
                anchors.fill: parent
                anchors.margins: 2
                radius: panelNibPreview.panel && !panelNibPreview.panel.eraserPreview && editor.nibShape === "square" ? 0 : width / 2
                color: "transparent"
                border.color: "black"
                border.width: 1
            }
        }
    }
}
