/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited and others
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

#include "vstpluginmetareader.h"

#include "vsttypes.h"
#include "vsterrors.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::vst;

audio::AudioResourceType VstPluginMetaReader::metaType() const
{
    return audio::AudioResourceType::VstPlugin;
}

bool VstPluginMetaReader::canReadMeta(const io::path_t& pluginPath) const
{
    return io::suffix(pluginPath) == VST3_PACKAGE_EXTENSION;
}

RetVal<AudioResourceMetaList> VstPluginMetaReader::readMeta(const io::path_t& pluginPath) const
{
    PluginModulePtr module = createModule(pluginPath);
    if (!module) {
        return make_ret(Err::NoPluginModule);
    }

    const auto& factory = module->getFactory();

    std::vector<ClassInfo> audioEffects;
    for (const ClassInfo& classInfo : factory.classInfos()) {
        if (classInfo.category() == kVstAudioEffectClass) {
            audioEffects.push_back(classInfo);
        }
    }

    if (audioEffects.empty()) {
        return make_ret(Err::NoAudioEffect);
    }

    std::string baseName = io::completeBasename(pluginPath).toStdString();
    bool multiComponent = audioEffects.size() > 1;

    AudioResourceMetaList result;
    std::set<std::string> usedIds;

    for (size_t i = 0; i < audioEffects.size(); ++i) {
        const ClassInfo& classInfo = audioEffects[i];

        std::string id;
        if (multiComponent) {
            id = classInfo.name();
            if (id.empty()) {
                id = baseName + " " + std::to_string(i + 1);
            }
            if (muse::contains(usedIds, id)) {
                id = classInfo.ID().toString();
            }
            usedIds.insert(id);
        } else {
            id = baseName;
        }

        muse::audio::AudioResourceMeta meta;
        meta.id = std::move(id);
        meta.type = muse::audio::AudioResourceType::VstPlugin;
        meta.attributes.emplace(muse::audio::CATEGORIES_ATTRIBUTE, String::fromStdString(classInfo.subCategoriesString()));
        meta.vendor = classInfo.vendor();
        meta.hasNativeEditorSupport = true;

        result.emplace_back(std::move(meta));
    }

    return RetVal<AudioResourceMetaList>::make_ok(result);
}
