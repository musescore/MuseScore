const fs = require('node:fs');
const { extractDoc } = require('./jsdoc_extractor.js');

const TEMP_DIR="./temp"
if (!fs.existsSync(TEMP_DIR)) {
    fs.mkdirSync(TEMP_DIR, { recursive: true });
} 

function saveFile(doc, outPath)
{
    fs.writeFileSync(outPath, doc);
}

function EXPECT_EQ(have, expect)
{
    if (have !== expect) {
        console.error("[NOT EQUAL] have:\n", have, ", expect:\n", expect);
        process.exit(1);
    }
}

async function enumTest() 
{
    const testFile = TEMP_DIR + "/enum.h";
    const testCode = `
        /** APIDOC
         * Question buttons
         * @memberof Qml
         * @enum
         */
        enum class ButtonCode {
            Ok = int(IInteractive::Button::Ok),
            Continue = int(IInteractive::Button::Continue)
        };
    `;

    saveFile(testCode, testFile);

    const doc = await extractDoc(testFile);
    console.log(doc)
}

async function classTest() {
    const testFile = TEMP_DIR + "/class.h";
    const testCode = `
        /** APIDOC
         * Class representing a score.
         * We can get the current score
         * @class Score
         * @memberof engraving
         * @hideconstructor
        */

        /** APIDOC
         * Create a new Cursor
         * @method
         * @returns {Cursor} cursor
        */
        Cursor* Score::newCursor()

        /** APIDOC
        * Extracts all lyrics in the score and returns them in a single string.
        * @method
        * @return {String} - lyrics string
        */
        QString Score::extractLyrics() const {}

        /** APIDOC
         * @readonly
         * @member {engraving.Lyric[]}
         * @since 4.7
         */
        QQmlListProperty<Lyrics> Score::lyrics() const {}

        /** APIDOC
         * Next measure, accounting for multimeasure rests.
         * @readonly
         * @q_property {engraving.Measure}
         * @name nextMeasureMM
         */
    `;

    saveFile(testCode, testFile);

    const doc = await extractDoc(testFile);
    console.log(doc)
}

async function main() 
{
    await enumTest();
   //await classTest();
}

main();