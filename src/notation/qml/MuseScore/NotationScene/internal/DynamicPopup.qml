import QtQuick 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0

StyledPopupView {
    id: root

    property NavigationSection notationViewNavigationSection: null
    property int navigationOrderStart: 0
    property int navigationOrderEnd: dynamicsNavPanel.order

    contentWidth: content.width
    contentHeight: content.height

    showArrow: false

    signal elementRectChanged(var elementRect)

    function updatePosition() {
        var h = root.contentHeight
        root.x = - root.contentWidth / 2
        root.y = root.parent.height / 2 + root.contentHeight
    }

    ColumnLayout {
        id: content

        width: 160
        height: 10

        DynamicPopupModel {
            id: dynamicModel

            onItemRectChanged: function(rect) {
                root.elementRectChanged(rect)
            }
        }

        Component.onCompleted: {
            dynamicModel.init()
        }

        NavigationPanel {
            id: dynamicsNavPanel
            name: "DynamicsPopup"
            direction: NavigationPanel.Vertical
            section: root.notationViewNavigationSection
            order: root.navigationOrderStart
            accessible.name: qsTrc("notation", "Dynamics Popup")
        }

        // StyledTextLabel {
        //     id: titleLabel

        //     text: qsTrc("notation", "Dynamics Popup")
        //     horizontalAlignment: Text.AlignLeft
        // }

    }
}
