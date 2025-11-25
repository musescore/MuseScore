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

function nameFromSig(line)
{
    let name = "";
    // Result Class::method(...)
    let colonIdx = line.lastIndexOf("::")
    if (colonIdx !== -1) {
        colonIdx += 2 // skip `::`
        let braceIdx = line.indexOf("(", colonIdx)
        name = line.substr(colonIdx, (braceIdx - colonIdx))
    }

    if (name === "") {
        // Result method(...)
        line = line.trim()
        let spaceIdx = line.indexOf(" ")
        if (spaceIdx !== -1) {
            spaceIdx += 1 // skip ` `
            let braceIdx = line.indexOf("(", spaceIdx)
            if (braceIdx !== -1) {
                name = line.substr(spaceIdx, (braceIdx - spaceIdx))
            }
        }
    }

    return name;
}

function enumName(line)
{
    let name = "";
    line = line.trim()
    let enumIdx = line.indexOf("enum");
    if (enumIdx === -1) {
        return name;
    }

    enumIdx += 4
    let braceIdx = line.indexOf("{", enumIdx) 
    if (braceIdx !== -1) {
        name = line.substr(enumIdx, (braceIdx - enumIdx))
        name = name.trim();
    }

    return name;
}

function enumKey(line) 
{
    let key = "";
    line = line.trim()
    let idx = line.indexOf('=')
    if (idx === -1) {
        idx = line.indexOf(',')
    } 

    if (idx !== -1) {
        let key = line.substr(0, idx)
        return key.trim();
    }

    return ""
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
    
    let state = {
        hasApidoc: false,
        apidocStarted: false,

        currentDoc: "",

        parentDoc: "", // namespace or class
        parentName: "",

        methodLookName: false,
        methods: [],

        propLookName: false,
        props: [],

        enumLookName: false,
        enumStarted: false,
        enums: [],
    }                                   

    for await (let line of rl) {
        line = line.trim()

        if (line.startsWith(APIDOC_BEGIN)) {
            // remove APIDOC
            line = line.replace("APIDOC", "");
            state.hasApidoc = true;
            state.apidocStarted = true;
            state.currentDoc = "";
        }

        if (state.apidocStarted && line.endsWith(APIDOC_END)) {
            state.currentDoc += line + "\n";
            state.apidocStarted = false;

            // check of kind 

            // get parent - namespace or class
            const namespaceMatch = state.currentDoc.match(/@namespace\s+(\S+)/);
            if (namespaceMatch) {
                state.parentName = namespaceMatch[1];
                state.parentDoc = state.currentDoc;
                continue;
            }

            const classMatch = state.currentDoc.match(/@class\s+(\S+)/);
            if (classMatch) {
                state.parentName = classMatch[1];
                state.parentDoc = state.currentDoc;
                continue;
            }

            // try add memberof to method
            if (state.parentName !== "") {
                if (state.currentDoc.includes('@method')) {
                    state.currentDoc = state.currentDoc.replace('@method', `@memberof ${state.parentName}\n* @method`);
                    state.methodLookName = true;
                    continue;
                }
            }

            // try get property 
            if (state.currentDoc.includes('@property')) {
                state.currentDoc = state.currentDoc.replace('/** ', `*`);
                state.currentDoc = state.currentDoc.replace('/**', `*`);
                state.currentDoc = state.currentDoc.replace('*/', ``);
                state.propLookName = true;
                continue;
            }

            // try get enum 
            if (state.currentDoc.includes('@enum')) {
                state.enumLookName = true;
                continue;
            }
        }

        if (state.apidocStarted) {
            state.currentDoc += line + "\n";
        }

         if (state.methodLookName) {
            let name = nameFromSig(line);
            if (name !== "") {
                state.methodLookName = false;
                state.currentDoc = state.currentDoc.replace('@method', `@method ${name}`);
                state.methods.push(state.currentDoc);
            }
        }

        if (state.propLookName) {
            let name = nameFromSig(line);
            if (name !== "") {
                state.propLookName = false;
                // add name 
                const regex = /@property\s+\{([^}]+)\}\s+/;
                state.currentDoc = state.currentDoc.replace(regex, `@property {$1} ${name} `);
                state.props.push(state.currentDoc);
            }
        }

        if (state.enumLookName) {
            let name = enumName(line);
            if (name !== "") {
                state.enumLookName = false;
                state.enumStarted = true;
                state.currentDoc += 'const ' + name + ' = {\n';
            }
        }

        if (state.enumStarted) {
            let key = enumKey(line);
            if (key !== "") {
                state.currentDoc += '\t' + key + ': "' + key + '",\n'
            }
        }

        if (state.enumStarted && line.startsWith('}')) {
            state.enumStarted = false;
            state.currentDoc += '};';
            state.enums.push(state.currentDoc);
        }
    }

    let doc = "";
    if (!state.hasApidoc) {
        return doc;
    }

    let propsDoc = "";
    for (const p of state.props) {
        propsDoc += p; 
    }

    doc = state.parentDoc.replace('*/', `${propsDoc}*/`);

    for (const en of state.enums) {
        doc += en
    }

    for (const m of state.methods) {
        doc += m;
    }

    return doc;
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
        const name = path.parse(file).base

        const doc = await extractDoc(file);
        if (doc !== "") {
            saveDoc(doc, out, name);
        }
    }

    return 0;
}

main()
