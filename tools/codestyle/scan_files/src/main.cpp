#include <iostream>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

static const char* VERSION = "0.1";

struct Filter {
    std::string ignoreFile;
    std::vector<std::string> exts;
};

template<typename T>
static bool contains(const std::vector<T>& vec, const T& val)
{
    auto it = std::find(vec.cbegin(), vec.cend(), val);
    return it != vec.cend();
}

static void split(const std::string& str, std::vector<std::string>& out, const std::string& delim)
{
    std::size_t current, previous = 0;
    current = str.find(delim);
    std::size_t delimLen = delim.length();

    while (current != std::string::npos) {
        out.push_back(str.substr(previous, current - previous));
        previous = current + delimLen;
        current = str.find(delim, previous);
    }
    out.push_back(str.substr(previous, current - previous));
}

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

void printHelp()
{
    std::cout << "Hello World! I am scan files!" << "\n";
    std::cout << "This is a utility for recursive scanning of files with the ability to filter by extension and ignore directories. \n";
    std::cout << "use:\n";
    std::cout << "-d  --dir /path                       root dir for scan\n";
    std::cout << "-i  --ignore filename                 if present this file, dir (and subdirs) will be ignored\n";
    std::cout << "-e  --extensions ext1,extn            allowed extensions\n";
    std::cout << "-h, --help                            this help\n";
    std::cout << "-v, --version                         print version\n";
}

int main(int argc, char** argv)
{
    // default
    std::string dir = ".";
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
            } else if (arg == "-d" || arg == "--dirr") {
                dir = args.at(++i);
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
                std::cout << "invalid option -- '" << arg << "', try 'scan_files --help' for more information.\n";
            }
        }
    }

    std::vector<fs::path> files;
    scan(files, fs::path(dir), filter);

    for (const fs::path& f: files) {
        std::cout << f.generic_string() << std::endl;
    }

    return 0;
}
