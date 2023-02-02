
// --test-case ./../MuseScore/vtest/vtest.js --test-case-context ./../MuseScore/vtest/vtest_context.json

const THIS_SCRIPT = api.context.globalVal("script_path")
const CONTEXT_PATH = api.context.globalVal("context_path")
const MSCORE_REF_BIN = api.context.globalVal("mscore_ref_bin")
const SCORE_DIR = api.context.globalVal("score_dir")
const CURRENT_DATA_DIR = api.context.globalVal("current_data_dir")
const REFERENCE_DATA_DIR = api.context.globalVal("reference_data_dir")
const COMPARISON_DIR = api.context.globalVal("comparison_dir")

const MODE = api.context.globalVal("mode")
const IS_REF_MODE = MODE === "ref"
const OUTPUT_DATA_DIR = IS_REF_MODE ? REFERENCE_DATA_DIR : CURRENT_DATA_DIR

// options
const DEFAULT = "default"
const SMALL = "small"
const MEDIUM = "medium"

var DIFF_NAME_LIST = {
    "default": [],
    "small": [],
    "medium": []
}

var testCase = {
    name: "Compare draw data",
    steps: [
        {name: "Generate draw data (default)", func: function() {
            generateDrawData(DEFAULT)
        }},
        {name: "Generate ref draw data (default)", func: function() {
            callRef(generateDrawData, DEFAULT)
        }},
        {name: "Create pngs (default) (debug step)", skip: true, func: function() {
            createDataPngs(DEFAULT)
        }},
        {name: "Compare draw data (default)", func: function() {
            compareDrawData(DEFAULT)
        }},
        {name: "Create diff pngs (default)", func: function() {
            createDiffPngs(DEFAULT)
        }},
        {name: "Generate draw data (small)", func: function() {
            generateDrawData(SMALL)
        }},
        {name: "Generate ref draw data (small)", func: function() {
            callRef(generateDrawData, SMALL)
        }},
        {name: "Create pngs (small) (debug step)", skip: true, func: function() {
            createDataPngs(SMALL)
        }},
        {name: "Compare draw data (small)", func: function() {
            compareDrawData(SMALL)
        }},
        {name: "Create diff pngs (small)", func: function() {
            createDiffPngs(SMALL)
        }},
        {name: "Generate draw data (medium)", func: function() {
            generateDrawData(MEDIUM)
        }},
        {name: "Generate ref draw data (medium)", func: function() {
            callRef(generateDrawData, MEDIUM)
        }},
        {name: "Create pngs (medium) (debug step)", skip: true, func: function() {
            createDataPngs(MEDIUM)
        }},
        {name: "Compare draw data (medium)", func: function() {
            compareDrawData(MEDIUM)
        }},
        {name: "Create diff pngs (medium)", func: function() {
            createDiffPngs(MEDIUM)
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

function callRef(func, funcArgs)
{
    let funcName = (typeof func === "function") ? func.name : func
    let funcArgsArr = (typeof funcArgs === "array") ? funcArgs : [funcArgs]

    let args = ["--test-case", THIS_SCRIPT,
                "--test-case-context", CONTEXT_PATH,
                "--test-case-context-value", JSON.stringify({"mode": "ref"}),
                "--test-case-func", funcName,
                "--test-case-func-args", JSON.stringify(funcArgsArr)
                ]

    api.process.execute(MSCORE_REF_BIN, args)
}

function makeOutputDataDir(optName)
{
    return OUTPUT_DATA_DIR+"/"+optName
}

function generateDrawData(optName)
{
    let opt = api.context.globalVal("opt_"+optName)
    api.diagnostics.generateDrawData(SCORE_DIR, makeOutputDataDir(optName), opt)
}

function compareDrawData(optName)
{
    const COMP_DIR = COMPARISON_DIR + "/" + optName
    const CURR_DIR = CURRENT_DATA_DIR + "/" + optName
    const REF_DIR = REFERENCE_DATA_DIR + "/" + optName

    api.filesystem.clear(COMP_DIR)

    let files = api.filesystem.scanFiles(REF_DIR, [], "FilesInCurrentDir").value
    for (let  i = 0; i < files.length; ++i) {
        let refFile = files[i]
        let fileName = api.filesystem.baseName(refFile)
        let currFile = CURR_DIR + "/" + fileName + ".json"
        let diffFile = COMP_DIR + "/" + fileName + ".diff.json"

        let ret = api.diagnostics.compareDrawData(refFile, currFile, diffFile)
        if (!ret.success) {
            api.log.info("DIFF DETECTED: " + fileName)
            DIFF_NAME_LIST[optName].push(fileName)
            api.filesystem.copy(currFile, COMP_DIR + "/" + fileName + ".json")
            api.filesystem.copy(refFile, COMP_DIR + "/" + fileName + ".ref.json")
        }
    }
}

function createDataPngs(optName)
{
    let files = api.filesystem.scanFiles(makeOutputDataDir(optName), [], "FilesInCurrentDir").value
    for (let  i = 0; i < files.length; ++i) {
        let file = files[i]
        api.diagnostics.drawDataToPng(file, file + ".png");
    }
}

function createDiffPngs(optName)
{
    const COMP_DIR = COMPARISON_DIR + "/" + optName
    const DIFF_LIST = DIFF_NAME_LIST[optName]

    for (let i = 0; i < DIFF_LIST.length; ++i) {
        let fileName = DIFF_LIST[i]
        let refFile  = COMP_DIR + "/" + fileName + ".ref.json"
        let currFile = COMP_DIR + "/" + fileName + ".json"
        let diffFile = COMP_DIR + "/" + fileName + ".diff.json"

        api.diagnostics.compareDrawData(refFile, currFile, diffFile)

        api.diagnostics.drawDataToPng(refFile, COMP_DIR + "/" + fileName + ".ref.png");
        api.diagnostics.drawDataToPng(currFile, COMP_DIR + "/" + fileName + ".png");
        api.diagnostics.drawDiffToPng(diffFile, refFile, COMP_DIR + "/" + fileName + ".diff.png");
    }
}
