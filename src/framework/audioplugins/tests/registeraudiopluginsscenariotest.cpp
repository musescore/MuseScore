/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include <gmock/gmock.h>

#include "audioplugins/internal/registeraudiopluginsscenario.h"

#include "global/tests/mocks/globalconfigurationmock.h"
#include "global/tests/mocks/interactivemock.h"
#include "global/tests/mocks/processmock.h"

#include "mocks/knownaudiopluginsregistermock.h"
#include "mocks/audiopluginsscannerregistermock.h"
#include "mocks/audiopluginsscannermock.h"
#include "mocks/audiopluginmetareaderregistermock.h"
#include "mocks/audiopluginmetareadermock.h"

#include "translation.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

using namespace muse;
using namespace muse::audio;
using namespace muse::audioplugins;
using namespace muse::io;

namespace muse::audioplugins {
class AudioPlugins_RegisterAudioPluginsScenarioTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_scenario = std::make_shared<RegisterAudioPluginsScenario>(modularity::globalCtx());
        m_globalConfiguration = std::make_shared<NiceMock<GlobalConfigurationMock> >();
        m_interactive = std::make_shared<InteractiveMock>();
        m_process = std::make_shared<ProcessMock>();
        m_scannerRegister = std::make_shared<NiceMock<AudioPluginsScannerRegisterMock> >();
        m_knownPlugins = std::make_shared<NiceMock<KnownAudioPluginsRegisterMock> >();
        m_scanners = { std::make_shared<NiceMock<AudioPluginsScannerMock> >() };
        m_metaReaderRegister = std::make_shared<NiceMock<AudioPluginMetaReaderRegisterMock> >();

        const auto metaReaderMock = std::make_shared<NiceMock<AudioPluginMetaReaderMock> >();
        m_metaReaders = { metaReaderMock };

        m_scenario->globalConfiguration.set(m_globalConfiguration);
        m_scenario->interactive.set(m_interactive);
        m_scenario->process.set(m_process);
        m_scenario->knownPluginsRegister.set(m_knownPlugins);
        m_scenario->scannerRegister.set(m_scannerRegister);
        m_scenario->metaReaderRegister.set(m_metaReaderRegister);

        ON_CALL(*m_globalConfiguration, appBinPath())
        .WillByDefault(Return(m_appPath));

        ON_CALL(*m_scannerRegister, scanners())
        .WillByDefault(ReturnRef(m_scanners));

        ON_CALL(*m_metaReaderRegister, readers())
        .WillByDefault(ReturnRef(m_metaReaders));

        ON_CALL(*metaReaderMock, metaType())
        .WillByDefault(Return(AudioResourceType::VstPlugin));

