
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

// { fileName: { opt_name: true }}
var DIFF_NAME_LIST = {}

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
        {name: "Create report", func: function() {
            createReport()
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

    let opt = {
        isCopySrc: true,
        isMakePng: true
    }

    let files = api.filesystem.scanFiles(REF_DIR, [], "FilesInCurrentDir").value
    for (let  i = 0; i < files.length; ++i) {
        let refFile = files[i]
        let fileName = api.filesystem.baseName(refFile)
        let currFile = CURR_DIR + "/" + fileName + ".json"
        let diffFile = COMP_DIR + "/" + fileName + ".diff.json"

        let ret = api.diagnostics.compareDrawData(refFile, currFile, diffFile, opt)
        if (!ret.success) {
            api.log.info("DIFF DETECTED: " + fileName)
            let fileInfo = DIFF_NAME_LIST[fileName]
            if (fileInfo === undefined) {
                fileInfo = {}
            }
            fileInfo[optName] = true
            DIFF_NAME_LIST[fileName] = fileInfo
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

function isHasDiff()
{
    return Object.keys(DIFF_NAME_LIST).length !== 0
}

function createReport()
{
    if (!isHasDiff()) {
        return
    }

    if (!String.prototype.format) {
        String.prototype.format = function() {
            var args = arguments;
            return this.replace(/{(\d+)}/g, function(match, number) {
                return typeof args[number] != 'undefined'
                        ? args[number]
                        : match
                ;
            });
        };
    }

    let html = '\
<html>
    <head>
    </head>
    <body style="background-color:#eee;">
';
    let makeBlock = function(opt, fn) {
        let block = '\
        <h2 id=\"{0}_{1}\">{0}: {1}<a href=\"#{0}_{1}\">#</a></h2>
        <div>
            <div>ref:</div><br/>
            <img src=\"{0}/{1}.ref.png\"><br/>
            <div>current:</div><br/>
            <img src=\"{0}/{1}.png\"><br/>
            <div>diff:</div>
            <img src=\"{0}/{1}.diff.png\"><br/>
        </div>
        '.format(opt, fn)

        return block
    }

    for (let fileName in DIFF_NAME_LIST) {
        let fileInfo = DIFF_NAME_LIST[fileName]
        for (let opt in fileInfo) {
            if (fileInfo[opt]) {
                html += "\n"
                html += makeBlock(opt, fileName)
            }
        }
    }

    html += '\
    </body>
</html>
';

    api.filesystem.writeTextFile(COMPARISON_DIR+"/vtest_compare.html", html)
}
