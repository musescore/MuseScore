/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "extpluginsloader.h"

#include "global/io/dir.h"
#include "global/io/file.h"
#include "global/io/fileinfo.h"

#include "log.h"

using namespace muse;
using namespace muse::extensions;
using namespace muse::extensions::legacy;

static Uri makeUri(const io::path_t& rootPath, const io::path_t& path)
{
    return Uri("musescore://extensions/v1" + path.toQString().sliced(rootPath.size()).toLower().toStdString());
}

std::map<std::string /*codeKey*/, Uri> ExtPluginsLoader::loadCodekeyUriMap(const io::path_t& defPath, const io::path_t& extPath) const
{
    std::map<std::string /*codeKey*/, Uri> data;

    auto loadUris = [this](std::map<std::string /*codeKey*/, Uri>& data, const io::path_t& rootPath) {
        io::paths_t paths = qmlsPaths(rootPath);
        for (const io::path_t& path : paths) {
            std::string codeKey = io::completeBasename(path).toStdString();
            Uri uri = makeUri(rootPath, path);
            data.insert({ codeKey, uri });
        }
    };

    loadUris(data, defPath);
    loadUris(data, extPath);

    //! NOTE These plugins have been ported to extensions
    data["colornotes"] = Uri("musescore://extensions/colornotes");
    data["addCourtesyAccidentals"] = Uri("musescore://extensions/courtesy_accidentals");

    return data;
}

ManifestList ExtPluginsLoader::loadManifestList(const io::path_t& defPath, const io::path_t& extPath) const
{
    TRACEFUNC;

    LOGD() << "try load extensions, def: " << defPath << ", user: " << extPath;

    ManifestList defaultManifests = manifestList(defPath);
    ManifestList externalManifests = manifestList(extPath);

    ManifestList retList;
    for (const Manifest& m : defaultManifests) {
        if (!m.isValid()) {
            continue;
        }
        retList.push_back(m);
    }

    for (Manifest& m : externalManifests) {
        if (!m.isValid()) {
            continue;
        }
        m.isRemovable = true;
        retList.push_back(m);
    }

    return retList;
}

ManifestList ExtPluginsLoader::manifestList(const io::path_t& rootPath) const
{
    ManifestList manifests;
    io::paths_t paths = qmlsPaths(rootPath);
    for (const io::path_t& path : paths) {
        Manifest manifest = parseManifest(rootPath, path);
        if (!manifest.isValid()) {
            continue;
        }
        manifest.path = path;
        resolvePaths(manifest, io::FileInfo(path).dirPath());
        manifests.push_back(manifest);
    }

    return manifests;
}

io::paths_t ExtPluginsLoader::qmlsPaths(const io::path_t& rootPath) const
{
    RetVal<io::paths_t> paths = io::Dir::scanFiles(rootPath, { "*.qml" });
    if (!paths.ret) {
        LOGE() << "failed scan files, err: " << paths.ret.toString()
               << ", path: " << rootPath;
    }

    return paths.val;
}

