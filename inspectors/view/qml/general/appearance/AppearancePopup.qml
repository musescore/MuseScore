import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2
import MuseScore.UiComponents 1.0
import "../../common"
import "internal"

StyledPopup {
    id: root

    property QtObject model: undefined

    height: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        height: implicitHeight
        width: parent.width

        spacing: 16

        HorizontalSpacingSection {
            leadingSpace: model ? model.leadingSpace : null
            barWidth: model ? model.barWidth : null
        }

        SeparatorLine { anchors.margins: -10 }

        VerticalSpacingSection {
            minimumDistance: model ? model.minimumDistance : null
        }

        SeparatorLine { anchors.margins: -10 }

        OffsetSection {
            horizontalOffset: model ? model.horizontalOffset : null
            verticalOffset: model ? model.verticalOffset : null
            isSnappedToGrid: model ? model.isSnappedToGrid : null

            onSnapToGridToggled: {
                if (model) {
                    model.isSnappedToGrid = snap
                }
            }

            onConfigureGridRequested: {
                if (model) {
                    model.configureGrid()
                }
            }
        }

        SeparatorLine { anchors.margins: -10 }

        ArrangeSection {
            onPushBackRequested: {
                if (root.model) {
                    root.model.pushBackInOrder()
                }
            }

            onPushFrontRequested: {
                if (root.model) {
                    root.model.pushFrontInOrder()
                }
            }
        }

        SeparatorLine { anchors.margins: -10 }

        ColorSection {
            color: root.model ? root.model.color : null
        }
    }
}
