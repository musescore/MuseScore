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

#include "global/serialization/json.h"

#include "audio/audiotypes.h"
#include "audiopluginsutils.h"

#include "log.h"

using namespace muse;
using namespace muse::audioplugins;
using namespace muse::audio;

namespace muse::audioplugins {
static const std::map<audio::AudioResourceType, std::string> RESOURCE_TYPE_TO_STRING_MAP {
    { audio::AudioResourceType::VstPlugin, "VstPlugin" },
};

static JsonObject attributesToJson(const AudioResourceAttributes& attributes)
{
    JsonObject result;

    for (auto it = attributes.cbegin(); it != attributes.cend(); ++it) {
        if (it->first == audio::PLAYBACK_SETUP_DATA_ATTRIBUTE) {
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
    result.set("type", muse::value(RESOURCE_TYPE_TO_STRING_MAP, meta.type, "Undefined"));
    result.set("hasNativeEditorSupport", meta.hasNativeEditorSupport);

    if (!meta.vendor.empty()) {
        result.set("vendor", meta.vendor);
    }

    JsonObject attributesJson = attributesToJson(meta.attributes);
    if (!attributesJson.empty()) {
        result.set("attributes", attributesJson);
    }

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
    result.type = muse::key(RESOURCE_TYPE_TO_STRING_MAP, object.value("type").toStdString());
    result.vendor = object.value("vendor").toStdString();
    result.hasNativeEditorSupport = object.value("hasNativeEditorSupport").toBool();

    JsonValue attributes = object.value("attributes");
    if (attributes.isObject()) {
        result.attributes = attributesFromJson(attributes.toObject());
    }

    return result;
}
}

Ret KnownAudioPluginsRegister::load()
{
    TRACEFUNC;

    m_loaded = false;
    m_pluginInfoMap.clear();
    m_pluginPaths.clear();

    io::path_t knownAudioPluginsPath = configuration()->knownAudioPluginsFilePath();
    if (!fileSystem()->exists(knownAudioPluginsPath)) {
        m_loaded = true;
        return muse::make_ok();
    }

    RetVal<ByteArray> file = fileSystem()->readFile(knownAudioPluginsPath);
    if (!file.ret) {
        return file.ret;
    }

    std::string err;
    JsonDocument json = JsonDocument::fromJson(file.val, &err);
    if (!err.empty()) {
        return Ret(static_cast<int>(Ret::Code::UnknownError), err);
    }

    JsonArray array = json.rootArray();

    for (size_t i = 0; i < array.size(); ++i) {
        JsonObject object = array.at(i).toObject();

        AudioPluginInfo info;
        info.meta = metaFromJson(object.value("meta").toObject());
        info.meta.attributes.emplace(audio::PLAYBACK_SETUP_DATA_ATTRIBUTE, mpe::GENERIC_SETUP_DATA_STRING);
        info.type = audioPluginTypeFromCategoriesString(info.meta.attributeVal(audio::CATEGORIES_ATTRIBUTE));
        info.path = object.value("path").toString();
        info.enabled = object.value("enabled").toBool();
        info.errorCode = object.value("errorCode").toInt();

        m_pluginPaths.insert(info.path);
        m_pluginInfoMap.emplace(info.meta.id, std::move(info));
    }

    m_loaded = true;
    return muse::make_ok();
}

std::vector<AudioPluginInfo> KnownAudioPluginsRegister::pluginInfoList(PluginInfoAccepted accepted) const
{
    if (!accepted) {
        return muse::values(m_pluginInfoMap);
    }

    std::vector<AudioPluginInfo> result;

    for (auto it = m_pluginInfoMap.cbegin(); it != m_pluginInfoMap.cend(); ++it) {
        if (accepted(it->second)) {
            result.push_back(it->second);
        }
    }

    return result;
}

const io::path_t& KnownAudioPluginsRegister::pluginPath(const AudioResourceId& resourceId) const
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
    return muse::contains(m_pluginPaths, pluginPath);
}

bool KnownAudioPluginsRegister::exists(const AudioResourceId& resourceId) const
{
    return muse::contains(m_pluginInfoMap, resourceId);
}

Ret KnownAudioPluginsRegister::registerPlugin(const AudioPluginInfo& info)
{
    IF_ASSERT_FAILED(m_loaded) {
        return false;
    }

    auto it = m_pluginInfoMap.find(info.meta.id);
    if (it != m_pluginInfoMap.end()) {
        IF_ASSERT_FAILED(it->second.path != info.path) {
            return false;
        }
    }

    m_pluginInfoMap.emplace(info.meta.id, info);
    m_pluginPaths.insert(info.path);

    Ret ret = writePluginsInfo();
    return ret;
}

Ret KnownAudioPluginsRegister::unregisterPlugin(const AudioResourceId& resourceId)
{
    IF_ASSERT_FAILED(m_loaded) {
        return false;
    }

    if (!exists(resourceId)) {
        return muse::make_ok();
    }

    for (const auto& pair : m_pluginInfoMap) {
        if (pair.first == resourceId) {
            muse::remove(m_pluginPaths, pair.second.path);
        }
    }

    m_pluginInfoMap.erase(resourceId);

    Ret ret = writePluginsInfo();
    return ret;
}

Ret KnownAudioPluginsRegister::writePluginsInfo()
{
    TRACEFUNC;

    JsonArray array;

    for (const auto& pair : m_pluginInfoMap) {
        const AudioPluginInfo& info = pair.second;

        JsonObject obj;
        obj.set("meta", metaToJson(info.meta));
        obj.set("path", info.path.toStdString());
        obj.set("enabled", info.enabled);

        if (info.errorCode != 0) {
            obj.set("errorCode", info.errorCode);
        }

        array << obj;
    }

    io::path_t knownAudioPluginsPath = configuration()->knownAudioPluginsFilePath();
    Ret ret = fileSystem()->writeFile(knownAudioPluginsPath, JsonDocument(array).toJson());

    return ret;
}
