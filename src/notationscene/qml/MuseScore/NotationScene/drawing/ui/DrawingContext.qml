// SPDX-License-Identifier: GPL-3.0-only
import QtQuick
import QtQuick.Window
import Muse.Ui
import Muse.UiComponents
import "../core/Context.js" as Actions
import "../core/Vector.js" as V

Item {
    id: root
    property var editor
    property var hit: null
    property var anchor: null
    property real clickX: 0
    property real clickY: 0
    property point popupPosition: Qt.point(0, 0)
    function finishEdit(apply) {
        if (editPopup.finishing)
            return;
        editPopup.finishing = true;
        Qt.callLater(function () {
            if (root.editor.nodeEditActive)
                root.editor.finishNodeEdit(apply);
            else
                root.editor.finishShapeEdit(apply);
            editPopup.finishing = false;
        });
    }
    function menuPosition(x, y, model) {
        var surface = menuAnchor.parent, pointer = editor.mapToItem(surface, x, y);
        if (editor.selected < 0)
            return Qt.point(pointer.x + 6, pointer.y + 6);
        var box = V.bounds([editor.shapes[editor.selected]]);
        var a = editor.mapToItem(surface, box.x, box.y), b = editor.mapToItem(surface, box.x + box.width, box.y + box.height);
        var left = Math.min(a.x, b.x), top = Math.min(a.y, b.y), right = Math.max(a.x, b.x), bottom = Math.max(a.y, b.y);
        var w = 290, h = Math.min(surface.height - 16, model.reduce(function (n, row) {
            return n + (row.id ? 32 : 8);
        }, 24));
        var options = [Qt.point(right + 8, pointer.y - 16), Qt.point(left - w - 8, pointer.y - 16), Qt.point(pointer.x - 24, top - h - 8), Qt.point(pointer.x - 24, bottom + 8)];
        var best = null, cost = Infinity;
        options.forEach(function (p) {
            p.x = Math.max(8, Math.min(p.x, surface.width - w - 8));
            p.y = Math.max(8, Math.min(p.y, surface.height - h - 8));
            var overlap = Math.max(0, Math.min(p.x + w, right) - Math.max(p.x, left)) * Math.max(0, Math.min(p.y + h, bottom) - Math.max(p.y, top));
            var distance = Math.hypot(Math.max(p.x - pointer.x, 0, pointer.x - p.x - w), Math.max(p.y - pointer.y, 0, pointer.y - p.y - h));
            var score = overlap * 1000 + distance;
            if (score < cost) {
                best = p;
                cost = score;
            }
        });
        return best;
    }
    function item(id, title, enabled) {
        return {
            id: id,
            title: qsTrc("drawing-tools", title),
            enabled: enabled === undefined ? true : enabled
        };
    }
    function openAt(x, y, forcedIndex) {
        clickX = x;
        clickY = y;
        hit = editor.pointAt(x, y);
        anchor = editor.anchorEditingActive ? Actions.anchorHit(editor, x, y) : null;
        if (hit)
            anchor = null;
        var target = hit ? hit.shape : V.hit(editor.shapes, x, y, 10 / editor.viewScale, editor.svgHitTest, editor.nativeShapeHitTest);
        var fromPanel = typeof forcedIndex === "number" && forcedIndex >= 0;
        if (fromPanel) {
            target = forcedIndex;
            hit = null;
            anchor = null;
        }
        if (editor.styleEditActive) {
            target = editor.styleEditIndex;
            if (hit && hit.shape === target) {
                if (!editor.styleNodeSelected(hit.handle.command))
                    editor.selectStyleNode(hit.handle.command);
            } else
                hit = null;
            anchor = null;
        }
        if (!editor.styleEditActive && target >= 0 && editor.selectionIndices().indexOf(target) < 0)
            editor.setSelection([target], target, true);
        var selected = editor.selected >= 0, model = [item("cut", "Cut", selected), item("copy", "Copy", selected), item("paste", "Paste", !!Actions.clipboard),
            {},
            item("rename", "Rename", selected), item("duplicate", "Duplicate", selected), item("delete", "Delete", selected),
            {},
            item("group", "Group", editor.selectionIndices().length > 1), item("ungroup", "Ungroup", selected && !!editor.shapes[editor.selected].selectionGroup), item("front", "Bring to front", selected), item("back", "Send to back", selected),
            {}
        ];
        model.push({
            id: "transform-menu",
            title: qsTrc("drawing-tools", "Transform"),
            enabled: selected,
            subitems: [item("transform", "Transform / resize"), item("flip-h", "Mirror horizontally"), item("flip-v", "Mirror vertically")]
        });
        model.push(item("edit-shape", "Edit Shape", editor.canEditShape(editor.selected)));
        if (!editor.canEditShape(editor.selected))
            model.pop();
        model.push(item("node-style", "Edit style", selected && editor.shapes[editor.selected].kind !== "text"));
        model.push(item("anchors", "Edit anchors", selected));
        if (hit) {
            model.push({}, item("remove-node", "Remove node"), item("smooth", "Smooth node"), item("corner", "Angular node"));
        } else if (selected && !anchor)
            model.push(item("add-node", "Add node here"));
        if (hit || anchor)
            model.push({}, item("link", "Link"), item("unlink", "Unlink"));
        if (editor.styleEditActive) {
            model = [];
            if (hit) {
                model.push(item("node-properties-copy", "Copy node properties"), item("node-properties-paste", "Paste node properties", !!editor.nodeStyleClipboard), item("node-properties-remove", "Delete node properties", (editor.shapes[target].segmentStyles || []).some(function (m) {
                    return editor.styleRangeEnd > editor.styleNodeCommand ? m.command >= editor.styleNodeCommand && m.command < editor.styleRangeEnd : m.command === editor.styleNodeCommand;
                })), {});
            }
            model.push(item("confirm-style", "Confirm (Enter)"), item("cancel-style", "Cancel (Esc)"));
        }
        popupPosition = fromPanel ? editor.mapToItem(menuAnchor.parent, x, y) : menuPosition(x, y, model);
        menuAnchor.x = popupPosition.x;
        menuAnchor.y = popupPosition.y;
        menu.show(Qt.point(0, 0), model);
    }
    function run(id) {
        var e = editor;
        if (id === "rename") {
            renameInput.text = e.shapes[e.selected].name || e.shapes[e.selected].kind || "SVG";
            renameIndex = e.selected;
            renamePopup.open();
            Qt.callLater(function () {
                renameInput.forceActiveFocus();
                renameInput.selectAll();
            });
            return;
        }
        if (id === "node-style") {
            e.beginLineStyleEdit(hit);
            return;
        }
        if (id === "node-properties-copy") {
            Actions.copyNodeProperties(e);
            return;
        }
        if (id === "node-properties-paste") {
            Actions.pasteNodeProperties(e);
            return;
        }
        if (id === "node-properties-remove") {
            Actions.removeNodeProperties(e);
            return;
        }
        if (id === "confirm-style" || id === "cancel-style") {
            finishEdit(id === "confirm-style");
            return;
        }
        var nodeAction = ["add-node", "remove-node", "smooth", "corner", "link", "unlink"].indexOf(id) >= 0 || id === "properties-paste" && !!hit;
        if (nodeAction) {
            var nextTool = id === "add-node" ? "addnode" : id === "remove-node" ? "deletenode" : id === "link" ? "anchors" : id === "unlink" ? "unlink" : "nodes";
            e.beginNodeEdit(nextTool);
        }
        e.gestureStarting();
        try {
            if (id === "edit-shape") {
                e.beginShapeEdit();
            } else if (id === "copy" || id === "cut")
                Actions.copy(e, id === "cut");
            else if (id === "paste")
                Actions.paste(e, clickX, clickY);
            else if (id === "properties-copy")
                Actions.copyProperties(e, hit);
            else if (id === "properties-paste")
                Actions.pasteProperties(e, hit);
            else if (id === "duplicate")
                e.duplicateSelected();
            else if (id === "delete")
                e.removeSelected();
            else if (id === "group")
                e.groupSelected();
            else if (id === "ungroup")
                e.ungroupSelected();
            else if (id === "front" || id === "back")
                Actions.reorder(e, id === "front");
            else if (id === "transform")
                e.tool = "transform";
            else if (id === "flip-h" || id === "flip-v")
                e.mirrorSelected(id === "flip-h");
            else if (id === "anchors")
                e.tool = "anchors";
            else if (id === "add-node")
                e.addNode(clickX, clickY);
            else if (id === "remove-node")
                e.deletePointAt(clickX, clickY);
            else if (id === "smooth" || id === "corner")
                Actions.curvature(e, hit, id === "smooth");
            else if (id === "unlink")
                Actions.unlink(e, Actions.targets(e, hit, anchor));
            else if (id === "link") {
                e.contextLinkTargets = Actions.targets(e, hit, anchor);
                e.status = qsTrc("drawing-tools", "Click an anchor or a drawing node to link.");
            }
        } finally {
            e.gestureFinished();
        }
    }
    Item {
        id: menuAnchor
        parent: root.Window.window ? root.Window.window.contentItem : root.editor
        width: 0
        height: 0
    }
    ContextMenuLoader {
        id: menu
        parent: menuAnchor
        onHandleMenuItem: function (itemId) {
            root.run(itemId);
        }
    }
    ShapeEditPopup {
        id: editPopup
        editor: root.editor
        parent: menuAnchor.parent
        onFinishRequested: function (apply) {
            root.finishEdit(apply);
        }
    }
    property int renameIndex: -1
    StyledPopupView {
        id: renamePopup
        parent: menuAnchor
        x: 0
        y: 0
        contentWidth: 260
        contentHeight: 80
        Column {
            width: parent.width
            spacing: 10
            Rectangle {
                width: parent.width
                height: 30
                color: ui.theme.backgroundPrimaryColor
                border.color: ui.theme.strokeColor
                TextInput {
                    id: renameInput
                    anchors.fill: parent
                    anchors.margins: 4
                    color: ui.theme.fontPrimaryColor
                    selectByMouse: true
                    onAccepted: {
                        root.editor.renameObject(root.renameIndex, text);
                        renamePopup.close();
                    }
                    Keys.onEscapePressed: renamePopup.close()
                }
            }
            FlatButton {
                text: qsTr("Rename")
                onClicked: {
                    root.editor.renameObject(root.renameIndex, renameInput.text);
                    renamePopup.close();
                }
            }
        }
    }
}