Manifest ExtPluginsLoader::parseManifest(const io::path_t& rootPath, const io::path_t& path) const
{
    ByteArray data;
    Ret ret = io::File::readFile(path, data);
    if (!ret) {
        LOGE() << "failed read file: " << path << ", err: " << ret.toString();
        return Manifest();
    }

    String content = String::fromUtf8(data);

    //! NOTE Check is MuseScore plugin
    {
        size_t bracketPos = 0;
        for (size_t i = 0; i < content.size(); ++i) {
            if (content.at(i) == Char(u'{')) {
                bracketPos = i;
                break;
            }
        }

        if (bracketPos == 0) {
            LOGD() << "Not MuseScore plugin, path: " << path;
            return Manifest();
        }

        String header = content.left(bracketPos);
        if (!content.contains(u"MuseScore")) {
            LOGD() << "Not MuseScore plugin, path: " << path;
            return Manifest();
        }
    }

    io::FileInfo fi(path);

    Manifest m;
    m.uri = makeUri(rootPath, path);
    m.type = Type::Macros;
    m.apiversion = 1;
    m.legacyPlugin = true;

    auto dropQuotes = [](String str) {
        while (str.back() == ';') {
            str.truncate(str.size() - 1);
        }
        if (str.size() < 3) {
            return String();
        }
        if (str.startsWith(u"qsTr(\"") && str.endsWith(u"\")")) {
            return str.mid(6, str.size() - 8);
        }

        if (str.startsWith(u"\"") && str.endsWith(u"\"")) {
            return str.mid(1, str.size() - 2);
        }

        return str;
    };

    String uiCtx = DEFAULT_UI_CONTEXT;
    int needProperties = 7; // title, description, pluginType, category, thumbnail, requiresScore, version
    int propertiesFound = 0;
    bool insideMuseScoreItem = false;
    size_t current, previous = 0;
    current = content.indexOf(u"\n");

    //! NOTE Supposed structure
    //! // comments
    //! import ...
    //! import ...
    //!
    //! MuseScore {
    //!
    //!     prop1: ...
    //!     prop2: ...
    //!     ...
    //!
    //!     As soon as the first open bracket is encountered,
    //!     we consider that the properties have ended.
    //!     Technically, there may be properties further down,
    //!     we just need to ask to move them up.
    //!
    //!     onRun: {..
    //!     function applyMirrorIntervals() {..
    //!     Item { ...
    //!

    while (current != std::string::npos) {
        String line = content.mid(previous, current - previous).trimmed();

        //! NOTE Needed for compatibility with 3.6
        //! We can add properties from 4.4 that are not in 3.6
        if (line.startsWith(u"//4.4 ")) {
            line = line.mid(6);
        }

        if (line.startsWith(u'/')) { // comment
            // noop
        } else if (line.endsWith(u'{')) {
            if (insideMuseScoreItem) {
                break;
            }

            if (!insideMuseScoreItem) {
                insideMuseScoreItem = true;
            }
        } else if (line.startsWith(u"title:")) {
            m.title = dropQuotes(line.mid(6).trimmed());
            ++propertiesFound;
        } else if (line.startsWith(u"description:")) {
            m.description = dropQuotes(line.mid(12).trimmed());
            ++propertiesFound;
        } else if (line.startsWith(u"pluginType:")) {
            String pluginType = dropQuotes(line.mid(11).trimmed());
            if (pluginType == "dialog" || pluginType == "dock") {
                m.type = Type::Form;
            }
            ++propertiesFound;
        } else if (line.startsWith(u"categoryCode:")) {
            m.category = dropQuotes(line.mid(13).trimmed());
            ++propertiesFound;
        } else if (line.startsWith(u"thumbnailName:")) {
            m.thumbnail = dropQuotes(line.mid(14).trimmed()).toStdString();
            ++propertiesFound;
        } else if (line.startsWith(u"requiresScore:")) {
            String requiresScore = dropQuotes(line.mid(14).trimmed());
            if (requiresScore == u"false") {
                uiCtx = "Any";
            }
            ++propertiesFound;
        } else if (line.startsWith(u"version:")) {
            m.version = dropQuotes(line.mid(8).trimmed());
            ++propertiesFound;
        }

        if (propertiesFound == needProperties) {
            break;
        }

        previous = current + 1;
        current = content.indexOf(u"\n", previous);
    }

    if (m.title.isEmpty()) {
        m.title = fi.baseName();
    }

    Action a;
    a.code = "main";
    a.type = m.type;
    a.title = m.title;
    a.uiCtx = uiCtx;
    a.apiversion = m.apiversion;
    a.legacyPlugin = m.legacyPlugin;
    a.path = fi.fileName();
    m.actions.push_back(std::move(a));

    return m;
}

void ExtPluginsLoader::resolvePaths(Manifest& m, const io::path_t& rootDirPath) const
{
    if (!m.thumbnail.empty()) {
        m.thumbnail = rootDirPath + "/" + m.thumbnail;
    }

    for (Action& a : m.actions) {
        a.path = rootDirPath + "/" + a.path;
    }
}
