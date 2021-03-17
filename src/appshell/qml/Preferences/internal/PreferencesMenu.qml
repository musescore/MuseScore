import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Item {
    id: root

    width: 220
    height: parent.height

    property alias model: treeView.model

    signal menuSelected(string id)

    Rectangle {
        id: background
        anchors.fill: parent
        color: ui.theme.backgroundPrimaryColor
    }

    ColumnLayout {
        anchors.fill: parent

        SearchField {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.margins: 12
        }

        TreeView {
            id: treeView
            Layout.fillWidth: true
            Layout.fillHeight: true

            alternatingRowColors: false
            headerVisible: false

            TableViewColumn {
                role: "itemRole"
            }

            style: TreeViewStyle {
                indentation: 0

                frame: Item {}
                incrementControl: Item {}
                decrementControl: Item {}
                handle: Item {}
                scrollBarBackground: Item {}
                branchDelegate: Item {}

                backgroundColor: background.color

                rowDelegate: Rectangle {
                    id: rowTreeDelegate

                    height: 36
                    width: parent.width
                    color: background.color
                }
            }

            itemDelegate: GradientTabButton {
                orientation: Qt.Horizontal

                spacing: 16
                leftPadding: spacing * styleData.depth

                normalStateFont: ui.theme.bodyFont
                selectedStateFont: ui.theme.bodyBoldFont

                title: Boolean(model) ? model.itemRole.title : ""
                checked: Boolean(model) && model.itemRole.id === treeView.model.currentMenuId

                iconComponent: StyledIconLabel {
                    width: 24
                    height: width
                    iconCode: Boolean(model) ? model.itemRole.icon : IconCode.NONE
                }

                Component.onCompleted: {
                    treeView.expand(styleData.index)
                }

                onClicked: {
                    treeView.model.currentMenuId = model.itemRole.id
                }
            }
        }
    }
}
