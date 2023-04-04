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

#include "audio/internal/plugins/knownaudiopluginsregister.h"
#include "global/tests/mocks/filesystemmock.h"
#include "audio/tests/mocks/audioconfigurationmock.h"

#include "serialization/json.h"

using ::testing::_;
using ::testing::Return;

using namespace mu::audio;
using namespace mu::io;

namespace mu::audio {
class Audio_KnownAudioPluginsRegisterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_knownPlugins = std::make_shared<KnownAudioPluginsRegister>();
        m_fileSystem = std::make_shared<FileSystemMock>();
        m_configuration = std::make_shared<AudioConfigurationMock>();

        m_knownPlugins->setfileSystem(m_fileSystem);
        m_knownPlugins->setconfiguration(m_configuration);

        m_knownAudioPluginsDir = "/test/some dir/audio_plugins";
        ON_CALL(*m_configuration, knownAudioPluginsDir())
        .WillByDefault(Return(m_knownAudioPluginsDir));
    }

    ByteArray pluginInfoToJson(const AudioPluginInfo& info) const
    {
        const std::map<AudioResourceType, std::string> RESOURCE_TYPE_TO_STR {
            { AudioResourceType::VstPlugin, "VstPlugin" },
        };

        JsonObject attributesObj;
        for (auto it = info.meta.attributes.cbegin(); it != info.meta.attributes.cend(); ++it) {
            attributesObj.set(it->first.toStdString(), it->second.toStdString());
        }

        JsonObject metaObj;
        metaObj.set("id", info.meta.id);
        metaObj.set("type", mu::value(RESOURCE_TYPE_TO_STR, info.meta.type, "Undefined"));
        metaObj.set("vendor", info.meta.vendor);
        metaObj.set("attributes", attributesObj);
        metaObj.set("hasNativeEditorSupport", info.meta.hasNativeEditorSupport);

        JsonObject mainObj;
        mainObj.set("meta", metaObj);
        mainObj.set("path", info.path.toStdString());
        mainObj.set("enabled", info.enabled);

        if (info.errorCode != 0) {
            mainObj.set("errorCode", info.errorCode);
        }

        return JsonDocument(mainObj).toJson();
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
        disabledPluginInfo.meta.attributes = { { audio::CATEGORIES_ATTRIBUTE, u"Instrument|Synth" },
            { audio::PLAYBACK_SETUP_DATA_ATTRIBUTE, mpe::GENERIC_SETUP_DATA_STRING } };
        disabledPluginInfo.errorCode = -1;
        plugins.push_back(disabledPluginInfo);

        paths_t infoPaths;
        for (const AudioPluginInfo& info : plugins) {
            infoPaths.push_back(pluginInfoPath(info.meta.vendor, info.meta.id));
        }

        ON_CALL(*m_fileSystem, scanFiles(m_knownAudioPluginsDir, std::vector<std::string> { "*.json" }, ScanMode::FilesInCurrentDir))
        .WillByDefault(Return(mu::RetVal<paths_t>::make_ok({ infoPaths })));

        for (size_t i = 0; i < infoPaths.size(); ++i) {
            mu::ByteArray data = pluginInfoToJson(plugins[i]);

            ON_CALL(*m_fileSystem, readFile(infoPaths[i]))
            .WillByDefault(Return(mu::RetVal<mu::ByteArray>::make_ok(data)));
        }

        return plugins;
    }

    io::path_t pluginInfoPath(const AudioResourceVendor& vendor, const AudioResourceId& resourceId) const
    {
        io::path_t fileName;

        if (vendor.empty()) {
            fileName = io::escapeFileName(resourceId);
        } else {
            fileName = io::escapeFileName(vendor + "_" + resourceId);
        }

        return m_knownAudioPluginsDir + "/" + fileName + ".json";
    }

    std::shared_ptr<KnownAudioPluginsRegister> m_knownPlugins;
    std::shared_ptr<FileSystemMock> m_fileSystem;
    std::shared_ptr<AudioConfigurationMock> m_configuration;

    path_t m_knownAudioPluginsDir;
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

TEST_F(Audio_KnownAudioPluginsRegisterTest, PluginInfoList)
{
    // [GIVEN] All known plugins
    std::vector<AudioPluginInfo> expectedPluginInfoList = setupTestData();

    // [WHEN] Load the info
    m_knownPlugins->load();

    // [WHEN] Request the info
    std::vector<AudioPluginInfo> actualPluginInfoList = m_knownPlugins->pluginInfoList();

    // [THEN] Successfully received the info
    EXPECT_EQ(actualPluginInfoList.size(), expectedPluginInfoList.size());
    for (const AudioPluginInfo& actualInfo : actualPluginInfoList) {
        EXPECT_TRUE(mu::contains(expectedPluginInfoList, actualInfo));
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
    newPluginInfo.meta.id = "new/plugin";
    newPluginInfo.meta.type = AudioResourceType::VstPlugin;
    newPluginInfo.path = "/path/to/new/plugin/plugin.vst";
    newPluginInfo.enabled = true;
    expectedPluginInfoList.push_back(newPluginInfo);

    // [THEN] The plugin will be written to the corresponding file
    mu::ByteArray expectedNewPluginData = pluginInfoToJson(newPluginInfo);
    EXPECT_CALL(*m_fileSystem, writeFile(pluginInfoPath(newPluginInfo.meta.vendor, newPluginInfo.meta.id), expectedNewPluginData))
    .WillOnce(Return(mu::make_ok()));

    // [WHEN] Register it
    mu::Ret ret = m_knownPlugins->registerPlugin(newPluginInfo);

    // [THEN] The plugin successfully registered
    EXPECT_TRUE(ret);

    actualPluginInfoList = m_knownPlugins->pluginInfoList();
    EXPECT_EQ(expectedPluginInfoList.size(), actualPluginInfoList.size());
    for (const AudioPluginInfo& actualInfo : actualPluginInfoList) {
        EXPECT_TRUE(mu::contains(expectedPluginInfoList, actualInfo));
        EXPECT_TRUE(m_knownPlugins->exists(actualInfo.path));
        EXPECT_TRUE(m_knownPlugins->exists(actualInfo.meta.id));
        EXPECT_EQ(actualInfo.path, m_knownPlugins->pluginPath(actualInfo.meta.id));
    }

    // [WHEN] Unregister the first plugin in the list
    AudioPluginInfo unregisteredPlugin = mu::takeFirst(expectedPluginInfoList);

    EXPECT_CALL(*m_fileSystem, remove(pluginInfoPath(unregisteredPlugin.meta.vendor, unregisteredPlugin.meta.id), false))
    .WillOnce(Return(mu::make_ok()));

    ret = m_knownPlugins->unregisterPlugin(unregisteredPlugin.meta.id);

    // [THEN] The plugin successfully unregistered
    EXPECT_TRUE(ret);
    EXPECT_FALSE(m_knownPlugins->exists(unregisteredPlugin.path));
    EXPECT_FALSE(m_knownPlugins->exists(unregisteredPlugin.meta.id));
    EXPECT_EQ(m_knownPlugins->pluginPath(unregisteredPlugin.meta.id), "");
    actualPluginInfoList = m_knownPlugins->pluginInfoList();
    EXPECT_FALSE(mu::contains(actualPluginInfoList, unregisteredPlugin));
}
