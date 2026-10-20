
const Log = require("MuseApi.Log");
const Interactive = require("MuseApi.Interactive");
const Engraving = require("MuseApi.Engraving");

const Element = Engraving.Element;

function main()
{
    Log.info("macros.js")

    const score = api.engraving.curScore;
    const measure = score.firstMeasure;
    console.log("measure.type:", measure.type
        , "Element.Measure:", Element.MEASURE
        , "api.engraving.Element.MEASURE:", api.engraving.Element.MEASURE
        , "Engraving.Element.MEASURE:", Engraving.Element.MEASURE
    )

    console.log("measure.hideWhenEmpty:", measure.hideWhenEmpty)

    if (measure.type === Element.MEASURE) {
        console.log("this is measure")
        Interactive.info("Macros", "This is a measure.")
    }
}
