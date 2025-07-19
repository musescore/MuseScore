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

#include <gtest/gtest.h>

#include "global/serialization/json.h"

#include "audioplugins/internal/knownaudiopluginsregister.h"

#include "global/tests/mocks/filesystemmock.h"
#include "mocks/audiopluginsconfigurationmock.h"

using ::testing::_;
using ::testing::Return;

using namespace muse;
using namespace muse::audioplugins;
using namespace muse::audio;
using namespace muse::io;

namespace muse::audioplugins {
class AudioPlugins_KnownAudioPluginsRegisterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_knownPlugins = std::make_shared<KnownAudioPluginsRegister>(modularity::globalCtx());
        m_fileSystem = std::make_shared<FileSystemMock>();
        m_configuration = std::make_shared<AudioPluginsConfigurationMock>();

        m_knownPlugins->fileSystem.set(m_fileSystem);
        m_knownPlugins->configuration.set(m_configuration);

        m_knownAudioPluginsFilePath = "/test/some dir/known_audio_plugins.json";
        ON_CALL(*m_configuration, knownAudioPluginsFilePath())
        .WillByDefault(Return(m_knownAudioPluginsFilePath));
    }

    ByteArray pluginInfoListToJson(const std::vector<AudioPluginInfo>& infoList) const
    {
        const std::map<AudioResourceType, std::string> RESOURCE_TYPE_TO_STR {
            { AudioResourceType::VstPlugin, "VstPlugin" },
        };

        JsonArray array;

        for (const AudioPluginInfo& info : infoList) {
            JsonObject attributesObj;
            for (auto it = info.meta.attributes.cbegin(); it != info.meta.attributes.cend(); ++it) {
                if (it->first == audio::PLAYBACK_SETUP_DATA_ATTRIBUTE) {
                    continue;
                }

                attributesObj.set(it->first.toStdString(), it->second.toStdString());
            }

            JsonObject metaObj;
            metaObj.set("id", info.meta.id);
            metaObj.set("type", muse::value(RESOURCE_TYPE_TO_STR, info.meta.type, "Undefined"));
            metaObj.set("hasNativeEditorSupport", info.meta.hasNativeEditorSupport);

            if (!info.meta.vendor.empty()) {
                metaObj.set("vendor", info.meta.vendor);
            }

            if (!attributesObj.empty()) {
                metaObj.set("attributes", attributesObj);
            }

            JsonObject mainObj;
            mainObj.set("meta", metaObj);
            mainObj.set("path", info.path.toStdString());
            mainObj.set("enabled", info.enabled);

            if (info.errorCode != 0) {
                mainObj.set("errorCode", info.errorCode);
            }

            array << mainObj;
        }

        return JsonDocument(array).toJson();
    }

    std::vector<AudioPluginInfo> setupTestData()
    {
        std::vector<AudioPluginInfo> plugins;

        AudioPluginInfo pluginInfo1;
        pluginInfo1.type = AudioPluginType::Fx;
        pluginInfo1.path = "/some/path/to/vst/plugin/AAA.vst3";
        pluginInfo1.meta.id = "AAA";
        pluginInfo1.meta.type = AudioResourceType::VstPlugin;
        pluginInfo1.meta.vendor = "Some vendor";
        pluginInfo1.meta.attributes = { { audio::CATEGORIES_ATTRIBUTE, u"Fx|Reverb" },
            { audio::PLAYBACK_SETUP_DATA_ATTRIBUTE, mpe::GENERIC_SETUP_DATA_STRING } };
        pluginInfo1.enabled = true;
        plugins.push_back(pluginInfo1);

        AudioPluginInfo pluginInfo2;
        pluginInfo2.type = AudioPluginType::Fx;
        pluginInfo2.path = "/some/path/to/vst/plugin/BBB.vst3";
        pluginInfo2.meta.id = "BBB";
        pluginInfo2.meta.type = AudioResourceType::VstPlugin;
        pluginInfo2.meta.vendor = "Another vendor";
        pluginInfo2.meta.attributes = { { audio::CATEGORIES_ATTRIBUTE, u"Fx|Distortion" },
            { audio::PLAYBACK_SETUP_DATA_ATTRIBUTE, mpe::GENERIC_SETUP_DATA_STRING } };
        pluginInfo2.enabled = true;
        plugins.push_back(pluginInfo2);

        AudioPluginInfo disabledPluginInfo;
        disabledPluginInfo.type = AudioPluginType::Instrument;
        disabledPluginInfo.path = "/some/path/to/vst/plugin/CCC.vst3";
        disabledPluginInfo.meta.id = "CCC";
        disabledPluginInfo.meta.type = AudioResourceType::VstPlugin;
        disabledPluginInfo.enabled = false;
        disabledPluginInfo.meta.attributes = {
            { audio::CATEGORIES_ATTRIBUTE, u"Instrument|Synth" },
            { audio::PLAYBACK_SETUP_DATA_ATTRIBUTE, mpe::GENERIC_SETUP_DATA_STRING }
        };
        disabledPluginInfo.errorCode = -1;
        plugins.push_back(disabledPluginInfo);

        ByteArray data = pluginInfoListToJson(plugins);
        ON_CALL(*m_fileSystem, readFile(m_knownAudioPluginsFilePath))
        .WillByDefault(Return(RetVal<ByteArray>::make_ok(data)));

        return plugins;
    }

    std::shared_ptr<KnownAudioPluginsRegister> m_knownPlugins;
    std::shared_ptr<FileSystemMock> m_fileSystem;
    std::shared_ptr<AudioPluginsConfigurationMock> m_configuration;

    path_t m_knownAudioPluginsFilePath;
};

