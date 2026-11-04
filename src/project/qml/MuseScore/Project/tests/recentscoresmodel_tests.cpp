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

#include "global/dataformatter.h"
#include "global/io/path.h"

#include "project/qml/MuseScore/Project/internal/ScoresPage/recentscoresmodel.h"

#include "project/tests/mocks/projectconfigurationmock.h"
#include "project/tests/mocks/recentfilescontrollermock.h"
#include "global/tests/mocks/filesystemmock.h"

using namespace ::testing;
using namespace mu::project;
using namespace muse;

namespace {
const QString NEW_SCORE_ITEM("<new>");
const QString NO_RESULTS_ITEM("<noResults>");

struct ModelEvent {
    enum class Type {
        Reset,
        Inserted,
        Removed,
        Changed,
    };

    Type type = Type::Reset;
    int first = -1;
    int last = -1;

    bool operator==(const ModelEvent& other) const
    {
        return type == other.type && first == other.first && last == other.last;
    }
};

void PrintTo(const ModelEvent& event, std::ostream* os)
{
    switch (event.type) {
    case ModelEvent::Type::Reset:
        *os << "Reset";
        return;
    case ModelEvent::Type::Inserted:
        *os << "Inserted ";
        break;
    case ModelEvent::Type::Removed:
        *os << "Removed ";
        break;
    case ModelEvent::Type::Changed:
        *os << "Changed ";
        break;
    }

    *os << event.first << "-" << event.last;
}

ModelEvent reset()
{
    return ModelEvent { ModelEvent::Type::Reset };
}

ModelEvent inserted(int row)
{
    return ModelEvent { ModelEvent::Type::Inserted, row, row };
}

ModelEvent removed(int row)
{
    return ModelEvent { ModelEvent::Type::Removed, row, row };
}

ModelEvent changed(int row)
{
    return ModelEvent { ModelEvent::Type::Changed, row, row };
}
}

namespace mu::project {
class Project_RecentScoresModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_controller = std::make_shared<NiceMock<RecentFilesControllerMock> >();
        m_configuration = std::make_shared<NiceMock<ProjectConfigurationMock> >();
        m_fileSystem = std::make_shared<NiceMock<io::FileSystemMock> >();

        m_model.recentFilesController.set(m_controller);
        m_model.configuration.set(m_configuration);
        m_model.fileSystem.set(m_fileSystem);

        ON_CALL(*m_controller, recentFilesListChanged())
        .WillByDefault(Return(m_recentFilesListChanged));
    }

    void recordModelEvents()
    {
        QObject::connect(&m_model, &QAbstractItemModel::modelReset, [this]() {
            m_events.push_back(ModelEvent { ModelEvent::Type::Reset });
        });
        QObject::connect(&m_model, &QAbstractItemModel::rowsInserted, [this](const QModelIndex&, int first, int last) {
            m_events.push_back(ModelEvent { ModelEvent::Type::Inserted, first, last });
        });
        QObject::connect(&m_model, &QAbstractItemModel::rowsRemoved, [this](const QModelIndex&, int first, int last) {
            m_events.push_back(ModelEvent { ModelEvent::Type::Removed, first, last });
        });
        QObject::connect(&m_model, &QAbstractItemModel::dataChanged,
                         [this](const QModelIndex& topLeft, const QModelIndex& bottomRight) {
            m_events.push_back(ModelEvent { ModelEvent::Type::Changed, topLeft.row(), bottomRight.row() });
        });
    }

    QVariantMap score(int row) const
    {
        return m_model.data(m_model.index(row, 0), m_model.roleNames().key("score")).toMap();
    }

    QStringList paths() const
    {
        QStringList result;

        for (int row = 0; row < m_model.rowCount(); ++row) {
            const QVariantMap map = score(row);

            if (map.value("isCreateNew").toBool()) {
                result << NEW_SCORE_ITEM;
            } else if (map.value("isNoResultsFound").toBool()) {
                result << NO_RESULTS_ITEM;
            } else {
                result << map.value("path").toString();
            }
        }

        return result;
    }

    async::Channel<RecentFilesChange> m_recentFilesListChanged;
    std::vector<ModelEvent> m_events;

    RecentScoresModel m_model;

    std::shared_ptr<RecentFilesControllerMock> m_controller;
    std::shared_ptr<ProjectConfigurationMock> m_configuration;
    std::shared_ptr<io::FileSystemMock> m_fileSystem;
};
}

