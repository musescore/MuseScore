const fs = require('node:fs');
const path = require('node:path');
const readline = require('node:readline');

const VERSION = "0.2"
const QML_NS = "Qml"

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
    let braceIdx = line.indexOf("(");
    if (braceIdx === -1) {
        let colonIdx = line.lastIndexOf("::", braceIdx);
        if (colonIdx !== -1) {
            colonIdx += 2 // skip `::`
            name = line.substr(colonIdx, (braceIdx - colonIdx))
        }
    }

    if (name === "") {
        line = line.replace("Q_INVOKABLE", "");
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

function qmlPropName(line) 
{
    const words = line.split(" ");
    if (words.length < 4) {
        return ""
    }

    let name = words[2];
    const colonIdx = name.indexOf(":")
    if (colonIdx !== -1) {
        name = name.substr(0, colonIdx)
    }
    return name;
}

function apiPropArgs(line) {
    const startIdx = line.indexOf("(");
    const endIdx = line.indexOf(")", startIdx);
    if (startIdx !== -1 && endIdx !== -1) {
        const argsStr = line.substr(startIdx + 1, endIdx - startIdx -1);
        return argsStr.split(",");
    }   
    return [];
}

function qpropName(line)
{
    // Q_PROPERTY(bool enabled READ enabled)
    if (line.includes("Q_PROPERTY")) {
        const words = line.split(" ");
        for (let i = 0; i < words.length; ++i) {
            if (words[i] === "READ") {
                let name = words[i - 1];
                return name.trim()
            }
        }
    }

    // API_PROPERTY(subType, SUBTYPE)
    if (line.includes("API_PROPERTY(")) {
        const args = apiPropArgs(line);
        if (args.length >= 1) {
            return args[0].trim();
        } 
    }

    // API_PROPERTY_T(QColor, color,         COLOR)
    if (line.includes("API_PROPERTY_T(")) {
        const args = apiPropArgs(line);
        if (args.length >= 1) {
            return args[1].trim();
        } 
    }

    // API_PROPERTY_READ_ONLY_T(bool, selected, SELECTED)
    if (line.includes("API_PROPERTY_READ_ONLY_T(")) {
        const args = apiPropArgs(line);
        if (args.length >= 2) {
            return args[1].trim();
        } 
    }

    // API_PROPERTY_ENUM(enums::AutoOnOff, hideWhenEmpty, HIDE_WHEN_EMPTY)
    if (line.includes("API_PROPERTY_ENUM(")) {
        const args = apiPropArgs(line);
        if (args.length >= 2) {
            return args[1].trim();
        } 
    }

    return "";
}

function enumName(line)
{
    let name = "";
    line = line.trim()
    let startIdx = line.indexOf("enum");
    if (startIdx === -1) {
        return name;
    }

    startIdx += 4; // skip `enum`
    let classIdx = line.indexOf("class", startIdx);
    if (classIdx !== -1) {
        startIdx = classIdx;
        startIdx += 5; // skip `class`
    }

    let endIdx = line.indexOf(":", startIdx);
    if (endIdx === -1) {
        endIdx = line.indexOf("{", startIdx);
        if (endIdx === -1) {
            endIdx = line.length;
        }
    }

    name = line.substr(startIdx, (endIdx - startIdx))

    return name.trim();
}

function enumKey(line) 
{
    if (line.includes("@ignore")) {
        return "";
    }

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

const NAMESPACE_TYPE = "namespace"
const CLASS_TYPE = "class"
const DECLARE_TYPE = "declare"
const METHOD_TYPE = "method"
const MEMBER_TYPE = "member"
const QPROP_TYPE = "qproperty"
const PROP_TYPE = "property"
const ENUM_TYPE = "enum"

class Doc {
    type = "";
    isQml = false;
    comment = "";
    name = "";
    lookupName = false;
    lookupBody = false;

    constructor(type, comm) {
        this.type = type;
        this.comment = comm;
    }

    isCompleted() {
        return !this.lookupName && !this.lookupBody;
    }

    tagName() {
        const match = this.comment.match(/@name\s+(\S+)/);
        if (match) {
            return match[1];
        }
        return "";
    }

    removeTagName() {
        this.comment = this.comment.replace(/^\s*\*\s*.*@name.*$\n?/gm, '');
    }

    takeTagName() {
        const name = this.tagName();
        if (name !== "") {
            this.removeTagName();
        }
        return name;
    }

    addMemberof(tag, parentName) {
        this.comment = this.comment.replace(tag, `@memberof ${parentName}\n* ${tag}`);
    }
}

class NamespaceDoc extends Doc {

    constructor(comm) {
        super(NAMESPACE_TYPE, comm);
    }

    processComment(parentName) {
        const match = this.comment.match(/@namespace\s+(\S+)/);
        if (match) {
            this.isParent = true;
            this.name = match[1];
        }
    }
}

class ClassDoc extends Doc {

    constructor(comm) {
        super(CLASS_TYPE, comm);
    }

    processComment(parentName) {
        const match = this.comment.match(/@class\s+(\S+)/);
        if (match) {
            this.isParent = true;

            // For Qml we can explicitly specify memberof, 
            // for C++ memberof can be specified (preferably), but it may not.
            if (this.isQml) {
                this.name = QML_NS+"."+match[1];
                this.comment = this.comment.replace('@class', `@memberof ${QML_NS}\n* @class`);
            } else {
                const memberofMatch = this.comment.match(/@memberof\s+(\S+)/);
                if (memberofMatch) {
                    this.name = memberofMatch[1]+"."+match[1];
                } else {
                    this.name = match[1];
                }
            }
        }
    }
}

class DeclareDoc extends Doc {

    constructor(comm) {
        super(DECLARE_TYPE, comm);
    }

    processComment(parentName) {
        this.isParent = true;
        const match = this.comment.match(/@declare\s+(\S+)/);
        if (match) {
            const memberofMatch = this.comment.match(/@memberof\s+(\S+)/);
            if (memberofMatch) {
                this.name = memberofMatch[1]+"."+match[1];
            } else {
                this.name= match[1];
            }
        }
    }
}

class MethodDoc extends Doc {

    constructor(comm) {
        super(METHOD_TYPE, comm);
    }

    processComment(parentName) {
        this.addMemberof("@method", parentName);
        const name = this.takeTagName();
        if (name !== "") {
            this.processName(undefined, name);
        } else {
            this.lookupName = true;
        }
    }

    processName(line, name) {
        name = name ? name : nameFromSig(line);
        if (name !== "") {
            // add name
            this.comment = this.comment.replace('@method', `@method ${name}`);
            this.lookupName = false;
        }
    }
}

class MemberDoc extends Doc {

    constructor(comm) {
        super(MEMBER_TYPE, comm);
    }

    processComment(parentName) {
        this.addMemberof("@member", parentName);
        const name = this.takeTagName();
        if (name !== "") {
            this.processName(undefined, name);
        } else {
            this.lookupName = true;
        }
    }

    processName(line, name) {
        if (!name) {
            if (this.isQml) {
                name = qmlPropName(line);
            } else {
                name = nameFromSig(line);
            }
        }

        if (name !== "") {
            // add name 
            const regex = /@member\s+\{([^}]+)\}\s+/;
            this.comment = this.comment.replace(regex, `@member {$1} ${name}\n`);
            this.lookupName = false;
        }
    }
}

class QPropDoc extends Doc {

    constructor(comm) {
        super(QPROP_TYPE, comm);
    }

    processComment(parentName) {
        // add memberof and make as member
        this.comment = this.comment.replace('@q_property', `@memberof ${parentName}\n* @member`);
        const name = this.takeTagName();
        if (name !== "") {
            this.processName(undefined, name);
        } else {
            this.lookupName = true;
        }
    }

    processName(line, name) {
        name = name ? name : qpropName(line);
        if (name !== "") {
            // add name 
            const regex = /@member\s+\{([^}]+)\}\s+/;
            this.comment = this.comment.replace(regex, `@member {$1} ${name}\n`);
            this.lookupName = false;
        }
    }
}

