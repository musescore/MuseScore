import QtQuick

import MuseApi.Controls
import MuseApi.Engraving

ExtensionBlank {

    id: root

    implicitHeight: 400
    implicitWidth: 400

    color: api.theme.backgroundPrimaryColor

    Component.onCompleted: {
        const score = api.engraving.curScore;
        const measure = score.firstMeasure;
        console.log("measure.type:", measure.type
                    , ", api.engraving.Element.MEASURE:", api.engraving.Element.MEASURE
                    , ", Element:", Element
                    , ", Element.MEASURE:", Element.MEASURE
                    , ", IconCode:", IconCode
                    , ", IconCode.STAR:", IconCode.STAR
                    )

        if (measure.type === api.engraving.Element.MEASURE) {
            console.log("this is measure")
        } else {
            console.log("this is not measure")
        }
    }

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
                icon: IconCode.STAR
                onClicked: {
                    let btn = api.interactive.question("Api tests", "Yes or No?", [ButtonCode.Yes, ButtonCode.No])
                    if (btn === ButtonCode.Yes) {
                        api.interactive.info("Api tests", "Your answer is Yes.")
                    } else {
                        api.interactive.warning("Api tests", "Your answer is " + btn)
                    }
                }
            }
        }
    }
}
