import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.UiComponents 1.0
import MuseScore.Languages 1.0

import "internal"

Item {
    id: root

    property string search: ""
    property string backgroundColor: ui.theme.backgroundPrimaryColor

    QtObject {
        id: privateProperties

        property var selectedLanguage: undefined
        property int sideMargin: 133

        function resetSelectedLanguage() {
            selectedLanguage = undefined
        }
    }

    onSearchChanged: {
        panel.close()
    }

    Component.onCompleted: {
        languageListModel.init()
    }

    LanguageListModel {
        id: languageListModel

        onProgress: {
            if (privateProperties.selectedLanguage.code !== languageCode) {
                return
            }

            panel.setProgress(status, indeterminate, current, total)
        }
        onFinish: {
            if (privateProperties.selectedLanguage.code !== item.code) {
                return
            }

            privateProperties.selectedLanguage = item
            panel.resetProgress()
        }
    }

    SortFilterProxyModel {
        id: filterModel

        sourceModel: languageListModel

        filters: [
            FilterValue {
                roleName: "name"
                roleValue: root.search
                compareType: CompareType.Contains
            }
        ]
    }

    Row {
        id: header

        property real itemWidth: parent.width / 2

        anchors.top: parent.top
        anchors.topMargin: 16
        anchors.left: parent.left
        anchors.leftMargin: privateProperties.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: privateProperties.sideMargin

        height: 48

        StyledTextLabel {
            width: header.itemWidth

            text: qsTrc("languages", "LANGUAGES")
            horizontalAlignment: Text.AlignLeft
            opacity: 0.5
            font.capitalization: Font.AllUppercase
        }

        StyledTextLabel {
            width: header.itemWidth

            text: qsTrc("languages", "STATUS")
            horizontalAlignment: Text.AlignLeft
            opacity: 0.5
            font.capitalization: Font.AllUppercase
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: view.top

        height: 8
        z: 1

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: root.backgroundColor
            }
            GradientStop {
                position: 1.0
                color: "transparent"
            }
        }
    }

    ListView {
        id: view

        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: panel.visible ? panel.top : parent.bottom

        model: filterModel

        clip: true

        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: StyledScrollBar {
            parent: view.parent

            anchors.top: parent.top
            anchors.bottom: panel.visible ? panel.top : parent.bottom
            anchors.right: parent.right
            anchors.rightMargin: 16

            visible: view.contentHeight > view.height
            z: 1
        }

        delegate: LanguageItem {
            width: view.width

            title: model.name
            statusTitle: model.statusTitle

            color: (index % 2 == 0) ? ui.theme.popupBackgroundColor
                                    : root.backgroundColor

            headerWidth: header.itemWidth
            sideMargin: 133

            onClicked: {
                forceActiveFocus()

                privateProperties.selectedLanguage = languageListModel.language(model.code)
                panel.open()
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: view.bottom

        height: 8
        z: 1

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: "transparent"
            }
            GradientStop {
                position: 1.0
                color: root.backgroundColor
            }
        }
    }

    InstallationPanel {
        id: panel

        property alias selectedLanguage: privateProperties.selectedLanguage

        height: 206

        title: Boolean(selectedLanguage) ? selectedLanguage.name : ""
        installed: Boolean(selectedLanguage) ? (selectedLanguage.status === LanguageStatus.Installed) : false
        hasUpdate: Boolean(selectedLanguage) ? (selectedLanguage.status === LanguageStatus.NeedUpdate) : false
        neutralButtonTitle: qsTrc("languages", "Open language preferences")
        background: view

        onSelectedLanguageChanged: {
            panel.resetProgress()
        }

        onInstallRequested: {
            Qt.callLater(languageListModel.install, selectedLanguage.code)
        }

        onUninstallRequested: {
            Qt.callLater(languageListModel.uninstall, selectedLanguage.code)
        }

        onUpdateRequested: {
            Qt.callLater(languageListModel.update, selectedLanguage.code)
        }

        onRestartRequested: {
            Qt.callLater(languageListModel.restart, selectedLanguage.code)
        }

        onNeutralButtonClicked: {
            Qt.callLater(languageListModel.openPreferences)
        }

        onClosed: {
            privateProperties.resetSelectedLanguage()
        }
    }
}
