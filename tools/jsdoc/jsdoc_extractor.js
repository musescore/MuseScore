const fs = require('node:fs');
const path = require('node:path');
const readline = require('node:readline');

const VERSION = "0.1"

var filter = {
    ignoreFile: "",
    exts: []
};

function scan(files, rootPath, filter)
{
    if (filter.ignoreFile !== "" && fs.existsSync(rootPath + "/" + filter.ignoreFile)) {
        // ignore
        return [];
    }

    const items = fs.readdirSync(rootPath, {withFileTypes: true, recursive: false})
    for (var i in items) {
        const item = items[i]

        const itemPath = rootPath + "/" + item.name
        if (item.isDirectory()) {
            // recursion
            scan(files, itemPath, filter)
        } else if (item.isFile()) {
            if ((filter.exts.length === 0) || filter.exts.includes(path.extname(itemPath))) {
                files.push(itemPath)
            }
        }
    }
}

async function extractDoc(file)
{
    const fileStream = fs.createReadStream(file)
    const rl = readline.createInterface({
                                            input: fileStream,
                                            crlfDelay: Infinity
                                        })

    const APIDOC_BEGIN = "/** APIDOC"
    const APIDOC_END = "*/"
    const APIDOC_NSPACE = "namespace:"
    const APIDOC_METHOD = "method"

    var state = {
        namespaceStared: false,
        namespaceName: "",

        methodDocContent: false,
        methodParams: [],
        methodLookName: false,
    };

    var doc = ""

    for await (var line of rl) {
        line = line.trim()

        if (line.startsWith(APIDOC_BEGIN)) {

            // remove /** APIDOC
            line = line.substring(APIDOC_BEGIN.length);
            line = line.trim()

            // check namespace
            // `namespace: interactive`
            if (line.startsWith(APIDOC_NSPACE)) {
                state.namespaceName = line.substring(APIDOC_NSPACE.length).trim()
                state.namespaceStared = true;
                doc += "/**\n"
            }
            // check method
            // `method`
            else if (line.startsWith(APIDOC_METHOD)) {
                doc += "\t/**\n"
                state.methodDocContent = true
            }

            continue;
        }

        if (line.startsWith(APIDOC_END)) {
            // write ns
            if (state.namespaceName !== "") {
                doc += "*/\n"
                doc += "const " + state.namespaceName + " = {\n\n"
                state.namespaceName = ""
            }
            // end method
            else if (state.methodDocContent) {
                state.methodDocContent = false
                state.methodLookName = true
            }

            continue;
        }

        if (state.namespaceName !== "") {
            doc += line + "\n"
            continue;
        }

        if (state.methodDocContent) {
            doc += "\t" + line + "\n"

            if (line.includes("@param")) {
                var words = line.split(' ');
                state.methodParams.push(words[3])
            }

            continue;
        }

        if (state.methodLookName) {
            // Result Class::method(...)
            var colonIdx = line.lastIndexOf("::")
            if (colonIdx !== -1) {
                colonIdx += 2 // skip ::
                var braceIdx = line.indexOf("(", colonIdx)
                var name = line.substr(colonIdx, (braceIdx - colonIdx))

                // write method
                doc += "\t*/\n"
                doc += "\t" + name + "(" + state.methodParams.join(', ') + ") {},\n\n"

                // cleanup state
                state.methodLookName = ""
                state.methodParams = []
            }
        }
    }

    if (state.namespaceStared) {
        doc += "};";
    }

    return doc
}

function saveDoc(doc, dir, name)
{
    const outPath = dir + "/" + name + ".js"
    fs.writeFileSync(outPath, doc);

    console.log("[saved] ", outPath)
}

function printVersion()
{
    console.log("version: ", VERSION)
}

function printHelp()
{
    var h = "Hello World! I am jsdoc extractor!\n"
    h += "This is a utility for scanning files, extracting documentation from them and creating JS files. \n"
    h += "use:\n"
    h += "-d  --dir /path                       root dir for scan\n"
    h += "-o  --out /path                       output path\n"
    h += "-i  --ignore filename                 if present this file, dir (and subdirs) will be ignored\n"
    h += "-e  --extensions .ext1,.extn          allowed extensions\n"
    h += "-h, --help                            this help\n"
    h += "-v, --version                         print version\n"

    console.log(h)
}

async function main()
{
    // default
    var dir = ".";
    var out = "./out";


    // parse args
    var args = process.argv.slice(2);
    console.log("args: ", args)
    {
        var i = -1;
        while (true) {
            ++i
            if (i >= args.length) {
                break;
            }

            const arg = args[i]

            if (arg == "-v" || arg == "--version") {
                printVersion()
                return 0
            }
            else if (arg == "-h" || arg == "--help") {
                printHelp()
                return 0
            }
            else if (arg == "-d" || arg == "--dir") {
                dir = args[++i]
            }
            else if (arg == "-o" || arg == "--out") {
                out = args[++i]
            }
            else if (arg == "-i" || arg == "--ignore") {
                filter.ignoreFile = args[++i]
            }
            else if (arg == "-e" || arg == "--extensions") {
                var extsStr = args[++i];
                exts = extsStr.split(',')
                for (const e of exts) {
                    if (e === "") {
                        continue;
                    }

                    if (e[0] !== '.') {
                        filter.exts.push('.' + e);
                    } else {
                        filter.exts.push(e);
                    }
                }
            }
            else {
                console.log("invalid option -- '", arg, "', try '--help' for more information.\n")
            }
        }
    }

    // scan
    var files = []
    scan(files, dir, filter)

    if (filter.length === 0) {
        console.log("not found files")
        return 0;
    }

    if (!fs.existsSync(out)) {
        fs.mkdirSync(out)
    }

    // extrac
    for (var i in files) {
        const file = files[i]
        const name = path.parse(file).name

        const doc = await extractDoc(file);
        if (doc !== "") {
            saveDoc(doc, out, name);
        }
    }

    return 0;
}

main()
