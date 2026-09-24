import QtQuick

import MuseApi.Controls
import MuseApi.Engraving
import MuseApi.Interactive
import MuseApi.Log

ExtensionBlank {

    id: root

    implicitHeight: 400
    implicitWidth: 400

    color: api.theme.backgroundPrimaryColor

    Component.onCompleted: {

        Log.info("Extension ApiTests Form loaded")

        const score = api.engraving.curScore;
        const measure = score.firstMeasure;
        console.log("measure.type:", measure.type
                    , ", api.engraving.Element.MEASURE:", api.engraving.Element.MEASURE
                    , ", Element:", Element
                    , ", Element.MEASURE:", Element.MEASURE
                    , ", Engraving.Element.MEASURE:", Engraving.Element.MEASURE
                    )

        if (measure.type === api.engraving.Element.MEASURE) {
            console.log("this is measure")
        } else {
            console.log("this is not measure")
        }

        console.log("TextStyleType:", api.engraving.TextStyleType
                    , ", TextStyleType.TITLE:", api.engraving.TextStyleType.TITLE
                    , ", TextStyleType.SUBTITLE:", api.engraving.TextStyleType.SUBTITLE
                    , ", TextStyleType.COPYRIGHT:", api.engraving.TextStyleType.COPYRIGHT
                    )
    }

    StyledFlickable {
        anchors.fill: parent

        Column {
            width: parent.width
            height: implicitHeight
            spacing: 8

            FlatButton {
                text: "Interactive.info"
                onClicked: {
                    Interactive.info("Api tests", "This is info")
                }
            }

            FlatButton {
                text: "Interactive.question"
                icon: IconCode.STAR
                onClicked: {
                    let btn = Interactive.question("Api tests", "Yes or No?", [ButtonCode.Yes, ButtonCode.No])
                    if (btn === ButtonCode.Yes) {
                        Interactive.info("Api tests", "Your answer is Yes.")
                    } else {
                        Interactive.warning("Api tests", "Your answer is " + btn)
                    }
                }
            }
        }
    }
}
