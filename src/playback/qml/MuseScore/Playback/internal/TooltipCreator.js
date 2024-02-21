var component = Qt.createComponent("Tooltip.qml");

function create(text, parent, properties) {
 if (typeof properties == "undefined") {
        properties = {
            anchors: {
                horizontalCenter: parent.horizontalCenter,
                bottom: parent.bottom,
                bottomMargin: parent.height / 8
            }
        };
    }
    properties.text = text;
    var tooltip = component.createObject(parent, properties);
    if (tooltip === null) {
 console.error("error creating tooltip: " + component.errorString());
 } else if (properties.anchors) {
 // manual anchor mapping necessary
 for (var anchor in properties.anchors) {
 tooltip.anchors[anchor] = properties.anchors[anchor];
 }
 }
 return tooltip;
}
