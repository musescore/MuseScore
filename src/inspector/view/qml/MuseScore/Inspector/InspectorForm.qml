import QtQuick 2.9
import QtQml.Models 2.3
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

import "common"
import "general"
import "notation"
import "text"
import "score"

FocusableItem {
    id: root

    property alias model: inspectorRepeater.model

    Rectangle {
        id: backgroundRect

        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor
    }

    InspectorListModel {
        id: inspectorListModel
    }

    Flickable {
        id: flickableArea

        anchors.top: tabTitleColumn.bottom
        anchors.topMargin: 12
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 24

        width: parent.width

        contentWidth: contentItem.childrenRect.width

        function updateContentHeight() {
            var resultContentHeight = 0

            for (var i = 0; i < inspectorRepeater.count; ++i) {
                resultContentHeight += inspectorRepeater.itemAt(i).contentHeight
            }

            flickableArea.contentHeight = resultContentHeight
        }

        function ensureContentVisible(delegateY, delegateContentHeight) {

            var contentBottomY = delegateY + delegateContentHeight

            if (contentBottomY > flickableArea.height) {
                flickableArea.contentY = contentBottomY - flickableArea.height
            } else {
                flickableArea.contentY = 0
            }
        }

        Behavior on contentY {
            NumberAnimation { duration: 250 }
        }

        Column {
            width: root.width

            spacing: 6

            Repeater {
                id: inspectorRepeater

                model: inspectorListModel

                delegate: ExpandableBlank {
                    id: expandableDelegate

                    property var contentHeight: implicitHeight

                    function viewBySectionType() {
                        flickableArea.contentY = 0

                        switch (inspectorData.sectionType) {
                        case Inspector.SECTION_GENERAL: return generalInspector
                        case Inspector.SECTION_TEXT: return textInspector
                        case Inspector.SECTION_NOTATION: return notationInspector
                        case Inspector.SECTION_SCORE_DISPLAY: return scoreInspector
                        case Inspector.SECTION_SCORE_APPEARANCE: return scoreAppearanceInspector
                        }
                    }

                    contentItemComponent: viewBySectionType()

                    Component.onCompleted: {
                        title = inspectorData.title
                    }

                    function updateContentHeight(newContentHeight) {
                        expandableDelegate.contentHeight = newContentHeight
                        flickableArea.updateContentHeight()
                        flickableArea.ensureContentVisible(y, newContentHeight)
                    }

                    Component {
                        id: generalInspector
                        GeneralInspectorView {
                            model: inspectorData
                            onContentExtended: expandableDelegate.updateContentHeight(contentHeight)
                        }
                    }
                    Component {
                        id: textInspector
                        TextInspectorView {
                            model: inspectorData
                            onContentExtended: expandableDelegate.updateContentHeight(contentHeight)
                        }
                    }
                    Component {
                        id: notationInspector
                        NotationInspectorView {
                            model: inspectorData
                            onContentExtended: expandableDelegate.updateContentHeight(contentHeight)
                        }
                    }
                    Component {
                        id: scoreInspector

                        ScoreDisplayInspectorView {
                            model: inspectorData
                            onContentExtended: expandableDelegate.updateContentHeight(contentHeight)
                        }
                    }
                    Component {
                        id: scoreAppearanceInspector

                        ScoreAppearanceInspectorView {
                            model: inspectorData
                            onContentExtended: expandableDelegate.updateContentHeight(contentHeight)
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: tabTitleBackgroundRect

        height: tabTitleColumn.height + 12
        width: parent.width

        color: ui.theme.backgroundPrimaryColor
    }

    Column {
        id: tabTitleColumn

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: 24

        spacing: 4

        StyledTextLabel {
            id: inspectorTitle

            text: qsTrc("inspector", "Inspector")
            font: ui.theme.largeBodyBoldFont
        }

        Rectangle {
            id: titleHighlighting

            height: 3
            width: inspectorTitle.width

            color: ui.theme.accentColor

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
