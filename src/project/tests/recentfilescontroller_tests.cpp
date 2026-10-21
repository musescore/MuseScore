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
#include <gmock/gmock.h>

#include <string>
#include <variant>
#include <vector>

#include <QThreadPool>

#include "project/internal/recentfilescontroller.h"

#include "global/async/processevents.h"
#include "global/serialization/json.h"
#include "global/types/bytearray.h"
#include "global/types/ret.h"

#include "mocks/projectconfigurationmock.h"
#include "global/tests/mocks/filesystemmock.h"
#include "multiwindows/tests/mocks/multiwindowsprovidermock.h"

using namespace ::testing;
using namespace mu::project;
using namespace muse;

namespace {
const std::string RECENT_FILES_RESOURCE_NAME("RECENT_FILES");
}

namespace mu::project {
class TestableRecentFilesController : public RecentFilesController
{
public:
    MOCK_METHOD(void, prependPlatformRecentFile, (const muse::io::path_t&), (override));
    MOCK_METHOD(void, clearPlatformRecentFiles, (), (override));
};

class Project_RecentFilesControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_configuration = std::make_shared<NiceMock<ProjectConfigurationMock> >();
        m_fileSystem = std::make_shared<NiceMock<io::FileSystemMock> >();
        m_multiWindowsProvider = std::make_shared<NiceMock<mi::MultiWindowsProviderMock> >();

        m_controller.configuration.set(m_configuration);
        m_controller.fileSystem.set(m_fileSystem);
        m_controller.multiwindowsProvider.set(m_multiWindowsProvider);

        ON_CALL(*m_multiWindowsProvider, resourceChanged()).WillByDefault(Return(m_resourceChanged));
    }

    void TearDown() override
    {
        async::processMessages();
        QThreadPool::globalInstance()->waitForDone();
    }

    void storeFiles(const char* json)
    {
        m_storedData = ByteArray(json);

        ON_CALL(*m_fileSystem, readFile(_)).WillByDefault(Invoke([this](const io::path_t&) {
            return RetVal<ByteArray>::make_ok(m_storedData);
        }));
    }

    void setExistingFiles(const std::vector<std::string>& paths)
    {
        for (const std::string& path : paths) {
            ON_CALL(*m_fileSystem, exists(io::path_t(path))).WillByDefault(Return(Ret(true)));
        }
    }

    void captureSavedFiles()
    {
        ON_CALL(*m_fileSystem, writeFile(_, _)).WillByDefault(Invoke([this](const io::path_t&, const ByteArray& data) {
            m_storedData = data;
            return Ret(true);
        }));
    }

    void recordChanges()
    {
        m_controller.recentFilesListChanged().onReceive(nullptr, [this](const RecentFilesChange& change) {
            m_changes.push_back(change);
        });
    }

    std::vector<std::string> paths() const
    {
        const RecentFilesList& files = m_controller.recentFilesList();

        std::vector<std::string> result;
        result.reserve(files.size());

        for (const RecentFile& file : files) {
            result.push_back(file.path.toStdString());
        }

        return result;
    }

    async::Channel<std::string> m_resourceChanged;
    NiceMock<TestableRecentFilesController> m_controller;
    ByteArray m_storedData;
    std::vector<RecentFilesChange> m_changes;

    std::shared_ptr<ProjectConfigurationMock> m_configuration;
    std::shared_ptr<io::FileSystemMock> m_fileSystem;
    std::shared_ptr<mi::MultiWindowsProviderMock> m_multiWindowsProvider;
};
}

// ==================================================
// Loading
// ==================================================

TEST_F(Project_RecentFilesControllerTest, Load_ReadsPathsAndDisplayNames)
{
    // [GIVEN] A stored list with a plain path and a path with a display name
    storeFiles(R"(["/scores/a.mscz", { "path": "/scores/b.mscz", "displayName": "B" }])");
    setExistingFiles({ "/scores/a.mscz", "/scores/b.mscz" });

    // [WHEN] Getting the list
    const RecentFilesList& files = m_controller.recentFilesList();

    // [THEN] Both files are loaded
    EXPECT_THAT(files, ElementsAre(RecentFile("/scores/a.mscz"), RecentFile("/scores/b.mscz", "B")));
}

