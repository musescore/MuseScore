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

#include "audio/internal/plugins/registeraudiopluginsscenario.h"

#include "global/tests/mocks/globalconfigurationmock.h"
#include "global/tests/mocks/interactivemock.h"
#include "global/tests/mocks/processmock.h"
#include "audio/tests/mocks/knownaudiopluginsregistermock.h"
#include "audio/tests/mocks/audiopluginsscannerregistermock.h"
#include "audio/tests/mocks/audiopluginsscannermock.h"
#include "audio/tests/mocks/audiopluginmetareaderregistermock.h"
#include "audio/tests/mocks/audiopluginmetareadermock.h"

#include "translation.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

using namespace mu::audio;
using namespace mu::io;
using namespace mu::framework;

namespace mu::audio {
class Audio_RegisterAudioPluginsScenarioTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_scenario = std::make_shared<RegisterAudioPluginsScenario>();
        m_globalConfiguration = std::make_shared<GlobalConfigurationMock>();
        m_interactive = std::make_shared<InteractiveMock>();
        m_process = std::make_shared<ProcessMock>();
        m_scannerRegister = std::make_shared<AudioPluginsScannerRegisterMock>();
        m_knownPlugins = std::make_shared<KnownAudioPluginsRegisterMock>();
        m_scanners = { std::make_shared<AudioPluginsScannerMock>() };
        m_metaReaderRegister = std::make_shared<AudioPluginMetaReaderRegisterMock>();
        m_metaReaders = { std::make_shared<AudioPluginMetaReaderMock>() };

        m_scenario->setglobalConfiguration(m_globalConfiguration);
        m_scenario->setinteractive(m_interactive);
        m_scenario->setprocess(m_process);
        m_scenario->setknownPluginsRegister(m_knownPlugins);
        m_scenario->setscannerRegister(m_scannerRegister);
        m_scenario->setmetaReaderRegister(m_metaReaderRegister);

        ON_CALL(*m_globalConfiguration, appBinPath())
        .WillByDefault(Return(m_appPath));

        ON_CALL(*m_scannerRegister, scanners())
        .WillByDefault(ReturnRef(m_scanners));

        ON_CALL(*m_metaReaderRegister, readers())
        .WillByDefault(ReturnRef(m_metaReaders));
    }

    std::shared_ptr<RegisterAudioPluginsScenario> m_scenario;
    std::shared_ptr<GlobalConfigurationMock> m_globalConfiguration;
    std::shared_ptr<InteractiveMock> m_interactive;
    std::shared_ptr<ProcessMock> m_process;
    std::shared_ptr<KnownAudioPluginsRegisterMock> m_knownPlugins;
    std::shared_ptr<AudioPluginsScannerRegisterMock> m_scannerRegister;
    std::vector<IAudioPluginsScannerPtr> m_scanners;
    std::shared_ptr<AudioPluginMetaReaderRegisterMock> m_metaReaderRegister;
    std::vector<IAudioPluginMetaReaderPtr> m_metaReaders;

    std::string m_appPath = "/some/path/to/MuseScore";
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

TEST_F(Audio_RegisterAudioPluginsScenarioTest, Init)
{
    // [THEN] The register is inited
    EXPECT_CALL(*m_knownPlugins, load())
    .WillOnce(Return(mu::make_ok()));

    // [WHEN] Init the scenario
    m_scenario->init();
}

TEST_F(Audio_RegisterAudioPluginsScenarioTest, RegisterNewPlugins)
{
    // [GIVEN] All found plugins
    paths_t foundPluginPaths = {
        "/some/test/path/to/plugin/AAA.vst3", // already registered
        "/some/test/path/to/plugin/BBB.vst3", // already registered
        "/some/test/path/to/plugin/CCC.vst3",
        "/some/test/path/to/plugin/DDD.vst3",
        "/some/test/path/to/plugin/FFF.vst3", // incompatible (will crash)
    };

    for (IAudioPluginsScannerPtr scanner : m_scanners) {
        AudioPluginsScannerMock* mock = dynamic_cast<AudioPluginsScannerMock*>(scanner.get());
        ASSERT_TRUE(mock);

        ON_CALL(*mock, scanPlugins())
        .WillByDefault(Return(foundPluginPaths));
    }

    AudioPluginInfo incompatiblePluginInfo;
    incompatiblePluginInfo.path = foundPluginPaths[4];
    incompatiblePluginInfo.meta.id = mu::io::filename(incompatiblePluginInfo.path).toStdString();
    incompatiblePluginInfo.enabled = false;
    incompatiblePluginInfo.errorCode = -1;

    // [GIVEN] Some plugins already exist in the register
    paths_t alreadyRegisteredPlugins {
        foundPluginPaths[0],
        foundPluginPaths[1],
    };

    for (const path_t& pluginPath : foundPluginPaths) {
        if (mu::contains(alreadyRegisteredPlugins, pluginPath)) {
            ON_CALL(*m_knownPlugins, exists(pluginPath))
            .WillByDefault(Return(true));
        } else {
            ON_CALL(*m_knownPlugins, exists(pluginPath))
            .WillByDefault(Return(false));
        }
    }

    // [THEN] The progress bar is shown
    EXPECT_CALL(*m_interactive, showProgress(mu::trc("audio", "Scanning audio plugins"), _))
    .WillOnce(Return(mu::make_ok()));

    // [THEN] Processes started only for unregistered plugins
    for (const path_t& pluginPath : foundPluginPaths) {
        std::vector<std::string> args = { "--register-audio-plugin", pluginPath.toStdString() };

        if (mu::contains(alreadyRegisteredPlugins, pluginPath)) {
            // Ignore already registered plugins
            EXPECT_CALL(*m_process, execute(_, args))
            .Times(0);
        } else if (incompatiblePluginInfo.path == pluginPath) {
            // Incompatible plugin detected
            EXPECT_CALL(*m_process, execute(m_appPath, args))
            .WillOnce(Return(-1));

            args = { "--register-failed-audio-plugin", pluginPath.toStdString(), "--", "-1" };

            EXPECT_CALL(*m_process, execute(m_appPath, args))
            .WillOnce(Return(0));
        } else {
            // Successfully registered plugins
            EXPECT_CALL(*m_process, execute(m_appPath, args))
            .WillOnce(Return(0));
        }
    }

    // [THEN] All plugins remain in the register
    EXPECT_CALL(*m_knownPlugins, unregisterPlugin(_))
    .Times(0);

    // [THEN] The register is refreshed
    EXPECT_CALL(*m_knownPlugins, load())
    .WillOnce(Return(mu::make_ok()));

    // [WHEN] Register new plugins
    mu::Ret ret = m_scenario->registerNewPlugins();

    // [THEN] Plugins successfully registered
    EXPECT_TRUE(ret);
}

