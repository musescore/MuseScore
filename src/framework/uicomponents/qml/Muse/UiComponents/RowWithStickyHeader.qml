import QtQuick 2.15
import Muse.Ui 1.0
import Muse.UiComponents 1.0

Item {
    id: root

    // Public properties
    property string headerTitle: ""
    property bool headerVisible: true
    property int headerWidth: 98
    property int headerHeight: implicitHeight - spacingAbove - spacingBelow
    property real spacingAbove: 4
    property real spacingBelow: 4
    property var model: undefined
    property Component delegateComponent

    // Sticky header functionality
    property Item parentFlickable: null

    // Layout properties
    property int spacing: 1 // for separators

    // Size calculation
    width: (headerVisible ? headerWidth + 30 : 0) + sectionContentList.width + (model ? model.count * spacing : 0)
    height: spacingAbove + sectionContentList.contentHeight + spacingBelow

    // Sticky header
    StyledTextLabel {
        id: stickyHeader

        visible: root.headerVisible

        // Sticky positioning - follows the flickable's contentX
        x: root.parentFlickable ?
           Math.max(0, Math.min(root.parentFlickable.contentX,
                                root.width - (root.headerWidth + 30))) : 0

        y: root.spacingAbove

        width: root.headerWidth + 30
        height: root.headerHeight

        z: 10 // Keep above other content

        leftPadding: 12
        rightPadding: 12
        horizontalAlignment: Qt.AlignRight
        text: root.headerTitle

        // Background to make header opaque during scrolling
        Rectangle {
            anchors.fill: parent
            anchors.margins: -1
            color: ui.theme.backgroundPrimaryColor
            border.width: 1
            border.color: ui.theme.strokeColor
            z: -1
        }
    }

    // Content list positioned after header
    ListView {
        id: sectionContentList

        x: root.headerVisible ? root.headerWidth + 30 + root.spacing : 0
        y: root.spacingAbove

        width: contentItem.childrenRect.width
        height: Math.max(1, contentHeight) // HACK: if the height is 0, the listview won't create any delegates
        contentHeight: contentItem.childrenRect.height

        interactive: false
        orientation: Qt.Horizontal
        spacing: root.spacing // for separators (will be rendered in MixerPanel.qml)

        model: root.model
        delegate: root.delegateComponent
    }

    // Update header position when parent flickable scrolls
    Connections {
        target: root.parentFlickable
        function onContentXChanged() {
            if (root.parentFlickable && root.headerVisible) {
                stickyHeader.x = Math.max(0, Math.min(root.parentFlickable.contentX,
                                                    root.width - (root.headerWidth + 30)))
            }
        }
    }
}
