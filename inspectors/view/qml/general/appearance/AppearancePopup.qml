import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2
import "../../common"
import "internal"

StyledPopup {
    id: root

    height: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        height: implicitHeight
        width: parent.width

        spacing: 16

        HorizontalSpacingSection { }

        SeparatorLine { anchors.margins: -10 }

        VerticalSpacingSection { }

        SeparatorLine { anchors.margins: -10 }

        OffsetSection { }

        SeparatorLine { anchors.margins: -10 }

        ArrangeSection { }

        SeparatorLine { anchors.margins: -10 }

        ColorSection { }
    }
}
