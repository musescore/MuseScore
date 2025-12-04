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
                text: "api.interactive.info"
                onClicked: {
                    api.interactive.info("Api tests", "This is info")
                }
            }

            FlatButton {
                text: "api.interactive.question"
                icon: IconCode.STAR
                onClicked: {
                    const score = api.engraving.curScore;
                    score.startCmd()
                    //score.addText("poet", "This is a LYRICIST (poet)");
                    score.addText(TextStyleType.LYRICIST, "This is a LYRICIST (poet)"); 
                    score.endCmd();
                }
            }
        }
    }
}
