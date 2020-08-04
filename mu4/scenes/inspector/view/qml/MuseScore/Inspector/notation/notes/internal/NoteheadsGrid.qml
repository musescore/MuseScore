import QtQuick 2.9
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../../common"

FocusableItem {
    id: root

    property alias noteHeadTypesModel: gridView.model
    property bool isIndeterminate: false

    function iconByNoteHeadType(noteHeadType) {
        switch (noteHeadType) {
        case NoteHead.HEAD_NORMAL: return "qrc:/resources/icons/notehead_normal.svg"
        case NoteHead.HEAD_CROSS: return "qrc:/resources/icons/notehead_cross.svg"
        case NoteHead.HEAD_PLUS: return "qrc:/resources/icons/notehead_plus.svg"
        case NoteHead.HEAD_XCIRCLE: return "qrc:/resources/icons/notehead_xcircle.svg"
        case NoteHead.HEAD_WITHX: return "qrc:/resources/icons/notehead_xellipse.svg"
        case NoteHead.HEAD_TRIANGLE_UP: return "qrc:/resources/icons/notehead_triangle_down.svg"
        case NoteHead.HEAD_TRIANGLE_DOWN: return "qrc:/resources/icons/notehead_triangle_up.svg"
        case NoteHead.HEAD_SLASHED1: return "qrc:/resources/icons/notehead_slashed_left.svg"
        case NoteHead.HEAD_SLASHED2: return "qrc:/resources/icons/notehead_slashed_right.svg"
        case NoteHead.HEAD_DIAMOND: return "qrc:/resources/icons/notehead_diamond.svg"
        case NoteHead.HEAD_DIAMOND_OLD: return "qrc:/resources/icons/notehead_diamond_old.svg"
        case NoteHead.HEAD_CIRCLED: return "qrc:/resources/icons/notehead_circled.svg"
        case NoteHead.HEAD_CIRCLED_LARGE: return "qrc:/resources/icons/notehead_circled.svg"
        case NoteHead.HEAD_LARGE_ARROW: return "qrc:/resources/icons/notehead_large_arrow.svg"
        case NoteHead.HEAD_BREVIS_ALT: return "qrc:/resources/icons/notehead_brevis_alt.svg"
        case NoteHead.HEAD_SLASH: return "qrc:/resources/icons/notehead_slash.svg"
        case NoteHead.HEAD_SOL: return "qrc:/resources/icons/notehead_sol.svg"
        case NoteHead.HEAD_LA: return "qrc:/resources/icons/notehead_la.svg"
        case NoteHead.HEAD_FA: return "qrc:/resources/icons/notehead_fa.svg"
        case NoteHead.HEAD_MI: return "qrc:/resources/icons/notehead_mi.svg"
        case NoteHead.HEAD_DO: return "qrc:/resources/icons/notehead_do.svg"
        case NoteHead.HEAD_RE: return "qrc:/resources/icons/notehead_re.svg"
        case NoteHead.HEAD_TI: return "qrc:/resources/icons/notehead_ti.svg"
        default: return "qrc:/resources/icons/note.svg"
        }
    }

    implicitHeight: gridView.height + 16
    width: parent.width

    Rectangle {
        height: root.implicitHeight
        width: parent.width

        color: ui.theme.textFieldColor

        radius: 3
    }

    GridView {
        id: gridView

        height: contentHeight
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8

        cellHeight: 40
        cellWidth: 40

        interactive: false

        delegate: FocusableItem {

            implicitHeight: gridView.cellHeight
            implicitWidth: gridView.cellWidth

            Icon {
                anchors.centerIn: parent

                pixelSize: 30
                icon: iconByNoteHeadType(headGroupRole)
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    noteHeadTypesModel.selectedHeadTypeIndex = index
                }
            }
        }

        highlight: Rectangle {
            color: ui.theme.accentColor
            opacity: ui.theme.accentOpacityNormal
            radius: 2
        }

        currentIndex: noteHeadTypesModel ? noteHeadTypesModel.selectedHeadTypeIndex : -1
    }
}