// ==================================================
// load()
// ==================================================

TEST_F(Project_RecentScoresModelTest, Load_ShowsRecentScoresAfterSpecialItems)
{
    // [GIVEN] Two recent files
    ON_CALL(*m_controller, recentFilesList())
    .WillByDefault(ReturnRefOfCopy(RecentFilesList { RecentFile("/scores/a.mscz"), RecentFile("/scores/b.mscz") }));

    // [WHEN] Loading the model
    m_model.load();

    // [THEN] The recent scores are shown after "New score" and "No results found"
    EXPECT_EQ(paths(), QStringList({ NEW_SCORE_ITEM, NO_RESULTS_ITEM, "/scores/a.mscz", "/scores/b.mscz" }));
    EXPECT_EQ(score(0).value("name").toString(), "New score");
    EXPECT_EQ(m_model.nonScoreItemIndices(), QList<int>({ 0, 1 }));
}

TEST_F(Project_RecentScoresModelTest, Load_ItemFields)
{
    // [GIVEN] A recent score with a custom name, and a recent MusicXML file
    ON_CALL(*m_controller, recentFilesList())
    .WillByDefault(ReturnRefOfCopy(RecentFilesList { RecentFile("/scores/a.mscz", "My score"),
                                                     RecentFile("/scores/b.musicxml") }));

    // [GIVEN] The score is a cloud score with a known file size
    ON_CALL(*m_fileSystem, fileSize(io::path_t("/scores/a.mscz")))
    .WillByDefault(Return(RetVal<uint64_t>::make_ok(2048)));
    ON_CALL(*m_configuration, isCloudProject(io::path_t("/scores/a.mscz")))
    .WillByDefault(Return(true));
    ON_CALL(*m_configuration, cloudScoreIdFromPath(io::path_t("/scores/a.mscz")))
    .WillByDefault(Return(7));

    // [WHEN] Loading the model
    m_model.load();

    // [THEN] The model exposes all their properties
    ASSERT_EQ(m_model.rowCount(), 4);

    const QVariantMap cloudScore = score(2);
    EXPECT_EQ(cloudScore.value("name").toString(), "My score");
    EXPECT_EQ(cloudScore.value("path").toString(), "/scores/a.mscz");
    EXPECT_EQ(cloudScore.value("suffix").toString(), "mscz");
    EXPECT_EQ(cloudScore.value("fileSize").toString(), DataFormatter::formatFileSize(2048).toQString());
    EXPECT_TRUE(cloudScore.value("isCloud").toBool());
    EXPECT_EQ(cloudScore.value("scoreId").toInt(), 7);
    EXPECT_FALSE(cloudScore.value("isCreateNew").toBool());
    EXPECT_FALSE(cloudScore.value("isNoResultsFound").toBool());

    // [THEN] The suffix is shown for files that aren't .mscz, and an unknown file size is empty
    const QVariantMap localFile = score(3);
    EXPECT_EQ(localFile.value("name").toString(), "b.musicxml");
    EXPECT_EQ(localFile.value("suffix").toString(), "musicxml");
    EXPECT_TRUE(localFile.value("fileSize").toString().isEmpty());
    EXPECT_FALSE(localFile.value("isCloud").toBool());
}

// ==================================================
// removeRecentScore()
// ==================================================

TEST_F(Project_RecentScoresModelTest, RemoveRecentScore_DelegatesToController)
{
    // [THEN] The controller is asked to remove the file
    EXPECT_CALL(*m_controller, removeRecentFile(io::path_t("/scores/a.mscz")));

    // [WHEN] Removing a recent score
    m_model.removeRecentScore("/scores/a.mscz");
}

// ==================================================
// Recent files list changed
// ==================================================

TEST_F(Project_RecentScoresModelTest, FileRemoved_RemovesOnlyItsRow)
{
    // [GIVEN] A loaded model
    ON_CALL(*m_controller, recentFilesList())
    .WillByDefault(ReturnRefOfCopy(RecentFilesList { RecentFile("/scores/a.mscz"), RecentFile("/scores/b.mscz"),
                                                     RecentFile("/scores/c.mscz") }));
    m_model.load();

    // [GIVEN] The model's notifications are recorded
    recordModelEvents();

    // [WHEN] A file is removed from the recent files
    m_recentFilesListChanged.send(RecentFileRemoved { "/scores/b.mscz" });

    // [THEN] Only its row is removed
    EXPECT_THAT(m_events, ElementsAre(removed(3)));
    EXPECT_EQ(paths(), QStringList({ NEW_SCORE_ITEM, NO_RESULTS_ITEM, "/scores/a.mscz", "/scores/c.mscz" }));
}

