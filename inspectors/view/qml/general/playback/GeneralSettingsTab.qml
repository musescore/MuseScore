import QtQuick 2.0
import QtQuick.Controls 2.0
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

        NoteExpandableBlank {
            id: noteExpandableBlank

            model: proxyModel ? proxyModel.notePlaybackModel : null
        }

        ArpeggioExpandableBlank {
            id: arpeggioExpandableBlank

            model: proxyModel ? proxyModel.arpeggioPlaybackModel : null
        }

        FermataExpandableBlank {
            id: fermataExpandableBlank

            model: proxyModel ? proxyModel.fermataPlaybackModel : null
        }

        PausesExpandableBlank {
            id: pauseExpandableBlank

            model: proxyModel ? proxyModel.breathPlaybackModel : null
        }

        GlissandoExpandableBlank {
            id: glissandoExpandableBlank

            model: proxyModel ? proxyModel.glissandoPlaybackModel : null
        }
    }
}

