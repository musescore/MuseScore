import QtQuick 2.15

Item {
    id: root

    property string title: ""
    property string uniqueName: ""

    property int minimumWidth: 0
    property int minimumHeight: 0
    property int maximumWidth: 0
    property int maximumHeight: 0

    property bool floatable: true
    property bool closable: true

    property string tabifyPanelName: ""

    default property Component content

    signal closed()
}
