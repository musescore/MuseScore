import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.UiComponents 1.0
import MuseScore.Extensions 1.0
import MuseScore.Languages 1.0
import MuseScore.Plugins 1.0

FocusScope {
    id: root

    property var color: ui.theme.backgroundSecondaryColor
    property string item: ""

    onItemChanged: {
        if (!Boolean(root.item)) {
            return
        }

        bar.selectPage(root.item)
    }

    Rectangle {
        anchors.fill: parent
        color: root.color

        MouseArea {
            anchors.fill: parent
            onClicked: {
                forceActiveFocus()
            }
        }
    }

    RowLayout {
        id: topLayout
        anchors.top: parent.top
        anchors.topMargin: 66
        anchors.left: parent.left
        anchors.right: parent.right

        spacing: 12

        StyledTextLabel {
            id: addonsLabel

            Layout.leftMargin: 133
            Layout.alignment: Qt.AlignLeft

            font: ui.theme.titleBoldFont

            text: qsTrc("appshell", "Add-ons")
        }

        Row {
            Layout.alignment: Qt.AlignHCenter

            spacing: 12

            SearchField {
                id: searchField

                onSearchTextChanged: {
                    categoryComboBox.selectedCategory = ""
                }
            }

            StyledComboBox {
                id: categoryComboBox

                width: searchField.width

                textRoleName: "text"
                valueRoleName: "value"

                visible: bar.canFilterByCategories

                property string selectedCategory: Boolean(value) ? value : ""

                displayText: qsTrc("appshell", "Category: ") + currentText

                function initModel() {
                    var categories = bar.categories()
                    var result = []

                    result.push({ "text": qsTrc("appshell", "All"), "value": "" })

                    for (var i = 0; i < categories.length; ++i) {
                        var category = categories[i]
                        result.push({ "text": category, "value": category })
                    }

                    model = result
                }

                Component.onCompleted: {
                    initModel()
                }
            }
        }

        Item {
            Layout.preferredWidth: addonsLabel.width
            Layout.rightMargin: 133
        }
    }

    TabBar {
        id: bar

        anchors.top: topLayout.bottom
        anchors.topMargin: 54
        anchors.horizontalCenter: parent.horizontalCenter

        contentHeight: 28
        spacing: 0

        property bool canFilterByCategories: bar.currentIndex === 0 || bar.currentIndex === 1

        function categories() {
            var result = []

            if (bar.currentIndex === 0) {
                result = pluginsComp.categories()
            }

            return result
        }

        function pageIndex(pageName) {
            switch (pageName) {
            case "plugins": return 0
            case "extensions": return 1
            case "languages": return 2
            }

            return 0
        }

        function selectPage(pageName) {
            currentIndex = pageIndex(pageName)
        }

        StyledTabButton {
            text: qsTrc("appshell", "Plugins")
            sideMargin: 22
            isCurrent: bar.currentIndex === 0
            backgroundColor: root.color
        }
        StyledTabButton {
            text: qsTrc("appshell", "Extensions")
            sideMargin: 22
            isCurrent: bar.currentIndex === 1
            backgroundColor: root.color
        }
        StyledTabButton {
            text: qsTrc("appshell", "Languages")
            sideMargin: 22
            isCurrent: bar.currentIndex === 2
            backgroundColor: root.color
        }
    }

    StackLayout {
        anchors.top: bar.bottom
        anchors.topMargin: 24
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        currentIndex: bar.currentIndex

        PluginsPage {
            id: pluginsComp

            search: searchField.searchText
            selectedCategory: categoryComboBox.selectedCategory
            backgroundColor: root.color
        }

        ExtensionsPage {
            id: extensionsComp

            search: searchField.searchText
            backgroundColor: root.color
        }

        LanguagesPage {
            id: languagesComp

            search: searchField.searchText
            backgroundColor: root.color
        }
    }
}