TEST_F(Project_RecentFilesControllerTest, Load_EmptyFile_UsesCompatData)
{
    // [GIVEN] The stored list is empty, but there is compat data
    ON_CALL(*m_configuration, compatRecentFilesData()).WillByDefault(Return(ByteArray(R"(["/scores/a.mscz"])")));
    setExistingFiles({ "/scores/a.mscz" });

    // [WHEN] Getting the list
    std::vector<std::string> loadedPaths = paths();

    // [THEN] The compat data is loaded
    EXPECT_THAT(loadedPaths, ElementsAre("/scores/a.mscz"));
}

TEST_F(Project_RecentFilesControllerTest, Load_InvalidJson_ReturnsEmptyList)
{
    // [GIVEN] The stored list is not valid JSON
    storeFiles("not json");

    // [THEN] The list is empty
    EXPECT_TRUE(m_controller.recentFilesList().empty());
}

TEST_F(Project_RecentFilesControllerTest, Load_NonexistentFile_IsRemovedAndSaved)
{
    // [GIVEN] A stored list with a file that no longer exists
    storeFiles(R"(["/scores/a.mscz", "/scores/b.mscz"])");
    setExistingFiles({ "/scores/a.mscz" });

    // [GIVEN] Saved files are written to the stored list
    captureSavedFiles();

    // [GIVEN] The changes are recorded
    recordChanges();

    // [WHEN] Getting the list
    std::vector<std::string> loadedPaths = paths();

    // [THEN] The nonexistent file is not in the list
    EXPECT_THAT(loadedPaths, ElementsAre("/scores/a.mscz"));

    // [WHEN] The deferred save is processed
    async::processMessages();

    // [THEN] The list without the nonexistent file is saved
    const JsonArray array = JsonDocument::fromJson(m_storedData).rootArray();
    ASSERT_EQ(array.size(), 1);
    EXPECT_EQ(array.at(0).toStdString(), "/scores/a.mscz");

    // [THEN] A reset is sent
    ASSERT_EQ(m_changes.size(), 1);
    EXPECT_TRUE(std::holds_alternative<RecentFilesReset>(m_changes.at(0)));
}

// ==================================================
// Saving
// ==================================================

TEST_F(Project_RecentFilesControllerTest, Save_WritesDisplayNameOnlyWhenSet)
{
    // [GIVEN] A loaded list with one file without a display name
    storeFiles(R"(["/scores/a.mscz"])");
    setExistingFiles({ "/scores/a.mscz", "/scores/b.mscz" });
    m_controller.recentFilesList();

    // [GIVEN] Saved files are written to the stored list
    captureSavedFiles();

    // [WHEN] Prepending a file with a display name
    m_controller.prependRecentFile(RecentFile("/scores/b.mscz", "B"));

    // [THEN] The file with a display name is saved as an object, the other one as a plain path
    const JsonArray array = JsonDocument::fromJson(m_storedData).rootArray();
    ASSERT_EQ(array.size(), 2);

    const JsonObject obj = array.at(0).toObject();
    EXPECT_EQ(obj["path"].toStdString(), "/scores/b.mscz");
    EXPECT_EQ(obj["displayName"].toStdString(), "B");

    EXPECT_TRUE(array.at(1).isString());
    EXPECT_EQ(array.at(1).toStdString(), "/scores/a.mscz");
}

// ==================================================
// Prepending
// ==================================================

