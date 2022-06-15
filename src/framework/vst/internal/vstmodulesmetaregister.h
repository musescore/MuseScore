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

#ifndef MU_VST_VSTMODULESMETAREGISTER_H
#define MU_VST_VSTMODULESMETAREGISTER_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include "modularity/ioc.h"
#include "io/file.h"
#include "audio/audiotypes.h"

#include "ivstconfiguration.h"
#include "vsttypes.h"

namespace mu::vst {
class VstModulesMetaRegister
{
    INJECT(vst, IVstConfiguration, config)
public:
    VstModulesMetaRegister() = default;
    ~VstModulesMetaRegister();

    using ModulesMap = std::unordered_map<audio::AudioResourceId, PluginModulePtr>;
    using PathMap = std::unordered_map<audio::AudioResourceId, io::path_t>;

    void init();
    void registerPlugins(const ModulesMap& modules, PathMap&& paths);

    const io::path_t& pluginPath(const audio::AudioResourceId& resourceId) const;
    const audio::AudioResourceMetaList& metaList(const VstPluginType type) const;

    bool isEmpty() const;

private:
    void clear();

    void readMetaList();
    void writeMetaList();

    QJsonObject metaToJson(const audio::AudioResourceMeta& meta) const;
    audio::AudioResourceMeta metaFromJson(const QJsonObject& object) const;
    VstPluginType pluginTypeFromString(const QString& string) const;
    const QString& pluginTypeToString(const VstPluginType type) const;

    io::File m_file;

    std::map<VstPluginType, audio::AudioResourceMetaList> m_metaMap;
    PathMap m_paths;
};
}

#endif // MU_VST_VSTMODULESMETAREGISTER_H
