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
        case MarkerTypes.TYPE_SEGNO: return qsTr("Segno");
        case MarkerTypes.TYPE_VARSEGNO: return qsTr("Segno variation")
        case MarkerTypes.TYPE_CODA: return qsTr("Coda")
        case MarkerTypes.TYPE_VARCODA: return qsTr("Varied coda")
        case MarkerTypes.TYPE_CODETTA: return qsTr("Codetta")
        case MarkerTypes.TYPE_FINE: return qsTr("Fine")
        case MarkerTypes.TYPE_TOCODA: return qsTr("To Coda")
        case MarkerTypes.TYPE_USER: return qsTr("Custom")
        }
    }

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        StyledTextLabel {
            text: qsTr("Marker type: ") + markerTypeToString(model ? model.type : null)
        }

        InspectorPropertyView {
            titleText: qsTr("Label")
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