        ON_CALL(*metaReaderMock, canReadMeta(_))
        .WillByDefault(Return(true));
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

TEST_F(AudioPlugins_RegisterAudioPluginsScenarioTest, Init)
{
    // [THEN] The register is inited
    EXPECT_CALL(*m_knownPlugins, load())
    .WillOnce(Return(muse::make_ok()));

    // [WHEN] Init the scenario
    m_scenario->init();
}

TEST_F(AudioPlugins_RegisterAudioPluginsScenarioTest, UpdatePluginsRegistry)
{
    // [GIVEN] All found plugins
    paths_t foundPluginPaths {
        "/some/test/path/to/plugin/AAA.vst3", // already registered
        "/some/test/path/to/plugin/BBB.vst3", // already registered
        "/some/test/path/to/plugin/CCC.vst3",
        "/some/test/path/to/plugin/DDD.vst3",
        "/some/test/path/to/plugin/FFF.vst3", // incompatible (will crash)
    };

    for (const IAudioPluginsScannerPtr& scanner : m_scanners) {
        AudioPluginsScannerMock* mock = dynamic_cast<AudioPluginsScannerMock*>(scanner.get());
        ASSERT_TRUE(mock);

        ON_CALL(*mock, scanPlugins())
        .WillByDefault(Return(foundPluginPaths));
    }

    // [GIVEN] Some plugins already exist in the register
    AudioPluginInfoList alreadyRegisteredPlugins;
    for (size_t i = 0; i < 2; ++i) {
        AudioPluginInfo info;
        info.path = foundPluginPaths[i];
        info.meta.id = io::completeBasename(foundPluginPaths[i]).toStdString();
        alreadyRegisteredPlugins.push_back(info);
    }

    ON_CALL(*m_knownPlugins, pluginInfoList(_))
    .WillByDefault(Return(alreadyRegisteredPlugins));

    for (const AudioPluginInfo& info : alreadyRegisteredPlugins) {
        ON_CALL(*m_knownPlugins, exists(info.path))
        .WillByDefault(Return(true));
    }

    // [THEN] The progress bar is shown
    EXPECT_CALL(*m_interactive, showProgress(muse::trc("audio", "Scanning audio plugins"), _))
    .Times(1);

    // [THEN] Already registered plugins are not processed again
    EXPECT_CALL(*m_process, execute(_, std::vector<std::string> { "--register-audio-plugin", foundPluginPaths[0].toStdString() }))
    .Times(0);
    EXPECT_CALL(*m_process, execute(_, std::vector<std::string> { "--register-audio-plugin", foundPluginPaths[1].toStdString() }))
    .Times(0);

    // [THEN] New compatible plugins are registered
    EXPECT_CALL(*m_process, execute(m_appPath, std::vector<std::string> { "--register-audio-plugin", foundPluginPaths[2].toStdString() }))
    .WillOnce(Return(0));
    EXPECT_CALL(*m_process, execute(m_appPath, std::vector<std::string> { "--register-audio-plugin", foundPluginPaths[3].toStdString() }))
    .WillOnce(Return(0));

    // [THEN] The incompatible plugin is registered as failed
    EXPECT_CALL(*m_process, execute(m_appPath, std::vector<std::string> { "--register-audio-plugin", foundPluginPaths[4].toStdString() }))
    .WillOnce(Return(-1));
    EXPECT_CALL(*m_process,
                execute(m_appPath,
                        std::vector<std::string> { "--register-failed-audio-plugin", foundPluginPaths[4].toStdString(), "--", "-1" }))
    .WillOnce(Return(0));

    // [THEN] All plugins remain in the register
    EXPECT_CALL(*m_knownPlugins, unregisterPlugins(_))
    .Times(0);

    // [THEN] Reloaded once inside registerNewPlugins, after subprocesses finish writing to disk
    EXPECT_CALL(*m_knownPlugins, load())
    .WillOnce(Return(muse::make_ok()));

    // [WHEN] Register new plugins
    m_scenario->updatePluginsRegistry();
}

TEST_F(AudioPlugins_RegisterAudioPluginsScenarioTest, UpdatePluginsRegistry_NoNewPlugins)
{
    // [GIVEN] All found plugins (all are already registered)
    paths_t foundPluginPaths {
        "/some/test/path/to/plugin/AAA.vst3",
        "/some/test/path/to/plugin/BBB.vst3",
        "/some/test/path/to/plugin/CCC.vst3",
    };

    for (const IAudioPluginsScannerPtr& scanner : m_scanners) {
        AudioPluginsScannerMock* mock = dynamic_cast<AudioPluginsScannerMock*>(scanner.get());
        ASSERT_TRUE(mock);

        ON_CALL(*mock, scanPlugins())
        .WillByDefault(Return(foundPluginPaths));
    }

    AudioPluginInfoList alreadyRegisteredPlugins;
    for (const path_t& pluginPath : foundPluginPaths) {
        AudioPluginInfo info;
        info.path = pluginPath;
        info.meta.id = io::completeBasename(pluginPath).toStdString();
        alreadyRegisteredPlugins.push_back(info);
    }

    ON_CALL(*m_knownPlugins, pluginInfoList(_))
    .WillByDefault(Return(alreadyRegisteredPlugins));

    for (const path_t& pluginPath : foundPluginPaths) {
        ON_CALL(*m_knownPlugins, exists(pluginPath))
        .WillByDefault(Return(true));
    }

    // [THEN] Don't register the plugins again
    EXPECT_CALL(*m_process, execute(_, _))
    .Times(0);

    EXPECT_CALL(*m_interactive, showProgress(_, _))
    .Times(0);

    EXPECT_CALL(*m_knownPlugins, unregisterPlugins(_))
    .Times(0);

    EXPECT_CALL(*m_knownPlugins, load())
    .Times(0);

    // [WHEN] Try to register the plugins again
    m_scenario->updatePluginsRegistry();
}

//! See: https://github.com/musescore/MuseScore/issues/16458
TEST_F(AudioPlugins_RegisterAudioPluginsScenarioTest, UpdatePluginsRegistry_UnregUninstalledPlugins)
{
    auto createPluginInfo = [](const io::path_t& path) {
        AudioPluginInfo info;
        info.type = AudioPluginType::Instrument;
        info.meta.id = io::completeBasename(path).toStdString();
        info.meta.type = AudioResourceType::VstPlugin;
        info.path = path;
        info.enabled = true;
        return info;
    };

    // [GIVEN] Already registered plugins
    AudioPluginInfoList knownPlugins;
    knownPlugins.push_back(createPluginInfo("/some/path/AAA.vst3"));
    knownPlugins.push_back(createPluginInfo("/some/path/BBB.vst3"));
    knownPlugins.push_back(createPluginInfo("/some/path/CCC.vst3"));
    knownPlugins.push_back(createPluginInfo("/some/path/DDD.vst3"));

    ON_CALL(*m_knownPlugins, pluginInfoList(_))
    .WillByDefault(Return(knownPlugins));

    // [GIVEN] Scanner only finds some plugins (AAA and BBB have been uninstalled)
    paths_t foundPluginPaths {
        "/some/path/CCC.vst3",
        "/some/path/DDD.vst3",
    };

    for (const IAudioPluginsScannerPtr& scanner : m_scanners) {
        AudioPluginsScannerMock* mock = dynamic_cast<AudioPluginsScannerMock*>(scanner.get());
        ASSERT_TRUE(mock);

        ON_CALL(*mock, scanPlugins())
        .WillByDefault(Return(foundPluginPaths));
    }

    for (const path_t& path : foundPluginPaths) {
        ON_CALL(*m_knownPlugins, exists(path))
        .WillByDefault(Return(true));
    }

    // [THEN] Unreg the uninstalled plugins
    AudioResourceIdList uninstalledPluginIdList {
        knownPlugins[0].meta.id, knownPlugins[1].meta.id
    };

    EXPECT_CALL(*m_knownPlugins, unregisterPlugins(uninstalledPluginIdList))
    .WillOnce(Return(make_ok()));

    // [THEN] No new plugins to process
    EXPECT_CALL(*m_process, execute(_, _))
    .Times(0);

    EXPECT_CALL(*m_interactive, showProgress(_, _))
    .Times(0);

    EXPECT_CALL(*m_knownPlugins, load())
    .Times(0);

    // [WHEN] Update registry
    m_scenario->updatePluginsRegistry();
}

//! A multi-component VST can expose several plugins (different IDs) from a single .vst3 path
//! When that path is still present on disk, none of the components should be re-registered
TEST_F(AudioPlugins_RegisterAudioPluginsScenarioTest, UpdatePluginsRegistry_SamePathDifferentIds_PluginPresent)
{
    path_t sharedPath = "/some/path/MultiPlugin.vst3";

    // [GIVEN] Two already registered components that share the same path
    AudioPluginInfoList knownPlugins;

    AudioPluginInfo mono;
    mono.path = sharedPath;
    mono.meta.id = "MultiPlugin_Mono";
    knownPlugins.push_back(mono);

    AudioPluginInfo stereo;
    stereo.path = sharedPath;
    stereo.meta.id = "MultiPlugin_Stereo";
    knownPlugins.push_back(stereo);

    ON_CALL(*m_knownPlugins, pluginInfoList(_))
    .WillByDefault(Return(knownPlugins));

    // [GIVEN] Scanner still finds the shared path
    for (const IAudioPluginsScannerPtr& scanner : m_scanners) {
        AudioPluginsScannerMock* mock = dynamic_cast<AudioPluginsScannerMock*>(scanner.get());
        ASSERT_TRUE(mock);

        ON_CALL(*mock, scanPlugins())
        .WillByDefault(Return(paths_t { sharedPath }));
    }

    ON_CALL(*m_knownPlugins, exists(sharedPath))
    .WillByDefault(Return(true));

    // [THEN] Neither component is re-registered or unregistered
    EXPECT_CALL(*m_process, execute(_, _))
    .Times(0);

    EXPECT_CALL(*m_interactive, showProgress(_, _))
    .Times(0);

    EXPECT_CALL(*m_knownPlugins, unregisterPlugins(_))
    .Times(0);

    EXPECT_CALL(*m_knownPlugins, load())
    .Times(0);

    // [WHEN] Update registry
    m_scenario->updatePluginsRegistry();
}

//! When a multi-component plugin is uninstalled, ALL of its component IDs must be unregistered
TEST_F(AudioPlugins_RegisterAudioPluginsScenarioTest, UpdatePluginsRegistry_SamePathDifferentIds_PluginMissing)
{
    path_t sharedPath = "/some/path/MultiPlugin.vst3";

    // [GIVEN] Two already registered components that share the same path
    AudioPluginInfoList knownPlugins;

    AudioPluginInfo mono;
    mono.path = sharedPath;
    mono.meta.id = "MultiPlugin_Mono";
    knownPlugins.push_back(mono);

    AudioPluginInfo stereo;
    stereo.path = sharedPath;
    stereo.meta.id = "MultiPlugin_Stereo";
    knownPlugins.push_back(stereo);

    ON_CALL(*m_knownPlugins, pluginInfoList(_))
    .WillByDefault(Return(knownPlugins));

    // [GIVEN] Scanner finds nothing — the plugin has been uninstalled
    for (const IAudioPluginsScannerPtr& scanner : m_scanners) {
        AudioPluginsScannerMock* mock = dynamic_cast<AudioPluginsScannerMock*>(scanner.get());
        ASSERT_TRUE(mock);

        ON_CALL(*mock, scanPlugins())
        .WillByDefault(Return(paths_t {}));
    }

    // [THEN] Both component IDs are unregistered
    AudioResourceIdList expectedIds { mono.meta.id, stereo.meta.id };
    EXPECT_CALL(*m_knownPlugins, unregisterPlugins(expectedIds))
    .WillOnce(Return(make_ok()));

    EXPECT_CALL(*m_process, execute(_, _))
    .Times(0);

    EXPECT_CALL(*m_interactive, showProgress(_, _))
    .Times(0);

    EXPECT_CALL(*m_knownPlugins, load())
    .Times(0);

    // [WHEN] Update registry
    m_scenario->updatePluginsRegistry();
}

TEST_F(AudioPlugins_RegisterAudioPluginsScenarioTest, RegisterPlugin)
{
    // [GIVEN] Some plugin we want to register
    path_t pluginPath = "/some/test/path/to/plugin/AAA.vst3";

    AudioResourceMetaList metaList;

    AudioResourceMeta pluginMeta1;
    pluginMeta1.id = "Mono plugin";
    pluginMeta1.attributes.insert({ muse::audio::CATEGORIES_ATTRIBUTE, u"Fx|Mono" });
    metaList.push_back(pluginMeta1);

    AudioResourceMeta pluginMeta2;
    pluginMeta2.id = "Stereo plugin";
    pluginMeta2.attributes.insert({ muse::audio::CATEGORIES_ATTRIBUTE, u"Fx|Stereo" });
    metaList.push_back(pluginMeta2);

    ASSERT_FALSE(m_metaReaders.empty());
    AudioPluginMetaReaderMock* mock = dynamic_cast<AudioPluginMetaReaderMock*>(m_metaReaders[0].get());
    ASSERT_TRUE(mock);

    ON_CALL(*mock, readMeta(pluginPath))
    .WillByDefault(Return(RetVal<AudioResourceMetaList>::make_ok(metaList)));

    // [THEN] The plugin has been registered
    AudioPluginInfoList expectedInfoList;
    expectedInfoList.reserve(metaList.size());

    for (const AudioResourceMeta& meta : metaList) {
        AudioPluginInfo expectedPluginInfo;
        expectedPluginInfo.type = AudioPluginType::Fx;
        expectedPluginInfo.meta = meta;
        expectedPluginInfo.path = pluginPath;
        expectedPluginInfo.enabled = true;
        expectedPluginInfo.errorCode = 0;
        expectedInfoList.emplace_back(std::move(expectedPluginInfo));
    }

    EXPECT_CALL(*m_knownPlugins, registerPlugins(expectedInfoList))
    .WillOnce(Return(true));

    // [WHEN] Register the plugin
    Ret ret = m_scenario->registerPlugin(pluginPath);

    // [THEN] The plugin successfully registered
    EXPECT_TRUE(ret);
}

TEST_F(AudioPlugins_RegisterAudioPluginsScenarioTest, RegisterFailedPlugin)
{
    // [GIVEN] Some incompatible plugin we want to register
    path_t pluginPath = "/some/test/path/to/plugin/AAA.vst3";

    // [THEN] The plugin has been registered
    AudioPluginInfo expectedPluginInfo;
    expectedPluginInfo.meta.id = io::completeBasename(pluginPath).toStdString();
    expectedPluginInfo.meta.type = AudioResourceType::VstPlugin;
    expectedPluginInfo.path = pluginPath;
    expectedPluginInfo.enabled = false;
    expectedPluginInfo.errorCode = -42;

    EXPECT_CALL(*m_knownPlugins, registerPlugins(AudioPluginInfoList { expectedPluginInfo }))
    .WillOnce(Return(true));

    // [WHEN] Register the incompatible plugin
    Ret ret = m_scenario->registerFailedPlugin(pluginPath, expectedPluginInfo.errorCode);

    // [THEN] The plugin successfully registered
    EXPECT_TRUE(ret);
}
