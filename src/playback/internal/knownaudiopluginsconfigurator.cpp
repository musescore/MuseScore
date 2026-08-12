/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#include "knownaudiopluginsconfigurator.h"

#include "global/serialization/json.h"

#include "audio/common/audiotypes.h"
#include "mpe/playbacksetupdata.h"

using namespace mu::playback;
using namespace muse::audioplugins;

void KnownAudioPluginsConfigurator::init()
{
    if (m_audioPluginConfiguration()) {
        PluginAttributes runtimeDefaults;
        runtimeDefaults.emplace(muse::audio::PLAYBACK_SETUP_DATA_ATTRIBUTE,
                                muse::mpe::GENERIC_SETUP_DATA_STRING);
        m_audioPluginConfiguration()->setRuntimeAttributeDefaults(runtimeDefaults);
    }

    // app-specific migrations only; framework registers v0->v2
    if (m_migrationRegister()) {
        // v2 -> v3: hasNativeEditorSupport -> meta.attributes
        m_migrationRegister()->registerMigration(2, [](const muse::JsonArray& plugins) {
            const std::string nativeEditorKey = muse::audio::HAS_NATIVE_EDITOR_SUPPORT_ATTRIBUTE.toStdString();
            muse::JsonArray out;
            for (size_t i = 0; i < plugins.size(); ++i) {
                muse::JsonObject obj = plugins.at(i).toObject();
                muse::JsonObject meta = obj.value("meta").toObject();
                if (meta.contains(nativeEditorKey)) {
                    muse::JsonObject attrs;
                    if (meta.contains("attributes")) {
                        attrs = meta.value("attributes").toObject();
                    }
                    const bool b = meta.value(nativeEditorKey).toBool();
                    attrs.set(nativeEditorKey, b ? std::string("true") : std::string("false"));
                    meta.set("attributes", attrs);

                    // JsonObject has no remove(); rebuild without the legacy key.
                    muse::JsonObject metaWithoutLegacy;
                    for (const std::string& k : meta.keys()) {
                        if (k == nativeEditorKey) {
                            continue;
                        }
                        metaWithoutLegacy.set(k, meta.value(k));
                    }
                    obj.set("meta", metaWithoutLegacy);
                }
                out << obj;
            }
            return out;
        });
    }
}
