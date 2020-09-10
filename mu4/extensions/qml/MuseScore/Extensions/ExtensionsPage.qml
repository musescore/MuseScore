import QtQuick 2.9

import MuseScore.UiComponents 1.0
import MuseScore.Extensions 1.0

Item {
    id: root

    property string search: ""

    Component.onCompleted: {
        extensionListModel.load()
    }

    QtObject {
        id: privateProperities

        property string selectedExtensionViewType: "undefined" // "installed" "notinstalled"
        property int selectedExtensionIndex: -1
        property var selectedExtension: undefined

        function resetSelectedExtension() {
            selectedExtensionIndex = -1
            selectedExtensionViewType = "undefined"
            selectedExtension = undefined
        }
    }

    ExtensionListModel {
        id: extensionListModel

        onProgress: {
            extensionPanel.setProgress(status, indeterminate, current, total)
        }
        onFinish: {
            extensionPanel.resetProgress()
            extensionPanel.hide()
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: flickable.top

        height: 8
        z: 1

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: ui.theme.backgroundPrimaryColor
            }
            GradientStop {
                position: 1.0
                color: "transparent"
            }
        }
    }

    Flickable {
        id: flickable

        anchors.top: parent.top
        anchors.topMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 133
        anchors.right: parent.right
        anchors.rightMargin: 133
        anchors.bottom: extensionPanel.visible ? extensionPanel.top : parent.bottom

        clip: true

        contentWidth: width
        contentHeight: extensionsColumn.height
        interactive: height < contentHeight

        Column {
            id: extensionsColumn

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            spacing: 20

            Rectangle {
                height: installedLabel.height + installedView.height + 6
                width: parent.width

                color: ui.theme.backgroundPrimaryColor

                visible: installedView.count > 0

                StyledTextLabel {
                    id: installedLabel
                    height: 18
                    font.bold: true
                    text: qsTrc("extensions", "Installed")
                }

                ExtensionsListView {
                    id: installedView

                    anchors.top: installedLabel.bottom
                    anchors.topMargin: 12

                    anchors.left: parent.left
                    anchors.leftMargin: -24
                    anchors.right: parent.right
                    anchors.rightMargin: -24

                    model: extensionListModel

                    selectedIndex: {
                        return privateProperities.selectedExtensionViewType === "installed" ?
                                    privateProperities.selectedExtensionIndex : -1
                    }

                    filters: [
                        FilterValue {
                            roleName: "name"
                            roleValue: root.search
                            compareType: CompareType.Contains
                        },
                        FilterValue {
                            roleName: "status"
                            roleValue: ExtensionStatus.Installed
                            compareType: CompareType.Equal
                        }
                    ]

                    onClicked: {
                        privateProperities.selectedExtensionViewType = "installed"
                        privateProperities.selectedExtensionIndex = index
                        privateProperities.selectedExtension = extension

                        extensionPanel.show()
                    }
                }
            }

            Rectangle {
                height: notInstalledLabel.height + notInstalledView.height + 6
                width: parent.width

                color: ui.theme.backgroundPrimaryColor

                visible: notInstalledView.count > 0

                StyledTextLabel {
                    id: notInstalledLabel
                    height: 18
                    font.bold: true
                    text: qsTrc("extensions", "Not Installed")
                }

                ExtensionsListView {
                    id: notInstalledView

                    anchors.top: notInstalledLabel.bottom
                    anchors.topMargin: 12

                    anchors.left: parent.left
                    anchors.leftMargin: -24
                    anchors.right: parent.right
                    anchors.rightMargin: -24

                    model: extensionListModel

                    selectedIndex: {
                        return privateProperities.selectedExtensionViewType === "notinstalled" ?
                                    privateProperities.selectedExtensionIndex : -1
                    }

                    filters: [
                        FilterValue {
                            roleName: "name"
                            roleValue: root.search
                            compareType: CompareType.Contains
                        },
                        FilterValue {
                            roleName: "status"
                            roleValue: ExtensionStatus.NoInstalled
                            compareType: CompareType.Equal
                        }
                    ]

                    onClicked: {
                        privateProperities.selectedExtensionViewType = "notinstalled"
                        privateProperities.selectedExtensionIndex = index
                        privateProperities.selectedExtension = extension

                        extensionPanel.show()
                    }
                }
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: flickable.bottom

        height: 8
        z:1

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: "transparent"
            }
            GradientStop {
                position: 1.0
                color: ui.theme.backgroundPrimaryColor
            }
        }
    }

    onSearchChanged: {
        extensionPanel.hide()
    }

    InstallationPanel {
        id: extensionPanel

        anchors.bottom: parent.bottom

        property alias selectedExtension: privateProperities.selectedExtension

        title: Boolean(selectedExtension) ? selectedExtension.name : ""
        description: Boolean(selectedExtension) ? selectedExtension.description : ""
        installed: Boolean(selectedExtension) ? (selectedExtension.status === ExtensionStatus.Installed) : false
        hasUpdate: Boolean(selectedExtension) ? (selectedExtension.status === ExtensionStatus.NeedUpdate) : false

        onInstallRequested: {
            Qt.callLater(extensionListModel.install, selectedExtension.code)
        }

        onUpdateRequested: {
            Qt.callLater(extensionListModel.update, selectedExtension.code)
        }

        onUninstallRequested: {
            Qt.callLater(extensionListModel.uninstall, selectedExtension.code)
        }

        onOpenFullDescriptionRequested: {
            // TODO: implement after getting the link of extension
            // Qt.callLater(extensionListModel.openFullDescription, code)
        }

        onClosed: {
            privateProperities.resetSelectedExtension()
        }
    }
}
