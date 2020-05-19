import QtQuick 2.9
import QtQml.Models 2.3
import MuseScore.Inspectors 3.3

import "common"
import "general"
import "notation"
import "text"

FocusableItem {
    id: root

    property alias inspectorListModel: inspectorListView.model

    Rectangle {
        id: backgroundRect

        anchors.fill: parent

        color: globalStyle.window
    }

    ListView {
        id: inspectorListView

        anchors.top: tabTitleColumn.bottom
        anchors.topMargin: 12
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 24

        width: parent.width
        clip: true
        interactive: true
        keyNavigationEnabled: true
        keyNavigationWraps: true

        delegate: ExpandableBlank {
            id: expandableDelegate

            function viewBySectionType() {

                switch (inspectorData.sectionType) {
                case Inspector.SECTION_GENERAL: return generalInspector
                case Inspector.SECTION_TEXT: return textInspector
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

            function ensureVisible(delegateContentHeight) {
                var contentBottomY = y + delegateContentHeight

                if (contentBottomY > inspectorListView.height) {
                    inspectorListView.contentY = contentBottomY - inspectorListView.height
                } else {
                    inspectorListView.contentY = 0
                }
            }

            Component {
                id: generalInspector
                GeneralInspectorView {
                    model: inspectorData
                    onContentExtended: expandableDelegate.ensureVisible(contentHeight)
                }
            }
            Component {
                id: textInspector
                TextInspectorView {
                    model: inspectorData
                    onContentExtended: expandableDelegate.ensureVisible(contentHeight)
                }
            }
            Component {
                id: notationInspector
                NotationInspectorView {
                    model: inspectorData
                    onContentExtended: expandableDelegate.ensureVisible(contentHeight)
                }
            }
        }

        spacing: 6

        Behavior on contentY {
            NumberAnimation { duration: 100 }
        }
    }

    Rectangle {
        id: tabTitleBackgroundRect

        height: tabTitleColumn.height + 12
        width: parent.width

        color: globalStyle.window
    }

    Column {
        id: tabTitleColumn

        anchors.top: parent.top
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

    FocusableItem {
        id: focusChainBreak

        onActiveFocusChanged: {
            parent.focus = false
        }
    }
}
