import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import "../../common"

StyledPopup {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    function markerTypeToString(type) {
        if (!type)
            return ""

        if (type.isUndefined)
            return "--"

        switch (type.value) {
        case MarkerTypes.TYPE_SEGNO: return qsTrc("inspector", "Segno");
        case MarkerTypes.TYPE_VARSEGNO: return qsTrc("inspector", "Segno variation")
        case MarkerTypes.TYPE_CODA: return qsTrc("inspector", "Coda")
        case MarkerTypes.TYPE_VARCODA: return qsTrc("inspector", "Varied coda")
        case MarkerTypes.TYPE_CODETTA: return qsTrc("inspector", "Codetta")
        case MarkerTypes.TYPE_FINE: return qsTrc("inspector", "Fine")
        case MarkerTypes.TYPE_TOCODA: return qsTrc("inspector", "To Coda")
        case MarkerTypes.TYPE_USER: return qsTrc("inspector", "Custom")
        }
    }

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        StyledTextLabel {
            text: qsTrc("inspector", "Marker type: ") + markerTypeToString(model ? model.type : null)
        }

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Label")
            propertyItem: model ? model.label : null

            TextInputField {
                isIndeterminate: model ? model.label.isUndefined : false
                currentText: model ? model.label.value : ""
                enabled: model ? model.label.isEnabled : false

                onCurrentTextEdited: {
                    if (!root.model) {
                        return
                    }

                    root.model.label.value = newTextValue
                }
            }
        }
    }
}
