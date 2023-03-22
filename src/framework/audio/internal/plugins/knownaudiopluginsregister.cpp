/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "knownaudiopluginsregister.h"

#include "serialization/json.h"

#include "log.h"

using namespace mu::audio;

namespace mu::audio {
static const std::map<AudioResourceType, std::string> RESOURCE_TYPE_TO_STRING_MAP {
    { AudioResourceType::VstPlugin, "VstPlugin" },
};

static JsonObject attributesToJson(const AudioResourceAttributes& attributes)
{
    JsonObject result;

    for (auto it = attributes.cbegin(); it != attributes.cend(); ++it) {
        if (it->second == audio::PLAYBACK_SETUP_DATA_ATTRIBUTE) {
            continue;
        }

        result.set(it->first.toStdString(), it->second.toStdString());
    }

    return result;
}

static JsonObject metaToJson(const AudioResourceMeta& meta)
{
    JsonObject result;

    result.set("id", meta.id);
    result.set("type", mu::value(RESOURCE_TYPE_TO_STRING_MAP, meta.type, "Undefined"));
    result.set("vendor", meta.vendor);
    result.set("attributes", attributesToJson(meta.attributes));
    result.set("hasNativeEditorSupport", meta.hasNativeEditorSupport);

    return result;
}

static AudioResourceAttributes attributesFromJson(const JsonObject& object)
{
    AudioResourceAttributes result;

    for (const std::string& key : object.keys()) {
        result.insert({ String::fromStdString(key), object.value(key).toString() });
    }

    return result;
}

static AudioResourceMeta metaFromJson(const JsonObject& object)
{
    AudioResourceMeta result;

    result.id = object.value("id").toStdString();
    result.type = mu::key(RESOURCE_TYPE_TO_STRING_MAP, object.value("type").toStdString());
    result.vendor = object.value("vendor").toStdString();
    result.attributes = attributesFromJson(object.value("attributes").toObject());
    result.hasNativeEditorSupport = object.value("hasNativeEditorSupport").toBool();

    return result;
}
}

mu::Ret KnownAudioPluginsRegister::load()
{
    TRACEFUNC;

    io::path_t knownAudioPluginsDir = configuration()->knownAudioPluginsDir();

    if (!fileSystem()->exists(knownAudioPluginsDir)) {
        fileSystem()->makePath(knownAudioPluginsDir);
    }

    RetVal<io::paths_t> paths = fileSystem()->scanFiles(knownAudioPluginsDir,
                                                        { "*.json" },
                                                        io::ScanMode::FilesInCurrentDir);
    if (!paths.ret) {
        return paths.ret;
    }

    m_pluginInfoMap.clear();
    m_pluginPaths.clear();

    for (const io::path_t& infoPath : paths.val) {
        RetVal<ByteArray> file = fileSystem()->readFile(infoPath);
        if (!file.ret) {
            LOGE() << file.ret.toString();
            continue;
        }

        std::string err;
        JsonDocument json = JsonDocument::fromJson(file.val, &err);
        if (!err.empty()) {
            LOGE() << err;
            continue;
        }

        JsonObject object = json.rootObject();

        AudioPluginInfo info;
        info.meta = metaFromJson(object.value("meta").toObject());
        info.meta.attributes.insert({ audio::PLAYBACK_SETUP_DATA_ATTRIBUTE, mpe::GENERIC_SETUP_DATA_STRING });
        info.type = audioPluginTypeFromCategoriesString(info.meta.attributeVal(audio::CATEGORIES_ATTRIBUTE).toStdString());
        info.path = object.value("path").toString();
        info.enabled = object.value("enabled").toBool();
        info.errorCode = object.value("errorCode").toInt();

        m_pluginInfoMap[info.meta.id] = info;
        m_pluginPaths.insert(info.path);
    }

    return make_ok();
}

std::vector<AudioPluginInfo> KnownAudioPluginsRegister::pluginInfoList(PluginInfoAccepted accepted) const
{
    if (!accepted) {
        return mu::values(m_pluginInfoMap);
    }

    std::vector<AudioPluginInfo> result;

    for (auto it = m_pluginInfoMap.cbegin(); it != m_pluginInfoMap.cend(); ++it) {
        if (accepted(it->second)) {
            result.push_back(it->second);
        }
    }

    return result;
}

const mu::io::path_t& KnownAudioPluginsRegister::pluginPath(const AudioResourceId& resourceId) const
{
    auto it = m_pluginInfoMap.find(resourceId);
    if (it == m_pluginInfoMap.end()) {
        static const io::path_t _dummy;
        return _dummy;
    }

    return it->second.path;
}

bool KnownAudioPluginsRegister::exists(const io::path_t& pluginPath) const
{
    return mu::contains(m_pluginPaths, pluginPath);
}

bool KnownAudioPluginsRegister::exists(const AudioResourceId& resourceId) const
{
    return mu::contains(m_pluginInfoMap, resourceId);
}

mu::Ret KnownAudioPluginsRegister::registerPlugin(const AudioPluginInfo& info)
{
    TRACEFUNC;

    JsonObject obj;
    obj.set("meta", metaToJson(info.meta));
    obj.set("path", info.path.toStdString());
    obj.set("enabled", info.enabled);

    if (info.errorCode != 0) {
        obj.set("errorCode", info.errorCode);
    }

    io::path_t path = pluginInfoPath(info.meta.vendor, info.meta.id);
    Ret ret = fileSystem()->writeFile(path, JsonDocument(obj).toJson());
    if (!ret) {
        return ret;
    }

    m_pluginInfoMap[info.meta.id] = info;
    m_pluginPaths.insert(info.path);

    return make_ok();
}

mu::Ret KnownAudioPluginsRegister::unregisterPlugin(const AudioResourceId& resourceId)
{
    TRACEFUNC;

    if (!exists(resourceId)) {
        return make_ok();
    }

    AudioPluginInfo info = m_pluginInfoMap[resourceId];
    io::path_t path = pluginInfoPath(info.meta.vendor, resourceId);

    Ret ret = fileSystem()->remove(path);
    if (!ret) {
        return ret;
    }

    mu::remove(m_pluginInfoMap, resourceId);
    mu::remove(m_pluginPaths, info.path);

    return make_ok();
}

mu::io::path_t KnownAudioPluginsRegister::pluginInfoPath(const AudioResourceVendor& vendor, const AudioResourceId& resourceId) const
{
    io::path_t fileName;

    if (vendor.empty()) {
        fileName = io::escapeFileName(resourceId);
    } else {
        fileName = io::escapeFileName(vendor + "_" + resourceId);
    }

    return configuration()->knownAudioPluginsDir() + "/" + fileName + ".json";
}