TEST_F(Project_RecentFilesControllerTest, Prepend_NewFile_PutsItFirst)
{
    // [GIVEN] A loaded list
    storeFiles(R"(["/scores/a.mscz", "/scores/b.mscz"])");
    setExistingFiles({ "/scores/a.mscz", "/scores/b.mscz", "/scores/c.mscz" });
    m_controller.recentFilesList();

    // [GIVEN] Saved files are written to the stored list
    captureSavedFiles();

    // [GIVEN] The changes are recorded
    recordChanges();

    // [THEN] The file is also added to the platform recent files
    EXPECT_CALL(m_controller, prependPlatformRecentFile(io::path_t("/scores/c.mscz")));

    // [WHEN] Prepending a new file
    m_controller.prependRecentFile(RecentFile("/scores/c.mscz"));

    // [THEN] The file is first in the list
    EXPECT_THAT(paths(), ElementsAre("/scores/c.mscz", "/scores/a.mscz", "/scores/b.mscz"));

    // [THEN] Only the prepended file is reported
    ASSERT_EQ(m_changes.size(), 1);
    const RecentFilePrepended* change = std::get_if<RecentFilePrepended>(&m_changes.at(0));
    ASSERT_NE(change, nullptr);
    EXPECT_EQ(change->file, RecentFile("/scores/c.mscz"));
}

TEST_F(Project_RecentFilesControllerTest, Prepend_ExistingFile_MovesItFirst)
{
    // [GIVEN] A loaded list
    storeFiles(R"(["/scores/a.mscz", "/scores/b.mscz", "/scores/c.mscz"])");
    setExistingFiles({ "/scores/a.mscz", "/scores/b.mscz", "/scores/c.mscz" });
    m_controller.recentFilesList();

    // [GIVEN] Saved files are written to the stored list
    captureSavedFiles();

    // [GIVEN] The changes are recorded
    recordChanges();

    // [WHEN] Prepending a file that is already in the list
    m_controller.prependRecentFile(RecentFile("/scores/c.mscz"));

    // [THEN] The file is moved to the top, not duplicated
    EXPECT_THAT(paths(), ElementsAre("/scores/c.mscz", "/scores/a.mscz", "/scores/b.mscz"));

    // [THEN] Only the prepended file is reported
    ASSERT_EQ(m_changes.size(), 1);
    EXPECT_TRUE(std::holds_alternative<RecentFilePrepended>(m_changes.at(0)));
}

TEST_F(Project_RecentFilesControllerTest, Prepend_NonexistentFileDropped_SendsReset)
{
    // [GIVEN] A loaded list
    storeFiles(R"(["/scores/a.mscz", "/scores/b.mscz"])");
    setExistingFiles({ "/scores/a.mscz", "/scores/b.mscz", "/scores/c.mscz" });
    m_controller.recentFilesList();

    // [GIVEN] One of the files was deleted after loading
    ON_CALL(*m_fileSystem, exists(io::path_t("/scores/b.mscz"))).WillByDefault(Return(Ret(false)));

    // [GIVEN] Saved files are written to the stored list
    captureSavedFiles();

    // [GIVEN] The changes are recorded
    recordChanges();

    // [WHEN] Prepending a new file
    m_controller.prependRecentFile(RecentFile("/scores/c.mscz"));

    // [THEN] The deleted file is dropped
    EXPECT_THAT(paths(), ElementsAre("/scores/c.mscz", "/scores/a.mscz"));

    // [THEN] A reset is sent, since more than one row changed
    ASSERT_EQ(m_changes.size(), 1);
    EXPECT_TRUE(std::holds_alternative<RecentFilesReset>(m_changes.at(0)));
}

TEST_F(Project_RecentFilesControllerTest, Prepend_InvalidFile_DoesNothing)
{
    // [GIVEN] A loaded list
    storeFiles(R"(["/scores/a.mscz"])");
    setExistingFiles({ "/scores/a.mscz" });
    m_controller.recentFilesList();

    // [GIVEN] The changes are recorded
    recordChanges();

    // [THEN] Nothing is saved or added to the platform recent files
    EXPECT_CALL(*m_fileSystem, writeFile(_, _)).Times(0);
    EXPECT_CALL(m_controller, prependPlatformRecentFile(_)).Times(0);

    // [WHEN] Prepending a file without a path
    m_controller.prependRecentFile(RecentFile());

    // [THEN] The list is unchanged
    EXPECT_THAT(paths(), ElementsAre("/scores/a.mscz"));

    // [THEN] Nothing is sent
    EXPECT_TRUE(m_changes.empty());
}

