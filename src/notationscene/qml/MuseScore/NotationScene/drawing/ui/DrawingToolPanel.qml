// SPDX-License-Identifier: GPL-3.0-only
import QtQuick
import QtQuick.Layouts
import Muse.Ui
import Muse.UiComponents
import "../.."

Item {
    id: root
    property var toolbox
    property var editor
    property string group: ""
    property bool pinned: false
    property bool disposed: false
    property real panelWidth: 310
    property real panelHeight: Math.max(180, Math.min(600, toolbox.availableHeight - 80))
    property alias settings: panelSettings
    readonly property bool isOpened: popup.isOpened
    width: 0
    height: 0
    function open() {
        popup.open();
    }
    function close() {
        popup.close();
    }
    function updatePosition() {
        if (!disposed)
            toolbox.placePanel(root);
    }
    StyledPopupView {
        id: popup
        parent: root
        x: 0
        y: 0
        showArrow: false
        padding: 12
        contentWidth: root.panelWidth
        contentHeight: root.panelHeight
        closeOnEscape: false
        closePolicies: root.pinned ? PopupView.NoAutoClose : PopupView.CloseOnPressOutsideParent
        onClosed: {
            panelSettings.sliderBusy = false;
            panelSettings.sliderPointerPreview = false;
            root.toolbox.panelClosed(root);
        }
        ColumnLayout {
            width: popup.contentWidth
            height: popup.contentHeight
            spacing: 8
            RowLayout {
                Layout.fillWidth: true
                StyledTextLabel {
                    Layout.fillWidth: true
                    text: root.toolbox.titleFor(root.group)
                    horizontalAlignment: Text.AlignLeft
                    elide: Text.ElideRight
                }
                FlatButton {
                    width: 30
                    height: 30
                    accentButton: root.pinned
                    toolTipTitle: root.pinned ? qsTr("Unpin panel") : qsTr("Pin panel")
                    margins: 4
                    contentItem: Component {
                        Canvas {
                            id: pinImage
                            implicitWidth: 20
                            implicitHeight: 20
                            width: implicitWidth
                            height: implicitHeight
                            rotation: root.pinned ? 0 : -35
                            property color glyphColor: ui.theme.fontPrimaryColor
                            onGlyphColorChanged: requestPaint()
                            onPaint: {
                                var c = getContext("2d");
                                c.reset();
                                c.strokeStyle = glyphColor;
                                c.lineWidth = 1.5;
                                c.lineCap = "round";
                                c.lineJoin = "round";
                                c.beginPath();
                                c.moveTo(6, 3);
                                c.lineTo(14, 3);
                                c.moveTo(7, 3);
                                c.lineTo(7, 8);
                                c.lineTo(4, 12);
                                c.lineTo(16, 12);
                                c.lineTo(13, 8);
                                c.lineTo(13, 3);
                                c.moveTo(10, 12);
                                c.lineTo(10, 18);
                                c.stroke();
                            }
                        }
                    }
                    onClicked: root.pinned = !root.pinned
                }
                FlatButton {
                    width: 30
                    height: 30
                    icon: IconCode.CROSS
                    toolTipTitle: qsTr("Close")
                    onClicked: root.close()
                }
            }
            Flow {
                Layout.fillWidth: true
                spacing: 4
                Repeater {
                    model: root.toolbox.toolGroups[root.group] || []
                    delegate: FlatButton {
                        required property var modelData
                        text: modelData.label
                        accentButton: root.editor && root.editor.tool === modelData.key
                        onClicked: {
                            if (modelData.key === "group")
                                root.editor.groupSelected();
                            else if (modelData.key === "ungroup")
                                root.editor.ungroupSelected();
                            else if (modelData.key === "fuse")
                                root.editor.mergeSelected();
                            else
                                root.toolbox.selectTool(modelData.key);
                        }
                    }
                }
            }
            StyledFlickable {
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: width
                contentHeight: body.implicitHeight
                clip: true
                Column {
                    id: body
                    width: parent.width - 12
                    spacing: 8
                    DrawingRepeatControls {
                        width: parent.width
                        editor: root.editor
                        visible: root.group === "repeat"
                    }
                    NativeDrawingSettings {
                        id: panelSettings
                        width: parent.width
                        editor: root.editor
                        group: root.group
                        panelOpen: popup.isOpened
                        visible: root.group !== "repeat"
                        navigationSection: popup.navigationSection
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                visible: root.group === "repeat"
                FlatButton {
                    Layout.fillWidth: true
                    icon: IconCode.TICK_RIGHT_ANGLE
                    text: qsTr("Confirm")
                    enabled: !!(root.editor && root.editor.repeatSession && root.editor.repeatSession.sourceId)
                    onClicked: root.editor.finishRepeatEdit(true)
                }
                FlatButton {
                    Layout.fillWidth: true
                    icon: IconCode.CROSS
                    text: qsTr("Cancel")
                    enabled: !!(root.editor && root.editor.repeatEditActive)
                    onClicked: root.editor.finishRepeatEdit(false)
                }
            }
        }
    }
}