TEST_F(Audio_RegisterAudioPluginsScenarioTest, RegisterNewPlugins_NoNewPlugins)
{
    // [GIVEN] All found plugins (all are already registered)
    paths_t foundPluginPaths = {
        "/some/test/path/to/plugin/AAA.vst3",
        "/some/test/path/to/plugin/BBB.vst3",
        "/some/test/path/to/plugin/CCC.vst3",
    };

    for (IAudioPluginsScannerPtr scanner : m_scanners) {
        AudioPluginsScannerMock* mock = dynamic_cast<AudioPluginsScannerMock*>(scanner.get());
        ASSERT_TRUE(mock);

        ON_CALL(*mock, scanPlugins())
        .WillByDefault(Return(foundPluginPaths));
    }

    for (const path_t& pluginPath : foundPluginPaths) {
        ON_CALL(*m_knownPlugins, exists(pluginPath))
        .WillByDefault(Return(true));
    }

    // [THEN] Don't register the plugins again
    EXPECT_CALL(*m_process, execute(_, _))
    .Times(0);

    EXPECT_CALL(*m_interactive, showProgress(_, _))
    .Times(0);

    EXPECT_CALL(*m_knownPlugins, load())
    .Times(0);

    // [WHEN] Try to register the plugins again
    mu::Ret ret = m_scenario->registerNewPlugins();

    // [THEN] No error
    EXPECT_TRUE(ret);
}

TEST_F(Audio_RegisterAudioPluginsScenarioTest, RegisterPlugin)
{
    // [GIVEN] Some plugin we want to register
    path_t pluginPath = "/some/test/path/to/plugin/AAA.vst3";

    AudioResourceMetaList metaList;

    AudioResourceMeta pluginMeta1;
    pluginMeta1.id = "Mono plugin";
    pluginMeta1.attributes.insert({ mu::audio::CATEGORIES_ATTRIBUTE, u"Fx|Mono" });
    metaList.push_back(pluginMeta1);

    AudioResourceMeta pluginMeta2;
    pluginMeta2.id = "Stereo plugin";
    pluginMeta2.attributes.insert({ mu::audio::CATEGORIES_ATTRIBUTE, u"Fx|Stereo" });
    metaList.push_back(pluginMeta2);

    ASSERT_FALSE(m_metaReaders.empty());
    AudioPluginMetaReaderMock* mock = dynamic_cast<AudioPluginMetaReaderMock*>(m_metaReaders[0].get());
    ASSERT_TRUE(mock);

    ON_CALL(*mock, canReadMeta(pluginPath))
    .WillByDefault(Return(true));

    ON_CALL(*mock, readMeta(pluginPath))
    .WillByDefault(Return(mu::RetVal<AudioResourceMetaList>::make_ok(metaList)));

    // [THEN] The plugin has been registered
    for (const AudioResourceMeta& meta : metaList) {
        AudioPluginInfo expectedPluginInfo;
        expectedPluginInfo.type = AudioPluginType::Fx;
        expectedPluginInfo.meta = meta;
        expectedPluginInfo.path = pluginPath;
        expectedPluginInfo.enabled = true;
        expectedPluginInfo.errorCode = 0;

        EXPECT_CALL(*m_knownPlugins, registerPlugin(expectedPluginInfo))
        .WillOnce(Return(true));
    }

    // [WHEN] Register the plugin
    mu::Ret ret = m_scenario->registerPlugin(pluginPath);

    // [THEN] The plugin successfully registered
    EXPECT_TRUE(ret);
}

TEST_F(Audio_RegisterAudioPluginsScenarioTest, RegisterFailedPlugin)
{
    // [GIVEN] Some incompatible plugin we want to register
    path_t pluginPath = "/some/test/path/to/plugin/AAA.vst3";

    // [THEN] The plugin has been registered
    AudioPluginInfo expectedPluginInfo;
    expectedPluginInfo.meta.id = mu::io::completeBasename(pluginPath).toStdString();
    expectedPluginInfo.meta.type = AudioResourceType::VstPlugin;
    expectedPluginInfo.path = pluginPath;
    expectedPluginInfo.enabled = false;
    expectedPluginInfo.errorCode = -42;

    EXPECT_CALL(*m_knownPlugins, registerPlugin(expectedPluginInfo))
    .WillOnce(Return(true));

    // [WHEN] Register the incompatible plugin
    mu::Ret ret = m_scenario->registerFailedPlugin(pluginPath, expectedPluginInfo.errorCode);

    // [THEN] The plugin successfully registered
    EXPECT_TRUE(ret);
}