inline bool operator==(const AudioPluginInfo& info1, const AudioPluginInfo& info2)
{
    bool equal = info1.type == info2.type;
    equal &= (info1.path == info2.path);
    equal &= (info1.meta == info2.meta);
    equal &= (info1.enabled == info2.enabled);
    equal &= (info1.errorCode == info2.errorCode);

    return equal;
}
}

TEST_F(AudioPlugins_KnownAudioPluginsRegisterTest, PluginInfoList)
{
    // [GIVEN] All known plugins
    std::vector<AudioPluginInfo> expectedPluginInfoList = setupTestData();

    // [GIVEN] File exists
    ON_CALL(*m_fileSystem, exists(m_knownAudioPluginsFilePath))
    .WillByDefault(Return(muse::make_ok()));

    // [WHEN] Load the info
    Ret ret = m_knownPlugins->load();

    // [THEN] Successfully loaded the info
    EXPECT_TRUE(ret);

    // [WHEN] Request the info
    std::vector<AudioPluginInfo> actualPluginInfoList = m_knownPlugins->pluginInfoList();

    // [THEN] Successfully received the info
    EXPECT_EQ(actualPluginInfoList.size(), expectedPluginInfoList.size());
    for (const AudioPluginInfo& actualInfo : actualPluginInfoList) {
        EXPECT_TRUE(muse::contains(expectedPluginInfoList, actualInfo));
        EXPECT_TRUE(m_knownPlugins->exists(actualInfo.path));
        EXPECT_TRUE(m_knownPlugins->exists(actualInfo.meta.id));
        EXPECT_EQ(actualInfo.path, m_knownPlugins->pluginPath(actualInfo.meta.id));
    }

    // [THEN] Make sure that exists() does not always return true
    EXPECT_FALSE(m_knownPlugins->exists(path_t("/path/to/nonexistent/plugin.vst3")));
    EXPECT_FALSE(m_knownPlugins->exists(AudioResourceId("nonexistent_plugin")));

    // [GIVEN] New plugin for registration
    AudioPluginInfo newPluginInfo;
    newPluginInfo.type = AudioPluginType::Instrument;
    newPluginInfo.meta.id = "DDD";
    newPluginInfo.meta.type = AudioResourceType::VstPlugin;
    newPluginInfo.path = "/path/to/new/plugin/plugin.vst";
    newPluginInfo.enabled = true;
    expectedPluginInfoList.push_back(newPluginInfo);

    // [THEN] All the plugins will be written to the file
    ByteArray expectedNewPluginsData = pluginInfoListToJson(expectedPluginInfoList);
    EXPECT_CALL(*m_fileSystem, writeFile(m_knownAudioPluginsFilePath, expectedNewPluginsData))
    .WillOnce(Return(muse::make_ok()));

    // [WHEN] Register it
    ret = m_knownPlugins->registerPlugin(newPluginInfo);

    // [THEN] The plugin successfully registered
    EXPECT_TRUE(ret);

    // [GIVEN] Same plugin (with the same resourceId) is installed in a different location
    AudioPluginInfo duplicatedPluginInfo = newPluginInfo;
    duplicatedPluginInfo.path = "/another/path/to/new/plugin/plugin.vst";
    expectedPluginInfoList.push_back(duplicatedPluginInfo);

    // [THEN] All the plugins will be written to the file
    expectedNewPluginsData = pluginInfoListToJson(expectedPluginInfoList);
    EXPECT_CALL(*m_fileSystem, writeFile(m_knownAudioPluginsFilePath, expectedNewPluginsData))
    .WillOnce(Return(muse::make_ok()));

    // [WHEN] Register it
    ret = m_knownPlugins->registerPlugin(duplicatedPluginInfo);

    // [THEN] The duplicated plugin successfully registered
    EXPECT_TRUE(ret);

    actualPluginInfoList = m_knownPlugins->pluginInfoList();
    EXPECT_EQ(expectedPluginInfoList.size(), actualPluginInfoList.size());
    for (const AudioPluginInfo& actualInfo : actualPluginInfoList) {
        EXPECT_TRUE(muse::contains(expectedPluginInfoList, actualInfo));
        EXPECT_TRUE(m_knownPlugins->exists(actualInfo.path));
        EXPECT_TRUE(m_knownPlugins->exists(actualInfo.meta.id));
    }

    // [GIVEN] We want to unregister the first plugin in the list
    AudioPluginInfo unregisteredPlugin = muse::takeFirst(expectedPluginInfoList);

    // [THEN] All the plugins will be written to the file (except the unregistered one)
    expectedNewPluginsData = pluginInfoListToJson(expectedPluginInfoList);
    EXPECT_CALL(*m_fileSystem, writeFile(m_knownAudioPluginsFilePath, expectedNewPluginsData))
    .WillOnce(Return(muse::make_ok()));

    // [WHEN] Unregister the plugin
    ret = m_knownPlugins->unregisterPlugin(unregisteredPlugin.meta.id);

    // [THEN] The plugin successfully unregistered
    EXPECT_TRUE(ret);
    EXPECT_FALSE(m_knownPlugins->exists(unregisteredPlugin.path));
    EXPECT_FALSE(m_knownPlugins->exists(unregisteredPlugin.meta.id));
    EXPECT_EQ(m_knownPlugins->pluginPath(unregisteredPlugin.meta.id), "");
    actualPluginInfoList = m_knownPlugins->pluginInfoList();
    EXPECT_FALSE(muse::contains(actualPluginInfoList, unregisteredPlugin));
}
