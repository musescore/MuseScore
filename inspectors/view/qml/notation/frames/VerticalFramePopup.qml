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

        HeightSection {
            heightProperty: model ? model.frameHeight : null
        }

        SeparatorLine { anchors.margins: -10 }

        VerticalGapsSection {
            gapAbove: model ? model.gapAbove : null
            gapBelow: model ? model.gapBelow : null
        }

        SeparatorLine { anchors.margins: -10 }

        HorizontalMarginsSection {
            frameLeftMargin: model ? model.frameLeftMargin : null
            frameRightMargin: model ? model.frameRightMargin : null
        }

        VerticalMarginsSection {
            frameTopMargin: model ? model.frameTopMargin : null
            frameBottomMargin: model ? model.frameBottomMargin : null
        }
    }
}
