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

#include <QJsonArray>

using namespace mu;
using namespace mu::vst;

static const std::map<VstPluginType, QString> PLUGIN_TYPE_MAP = {
    { VstPluginType::Instrument, "Instrument" },
    { VstPluginType::Fx, "Fx" }
};

VstModulesMetaRegister::~VstModulesMetaRegister()
{
    m_file.close();
}

void VstModulesMetaRegister::init()
{
    m_file = io::File(config()->knownPluginsFile());

    if (m_file.exists()) {
        readMetaList();
    }
}

void VstModulesMetaRegister::clear()
{
    m_paths.clear();
    m_metaMap.clear();
}

void VstModulesMetaRegister::registerPlugins(const ModulesMap& modules, PathMap&& paths)
{
    clear();

    m_paths = paths;

    static auto hasNativeEditorSupport = []() {
#ifdef Q_OS_LINUX
        //!Note Host applications on Linux should provide their own event loop via VST3 API,
        //!     otherwise it'll be impossible to launch native VST editor views
        return false;
#else
        return true;
#endif
    };

    for (const auto& pair : modules) {
        PluginModulePtr module = pair.second;
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

                meta.id = pair.first;
                meta.type = audio::AudioResourceType::VstPlugin;
                meta.vendor = factory.info().vendor();
                meta.hasNativeEditorSupport = hasNativeEditorSupport();

                m_metaMap[currentType].emplace_back(std::move(meta));
            }
        }
    }

    writeMetaList();
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

    return search->second;
}

bool VstModulesMetaRegister::isEmpty() const
{
    return m_metaMap.empty() && m_paths.empty();
}

void VstModulesMetaRegister::readMetaList()
{
    m_file.open(io::IODevice::OpenMode::ReadWrite);

    QJsonDocument json = QJsonDocument::fromJson(m_file.readAll().toQByteArrayNoCopy());

    for (const QJsonValue& val : json.array()) {
        QJsonObject object = val.toObject();
        QJsonValue metaJson = object.value(QStringLiteral("meta"));

        audio::AudioResourceMeta meta = metaFromJson(metaJson.toObject());
        io::path_t path(object.value(QStringLiteral("path")).toString().toStdString());
        VstPluginType type = pluginTypeFromString(object.value(QStringLiteral("type")).toString());

        m_paths.emplace(meta.id, std::move(path));
        m_metaMap[type].emplace_back(std::move(meta));
    }
}

void VstModulesMetaRegister::writeMetaList()
{
    m_file.open(io::IODevice::OpenMode::ReadWrite);

    QJsonArray rootArray;

    for (const auto& pair : m_metaMap) {
        const QString& typeStr = pluginTypeToString(pair.first);

        for (const audio::AudioResourceMeta& meta : pair.second) {
            QJsonObject obj;
            obj.insert(QStringLiteral("type"), typeStr);
            obj.insert(QStringLiteral("meta"), metaToJson(meta));
            obj.insert(QStringLiteral("path"), m_paths.at(meta.id).toQString());

            rootArray.append(obj);
        }
    }

    m_file.write(QJsonDocument(rootArray).toJson());
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