TEST_F(Project_RecentScoresModelTest, FileRemoved_UnknownFile_Reloads)
{
    // [GIVEN] A loaded model that is out of sync with the controller
    ON_CALL(*m_controller, recentFilesList())
    .WillByDefault(ReturnRefOfCopy(RecentFilesList { RecentFile("/scores/a.mscz") }));
    m_model.load();

    ON_CALL(*m_controller, recentFilesList())
    .WillByDefault(ReturnRefOfCopy(RecentFilesList { RecentFile("/scores/b.mscz") }));

    // [GIVEN] The model's notifications are recorded
    recordModelEvents();

    // [WHEN] A file that isn't shown is removed
    m_recentFilesListChanged.send(RecentFileRemoved { "/scores/c.mscz" });

    // [THEN] The model is reloaded from the controller
    EXPECT_THAT(m_events, ElementsAre(reset()));
    EXPECT_EQ(paths(), QStringList({ NEW_SCORE_ITEM, NO_RESULTS_ITEM, "/scores/b.mscz" }));
}

TEST_F(Project_RecentScoresModelTest, FilePrepended_NewFile_InsertsItAtTop)
{
    // [GIVEN] A loaded model
    ON_CALL(*m_controller, recentFilesList())
    .WillByDefault(ReturnRefOfCopy(RecentFilesList { RecentFile("/scores/a.mscz"), RecentFile("/scores/b.mscz") }));
    m_model.load();

    // [GIVEN] The model's notifications are recorded
    recordModelEvents();

    // [WHEN] A new file is opened
    m_recentFilesListChanged.send(RecentFilePrepended { RecentFile("/scores/c.mscz") });

    // [THEN] It is inserted right after "No results found"
    EXPECT_THAT(m_events, ElementsAre(inserted(2)));
    EXPECT_EQ(paths(), QStringList({ NEW_SCORE_ITEM, NO_RESULTS_ITEM, "/scores/c.mscz", "/scores/a.mscz", "/scores/b.mscz" }));
}

TEST_F(Project_RecentScoresModelTest, FilePrepended_ExistingFile_MovesItToTop)
{
    // [GIVEN] A loaded model
    ON_CALL(*m_controller, recentFilesList())
    .WillByDefault(ReturnRefOfCopy(RecentFilesList { RecentFile("/scores/a.mscz"), RecentFile("/scores/b.mscz"),
                                                     RecentFile("/scores/c.mscz") }));
    m_model.load();

    // [GIVEN] The model's notifications are recorded
    recordModelEvents();

    // [WHEN] A recent file is opened again
    m_recentFilesListChanged.send(RecentFilePrepended { RecentFile("/scores/c.mscz") });

    // [THEN] Its row is removed and inserted right after "No results found"
    EXPECT_THAT(m_events, ElementsAre(removed(4), inserted(2)));
    EXPECT_EQ(paths(), QStringList({ NEW_SCORE_ITEM, NO_RESULTS_ITEM, "/scores/c.mscz", "/scores/a.mscz", "/scores/b.mscz" }));
}

TEST_F(Project_RecentScoresModelTest, FilePrepended_FirstFile_StaysAtTop)
{
    // [GIVEN] A loaded model
    ON_CALL(*m_controller, recentFilesList())
    .WillByDefault(ReturnRefOfCopy(RecentFilesList { RecentFile("/scores/a.mscz"), RecentFile("/scores/b.mscz") }));
    m_model.load();

    // [GIVEN] The model's notifications are recorded
    recordModelEvents();

    // [WHEN] The first recent file is opened again, with a different display name
    m_recentFilesListChanged.send(RecentFilePrepended { RecentFile("/scores/a.mscz", "My score") });

    // [THEN] It stays at the top, with the new data
    EXPECT_THAT(m_events, ElementsAre(removed(2), inserted(2)));
    EXPECT_EQ(paths(), QStringList({ NEW_SCORE_ITEM, NO_RESULTS_ITEM, "/scores/a.mscz", "/scores/b.mscz" }));
    EXPECT_EQ(score(2).value("name").toString(), "My score");
}

