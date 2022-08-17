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

#include <QJsonObject>
#include <QString>

#include "modularity/ioc.h"
#include "io/path.h"
#include "audio/audiotypes.h"
#include "io/ifilesystem.h"

#include "ivstconfiguration.h"
#include "vsttypes.h"

namespace mu::vst {
class VstModulesMetaRegister
{
    INJECT(vst, IVstConfiguration, config)
    INJECT(vst, io::IFileSystem, fileSystem)
public:
    VstModulesMetaRegister() = default;

    using ModulesMap = std::unordered_map<audio::AudioResourceId, PluginModulePtr>;
    using PathMap = std::unordered_map<audio::AudioResourceId, io::path_t>;

    void init();
    void registerPath(const audio::AudioResourceId& resourceId, const io::path_t& path);
    void registerPlugin(const audio::AudioResourceId& resourceId, PluginModulePtr module, const bool enabled = true);

    const io::path_t& pluginPath(const audio::AudioResourceId& resourceId) const;
    const audio::AudioResourceMetaList& metaList(const VstPluginType type) const;

    bool exists(const audio::AudioResourceId& resourceId) const;
    bool exists(const io::path_t& path) const;
    bool isEmpty() const;

private:
    void clear();

    void load();
    void save();

    QJsonObject metaToJson(const audio::AudioResourceMeta& meta) const;
    audio::AudioResourceMeta metaFromJson(const QJsonObject& object) const;
    VstPluginType pluginTypeFromString(const QString& string) const;
    const QString& pluginTypeToString(const VstPluginType type) const;

    bool hasNativeEditorSupport() const;

    io::path_t m_knownPluginsDir;

    std::map<VstPluginType, audio::AudioResourceMetaSet> m_metaMap;
    PathMap m_paths;
};
}

#endif // MU_VST_VSTMODULESMETAREGISTER_H
