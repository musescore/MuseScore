pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene

AbstractElementPopup {
    id: root

    property NavigationSection notationViewNavigationSection: null
    property int navigationOrderStart: 0
    property int navigationOrderEnd: articulationsNavPanel.order

    property int buttonWidth: 32
    property int buttonHeight: 30

    // Page 0 (default) is the base page (the 5 basic articulation types) - its left chevron
    // pages left into page 1, the 4 Accent/Marcato combos, whose right chevron pages back.
    property int currentPage: 0

    margins: 4

    contentWidth: content.width
    contentHeight: content.height

    showArrow: false

    focusPolicies: PopupView.DefaultFocus & ~PopupView.ClickFocus
    // Always below, regardless of the articulation's own placement: matches DynamicPopup's
    // real-world behavior, and an above-placed articulation sits close enough to its note that
    // a popup opening above it tends to cover the mark itself.
    placementPolicies: PopupView.PreferBelow

    model: ArticulationPopupModel {
        id: articulationModel
    }

    RowLayout {
        id: content

        // Fixed to fit the widest page (the 5-item base page) with no slack, so the popup's
        // width stays constant across pages; the fillWidth Items around the Repeater absorb
        // the leftover space on the narrower combo page, keeping its buttons centered. Only one
        // chevron is visible at a time, so it doesn't factor into this beyond its own 16px.
        width: 183
        spacing: 1

        NavigationPanel {
            id: articulationsNavPanel
            name: "ArticulationsPopup"
            direction: NavigationPanel.Horizontal
            section: root.notationViewNavigationSection
            order: root.navigationOrderStart
            accessible.name: qsTrc("notation", "Articulations popup")

            onNavigationEvent: function(event) {
                if (event.type === NavigationEvent.Escape) {
                    root.close()
                }
            }
        }

        function togglePage() {
            const newPage = (root.currentPage + 1) % articulationModel.pages.length
            root.currentPage = newPage

            // Arriving on the combo page (1) via the left chevron/key should focus its last
            // item (the one spatially adjacent to where the user came from); arriving back on
            // the base page (0) via the right chevron/key should focus its first item.
            Qt.callLater(requestNavigationActive, newPage === 1 ? -1 : 0)
        }

        function requestNavigationActive(index) {
            (articulationRepeater.itemAt(index < 0 ? articulationRepeater.count - 1 : index) as FlatButton).navigation.requestActive()
        }

        FlatButton {
            id: leftButton

            implicitWidth: 16
            transparent: true
            visible: root.currentPage === 0

            contentItem: StyledIconLabel {
                iconCode: IconCode.CHEVRON_LEFT
                font.pixelSize: 16
            }

            onClicked: {
                content.togglePage()
            }
        }

        Item {
            Layout.fillWidth: true
        }

        Repeater {
            id: articulationRepeater

            model: articulationModel.pages[root.currentPage]

            delegate: FlatButton {
                id: articulationButton

                required property var modelData
                required property int index

                implicitWidth: root.buttonWidth
                implicitHeight: root.buttonHeight
                transparent: true

                navigation.panel: articulationsNavPanel
                navigation.order: index
                accessible.name: modelData.accessibleName
                toolTipTitle: modelData.accessibleName

                navigation.onNavigationEvent: function(event) {
                    switch (event.type) {
                    case NavigationEvent.Up:
                    case NavigationEvent.Left: {
                        if (index == 0 && root.currentPage === 0) {
                            content.togglePage()

                            event.accepted = true
                        }

                        break
                    }

                    case NavigationEvent.Right:
                    case NavigationEvent.Down: {
                        if (index == articulationRepeater.count - 1 && root.currentPage === 1) {
                            content.togglePage()

                            event.accepted = true
                        }

                        break
                    }
                    }
                }

                contentItem: StyledTextLabel {
                    text: articulationButton.modelData.text
                    font.family: articulationModel.fontFamily
                    font.pixelSize: 30

                    anchors.centerIn: parent
                }

                onClicked: {
                    articulationModel.changeArticulation(root.currentPage, index)
                }
            }
        }

        Item {
            Layout.fillWidth: true
        }

        FlatButton {
            id: rightButton

            implicitWidth: 16
            transparent: true
            visible: root.currentPage === 1

            contentItem: StyledIconLabel {
                iconCode: IconCode.CHEVRON_RIGHT
                font.pixelSize: 16
            }

            onClicked: {
                content.togglePage()
            }
        }
    }
}
