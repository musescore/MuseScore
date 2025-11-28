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
        enum ButtonCode {
            Ok = int(IInteractive::Button::Ok),
            Continue = int(IInteractive::Button::Continue)
        };
    `;

    const expectCode = `/** 
* Question buttons
* @memberof Qml
* @enum
*/
const ButtonCode = {
	Ok: "Ok",
	Continue: "Continue",
};`;

    saveFile(testCode, testFile);

    const doc = await extractDoc(testFile);
    console.log(doc)
    
   // EXPECT_EQ(doc, expectCode)
}

async function main() 
{
    await enumTest();
}

main();