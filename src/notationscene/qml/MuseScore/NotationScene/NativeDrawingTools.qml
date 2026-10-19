// SPDX-License-Identifier: GPL-3.0-only
import QtQuick
import QtQuick.Window
import QtQuick.Controls as Controls
import QtQuick.Layouts
import QtQuick.Dialogs as Dialogs
import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene
import "drawing/ui"

Item {
    id: root
    implicitWidth: 64
    property NavigationSection navigationSection: null
    property NavigationPanel navigationPanel: NavigationPanel {
        name: "NativeDrawingTools"
        section: root.navigationSection
        direction: NavigationPanel.Vertical
        order: 1
    }
    readonly property var popupHost: Controls.Overlay.overlay || (Window.window ? Window.window.contentItem : null)
    readonly property real availableWidth: popupHost ? popupHost.width : Math.max(320, width)
    readonly property real availableHeight: popupHost ? popupHost.height : Math.max(160, height)
    onWidthChanged: if (options.isOpened)
        options.updatePosition()
    onHeightChanged: if (options.isOpened)
        options.updatePosition()
    property var panels: []
    property var activePanel: null
    QtObject {
        id: settingsProxy
        property string helpText: ""
    }
    readonly property var settings: activePanel ? activePanel.settings : settingsProxy
    QtObject {
        id: panelState
        readonly property bool sliderBusy: root.panels.some(function (p) { return !p.disposed && p.isOpened && p.settings.sliderBusy; })
        readonly property bool pointerPreview: root.panels.some(function (p) { return !p.disposed && p.isOpened && p.settings.pointerPreview; })
        readonly property bool eraserPreview: root.activePanel ? root.activePanel.settings.eraserPreview : false
    }
    QtObject {
        id: options
        readonly property bool isOpened: !!root.activePanel && root.activePanel.isOpened
        function close() {
            if (root.activePanel && !root.activePanel.pinned)
                root.activePanel.close();
        }
        function updatePosition() {
            root.panels.forEach(function (p) {
                p.updatePosition();
            });
        }
    }
    function titleFor(key) {
        return ({
                objects: qsTr("Objects"),
                repeat: qsTr("Repeat along path"),
                select: qsTr("Selection"),
                pencil: qsTr("Pencil"),
                pen: qsTr("Pen"),
                shapes: qsTr("Shapes"),
                paint: qsTr("Painting"),
                nodes: qsTr("Nodes"),
                colors: qsTr("Colours"),
                combine: qsTr("Groups"),
                text: qsTr("Text"),
                eraser: qsTr("Eraser")
            })[key] || key;
    }
    function placePanel(panel) {
        var w = panel.panelWidth + 28, h = panel.panelHeight + 28, used = panels.filter(function (p) {
            return p !== panel && p.isOpened;
        });
        var best = null, cost = Infinity;
        for (var x = Math.max(8, availableWidth - width - w - 8); x >= 8; x -= w + 8)
            for (var y = 70; y + h <= availableHeight - 8; y += h + 8) {
                var overlap = 0;
                used.forEach(function (p) {
                    overlap += Math.max(0, Math.min(x + w, p.x + p.panelWidth + 28) - Math.max(x, p.x)) * Math.max(0, Math.min(y + h, p.y + p.panelHeight + 28) - Math.max(y, p.y));
                });
                if (overlap < cost) {
                    cost = overlap;
                    best = {
                        x: x,
                        y: y
                    };
                }
                if (!cost)
                    break;
            }
        if (!best)
            best = {
                x: Math.max(8, availableWidth - w - width - 8),
                y: 8
            };
        panel.x = best.x;
        panel.y = best.y;
    }
    function panelClosed(panel) {
        if (panel.disposed)
            return;
        panel.disposed = true;
        panels = panels.filter(function (p) {
            return p !== panel;
        });
        if (activePanel === panel) {
            activePanel = null;
            DrawingSession.panel = panels.length ? panelState : null;
        }
        Qt.callLater(function () {
            panel.destroy();
        });
    }
    Component {
        id: panelFactory
        DrawingToolPanel {
            toolbox: root
            editor: root.editor
        }
    }
    property string group: ""
    property var activeButton: null
    property string shapeTool: "rectangle"
    property string pointTool: "nodes"
    property string selectionTool: "select"
    readonly property var editor: DrawingSession.editor
    readonly property var toolGroups: ({
            select: [
                {
                    key: "select",
                    label: qsTrc("drawing-tools", "Select")
                },
                {
                    key: "transform",
                    label: qsTrc("drawing-tools", "Transform selection")
                }
            ],
            combine: [
                {
                    key: "group",
                    label: qsTrc("drawing-tools", "Group drawings")
                },
                {
                    key: "ungroup",
                    label: qsTrc("drawing-tools", "Ungroup drawings")
                },
                {
                    key: "fuse",
                    label: qsTrc("drawing-tools", "Fuse drawings")
                }
            ],
            pencil: [
                {
                    key: "pencil",
                    label: qsTrc("drawing-tools", "Pencil")
                }
            ],
            pen: [
                {
                    key: "pen",
                    label: qsTrc("drawing-tools", "Bézier pen")
                }
            ],
            text: [
                {
                    key: "text",
                    label: qsTrc("drawing-tools", "Text")
                },
                {
                    key: "characters",
                    label: qsTrc("drawing-tools", "Special and musical characters")
                }
            ],
            paint: [
                {
                    key: "paint-fill",
                    label: qsTrc("drawing-tools", "Paint fill")
                },
                {
                    key: "paint-stroke",
                    label: qsTrc("drawing-tools", "Paint stroke")
                },
                {
                    key: "gradient-fill",
                    label: qsTrc("drawing-tools", "Fill gradient")
                },
                {
                    key: "gradient-stroke",
                    label: qsTrc("drawing-tools", "Stroke gradient")
                }
            ],
            objects: [],
            repeat: [],
            shapes: [
                {
                    key: "wave",
                    label: qsTrc("drawing-tools", "Wave")
                },
                {
                    key: "polygon",
                    label: qsTrc("drawing-tools", "Polygon")
                },
                {
                    key: "star",
                    label: qsTrc("drawing-tools", "Star")
                },
                {
                    key: "rounded-rectangle",
                    label: qsTrc("drawing-tools", "Rounded rectangle")
                },
                {
                    key: "diamond",
                    label: qsTrc("drawing-tools", "Diamond")
                },
                {
                    key: "cross",
                    label: qsTrc("drawing-tools", "Cross")
                },
                {
                    key: "line",
                    label: qsTrc("drawing-tools", "Line")
                },
                {
                    key: "rectangle",
                    label: qsTrc("drawing-tools", "Rectangle")
                },
                {
                    key: "ellipse",
                    label: qsTrc("drawing-tools", "Ellipse")
                },
                {
                    key: "triangle",
                    label: qsTrc("drawing-tools", "Triangle")
                },
                {
                    key: "right-triangle",
                    label: qsTrc("drawing-tools", "Right triangle")
                },
                {
                    key: "arrow",
                    label: qsTrc("drawing-tools", "Arrow")
                },
                {
                    key: "arrow-double",
                    label: qsTrc("drawing-tools", "Double arrow")
                },
                {
                    key: "arrow-filled",
                    label: qsTrc("drawing-tools", "Filled arrow")
                }
            ],
            nodes: [
                {
                    key: "nodes",
                    label: qsTrc("drawing-tools", "Edit nodes")
                },
                {
                    key: "addnode",
                    label: qsTrc("drawing-tools", "Add node")
                },
                {
                    key: "deletenode",
                    label: qsTrc("drawing-tools", "Delete node")
                },
                {
                    key: "join",
                    label: qsTrc("drawing-tools", "Join nodes")
                },
                {
                    key: "anchors",
                    label: qsTrc("drawing-tools", "Link nodes")
                },
                {
                    key: "unlink",
                    label: qsTrc("drawing-tools", "Unlink nodes")
                }
            ],
            eraser: [
                {
                    key: "eraser",
                    label: qsTrc("drawing-tools", "Erase entire line")
                },
                {
                    key: "eraser-partial",
                    label: qsTrc("drawing-tools", "Erase touched areas")
                },
                {
                    key: "erase-rectangle",
                    label: qsTrc("drawing-tools", "Rectangular eraser")
                },
                {
                    key: "erase-ellipse",
                    label: qsTrc("drawing-tools", "Elliptical eraser")
                },
                {
                    key: "erase-triangle",
                    label: qsTrc("drawing-tools", "Triangular eraser")
                }
            ]
        })
    function selectTool(key) {
        if (!editor)
            return;
        editor.finishTextEditing();
        editor.tool = key === "eraser-partial" ? "eraser" : key;
        DrawingSession.setDrawingMode(true);
        if (key === "eraser" || key === "eraser-partial")
            editor.eraserMode = key === "eraser" ? "whole" : "partial";
        editor.forceActiveFocus();
        if (key === "select" || key === "transform")
            selectionTool = key;
        if (editor.isBasicShape(key))
            shapeTool = key;
        if (["nodes", "addnode", "deletenode", "join", "anchors", "unlink"].indexOf(key) >= 0)
            pointTool = key;
    }
    function shortcutFor(key) {
        return ({
                select: "V",
                transform: "Shift+V",
                nodes: "A",
                addnode: "Shift+A",
                deletenode: "Shift+A",
                join: "Shift+A",
                anchors: "Shift+A",
                unlink: "Shift+A",
                pencil: "N",
                pen: "P",
                eraser: "E",
                "eraser-partial": "Shift+E",
                "erase-rectangle": "Shift+E",
                "erase-ellipse": "Shift+E",
                "erase-triangle": "Shift+E",
                rectangle: "M",
                ellipse: "L",
                line: "Shift+M",
                triangle: "Shift+M",
                "right-triangle": "Shift+M",
                arrow: "Shift+M",
                "arrow-double": "Shift+M",
                "arrow-filled": "Shift+M",
                text: "T",
                characters: "Shift+T",
                colors: "X",
                "paint-fill": "B",
                "paint-stroke": "Shift+B",
                "gradient-fill": "G",
                "gradient-stroke": "Shift+G"
            })[key] || "";
    }
    function toolHelp(key) {
        var help = toolDescription(key), shortcut = shortcutFor(key);
        return help + (shortcut ? " (" + shortcut + ")" : "") + ((key === "select" || key === "transform") ? " · Ctrl+D" : "");
    }
    readonly property var descriptionKeys: {
        var keys = ["colors", "shapes", "combine"];
        Object.keys(toolGroups).forEach(function (group) {
            toolGroups[group].forEach(function (tool) {
                if (keys.indexOf(tool.key) < 0)
                    keys.push(tool.key);
            });
        });
        return keys;
    }
    function toolDescription(key) {
        if (key === "repeat")
            return qsTr("Repeat an SVG along a drawn path.");
        if (key === "paint" || key.indexOf("paint-") === 0)
            return qsTrc("drawing-tools", "Click a drawing to paint its fill or stroke. Painting applies to the selected drawings together.");
        if (key.indexOf("gradient-") === 0)
            return qsTrc("drawing-tools", "Drag to set the gradient transition. Drag its ends to change the extent. Select a colour square to edit it; slide it along the axis to space colours. Add colours in this panel. Controls appear only in Gradient mode.");
        if (key === "colors")
            return qsTrc("drawing-tools", "Choose primary and secondary colours. X switches the active colour.");
        if (key === "pen")
            return qsTrc("drawing-tools", "Click or drag nodes. Enter or double-click the last node finishes a path. Connect to the first node to close a shape and keep drawing.");
        if (key === "select")
            return qsTrc("drawing-tools", "Click or drag a box to select drawings. Shift-click adds or removes a drawing. Drag the selection to move it.");
        if (key === "transform")
            return qsTrc("drawing-tools", "Transform selection") + ". " + qsTrc("drawing-tools", "Shift: mirrored sides. Ctrl: proportional size. Centre node: rotate.");
        if (key === "group" || key === "ungroup" || key === "fuse")
            return qsTrc("drawing-tools", "Grouping keeps individual nodes and attachments. Fusing creates one editable drawing with one initial anchor.");
        if (key === "nodes")
            return qsTrc("drawing-tools", "Drag nodes to reshape the drawing. Green endpoints belong to a derived fragment.");
        if (key === "anchors")
            return qsTrc("drawing-tools", "Drag a node or text close to a drawing node or rhythmic anchor to attach it.");
        if (key === "unlink")
            return qsTrc("drawing-tools", "Click an attachment line to unlink it. The red cross appears only over attachment lines.");
        if (key.indexOf("erase-") === 0)
            return qsTrc("drawing-tools", "Drag an eraser shape. Only drawing areas inside it are removed.");
        if (key === "eraser-partial")
            return qsTrc("drawing-tools", "Erase only the areas touched by the circular eraser.");
        if (key === "eraser")
            return qsTrc("drawing-tools", "Erase the entire drawing touched by the eraser.");
        if (key === "text" || key === "characters")
            return qsTrc("drawing-tools", "Drag a box on the score to type. Double-click existing text to edit. Ctrl+Enter finishes.");
        if (key === "pencil")
            return qsTrc("drawing-tools", "Drag to draw. Alt constrains the stroke to horizontal, vertical or diagonal movement.");
        if (key.indexOf("arrow") === 0)
            return qsTrc("drawing-tools", "Select an arrow. Drag the yellow diamond horizontally for head size, vertically for its opening. Cyan changes length and direction; lime changes hollow shaft width. The head stays unanchored unless you use Link nodes.");
        return qsTrc("drawing-tools", "Shift draws from the centre; Ctrl keeps proportions; Alt constrains direction.");
    }
    function openGroup(key, button) {
        if (!editor)
            return;
        group = key;
        if (key === "repeat")
            editor.beginRepeatEdit();
        else if (key !== "combine" && key !== "colors" && key !== "objects")
            selectTool(key === "select" ? selectionTool : key === "shapes" ? shapeTool : key === "nodes" ? pointTool : key === "paint" ? (editor.paintingTool ? editor.tool : "paint-fill") : key);
        var retained = panels.find(function (p) {
            return p.group === key && !p.disposed;
        });
        if (retained) {
            activePanel = retained;
            DrawingSession.panel = panelState;
            return;
        }
        if (activePanel && !activePanel.pinned)
            activePanel.close();
        group = key;
        activeButton = button;
        var panel = panelFactory.createObject(popupHost, {
            group: key,
            pinned: false
        }) as DrawingToolPanel;
        panels = panels.concat([panel]);
        activePanel = panel;
        DrawingSession.panel = panelState;
        placePanel(panel);
        panel.open();
    }
    function importSvg(url) {
        var v = DrawingSession.view, e = editor;
        if (!e || !v)
            return;
        var f = v.drawingViewTransform;
        try {
            e.importSvg(v.loadSvgDrawing(url), (f.left || 0) + (f.visibleWidth || e.width) / 2, (f.top || 0) + (f.visibleHeight || e.height) / 2);
        } catch (error) {
            e.status = error.message;
        }
    }
    Flickable {
        id: toolScroll
        anchors.fill: parent
        anchors.topMargin: 22
        anchors.bottomMargin: 22
        contentWidth: width
        contentHeight: toolColumn.height + 16
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        function scrollBy(amount) {
            contentY = Math.max(0, Math.min(contentHeight - height, contentY + amount));
        }
        onContentYChanged: if (options.isOpened)
            options.updatePosition()
        Controls.ScrollBar.vertical: StyledScrollBar {
            policy: Controls.ScrollBar.AsNeeded
            thickness: 6
        }
        Column {
            id: toolColumn
            opacity: DrawingSession.modeActive ? 1 : 0.45
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 6
            FlatButton {
                id: colourButton
                width: 40
                height: 44
                transparent: true
                property bool drawingToolTip: true
                toolTipTitle: qsTrc("drawing-tools", "Colours")
                toolTipShortcut: "X"
                onClicked: {
                    DrawingSession.setDrawingMode(true);
                    root.openGroup("colors", colourButton);
                }
                Connections {
                    target: colourButton.mouseArea
                    function onContainsMouseChanged() {
                        settings.helpText = colourButton.mouseArea.containsMouse ? root.toolHelp("colors") : "";
                    }
                }
                contentItem: Component {
                    Item {
                        width: 40
                        height: 44
                        Rectangle {
                            x: 15
                            y: 14
                            width: 23
                            height: 23
                            radius: 4
                            color: root.editor ? root.editor.secondaryColor : "white"
                            border.width: 1
                            border.color: ui.theme.fontPrimaryColor
                        }
                        Rectangle {
                            x: 5
                            y: 5
                            width: 25
                            height: 25
                            radius: 4
                            color: root.editor ? root.editor.strokeColor : "black"
                            border.width: 1
                            border.color: ui.theme.fontPrimaryColor
                        }
                    }
                }
            }
            Item {
                width: 40
                height: 8
            }
            SeparatorLine {
                width: 40
            }
            Repeater {
                model: [
                    {
                        key: "select",
                        label: qsTrc("drawing-tools", "Select")
                    },
                    {
                        key: "pencil",
                        label: qsTrc("drawing-tools", "Pencil")
                    },
                    {
                        key: "pen",
                        label: qsTrc("drawing-tools", "Bézier pen")
                    },
                    {
                        key: "shapes",
                        label: qsTrc("drawing-tools", "Basic shapes")
                    },
                    {
                        key: "paint",
                        label: qsTrc("drawing-tools", "Painting and gradients")
                    },
                    {
                        key: "repeat",
                        label: qsTr("Repeat along path")
                    },
                    {
                        key: "nodes",
                        label: qsTrc("drawing-tools", "Nodes and attachments")
                    },
                    {
                        key: "eraser",
                        label: qsTrc("drawing-tools", "Eraser")
                    },
                    {
                        key: "text",
                        label: qsTrc("drawing-tools", "Text")
                    }
                ]
                delegate: NativeDrawingIconButton {
                    id: groupButton
                    required property var modelData
                    required property int index
                    glyph: modelData.key === "select" ? root.selectionTool : modelData.key === "shapes" ? root.shapeTool : modelData.key === "nodes" ? root.pointTool : modelData.key
                    grouped: true
                    toolTipTitle: modelData.label
                    toolTipShortcut: root.shortcutFor(modelData.key)
                    accentButton: DrawingSession.modeActive && root.editor && (root.toolGroups[modelData.key] || []).some(function (t) {
                        return t.key === root.editor.tool;
                    })
                    onHoveredChanged: settings.helpText = hovered ? root.toolHelp(modelData.key) : ""
                    onClicked: root.openGroup(modelData.key, groupButton)
                }
            }
            SeparatorLine {
                width: 40
            }
            NativeDrawingIconButton {
                id: combineButton
                glyph: "group"
                grouped: true
                toolTipTitle: qsTrc("drawing-tools", "Group and fuse drawings")
                onClicked: {
                    DrawingSession.setDrawingMode(true);
                    root.openGroup("combine", combineButton);
                }
            }
            NativeDrawingIconButton {
                glyph: "objects"
                toolTipTitle: qsTrc("drawing-tools", "Objects / layers")
                onClicked: {
                    DrawingSession.setDrawingMode(true);
                    root.openGroup("objects", this);
                }
            }
            NativeDrawingIconButton {
                glyph: "characters"
                toolTipTitle: qsTrc("drawing-tools", "Special and musical characters")
                toolTipShowLocked: false
                onClicked: if (DrawingSession.view) {
                    DrawingSession.setDrawingMode(true);
                    root.editor.tool = "characters";
                    DrawingSession.view.openDrawingCharacters();
                }
            }
            NativeDrawingIconButton {
                glyph: "import"
                toolTipTitle: qsTrc("drawing-tools", "Import SVG")
                onClicked: {
                    DrawingSession.setDrawingMode(true);
                    svgDialog.open();
                }
            }
            NativeDrawingIconButton {
                glyph: "export"
                toolTipTitle: qsTrc("drawing-tools", "Export SVG")
                onClicked: {
                    DrawingSession.setDrawingMode(true);
                    exportDialog.open();
                }
            }
            SeparatorLine {
                width: 40
            }
            NativeDrawingIconButton {
                glyph: "undo"
                toolTipTitle: qsTrc("drawing-tools", "Undo")
                onClicked: if (root.editor) {
                    DrawingSession.setDrawingMode(true);
                    root.editor.undo();
                }
            }
            NativeDrawingIconButton {
                glyph: "redo"
                toolTipTitle: qsTrc("drawing-tools", "Redo")
                onClicked: if (root.editor) {
                    DrawingSession.setDrawingMode(true);
                    root.editor.redo();
                }
            }
            NativeDrawingIconButton {
                glyph: "delete"
                toolTipTitle: qsTrc("drawing-tools", "Delete selection")
                onClicked: if (root.editor) {
                    DrawingSession.setDrawingMode(true);
                    root.editor.removeSelected();
                }
            }
            NativeDrawingIconButton {
                glyph: root.editor && root.editor.anchorGridMode === "note" ? "anchor-note" : "anchor-subdivision"
                toolTipShowLocked: false
                toolTipTitle: root.editor && root.editor.anchorGridMode === "note" ? qsTrc("drawing-tools", "Anchor to notes") : qsTrc("drawing-tools", "Anchor to subdivisions")
                toolTipDescription: qsTrc("drawing-tools", "Click to switch between note positions and rhythmic subdivisions.")
                onClicked: if (root.editor) {
                    DrawingSession.setDrawingMode(true);
                    root.editor.anchorGridMode = root.editor.anchorGridMode === "note" ? "subdivision" : "note";
                    root.editor.forceActiveFocus();
                }
            }
        }
    }
    FlatButton {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: 40
        height: 20
        icon: IconCode.SMALL_ARROW_UP
        transparent: true
        visible: toolScroll.contentY > 1
        toolTipTitle: qsTrc("drawing-tools", "Scroll tools up")
        onClicked: toolScroll.scrollBy(-138)
    }
    FlatButton {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: 40
        height: 20
        icon: IconCode.SMALL_ARROW_DOWN
        transparent: true
        visible: toolScroll.contentY + toolScroll.height < toolScroll.contentHeight - 1
        toolTipTitle: qsTrc("drawing-tools", "Scroll tools down")
        onClicked: toolScroll.scrollBy(138)
    }
    Connections {
        target: DrawingSession
        function onModeActiveChanged() {
            if (!DrawingSession.modeActive)
                root.panels.slice().forEach(function (p) {
                    p.close();
                });
        }
    }
    Connections {
        target: root.editor
        function onToolChanged() {
            var key = root.editor.tool;
            if (key === "select" || key === "transform")
                root.selectionTool = key;
            if (root.editor.isBasicShape(key))
                root.shapeTool = key;
            if (["nodes", "addnode", "deletenode", "join", "anchors", "unlink"].indexOf(key) >= 0)
                root.pointTool = key;
        }
        function onTextEditRequested() {
            options.close();
        }
    }
    Connections {
        target: root.Window.window
        function onXChanged() {
            if (options.isOpened)
                options.updatePosition();
        }
        function onYChanged() {
            if (options.isOpened)
                options.updatePosition();
        }
        function onScreenChanged() {
            if (options.isOpened)
                options.updatePosition();
        }
    }
    Connections {
        target: root.popupHost
        function onWidthChanged() {
            if (options.isOpened)
                options.updatePosition();
        }
        function onHeightChanged() {
            if (options.isOpened)
                options.updatePosition();
        }
    }
    Dialogs.FileDialog {
        id: svgDialog
        title: qsTrc("drawing-tools", "Import SVG")
        fileMode: Dialogs.FileDialog.OpenFile
        nameFilters: [qsTrc("drawing-tools", "SVG files (*.svg)")]
        selectedNameFilter.index: 0
        defaultSuffix: "svg"
        onAccepted: root.importSvg(selectedFile)
    }
    Dialogs.FileDialog {
        id: exportDialog
        title: qsTrc("drawing-tools", "Export SVG")
        fileMode: Dialogs.FileDialog.SaveFile
        nameFilters: [qsTrc("drawing-tools", "SVG files (*.svg)")]
        selectedNameFilter.index: 0
        defaultSuffix: "svg"
        onAccepted: if (root.editor && DrawingSession.view)
            root.editor.status = DrawingSession.view.saveSvgDrawing(root.editor.exportSvg(), "", selectedFile)
    }
}