class PropDoc extends Doc {

    constructor(comm) {
        super(PROP_TYPE, comm);
    }

    processComment(parentName) {
        this.comment = this.comment.replace('/** ', `*`);
        this.comment = this.comment.replace('/**', `*`);
        this.comment = this.comment.replace('*/', ``);
        this.lookupName = true;
    }

    processName(line, name) {
        if (!name) {
            if (this.isQml) {
                name = qmlPropName(line);
            } else {
                name = nameFromSig(line);
            }
        }

        if (name !== "") {
            // add name 
            const regex = /@property\s+\{([^}]+)\}\s+/;
            this.comment = this.comment.replace(regex, `@property {$1} ${name} `);
            this.lookupName = false;
        }
    }
}

class EnumDoc extends Doc {

    constructor(comm) {
        super(ENUM_TYPE, comm);
    }

    processComment(parentName) {
        const name = this.takeTagName();
        if (name !== "") {
            this.processName(undefined, name);
        } else {
            this.lookupName = true;
        }
    }

    processName(line, name) {
        name = name ? name : enumName(line);
        if (name !== "") {
            this.comment += `const ${name} = {\n`;
            this.lookupName = false;
            this.lookupBody = true;
        }
    }

    processBody(line) {
        let key = enumKey(line);
        if (key !== "") {
            this.comment += '\t' + key + ': "' + key + '",\n'
        }

        if (line.startsWith('}')) {
            this.lookupBody = false;
            this.comment += '};';
        }
    }
}

