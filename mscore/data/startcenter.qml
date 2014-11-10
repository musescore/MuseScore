import QtQuick 2.3
import QtWebKit 3.0

Rectangle {
    width:  600
    height: 400

    WebView {
        anchors.fill: parent
        url: "http://www.musescore.org"
        }
    }
