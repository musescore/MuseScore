import QtQuick 2.7
import QtQuick.Layouts 1.3

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.UserScores 1.0

Item {
    TemplatesModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16

        spacing: 16

        TitleListView {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width / 4

            listTitle: qsTrc("userscores", "Category")
            model: model.categoriesTitles
            searchEnabled: false
            booldFont: true

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

        SeparatorLine { orientation: Qt.Vertical }

        Item { Layout.preferredWidth: 30 }

        SeparatorLine { orientation: Qt.Vertical }

        TemplatePreview {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width / 3
        }

        SeparatorLine { orientation: Qt.Vertical }

        Column {
            Layout.minimumWidth: 30
            Layout.alignment: Qt.AlignVCenter

            spacing: 16

            FlatButton {
                icon: IconCode.ZOOM_IN
            }

            FlatButton {
                icon: IconCode.ZOOM_OUT
            }
        }
    }
}
