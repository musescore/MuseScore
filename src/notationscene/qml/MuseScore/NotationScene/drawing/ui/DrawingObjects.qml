// SPDX-License-Identifier: GPL-3.0-only
import QtQuick
import Muse.Ui
import Muse.UiComponents
import "../core/Context.js" as Actions

Column {
    id: root
    property var editor
    width: parent.width
    spacing: 1
    property var collapsed: ({})
    property var draggedIds: []
    property int dropIndex: -1
    property string dropMode: ""
    property int editingIndex: -1
    property bool editingGroup: false
    readonly property var rows: {
        var out = [];
        if (!editor || !visible)
            return out;
        Actions.rows(editor).forEach(function (row) {
            out.push(row);
            if (row.group && !collapsed[row.id])
                row.children.forEach(function (child) {
                    out.push(Object.assign({}, child, {
                        parentGroup: row.id,
                        indent: 32
                    }));
                });
        });
        return out;
    }
    function commitName(text) {
        if (editingIndex < 0)
            return;
        var index = editingIndex, group = editingGroup;
        editingIndex = -1;
        if (group)
            editor.renameGroup(editor.shapes[index].selectionGroup, text);
        else
            editor.renameObject(index, text);
    }
    function dropAt(x, y) {
        dropIndex = -1;
        dropMode = "";
        for (var i = 0; i < rows.length; i++) {
            var item = items.itemAt(i);
            if (!item)
                continue;
            var p = item.mapFromItem(root, x, y);
            if (p.y >= 0 && p.y <= item.height) {
                dropIndex = i;
                dropMode = p.y < item.height * 0.25 ? "before" : p.y > item.height * 0.75 ? "after" : "inside";
                break;
            }
        }
    }
    Repeater {
        id: items
        model: root.rows
        delegate: Item {
            id: rowItem
            required property int index
            required property var modelData
            width: root.width
            height: 30
            Rectangle {
                anchors.fill: parent
                color: root.dropIndex === rowItem.index && root.dropMode === "inside" ? "#882675e6" : root.editor.selectionIndices().indexOf(rowItem.modelData.index === undefined ? rowItem.modelData.indices[0] : rowItem.modelData.index) >= 0 ? "#332675e6" : "transparent"
                radius: 2
            }
            Rectangle {
                x: 0
                width: parent.width
                height: 2
                y: root.dropMode === "after" ? parent.height - 1 : 0
                color: "#2675e6"
                visible: root.dropIndex === rowItem.index && root.dropMode !== "inside"
            }
            StyledIconLabel {
                x: 2
                y: 7
                visible: !!rowItem.modelData.group
                iconCode: root.collapsed[rowItem.modelData.id] ? IconCode.SMALL_ARROW_RIGHT : IconCode.SMALL_ARROW_DOWN
                width: 16
                height: 16
            }
            MouseArea {
                x: 0
                width: 20
                height: parent.height
                visible: !!rowItem.modelData.group
                onClicked: {
                    var v = Object.assign({}, root.collapsed);
                    v[rowItem.modelData.id] = !v[rowItem.modelData.id];
                    root.collapsed = v;
                }
            }
            StyledTextLabel {
                x: (rowItem.modelData.group ? 22 : 6) + (rowItem.modelData.indent || 0)
                height: parent.height
                width: parent.width - x - 4
                text: rowItem.modelData.title
                horizontalAlignment: Text.AlignLeft
                elide: Text.ElideRight
                visible: !nameInput.visible
            }
            TextInput {
                id: nameInput
                x: (rowItem.modelData.group ? 22 : 6) + (rowItem.modelData.indent || 0)
                height: parent.height
                width: parent.width - x - 4
                color: ui.theme.fontPrimaryColor
                selectByMouse: true
                verticalAlignment: TextInput.AlignVCenter
                visible: root.editingIndex === (rowItem.modelData.index === undefined ? rowItem.modelData.indices[0] : rowItem.modelData.index) && root.editingGroup === !!rowItem.modelData.group
                onVisibleChanged: if (visible) {
                    text = rowItem.modelData.title;
                    forceActiveFocus();
                    selectAll();
                }
                onAccepted: root.commitName(text)
                onEditingFinished: if (visible)
                    root.commitName(text)
                Keys.onEscapePressed: root.editingIndex = -1
            }
            MouseArea {
                x: rowItem.modelData.group ? 22 : 0
                width: parent.width - x
                height: parent.height
                visible: !nameInput.visible
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                property point start
                property bool moving: false
                onPressed: function (mouse) {
                    start = Qt.point(mouse.x, mouse.y);
                    moving = false;
                }
                onClicked: function (mouse) {
                    if (moving)
                        return;
                    var indices = rowItem.modelData.indices || [rowItem.modelData.index], primary = indices[0];
                    root.editor.setSelection(indices, primary, true);
                    if (mouse.button === Qt.RightButton) {
                        var p = mapToItem(root.editor, mouse.x, mouse.y);
                        root.editor.openObjectContext(primary, p.x, p.y);
                    }
                }
                onDoubleClicked: {
                    root.editingGroup = !!rowItem.modelData.group;
                    root.editingIndex = rowItem.modelData.index === undefined ? rowItem.modelData.indices[0] : rowItem.modelData.index;
                }
                onPositionChanged: function (mouse) {
                    if (!(mouse.buttons & Qt.LeftButton))
                        return;
                    if (!moving && Math.hypot(mouse.x - start.x, mouse.y - start.y) > 6) {
                        moving = true;
                        root.draggedIds = (rowItem.modelData.indices || [rowItem.modelData.index]).map(function (i) {
                            return root.editor.shapes[i].drawId;
                        });
                    }
                    if (moving) {
                        var p = mapToItem(root, mouse.x, mouse.y);
                        root.dropAt(p.x, p.y);
                    }
                }
                onReleased: {
                    if (moving && root.dropIndex >= 0)
                        Actions.moveObjects(root.editor, root.draggedIds, root.rows[root.dropIndex], root.dropMode);
                    root.dropIndex = -1;
                    root.draggedIds = [];
                }
                onCanceled: {
                    root.dropIndex = -1;
                    root.draggedIds = [];
                }
            }
        }
    }
}