TEST_F(Project_RecentScoresModelTest, FilePathChanged_UpdatesItsRowInPlace)
{
    // [GIVEN] A loaded model
    ON_CALL(*m_controller, recentFilesList())
    .WillByDefault(ReturnRefOfCopy(RecentFilesList { RecentFile("/scores/a.mscz"), RecentFile("/scores/b.mscz"),
                                                     RecentFile("/scores/c.mscz") }));
    m_model.load();

    // [GIVEN] The model's notifications are recorded
    recordModelEvents();

    // [WHEN] A file is moved on disk
    m_recentFilesListChanged.send(RecentFilePathChanged { "/scores/b.mscz", RecentFile("/other/d.mscz") });

    // [THEN] Its row is updated in place
    EXPECT_THAT(m_events, ElementsAre(changed(3)));
    EXPECT_EQ(paths(), QStringList({ NEW_SCORE_ITEM, NO_RESULTS_ITEM, "/scores/a.mscz", "/other/d.mscz", "/scores/c.mscz" }));
    EXPECT_EQ(score(3).value("name").toString(), "d");
}

TEST_F(Project_RecentScoresModelTest, FilePathChanged_UnknownFile_Reloads)
{
    // [GIVEN] A loaded model that is out of sync with the controller
    ON_CALL(*m_controller, recentFilesList())
    .WillByDefault(ReturnRefOfCopy(RecentFilesList { RecentFile("/scores/a.mscz") }));
    m_model.load();

    ON_CALL(*m_controller, recentFilesList())
    .WillByDefault(ReturnRefOfCopy(RecentFilesList { RecentFile("/scores/a.mscz"), RecentFile("/other/d.mscz") }));

    // [GIVEN] The model's notifications are recorded
    recordModelEvents();

    // [WHEN] A file that isn't shown is moved on disk
    m_recentFilesListChanged.send(RecentFilePathChanged { "/scores/b.mscz", RecentFile("/other/d.mscz") });

    // [THEN] The model is reloaded from the controller
    EXPECT_THAT(m_events, ElementsAre(reset()));
    EXPECT_EQ(paths(), QStringList({ NEW_SCORE_ITEM, NO_RESULTS_ITEM, "/scores/a.mscz", "/other/d.mscz" }));
}

TEST_F(Project_RecentScoresModelTest, Reset_ReloadsFromController)
{
    // [GIVEN] A loaded model
    ON_CALL(*m_controller, recentFilesList())
    .WillByDefault(ReturnRefOfCopy(RecentFilesList { RecentFile("/scores/a.mscz"), RecentFile("/scores/b.mscz") }));
    m_model.load();

    // [GIVEN] The model's notifications are recorded
    recordModelEvents();

    // [WHEN] The recent files are reset to a different list
    ON_CALL(*m_controller, recentFilesList())
    .WillByDefault(ReturnRefOfCopy(RecentFilesList { RecentFile("/scores/c.mscz") }));
    m_recentFilesListChanged.send(RecentFilesReset {});

    // [THEN] The model is reloaded from the controller
    EXPECT_THAT(m_events, ElementsAre(reset()));
    EXPECT_EQ(paths(), QStringList({ NEW_SCORE_ITEM, NO_RESULTS_ITEM, "/scores/c.mscz" }));
}

TEST_F(Project_RecentScoresModelTest, Reset_SameFiles_DoesNotResetModel)
{
    // [GIVEN] A loaded model
    ON_CALL(*m_controller, recentFilesList())
    .WillByDefault(ReturnRefOfCopy(RecentFilesList { RecentFile("/scores/a.mscz"), RecentFile("/scores/b.mscz") }));
    m_model.load();

    // [GIVEN] The model's notifications are recorded
    recordModelEvents();

    // [WHEN] A reset is sent, but the recent files are the same
    m_recentFilesListChanged.send(RecentFilesReset {});

    // [THEN] Nothing changes
    EXPECT_TRUE(m_events.empty());
    EXPECT_EQ(paths(), QStringList({ NEW_SCORE_ITEM, NO_RESULTS_ITEM, "/scores/a.mscz", "/scores/b.mscz" }));
}
