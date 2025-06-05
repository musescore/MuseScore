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
    AudioResourceMetaList result;

    for (const ClassInfo& classInfo : factory.classInfos()) {
        if (classInfo.category() != kVstAudioEffectClass) {
            continue;
        }

        muse::audio::AudioResourceMeta meta;
        meta.id = io::completeBasename(pluginPath).toStdString();
        meta.type = muse::audio::AudioResourceType::VstPlugin;
        meta.attributes.emplace(muse::audio::CATEGORIES_ATTRIBUTE, String::fromStdString(classInfo.subCategoriesString()));
        meta.vendor = classInfo.vendor();
        meta.hasNativeEditorSupport = true;

        result.emplace_back(std::move(meta));
        break;
    }

    if (result.empty()) {
        return make_ret(Err::NoAudioEffect);
    }

    return RetVal<AudioResourceMetaList>::make_ok(result);
}
