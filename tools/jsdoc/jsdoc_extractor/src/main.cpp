#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

#include "utils.h"

namespace fs = std::filesystem;

static const char* VERSION = "0.1";
static const std::string APIDOC_BEGIN = "/** APIDOC";
static const std::string APIDOC_END = "*/";
static const std::string APIDOC_NSPACE = "namespace:";
static const std::string APIDOC_METHOD = "method:";

struct Filter {
    std::string ignoreFile;
    std::vector<std::string> exts;
};

static void scan(std::vector<fs::path>& files, const fs::path& root, const Filter& filter)
{
    if (!filter.ignoreFile.empty() && fs::exists(root / fs::path(filter.ignoreFile))) {
        // ignore
        return;
    }

    for (auto& item : fs::directory_iterator(root)) {
        fs::path itemPath = item.path();
        if (fs::is_directory(item.path())) {
            // recursion
            scan(files, itemPath, filter);
        } else if (fs::is_regular_file(itemPath)) {
            if (filter.exts.empty() || contains(filter.exts, itemPath.extension().string())) {
                files.push_back(itemPath);
            }
        }
    }
}

static std::string extracDoc(const fs::path& fname)
{
    std::ifstream f;
    f.open(fname);

    struct State {
        bool nspaceStared = false;
        std::string nspaceName;
        std::string methodName;
    };

    std::string doc;
    std::stringstream ts;
    State state;

    std::string line;
    while (f) {
        std::getline(f, line);

        trim(line);

        if (startsWith(line, APIDOC_BEGIN)) {
            // remove /** APIDOC
            line = line.substr(APIDOC_BEGIN.size());
            ltrim(line);

            // check namespace
            // namespace: interactive
            if (startsWith(line, APIDOC_NSPACE)) {
                state.nspaceName = line.substr(APIDOC_NSPACE.size());
                ltrim(state.nspaceName);
                state.nspaceStared = true;
                ts << "/**\n";
            }
            // check method
            // method: info(title, text)
            else if (startsWith(line, APIDOC_METHOD)) {
                state.methodName = line.substr(APIDOC_METHOD.size());
                ltrim(state.methodName);
                ts << "\t" << "/**\n";
            }

            continue;
        }

        if (startsWith(line, APIDOC_END)) {
            // write ns
            if (!state.nspaceName.empty()) {
                ts << "*/\n";
                ts << "const " << state.nspaceName << " = {\n";
                state.nspaceName.clear();
            }
            // write method
            else if (!state.methodName.empty()) {
                ts << "\t*/\n";
                ts << "\t" << state.methodName << " {},\n";
                ts << "\n";
                state.methodName.clear();
            }

            continue;
        }

        if (!state.nspaceName.empty()) {
            ts << line << "\n";
            continue;
        }

        if (!state.methodName.empty()) {
            ts << "\t" << line << "\n";
            continue;
        }
    }

    if (state.nspaceStared) {
        ts << "};";
    }

    return ts.str();
}

static void saveDoc(const std::string& doc, const fs::path& outDir, const std::string& name)
{
    fs::path fname = outDir / name;
    fname += ".js";
    std::fstream(fname, std::ios::out | std::ios::trunc) << doc;
}

static void printHelp()
{
    std::cout << "Hello World! I am jsdoc extractor!" << "\n";
    std::cout << "This is a utility for scanning files, extracting documentation from them and creating JS files. \n";
    std::cout << "use:\n";
    std::cout << "-d  --dir /path                       root dir for scan\n";
    std::cout << "-o  --out /path                       output path\n";
    std::cout << "-i  --ignore filename                 if present this file, dir (and subdirs) will be ignored\n";
    std::cout << "-e  --extensions ext1,extn            allowed extensions\n";
    std::cout << "-h, --help                            this help\n";
    std::cout << "-v, --version                         print version\n";
}

int main(int argc, char** argv)
{
    // default
    std::string dir = ".";
    std::string out = "./out";
    Filter filter;

    // parse args
    {
        std::vector<std::string> args;
        for (int a = 0; a < argc; ++a) {
            args.push_back(argv[a]);
        }

        for (size_t i = 0; i < args.size(); ++i) {
            if (i == 0) {
                continue;
            }

            const std::string& arg = args.at(i);
            if (arg == "-v" || arg == "--version") {
                std::cout << "scan_files version: " << VERSION << "\n";
            } else if (arg == "-h" || arg == "--help") {
                printHelp();
            } else if (arg == "-d" || arg == "--dir") {
                dir = args.at(++i);
            } else if (arg == "-o" || arg == "--out") {
                out = args.at(++i);
            } else if (arg == "-i" || arg == "--ignore") {
                filter.ignoreFile = args.at(++i);
            } else if (arg == "-e" || arg == "--extensions") {
                std::string extsStr = args.at(++i);
                std::vector<std::string> exts;
                split(extsStr, exts, ",");
                for (const std::string& e : exts) {
                    if (e.empty()) {
                        continue;
                    }

                    if (e.at(0) != '.') {
                        filter.exts.push_back('.' + e);
                    } else {
                        filter.exts.push_back(e);
                    }
                }
            } else {
                std::cout << "invalid option -- '" << arg << "', try '--help' for more information.\n";
            }
        }
    }

    std::vector<fs::path> files;
    scan(files, fs::path(dir), filter);

    if (files.empty()) {
        std::cout << "not found files\n";
        return 0;
    }

    std::filesystem::create_directories(out);

    for (const fs::path& f: files) {
        if (f.stem() != "interactiveapi") {
            continue;
        }

        std::cout << f.generic_string() << std::endl;
        std::string doc = extracDoc(f);
        if (!doc.empty()) {
            saveDoc(doc, out, f.stem());
        }
    }

    return 0;
}
