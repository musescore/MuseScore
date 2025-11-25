import QtQuick

import MuseApi.Extensions
//import MuseApi.Controls
import Muse.UiComponents

ExtensionBlank {

    id: root

    implicitHeight: 400
    implicitWidth: 400

    color: api.theme.backgroundPrimaryColor

    StyledFlickable {
        anchors.fill: parent

        Column {
            width: parent.width
            height: implicitHeight
            spacing: 8

            FlatButton {
                text: "api.interactive.info"
                onClicked: {
                    api.interactive.info("Api tests", "This is info")
                }
            }

            FlatButton {
                text: "api.interactive.question"
                onClicked: {
                    let btn = api.interactive.question("Api tests", "Yes or No?", [Button.Yes, Button.No])
                    if (btn === Button.Yes) {
                        api.interactive.info("Api tests", "Your answer is Yes.")
                    } else {
                        api.interactive.warning("Api tests", "Your answer is " + btn)
                    }
                }
            }
        }
    }
}
