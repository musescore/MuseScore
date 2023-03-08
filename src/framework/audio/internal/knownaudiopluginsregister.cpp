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

#include <QJsonDocument>
#include <QJsonObject>

#include "log.h"

using namespace mu::audio;

static const std::map<AudioPluginType, QString> PLUGIN_TYPE_TO_STRING_MAP = {
    { AudioPluginType::Undefined, "Undefined" },
    { AudioPluginType::Instrument, "Instrument" },
    { AudioPluginType::Fx, "Fx" },
};

static const std::map<AudioResourceType, QString> RESOURCE_TYPE_TO_STRING_MAP {
    { AudioResourceType::VstPlugin, "VstPlugin" },
};

static QJsonObject metaToJson(const AudioResourceMeta& meta)
{
    QJsonObject result;

    result.insert(QStringLiteral("id"), QString::fromStdString(meta.id));
    result.insert(QStringLiteral("type"), mu::value(RESOURCE_TYPE_TO_STRING_MAP, meta.type, "Undefined"));
    result.insert(QStringLiteral("vendor"), QString::fromStdString(meta.vendor));
    result.insert(QStringLiteral("hasNativeEditorSupport"), meta.hasNativeEditorSupport);

    return result;
}

static AudioResourceMeta metaFromJson(const QJsonObject& object)
{
    AudioResourceMeta result;

    result.id = object.value(QStringLiteral("id")).toString().toStdString();
    result.type = mu::key(RESOURCE_TYPE_TO_STRING_MAP, object.value(QString("type")).toString());
    result.vendor = object.value(QStringLiteral("vendor")).toString().toStdString();
    result.hasNativeEditorSupport = object.value(QStringLiteral("hasNativeEditorSupport")).toBool();

    return result;
}

void KnownAudioPluginsRegister::init()
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
        LOGE() << paths.ret.toString();
    }

    for (const io::path_t& infoPath : paths.val) {
        RetVal<ByteArray> file = fileSystem()->readFile(infoPath);
        if (!file.ret) {
            LOGE() << file.ret.toString();
            continue;
        }

        QJsonDocument json = QJsonDocument::fromJson(file.val.toQByteArrayNoCopy());
        QJsonObject object = json.object();

        AudioPluginInfo info;
        info.type = mu::key(PLUGIN_TYPE_TO_STRING_MAP, object.value(QStringLiteral("type")).toString());
        info.meta = metaFromJson(object.value(QStringLiteral("meta")).toObject());
        info.path = object.value(QStringLiteral("path")).toString().toStdString();
        info.enabled = object.value(QStringLiteral("enabled")).toBool();
        info.errorCode = object.value(QStringLiteral("errorCode")).toInt(0);

        if (info.errorCode == 0) {
            info.meta.attributes = { { u"playbackSetupData", mpe::GENERIC_SETUP_DATA_STRING } };
        }

        m_pluginInfoMap[info.meta.id] = info;
        m_pluginPaths.insert(info.path);
    }
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
        static const io::path_t dummy;
        return dummy;
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

    QJsonObject obj;
    obj.insert(QStringLiteral("type"), mu::value(PLUGIN_TYPE_TO_STRING_MAP, info.type));
    obj.insert(QStringLiteral("meta"), metaToJson(info.meta));
    obj.insert(QStringLiteral("path"), info.path.toQString());
    obj.insert(QStringLiteral("enabled"), info.enabled);

    if (info.errorCode != 0) {
        obj.insert(QStringLiteral("errorCode"), info.errorCode);
    }

    io::path_t path = pluginInfoPath(info.meta.id);
    Ret ret = fileSystem()->writeFile(path, ByteArray::fromQByteArrayNoCopy(QJsonDocument(obj).toJson()));
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

    io::path_t path = pluginInfoPath(resourceId);
    Ret ret = fileSystem()->remove(path);
    if (!ret) {
        return ret;
    }

    AudioPluginInfo info = mu::take(m_pluginInfoMap, resourceId);
    mu::remove(m_pluginPaths, info.path);

    return make_ok();
}

mu::io::path_t KnownAudioPluginsRegister::pluginInfoPath(const AudioResourceId& resourceId) const
{
    return configuration()->knownAudioPluginsDir() + "/" + resourceId + ".json";
}
