import QtQuick 2.0
import MuseScore.UiComponents 1.0
import "../../common"
import "internal"

FocusableItem {
    id: root

    property QtObject proxyModel: null

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 4

        DynamicsExpandableBlank {
            id: dynamicsExpandableBlank

            model: proxyModel ? proxyModel.dynamicPlaybackModel : null
        }

        HairpinsExpandableBlank {
            id: hairpinsExpandableBlank

            model: proxyModel ? proxyModel.hairpinPlaybackModel : null
        }
    }
}

