
// --test-case ./../MuseScore/vtest/vtest.js --test-case-context ./../MuseScore/vtest/vtest_context.json

const THIS_SCRIPT = api.context.globalVal("script_path")
const CONTEXT_PATH = api.context.globalVal("context_path")
const MSCORE_REF_BIN = api.context.globalVal("mscore_ref_bin")
const MODE = api.context.globalVal("mode")
const SCORE_DIR = api.context.globalVal("score_dir")
const CURRENT_DATA_DIR = api.context.globalVal("current_data_dir")
const REFERENCE_DATA_DIR = api.context.globalVal("reference_data_dir")
const COMPARISON_DIR = api.context.globalVal("comparison_dir")

const IS_REF_MODE = MODE === "ref"
const OUTPUT_DIR = IS_REF_MODE ? REFERENCE_DATA_DIR : CURRENT_DATA_DIR

var DIFF_NAME_LIST = []

var testCase = {
    name: "Compare draw data",
    steps: [
        {name: "Generate draw data", func: function() {
            generateDrawData()
        }},
        {name: "Generate ref draw data", func: function() {
            callRef(generateDrawData)
        }},
        {name: "Compare draw data", func: function() {

            api.filesystem.clear(COMPARISON_DIR)

            let rv = api.filesystem.scanFiles(REFERENCE_DATA_DIR, [], "FilesInCurrentDir")
            fatalIfFailed(rv)
            //api.log.info("scanFiles: " + JSON.stringify(rv))

            let files = rv.value
            for (let  i = 0; i < files.length; ++i) {
                let refFile = files[i]
                let fileName = api.filesystem.baseName(refFile)
                let currFile = CURRENT_DATA_DIR + "/" + fileName + ".json"
                let diffFile = COMPARISON_DIR + "/" + fileName + ".diff.json"

                let ret = api.diagnostics.compareDrawData(refFile, currFile, diffFile)
                if (!ret.success) {
                    api.log.info("DIFF DETECTED: " + fileName)
                    DIFF_NAME_LIST.push(fileName)
                    api.filesystem.copy(currFile, COMPARISON_DIR + "/" + fileName + ".json")
                    api.filesystem.copy(refFile, COMPARISON_DIR + "/" + fileName + ".ref.json")
                }
            }
        }},
        {name: "Create pngs", func: function() {
            for (let i = 0; i < DIFF_NAME_LIST.length; ++i) {
                let fileName = DIFF_NAME_LIST[i]
                let refFile  = COMPARISON_DIR + "/" + fileName + ".ref.json"
                let currFile = COMPARISON_DIR + "/" + fileName + ".json"
                let diffFile = COMPARISON_DIR + "/" + fileName + ".diff.json"

                api.diagnostics.compareDrawData(refFile, currFile, diffFile)

                api.diagnostics.drawDataToPng(refFile, COMPARISON_DIR + "/" + fileName + ".ref.png");
                api.diagnostics.drawDataToPng(currFile, COMPARISON_DIR + "/" + fileName + ".png");
                api.diagnostics.drawDiffToPng(diffFile, refFile, COMPARISON_DIR + "/" + fileName + ".diff.png");
            }
        }},
    ]
};

function main()
{
    api.log.info("Hello from VTest.js")

    api.log.info("MODE: " + MODE)

    api.autobot.setInterval(100)
    api.autobot.runTestCase(testCase)
}

function fatalIfFailed(r)
{
    if (!r.success) {
        api.autobot.fatal(r.text)
    }
}

function callRef(func)
{
    let funcName = (typeof func === "function") ? func.name : func

    let args = ["--test-case", THIS_SCRIPT,
                "--test-case-context", CONTEXT_PATH,
                "--test-case-context-value", JSON.stringify({"mode": "ref"}),
                "--test-case-func", funcName]

    api.process.execute(MSCORE_REF_BIN, args)
}

function generateDrawData()
{
    api.diagnostics.generateDrawData(SCORE_DIR, OUTPUT_DIR)
}