async function extractDoc(file)
{   
    const isQml = path.parse(file).ext === ".qml";

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

        currentComment: "",
        parentDoc: null,
        doc: null,
        docs: [],
        
        doclets: [],

        makeDoclet: function() {
            let doclet = "";

            if (state.parentDoc) {
                let propsDoc = "";
                for (const d of state.docs) {
                    if (d.type === PROP_TYPE) {
                        propsDoc += d.comment; 
                    }
                }

                doclet = state.parentDoc.comment.replace('*/', `${propsDoc}*/`);
            }

            for (const d of state.docs) {
                if (d.type !== PROP_TYPE) {
                    doclet += d.comment;
                }
            }

            this.doclets.push(doclet);

            // reset
            this.currentComment = "";
            this.parentDoc = null;
            this.doc = null;
            this.docs = [];
        },
    }                                   

    for await (let line of rl) {
        line = line.trim()

        // start APIDOC
        if (line.startsWith(APIDOC_BEGIN)) {
            // remove APIDOC
            line = line.replace("APIDOC", "");
            state.hasApidoc = true;
            state.apidocStarted = true;
            state.currentComment = "";
        }

        // collect APIDOC comment
        if (state.apidocStarted) {
            state.currentComment += line + "\n";
        }

        // end APIDOC
        if (state.apidocStarted && line.endsWith(APIDOC_END)) {
            state.apidocStarted = false;

            // check of kind 
            if (state.currentComment.includes('@namespace')) {
                state.doc = new NamespaceDoc(state.currentComment);
            } else if (state.currentComment.includes('@class')) {
                state.doc = new ClassDoc(state.currentComment);
            } else if (state.currentComment.includes('@declare')) {
                state.doc = new DeclareDoc(state.currentComment);
            } else if (state.currentComment.includes('@method')) {
                state.doc = new MethodDoc(state.currentComment)
            } else if (state.currentComment.includes('@member ')) {
                state.doc = new MemberDoc(state.currentComment)
            } else if (state.currentComment.includes('@property')) {
                state.doc = new PropDoc(state.currentComment)
            } else if (state.currentComment.includes('@q_property')) {
                state.doc = new QPropDoc(state.currentComment)
            } else if (state.currentComment.includes('@enum')) {
                state.doc = new EnumDoc(state.currentComment)
            }

            state.doc.isQml = isQml;
            state.doc.processComment(state.parentDoc ? state.parentDoc.name : undefined)

            if (state.doc.isParent) {  // like namespace, class, declare
                if (state.parentDoc) {
                    state.makeDoclet();
                } 
                state.parentDoc = state.doc; 
            } else {
                if (state.doc.isCompleted()) {
                    // finish
                    state.docs.push(state.doc);
                }
            }
        }

        // look name if need after APIDOC
        if (state.doc && state.doc.lookupName) {
            state.doc.processName(line);
            if (state.doc.isCompleted()) {
                // finish
                state.docs.push(state.doc);
            }
        }

        // look body if need after APIDOC
        if (state.doc && state.doc.lookupBody) {
            state.doc.processBody(line);

            if (state.doc.isCompleted()) {
                // finish
                state.docs.push(state.doc);
            }
        }
    }

    if (!state.hasApidoc) {
        return "";
    }

    // last 
    state.makeDoclet();

    let doc = "";
    for (let d of state.doclets) {
        doc += d;
        doc += '\n\n\n'
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
    if (args.length === 0) {
        return;
    }

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

module.exports = {
    extractDoc,
};