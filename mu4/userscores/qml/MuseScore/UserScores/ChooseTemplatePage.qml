import QtQuick 2.7
import QtQuick.Layouts 1.3

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.UserScores 1.0

Item {
    property alias selectedTemplatePath: model.currentTemplatePath

    TemplatesModel {
        id: model

        onCurrentTemplateChanged: {
            templatePreview.load(model.currentTemplatePath)
        }
    }

    Component.onCompleted: {
        model.load()
    }

    RowLayout {
        id: row

        anchors.fill: parent

        spacing: 12

        TitleListView {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width / 4

            listTitle: qsTrc("userscores", "Category")
            model: model.categoriesTitles
            searchEnabled: false
            boldFont: true

            onTitleClicked: {
                model.setCurrentCategory(index)
            }
        }

        SeparatorLine { orientation: Qt.Vertical }

        TitleListView {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width / 4

            listTitle: qsTrc("userscores", "Template")
            model: model.templatesTitles

            onTitleClicked: {
                model.setCurrentTemplate(index)
            }

            onSearchTextChanged: {
                model.setSearchText(searchText)
            }
        }

        SeparatorLine { orientation: Qt.Vertical; Layout.rightMargin: 30 + row.spacing }

        SeparatorLine { orientation: Qt.Vertical }

        TemplatePreview {
            id: templatePreview

            Layout.fillHeight: true
            Layout.fillWidth: true
        }

        SeparatorLine { orientation: Qt.Vertical }

        Column {
            Layout.preferredWidth: 30
            Layout.alignment: Qt.AlignVCenter

            spacing: 12

            FlatButton {
                icon: IconCode.ZOOM_IN

                onClicked: {
                    templatePreview.zoomIn()
                }
            }

            FlatButton {
                icon: IconCode.ZOOM_OUT

                onClicked: {
                    templatePreview.zoomOut()
                }
            }
        }
    }
}