// ==================================================
// Moving
// ==================================================

TEST_F(Project_RecentFilesControllerTest, Move_ReplacesFileInPlace)
{
    // [GIVEN] A loaded list
    storeFiles(R"(["/scores/a.mscz", "/scores/b.mscz", "/scores/c.mscz"])");
    setExistingFiles({ "/scores/a.mscz", "/scores/b.mscz", "/scores/c.mscz", "/other/d.mscz" });
    m_controller.recentFilesList();

    // [GIVEN] Saved files are written to the stored list
    captureSavedFiles();

    // [GIVEN] The changes are recorded
    recordChanges();

    // [WHEN] Moving a file
    m_controller.moveRecentFile("/scores/b.mscz", RecentFile("/other/d.mscz"));

    // [THEN] The file is replaced at the same position
    EXPECT_THAT(paths(), ElementsAre("/scores/a.mscz", "/other/d.mscz", "/scores/c.mscz"));

    // [THEN] The path change is reported
    ASSERT_EQ(m_changes.size(), 1);
    const RecentFilePathChanged* change = std::get_if<RecentFilePathChanged>(&m_changes.at(0));
    ASSERT_NE(change, nullptr);
    EXPECT_EQ(change->oldPath, io::path_t("/scores/b.mscz"));
    EXPECT_EQ(change->file, RecentFile("/other/d.mscz"));
}

TEST_F(Project_RecentFilesControllerTest, Move_UnknownFile_DoesNothing)
{
    // [GIVEN] A loaded list
    storeFiles(R"(["/scores/a.mscz"])");
    setExistingFiles({ "/scores/a.mscz" });
    m_controller.recentFilesList();

    // [GIVEN] The changes are recorded
    recordChanges();

    // [THEN] Nothing is saved
    EXPECT_CALL(*m_fileSystem, writeFile(_, _)).Times(0);

    // [WHEN] Moving a file that is not in the list
    m_controller.moveRecentFile("/scores/x.mscz", RecentFile("/other/d.mscz"));

    // [THEN] The list is unchanged
    EXPECT_THAT(paths(), ElementsAre("/scores/a.mscz"));

    // [THEN] Nothing is sent
    EXPECT_TRUE(m_changes.empty());
}

// ==================================================
// Removing
// ==================================================

TEST_F(Project_RecentFilesControllerTest, Remove_RemovesOnlyThatFile)
{
    // [GIVEN] A loaded list
    storeFiles(R"(["/scores/a.mscz", "/scores/b.mscz", "/scores/c.mscz"])");
    setExistingFiles({ "/scores/a.mscz", "/scores/b.mscz", "/scores/c.mscz" });
    m_controller.recentFilesList();

    // [GIVEN] Saved files are written to the stored list
    captureSavedFiles();

    // [GIVEN] The changes are recorded
    recordChanges();

    // [WHEN] Removing a file
    m_controller.removeRecentFile("/scores/b.mscz");

    // [THEN] Only that file is removed
    EXPECT_THAT(paths(), ElementsAre("/scores/a.mscz", "/scores/c.mscz"));

    // [THEN] The removal is reported
    ASSERT_EQ(m_changes.size(), 1);
    const RecentFileRemoved* change = std::get_if<RecentFileRemoved>(&m_changes.at(0));
    ASSERT_NE(change, nullptr);
    EXPECT_EQ(change->path, io::path_t("/scores/b.mscz"));
}

TEST_F(Project_RecentFilesControllerTest, Remove_UnknownFile_DoesNothing)
{
    // [GIVEN] A loaded list
    storeFiles(R"(["/scores/a.mscz"])");
    setExistingFiles({ "/scores/a.mscz" });
    m_controller.recentFilesList();

    // [GIVEN] The changes are recorded
    recordChanges();

    // [THEN] Nothing is saved
    EXPECT_CALL(*m_fileSystem, writeFile(_, _)).Times(0);

    // [WHEN] Removing a file that is not in the list, and an empty path
    m_controller.removeRecentFile("/scores/x.mscz");
    m_controller.removeRecentFile("");

    // [THEN] The list is unchanged
    EXPECT_THAT(paths(), ElementsAre("/scores/a.mscz"));

    // [THEN] Nothing is sent
    EXPECT_TRUE(m_changes.empty());
}

