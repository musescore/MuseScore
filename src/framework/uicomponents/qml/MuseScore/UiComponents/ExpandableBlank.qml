import QtQuick 2.9
import QtGraphicalEffects 1.0
import MuseScore.UiComponents 1.0

FocusableItem {
    id: root

    property alias contentItemComponent: contentLoader.sourceComponent
    property alias menuItemComponent: expandableSection.menuItemComponent

    property alias icon: expandableSection.icon
    property alias iconPixelSize: expandableSection.iconPixelSize

    property alias title: expandableSection.title

    property alias isExpanded: expandableSection.isExpanded

    implicitHeight: contentColumn.height
    implicitWidth: parent.width

    Keys.onSpacePressed: {
        root.isExpanded = !root.isExpanded
    }

    Rectangle {
        id: backgroundRect

        height: contentColumn.height
        width: root.width

        color: ui.theme.backgroundPrimaryColor
    }

    Column {
        id: contentColumn

        height: childrenRect.height
        width: root.width

        spacing: 12

        ExpandableBlankSection {
            id: expandableSection
        }

        Loader {
            id: contentLoader

            property alias yScale: scalingFactor.yScale
            property int contentHorizontalPadding: 4

            function getContentHeight() {
                return implicitHeight === 0 ? implicitHeight : implicitHeight + contentHorizontalPadding * 2
            }

            height: getContentHeight() * yScale
            width: root.width

            enabled: root.isExpanded

            opacity: 0

            transform: Scale {
                id: scalingFactor

                yScale: 1
            }
        }
    }

    states: [
        State {
            name: "EXPANDED"
            when: root.isExpanded

            PropertyChanges { target: contentLoader; opacity: 1.0
                                                     yScale: 1 }
        },

        State {
            name: "COLLAPSED"
            when: !root.isExpanded

            PropertyChanges { target: contentLoader; opacity: 0.0
                                                     yScale: 0 }
        }
    ]

    transitions: Transition {
        NumberAnimation {
            properties: "opacity, yScale"
            duration: 100
        }
    }
}
