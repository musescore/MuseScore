
const Element = api.engraving.Element;

function main()
{
    console.log("macros.js")

    const score = api.engraving.curScore;
    const measure = score.firstMeasure;
    console.log("measure.type:", measure.type, "Element.Measure:", Element.MEASURE)

    if (measure.type === Element.MEASURE) {
        console.log("this is measure")
    }
}
