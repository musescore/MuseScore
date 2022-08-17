/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "vstmodulesmetaregister.h"

#include <QJsonDocument>

#include "io/file.h"

using namespace mu;
using namespace mu::vst;

static const std::map<VstPluginType, QString> PLUGIN_TYPE_MAP = {
    { VstPluginType::Instrument, "Instrument" },
    { VstPluginType::Fx, "Fx" }
};

void VstModulesMetaRegister::init()
{
    m_knownPluginsDir = config()->knownPluginsDir();

    load();
}

void VstModulesMetaRegister::registerPath(const audio::AudioResourceId& resourceId, const io::path_t& path)
{
    if (!fileSystem()->exists(m_knownPluginsDir)) {
        fileSystem()->makePath(m_knownPluginsDir);
    }

    io::File file(m_knownPluginsDir + "/" + resourceId + ".json");
    file.open(io::IODevice::WriteOnly);

    QJsonObject obj;
    obj.insert(QStringLiteral("enabled"), false);
    obj.insert(QStringLiteral("type"), "");
    obj.insert(QStringLiteral("meta"), "");
    obj.insert(QStringLiteral("path"), path.toQString());

    m_paths.emplace(resourceId, path);

    file.write(QJsonDocument(obj).toJson());
    file.close();
}

void VstModulesMetaRegister::registerPlugin(const audio::AudioResourceId& resourceId, PluginModulePtr module, const bool enabled)
{
    auto pluginPath = m_paths.find(resourceId);
    if (pluginPath == m_paths.cend()) {
        return;
    }

    io::File file(m_knownPluginsDir + "/" + resourceId + ".json");
    file.open(io::IODevice::WriteOnly);

    const auto& factory = module->getFactory();

    for (auto& classInfo : factory.classInfos()) {
        if (classInfo.category() != kVstAudioEffectClass) {
            continue;
        }

        std::string subCategoriesStr = classInfo.subCategoriesString();

        for (const auto& type : PLUGIN_TYPE_MAP) {
            VstPluginType currentType = VstPluginType::Undefined;

            if (subCategoriesStr.find(type.second.toStdString()) != std::string::npos) {
                currentType = type.first;
            }

            if (currentType == VstPluginType::Undefined) {
                continue;
            }

            audio::AudioResourceMeta meta;

            meta.id = resourceId;
            meta.type = audio::AudioResourceType::VstPlugin;
            meta.vendor = factory.info().vendor();
            meta.hasNativeEditorSupport = hasNativeEditorSupport();

            QJsonObject obj;
            obj.insert(QStringLiteral("enabled"), enabled);
            obj.insert(QStringLiteral("type"), pluginTypeToString(currentType));
            obj.insert(QStringLiteral("meta"), metaToJson(meta));
            obj.insert(QStringLiteral("path"), pluginPath->second.toQString());

            m_metaMap[currentType].insert(std::move(meta));

            file.write(QJsonDocument(obj).toJson());
            file.close();

            return;
        }
    }
}

void VstModulesMetaRegister::clear()
{
    m_paths.clear();
    m_metaMap.clear();
}

const io::path_t& VstModulesMetaRegister::pluginPath(const audio::AudioResourceId& resourceId) const
{
    auto search = m_paths.find(resourceId);

    if (search == m_paths.cend()) {
        static io::path_t empty;
        return empty;
    }

    return search->second;
}

const audio::AudioResourceMetaList& VstModulesMetaRegister::metaList(const VstPluginType type) const
{
    auto search = m_metaMap.find(type);
    if (search == m_metaMap.cend()) {
        static audio::AudioResourceMetaList empty;
        return empty;
    }

    static audio::AudioResourceMetaList result;
    result.clear();
    result.insert(result.cbegin(), search->second.cbegin(), search->second.cend());

    return result;
}

bool VstModulesMetaRegister::exists(const audio::AudioResourceId& resourceId) const
{
    return !pluginPath(resourceId).empty();
}

bool VstModulesMetaRegister::exists(const io::path_t& path) const
{
    return !pluginPath(io::basename(path).toStdString()).empty();
}

bool VstModulesMetaRegister::isEmpty() const
{
    return m_metaMap.empty() && m_paths.empty();
}

void VstModulesMetaRegister::load()
{
    RetVal<io::paths_t> paths = fileSystem()->scanFiles(m_knownPluginsDir,
                                                        { "*.json" },
                                                        io::ScanMode::FilesInCurrentDir);

    for (const io::path_t& path : paths.val) {
        io::File file(path);
        file.open(io::IODevice::ReadOnly);

        QJsonDocument json = QJsonDocument::fromJson(file.readAll().toQByteArrayNoCopy());
        QJsonObject object = json.object();

        bool isEnabled = object.value(QStringLiteral("enabled")).toBool();
        if (isEnabled) {
            QJsonValue metaJson = object.value(QStringLiteral("meta"));

            audio::AudioResourceMeta meta = metaFromJson(metaJson.toObject());
            VstPluginType type = pluginTypeFromString(object.value(QStringLiteral("type")).toString());

            m_metaMap[type].insert(std::move(meta));
        }

        io::path_t pluginPath(object.value(QStringLiteral("path")).toString().toStdString());
        m_paths.emplace(io::basename(pluginPath).toStdString(), std::move(pluginPath));

        file.close();
    }
}

QJsonObject VstModulesMetaRegister::metaToJson(const audio::AudioResourceMeta& meta) const
{
    QJsonObject result;

    result.insert(QStringLiteral("id"), QString::fromStdString(meta.id));
    result.insert(QStringLiteral("vendor"), QString::fromStdString(meta.vendor));
    result.insert(QStringLiteral("hasNativeEditorSupport"), meta.hasNativeEditorSupport);

    return result;
}

audio::AudioResourceMeta VstModulesMetaRegister::metaFromJson(const QJsonObject& object) const
{
    audio::AudioResourceMeta result;

    result.id = object.value(QStringLiteral("id")).toString().toStdString();
    result.type = audio::AudioResourceType::VstPlugin;
    result.vendor = object.value(QStringLiteral("vendor")).toString().toStdString();
    result.hasNativeEditorSupport = object.value(QStringLiteral("hasNativeEditorSupport")).toBool();

    return result;
}

VstPluginType VstModulesMetaRegister::pluginTypeFromString(const QString& string) const
{
    for (const auto& pair : PLUGIN_TYPE_MAP) {
        if (pair.second == string) {
            return pair.first;
        }
    }

    return VstPluginType::Undefined;
}

const QString& VstModulesMetaRegister::pluginTypeToString(const VstPluginType type) const
{
    auto search = PLUGIN_TYPE_MAP.find(type);
    if (search == PLUGIN_TYPE_MAP.cend()) {
        static QString empty;
        return empty;
    }

    return search->second;
}

bool VstModulesMetaRegister::hasNativeEditorSupport() const
{
#ifdef Q_OS_LINUX
    //!Note Host applications on Linux should provide their own event loop via VST3 API,
    //!     otherwise it'll be impossible to launch native VST editor views
    return false;
#else
    return true;
#endif
}
