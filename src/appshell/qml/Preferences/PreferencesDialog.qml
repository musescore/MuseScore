import QtQuick 2.15
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

import "internal"

QmlDialog {
    id: root

    width: 880
    height: 600

    title: qsTrc("appshell", "Preferences")

    property string currentPageId: ""

    property QtObject privatesProperties: QtObject {
        property var pagesObjects: (new Map())
        property bool inited: false
    }

    Rectangle {
        id: content

        anchors.fill: parent

        color: ui.theme.backgroundSecondaryColor

        Component.onCompleted: {
            preferencesModel.load(root.currentPageId)

            initPagesObjects()

            root.privatesProperties.inited = true
        }

        function initPagesObjects() {
            var pages = preferencesModel.availablePages()
            for (var i in pages) {
                var pageInfo = pages[i]

                var pagePath = Boolean(pageInfo.path) ? pageInfo.path : "Preferences/StubPreferencesPage.qml"
                var pageComponent = Qt.createComponent("../" + pagePath)

                var obj = pageComponent.createObject(stack)

                if (!Boolean(obj)) {
                    continue
                }

                obj.hideRequested.connect(function() {
                    root.hide()
                })

                root.privatesProperties.pagesObjects[pageInfo.id] = obj
            }
        }

        PreferencesModel {
            id: preferencesModel
        }

        SeparatorLine { id: topSeparator; anchors.top: parent.top }

        ColumnLayout {
            anchors.fill: parent

            spacing: 0

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true

                spacing: 0

                PreferencesMenu {
                    id: menu

                    Layout.fillHeight: true
                    Layout.preferredWidth: 220

                    model: preferencesModel
                }

                SeparatorLine { orientation: Qt.Vertical }

                StackLayout {
                    id: stack

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.margins: 30

                    currentIndex: {
                        var keys = Object.keys(root.privatesProperties.pagesObjects)
                        return keys.indexOf(preferencesModel.currentPageId)
                    }
                }
            }

            SeparatorLine { }

            PreferencesButtonsPanel {
                id: buttonsPanel

                Layout.fillWidth: true
                Layout.preferredHeight: 70

                onRevertFactorySettingsRequested: {
                    preferencesModel.resetFactorySettings()
                }

                onRejectRequested: {
                    preferencesModel.cancel()
                    root.reject()
                }

                onApplyRequested: {
                    preferencesModel.apply()

                    var ok = true
                    var pages = preferencesModel.availablePages()

                    for (var i in pages) {
                        var page = pages[i]
                        var obj = root.privatesProperties.pagesObjects[page.id]
                        ok &= obj.apply()
                    }

                    if (ok) {
                        root.hide()
                    }
                }
            }
        }
    }
}