// ==================================================
// Clearing
// ==================================================

TEST_F(Project_RecentFilesControllerTest, Clear_RemovesAllFiles)
{
    // [GIVEN] A loaded list
    storeFiles(R"(["/scores/a.mscz", "/scores/b.mscz"])");
    setExistingFiles({ "/scores/a.mscz", "/scores/b.mscz" });
    m_controller.recentFilesList();

    // [GIVEN] Saved files are written to the stored list
    captureSavedFiles();

    // [GIVEN] The changes are recorded
    recordChanges();

    // [THEN] The platform recent files are also cleared
    EXPECT_CALL(m_controller, clearPlatformRecentFiles());

    // [WHEN] Clearing the list
    m_controller.clearRecentFiles();

    // [THEN] The list is empty
    EXPECT_TRUE(m_controller.recentFilesList().empty());

    // [THEN] A reset is sent
    ASSERT_EQ(m_changes.size(), 1);
    EXPECT_TRUE(std::holds_alternative<RecentFilesReset>(m_changes.at(0)));
}

// ==================================================
// Changes from other windows
// ==================================================

TEST_F(Project_RecentFilesControllerTest, ResourceChanged_ReloadsAndSendsReset)
{
    // [GIVEN] The controller is initialized
    m_controller.init();

    // [GIVEN] A loaded list
    storeFiles(R"(["/scores/a.mscz"])");
    setExistingFiles({ "/scores/a.mscz", "/scores/b.mscz" });
    m_controller.recentFilesList();

    // [GIVEN] The changes are recorded
    recordChanges();

    // [GIVEN] The stored list was changed by another window
    storeFiles(R"(["/scores/b.mscz", "/scores/a.mscz"])");

    // [WHEN] The resource change is reported
    m_resourceChanged.send(RECENT_FILES_RESOURCE_NAME);

    // [THEN] A reset is sent
    ASSERT_EQ(m_changes.size(), 1);
    EXPECT_TRUE(std::holds_alternative<RecentFilesReset>(m_changes.at(0)));

    // [THEN] The new list is loaded
    EXPECT_THAT(paths(), ElementsAre("/scores/b.mscz", "/scores/a.mscz"));
}

TEST_F(Project_RecentFilesControllerTest, ResourceChanged_OtherResource_IsIgnored)
{
    // [GIVEN] The controller is initialized
    m_controller.init();

    // [GIVEN] The changes are recorded
    recordChanges();

    // [WHEN] A change of another resource is reported
    m_resourceChanged.send("OTHER_RESOURCE");

    // [THEN] Nothing is sent
    EXPECT_TRUE(m_changes.empty());
}

TEST_F(Project_RecentFilesControllerTest, ResourceChanged_WhileSaving_IsIgnored)
{
    // [GIVEN] The controller is initialized
    m_controller.init();

    // [GIVEN] A loaded list
    storeFiles(R"(["/scores/a.mscz", "/scores/b.mscz"])");
    setExistingFiles({ "/scores/a.mscz", "/scores/b.mscz" });
    m_controller.recentFilesList();

    // [GIVEN] The resource change is reported back while saving
    ON_CALL(*m_multiWindowsProvider, notifyAboutResourceChanged(RECENT_FILES_RESOURCE_NAME))
    .WillByDefault(Invoke([this](const std::string& name) {
        m_resourceChanged.send(name);
    }));

    // [GIVEN] The changes are recorded
    recordChanges();

    // [WHEN] Removing a file
    m_controller.removeRecentFile("/scores/b.mscz");

    // [THEN] Only the removal is sent, not a reset
    ASSERT_EQ(m_changes.size(), 1);
    EXPECT_TRUE(std::holds_alternative<RecentFileRemoved>(m_changes.at(0)));
}
