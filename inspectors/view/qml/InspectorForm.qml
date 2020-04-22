import QtQuick 2.9
import QtQml.Models 2.3
import MuseScore.Inspectors 3.3

import "common"
import "general"
import "notation"

FocusableItem {
    id: root

    property alias inspectorListModel: presenterModel.model

    Rectangle {
        id: backgroundRect

        anchors.fill: parent

        color: globalStyle.window
    }

    Column {
        id: tabTitleColumn

        anchors.left: parent.left
        anchors.leftMargin: 24

        spacing: 4

        StyledTextLabel {
            id: inspectorTitle

            text: qsTr("Inspector")
            font.bold: true
            font.pixelSize: globalStyle.font.pixelSize * 1.2
        }

        Rectangle {
            id: titleHighlighting

            height: 3
            width: inspectorTitle.width

            color: globalStyle.voice1Color

            radius: 2
        }
    }

    DelegateModel {
        id: presenterModel

        model: root.inspectorListModel

        delegate: ExpandableBlank {

            function viewBySectionType() {

                switch (inspectorData.sectionType) {
                case Inspector.SECTION_GENERAL: return generalInspector
                case Inspector.SECTION_NOTATION: return notationInspector
                }
            }

            contentItemComponent: viewBySectionType()

            menuItemComponent: InspectorMenu {
                onResetToDefaultsRequested: {
                    inspectorData.requestResetToDefaults()
                }
            }

            Component.onCompleted: {
                title = inspectorData.title
            }

            Component { id: generalInspector; GeneralInspectorView { model: inspectorData } }
            Component { id: notationInspector; NotationInspectorView { model: inspectorData } }
        }
    }

    ListView {
        id: inspectorListView

        anchors.top: tabTitleColumn.bottom
        anchors.topMargin: 12
        anchors.left: parent.left
        anchors.leftMargin: 24

        height: parent.height
        width: parent.width

        model: presenterModel
    }

    FocusableItem {
        id: focusChainBreak

        onActiveFocusChanged: {
            parent.focus = false
        }
    }
}
