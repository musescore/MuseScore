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
        property var pagesComponents: (new Map())
        property bool inited: false
    }

    Rectangle {
        id: content

        anchors.fill: parent

        color: ui.theme.backgroundSecondaryColor

        Component.onCompleted: {
            preferencesModel.load(root.currentPageId)

            createPagesComponents()

            root.privatesProperties.inited = true
        }

        function createPagesComponents() {
            var pages = preferencesModel.availablePages()
            for (var i in pages) {
                var pageInfo = pages[i]

                var pagePath = Boolean(pageInfo.path) ? pageInfo.path : "Preferences/StubPreferencesPage.qml"
                var pageComponent = Qt.createComponent("../" + pagePath);

                root.privatesProperties.pagesComponents[pageInfo.id] = pageComponent
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

                Loader {
                    id: loader

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.margins: 30

                    sourceComponent: Boolean(root.privatesProperties.inited) ?
                                         root.privatesProperties.pagesComponents[preferencesModel.currentPageId] : null

                    onLoaded: {
                        if (!Boolean(loader.item.hideRequested)) {
                            return
                        }

                        loader.item.hideRequested.connect(function() {
                            root.hide()
                        })
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
                    root.reject()
                }

                onApplyRequested: {
                    if (preferencesModel.apply()) {
                        root.hide()
                    }
                }
            }
        }
    }
}
