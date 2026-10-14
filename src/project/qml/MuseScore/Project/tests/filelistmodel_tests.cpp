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
#include "global/translation.h"
#include "ui/view/iconcodes.h"

#include "project/qml/MuseScore/Project/internal/ConvertFileToScore/filelistmodel.h"

#include "global/tests/mocks/filesystemmock.h"

using namespace ::testing;
using namespace mu::project;
using namespace muse;
using namespace muse::ui;

namespace mu::project {
class Project_FileListModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_fileSystem = std::make_shared<NiceMock<io::FileSystemMock> >();
        m_model.fileSystem.set(m_fileSystem);
    }

    void setFileSize(const QString& path, uint64_t sizeBytes)
    {
        ON_CALL(*m_fileSystem, fileSize(io::path_t(path)))
        .WillByDefault(Return(RetVal<uint64_t>::make_ok(sizeBytes)));
    }

    int role(const QByteArray& name) const
    {
        return m_model.roleNames().key(name);
    }

    QVariant data(int row, const QByteArray& roleName) const
    {
        return m_model.data(m_model.index(row, 0), role(roleName));
    }

    FileListModel m_model;
    std::shared_ptr<io::FileSystemMock> m_fileSystem;
    ConvertConfig m_config;
};
}

// ==================================================
// Initial state
// ==================================================

TEST_F(Project_FileListModelTest, InitialState_IsEmpty)
{
    // [GIVEN] A freshly created model
    // [THEN] It has no files, no category and no limits
    EXPECT_EQ(m_model.rowCount(), 0);
    EXPECT_TRUE(m_model.paths().isEmpty());
    EXPECT_EQ(m_model.fileCategory(), FileCategory::Unknown);
    EXPECT_EQ(m_model.fileCategoryQml(), FileListModel::FileCategoryQml::Unknown);
    EXPECT_EQ(m_model.fileIconCode(), int(IconCode::Code::NEW_FILE));
    EXPECT_TRUE(m_model.combinedFilesNote().isEmpty());
    EXPECT_TRUE(m_model.usedSizeLabel().isEmpty());
    EXPECT_FALSE(m_model.exceedsLimits());
    EXPECT_EQ(m_model.convertLimits().value("maxFileCount").toInt(), 0);
    EXPECT_EQ(m_model.convertLimits().value("maxCombinedSizeBytes").toLongLong(), 0);
}

TEST_F(Project_FileListModelTest, RoleNames)
{
    const QList<QByteArray> roles = m_model.roleNames().values();

    EXPECT_EQ(roles.size(), 3);
    EXPECT_TRUE(roles.contains("pathRole"));
    EXPECT_TRUE(roles.contains("fileNameRole"));
    EXPECT_TRUE(roles.contains("fileSizeRole"));
}

// ==================================================
// load()
// ==================================================

TEST_F(Project_FileListModelTest, Load_SetsPathsAndNotifies)
{
    // [THEN] Count the pathsChanged notifications
    int pathsChangedCount = 0;
    QObject::connect(&m_model, &FileListModel::pathsChanged, [&pathsChangedCount]() { ++pathsChangedCount; });

    // [WHEN] Loading two files
    const QStringList paths { "/some/first.png", "/some/second.png" };
    m_model.load(paths, m_config);

    // [THEN] The paths are stored, in order
    EXPECT_EQ(m_model.paths(), paths);
    EXPECT_EQ(m_model.rowCount(), paths.size());
    EXPECT_EQ(pathsChangedCount, 1);
}

TEST_F(Project_FileListModelTest, Load_ResolvesCategoryFromFirstPath)
{
    m_model.load({ "/some/file.pdf" }, m_config);
    EXPECT_EQ(m_model.fileCategory(), FileCategory::Pdf);
    EXPECT_EQ(m_model.fileCategoryQml(), FileListModel::FileCategoryQml::Pdf);

    m_model.load({ "/some/file.png", "/some/file.mp3" }, m_config);
    EXPECT_EQ(m_model.fileCategory(), FileCategory::Image);
    EXPECT_EQ(m_model.fileCategoryQml(), FileListModel::FileCategoryQml::Image);

    m_model.load({ "/some/file.mp3" }, m_config);
    EXPECT_EQ(m_model.fileCategory(), FileCategory::Audio);
    EXPECT_EQ(m_model.fileCategoryQml(), FileListModel::FileCategoryQml::Audio);

    m_model.load({ "/some/file.xyz" }, m_config);
    EXPECT_EQ(m_model.fileCategory(), FileCategory::Unknown);

    m_model.load({}, m_config);
    EXPECT_EQ(m_model.fileCategory(), FileCategory::Unknown);
}

TEST_F(Project_FileListModelTest, Load_SetsConvertLimitsForCategory)
{
    // [GIVEN] Different limits for every file category
    m_config.audio2score.file.maxFiles = 1;
    m_config.audio2score.file.maxFileSizeBytes = 100;
    m_config.omr.pdf.maxFiles = 2;
    m_config.omr.pdf.maxFileSizeBytes = 200;
    m_config.omr.images.maxFiles = 3;
    m_config.omr.images.maxFileSizeBytes = 300;

    // [THEN] Count the convertLimitsChanged notifications
    int convertLimitsChangedCount = 0;
    QObject::connect(&m_model, &FileListModel::convertLimitsChanged, [&convertLimitsChangedCount]() { ++convertLimitsChangedCount; });

    // [WHEN] Loading an audio file
    m_model.load({ "/some/file.mp3" }, m_config);

    // [THEN] The audio limits are used
    EXPECT_EQ(m_model.convertLimits().value("maxFileCount").toInt(), m_config.audio2score.file.maxFiles);
    EXPECT_EQ(m_model.convertLimits().value("maxCombinedSizeBytes").toLongLong(), m_config.audio2score.file.maxFileSizeBytes);
    EXPECT_EQ(convertLimitsChangedCount, 1);

    // [WHEN] Loading a PDF file
    m_model.load({ "/some/file.pdf" }, m_config);

    // [THEN] The PDF limits are used
    EXPECT_EQ(m_model.convertLimits().value("maxFileCount").toInt(), m_config.omr.pdf.maxFiles);
    EXPECT_EQ(m_model.convertLimits().value("maxCombinedSizeBytes").toLongLong(), m_config.omr.pdf.maxFileSizeBytes);
    EXPECT_EQ(convertLimitsChangedCount, 2);

    // [WHEN] Loading an image
    m_model.load({ "/some/file.png" }, m_config);

    // [THEN] The image limits are used
    EXPECT_EQ(m_model.convertLimits().value("maxFileCount").toInt(), m_config.omr.images.maxFiles);
    EXPECT_EQ(m_model.convertLimits().value("maxCombinedSizeBytes").toLongLong(), m_config.omr.images.maxFileSizeBytes);
    EXPECT_EQ(convertLimitsChangedCount, 3);

    // [WHEN] Loading another image
    m_model.load({ "/some/other.jpg" }, m_config);

    // [THEN] The limits are unchanged, so no notification is sent
    EXPECT_EQ(convertLimitsChangedCount, 3);
}

// ==================================================
// setPaths() / clear()
// ==================================================

TEST_F(Project_FileListModelTest, SetPaths_UsesGivenCategory)
{
    // [WHEN] Setting paths with an explicit category, which doesn't match the file suffix
    m_model.setPaths({ "/some/file.xyz" }, FileCategory::Audio);

    // [THEN] The given category wins
    EXPECT_EQ(m_model.fileCategory(), FileCategory::Audio);
}

TEST_F(Project_FileListModelTest, SetPaths_SamePaths_DoesNothing)
{
    // [GIVEN] Some paths
    const QStringList paths { "/some/file.png" };
    m_model.setPaths(paths, FileCategory::Image);

    // [THEN] Count the pathsChanged notifications
    int pathsChangedCount = 0;
    QObject::connect(&m_model, &FileListModel::pathsChanged, [&pathsChangedCount]() { ++pathsChangedCount; });

    // [WHEN] Setting the same paths again
    m_model.setPaths(paths, FileCategory::Image);

    // [THEN] Nothing is notified
    EXPECT_EQ(pathsChangedCount, 0);
}

TEST_F(Project_FileListModelTest, Clear_ResetsToInitialState)
{
    // [GIVEN] PDF files that exceed the limits
    m_config.omr.pdf.maxFiles = 1;
    m_config.omr.pdf.maxFileSizeBytes = 1000;
    setFileSize("/some/first.pdf", 800);
    setFileSize("/some/second.pdf", 700);
    m_model.load({ "/some/first.pdf", "/some/second.pdf" }, m_config);
    ASSERT_TRUE(m_model.exceedsLimits());
    ASSERT_FALSE(m_model.usedSizeLabel().isEmpty());

    // [THEN] Count the pathsChanged notifications
    int pathsChangedCount = 0;
    QObject::connect(&m_model, &FileListModel::pathsChanged, [&pathsChangedCount]() { ++pathsChangedCount; });

    // [WHEN] Clearing the model
    m_model.clear();

    // [THEN] It is back in the initial state
    EXPECT_EQ(m_model.rowCount(), 0);
    EXPECT_TRUE(m_model.paths().isEmpty());
    EXPECT_EQ(m_model.fileCategory(), FileCategory::Unknown);
    EXPECT_EQ(m_model.fileCategoryQml(), FileListModel::FileCategoryQml::Unknown);
    EXPECT_EQ(m_model.fileIconCode(), int(IconCode::Code::NEW_FILE));
    EXPECT_TRUE(m_model.combinedFilesNote().isEmpty());
    EXPECT_TRUE(m_model.usedSizeLabel().isEmpty());
    EXPECT_FALSE(m_model.exceedsLimits());
    EXPECT_EQ(m_model.convertLimits().value("maxFileCount").toInt(), 0);
    EXPECT_EQ(m_model.convertLimits().value("maxCombinedSizeBytes").toLongLong(), 0);
    EXPECT_EQ(pathsChangedCount, 1);
}

// ==================================================
// data() / get()
// ==================================================

TEST_F(Project_FileListModelTest, Data_ReturnsPathNameAndSize)
{
    // [GIVEN] A file of a known size
    const QString path = "/some/score.png";
    const uint64_t sizeBytes = 2048;
    setFileSize(path, sizeBytes);
    m_model.load({ path }, m_config);

    // [THEN] Every role returns the expected value
    EXPECT_EQ(data(0, "pathRole").toString(), path);
    EXPECT_EQ(data(0, "fileNameRole").toString(), "score.png");
    EXPECT_EQ(data(0, "fileSizeRole").toString(), DataFormatter::formatFileSize(sizeBytes).toQString());
}

TEST_F(Project_FileListModelTest, Data_FileSizeUnavailable_FallsBackToZero)
{
    // [GIVEN] A file whose size can't be read
    const QString path = "/some/score.png";
    ON_CALL(*m_fileSystem, fileSize(io::path_t(path)))
    .WillByDefault(Return(RetVal<uint64_t>::make_ret(make_ret(Ret::Code::UnknownError))));
    m_model.load({ path }, m_config);

    // [THEN] Its size is reported as zero
    EXPECT_EQ(data(0, "fileSizeRole").toString(), DataFormatter::formatFileSize(0).toQString());
}

TEST_F(Project_FileListModelTest, Data_InvalidIndexOrRole_ReturnsInvalidVariant)
{
    m_model.load({ "/some/file.png" }, m_config);

    EXPECT_FALSE(m_model.data(QModelIndex(), role("pathRole")).isValid());
    EXPECT_FALSE(m_model.data(m_model.index(1, 0), role("pathRole")).isValid());
    EXPECT_FALSE(m_model.data(m_model.index(0, 0), Qt::DisplayRole).isValid());
}

TEST_F(Project_FileListModelTest, Get_ReturnsAllRolesByName)
{
    // [GIVEN] A file of a known size
    const QString path = "/some/score.png";
    const uint64_t sizeBytes = 2048;
    setFileSize(path, sizeBytes);
    m_model.load({ path }, m_config);

    // [WHEN] Getting an item
    const QVariantMap item = m_model.get(0);

    // [THEN] It contains every role, keyed by role name
    EXPECT_EQ(item.size(), m_model.roleNames().size());
    EXPECT_EQ(item.value("pathRole").toString(), path);
    EXPECT_EQ(item.value("fileNameRole").toString(), "score.png");
    EXPECT_EQ(item.value("fileSizeRole").toString(), DataFormatter::formatFileSize(sizeBytes).toQString());
}

// ==================================================
// fileIconCode() / combinedFilesNote()
// ==================================================

TEST_F(Project_FileListModelTest, FileIconCode_DependsOnCategory)
{
    m_model.load({ "/some/file.png" }, m_config);
    EXPECT_EQ(m_model.fileIconCode(), int(IconCode::Code::IMAGE_MOUNTAINS));

    m_model.load({ "/some/file.mp3" }, m_config);
    EXPECT_EQ(m_model.fileIconCode(), int(IconCode::Code::MUSIC_NOTES));

    m_model.load({ "/some/file.pdf" }, m_config);
    EXPECT_EQ(m_model.fileIconCode(), int(IconCode::Code::NEW_FILE));

    m_model.load({ "/some/file.xyz" }, m_config);
    EXPECT_EQ(m_model.fileIconCode(), int(IconCode::Code::NEW_FILE));
}

TEST_F(Project_FileListModelTest, CombinedFilesNote_OnlyForMultipleImages)
{
    const QString note = muse::qtrc("project/convert", "Images will be combined into one score in the order shown here");

    // [GIVEN] A single image
    m_model.load({ "/some/first.png" }, m_config);
    EXPECT_TRUE(m_model.combinedFilesNote().isEmpty());

    // [GIVEN] Multiple images
    m_model.load({ "/some/first.png", "/some/second.png" }, m_config);
    EXPECT_EQ(m_model.combinedFilesNote(), note);

    // [GIVEN] Multiple non-image files
    m_model.load({ "/some/first.mp3", "/some/second.mp3" }, m_config);
    EXPECT_TRUE(m_model.combinedFilesNote().isEmpty());
}

// ==================================================
// exceedsLimits()
// ==================================================

TEST_F(Project_FileListModelTest, ExceedsLimits_NoLimits_NeverExceeds)
{
    // [GIVEN] A config without any limits, and several non-empty files
    setFileSize("/some/first.png", 1000);
    setFileSize("/some/second.png", 1000);

    // [THEN] Count the exceedsLimitsChanged notifications
    int exceedsLimitsChangedCount = 0;
    QObject::connect(&m_model, &FileListModel::exceedsLimitsChanged, [&exceedsLimitsChangedCount]() { ++exceedsLimitsChangedCount; });

    // [WHEN] Loading them
    m_model.load({ "/some/first.png", "/some/second.png" }, m_config);

    // [THEN] The limits are not exceeded
    EXPECT_FALSE(m_model.exceedsLimits());
    EXPECT_EQ(exceedsLimitsChangedCount, 0);
}

TEST_F(Project_FileListModelTest, ExceedsLimits_ByFileCount)
{
    // [GIVEN] At most two images are allowed
    m_config.omr.images.maxFiles = 2;

    // [THEN] Count the exceedsLimitsChanged notifications
    int exceedsLimitsChangedCount = 0;
    QObject::connect(&m_model, &FileListModel::exceedsLimitsChanged, [&exceedsLimitsChangedCount]() { ++exceedsLimitsChangedCount; });

    // [WHEN] Loading two images
    m_model.load({ "/some/first.png", "/some/second.png" }, m_config);

    // [THEN] The limits are not exceeded
    EXPECT_FALSE(m_model.exceedsLimits());

    // [WHEN] Loading three images
    m_model.load({ "/some/first.png", "/some/second.png", "/some/third.png" }, m_config);

    // [THEN] The limits are exceeded
    EXPECT_TRUE(m_model.exceedsLimits());
    EXPECT_EQ(exceedsLimitsChangedCount, 1);
}

TEST_F(Project_FileListModelTest, ExceedsLimits_ByCombinedSize)
{
    // [GIVEN] The combined size of all images is limited
    m_config.omr.images.maxFileSizeBytes = 1000;
    setFileSize("/some/first.png", 600);
    setFileSize("/some/second.png", 400);
    setFileSize("/some/third.png", 1);

    // [THEN] Count the exceedsLimitsChanged notifications
    int exceedsLimitsChangedCount = 0;
    QObject::connect(&m_model, &FileListModel::exceedsLimitsChanged, [&exceedsLimitsChangedCount]() { ++exceedsLimitsChangedCount; });

    // [WHEN] Loading images that add up to exactly the limit
    m_model.load({ "/some/first.png", "/some/second.png" }, m_config);

    // [THEN] The limits are not exceeded
    EXPECT_FALSE(m_model.exceedsLimits());

    // [WHEN] Loading one more image
    m_model.load({ "/some/first.png", "/some/second.png", "/some/third.png" }, m_config);

    // [THEN] The limits are exceeded
    EXPECT_TRUE(m_model.exceedsLimits());
    EXPECT_EQ(exceedsLimitsChangedCount, 1);
}

// ==================================================
// usedSizeLabel()
// ==================================================

TEST_F(Project_FileListModelTest, UsedSizeLabel_HiddenBelowThreshold)
{
    // [GIVEN] An image that uses less than 75% of the combined size limit
    m_config.omr.images.maxFileSizeBytes = 1000;
    setFileSize("/some/file.png", 749);

    // [THEN] Count the usedSizeLabelChanged notifications
    int usedSizeLabelChangedCount = 0;
    QObject::connect(&m_model, &FileListModel::usedSizeLabelChanged, [&usedSizeLabelChangedCount]() { ++usedSizeLabelChangedCount; });

    // [WHEN] Loading it
    m_model.load({ "/some/file.png" }, m_config);

    // [THEN] The label is hidden
    EXPECT_TRUE(m_model.usedSizeLabel().isEmpty());
    EXPECT_EQ(usedSizeLabelChangedCount, 0);
}

TEST_F(Project_FileListModelTest, UsedSizeLabel_ShownFromThreshold)
{
    // [GIVEN] An image that uses 75% of the combined size limit
    m_config.omr.images.maxFileSizeBytes = 1000;
    const uint64_t usedSizeBytes = 750;
    setFileSize("/some/file.png", usedSizeBytes);

    // [THEN] Count the usedSizeLabelChanged notifications
    int usedSizeLabelChangedCount = 0;
    QObject::connect(&m_model, &FileListModel::usedSizeLabelChanged, [&usedSizeLabelChangedCount]() { ++usedSizeLabelChangedCount; });

    // [WHEN] Loading it
    m_model.load({ "/some/file.png" }, m_config);

    // [THEN] The label shows the used and the maximum size
    const QString expectedLabel = muse::qtrc("project/convert", "%1/%2 used")
                                  .arg(DataFormatter::formatFileSize(usedSizeBytes).toQString(),
                                       DataFormatter::formatFileSize(size_t(m_config.omr.images.maxFileSizeBytes)).toQString());
    EXPECT_EQ(m_model.usedSizeLabel(), expectedLabel);
    EXPECT_EQ(usedSizeLabelChangedCount, 1);
}

TEST_F(Project_FileListModelTest, UsedSizeLabel_HiddenWithoutSizeLimit)
{
    // [GIVEN] A config without a size limit, and a non-empty image
    setFileSize("/some/file.png", 750);

    // [WHEN] Loading it
    m_model.load({ "/some/file.png" }, m_config);

    // [THEN] The label is hidden
    EXPECT_TRUE(m_model.usedSizeLabel().isEmpty());
}

// ==================================================
// removeAt()
// ==================================================

TEST_F(Project_FileListModelTest, RemoveAt_RemovesFile)
{
    // [GIVEN] Three files
    m_model.load({ "/some/first.png", "/some/second.png", "/some/third.png" }, m_config);

    // [THEN] Count the pathsChanged and rowsRemoved notifications
    int pathsChangedCount = 0;
    QObject::connect(&m_model, &FileListModel::pathsChanged, [&pathsChangedCount]() { ++pathsChangedCount; });
    int rowsRemovedCount = 0;
    QObject::connect(&m_model, &FileListModel::rowsRemoved, [&rowsRemovedCount]() { ++rowsRemovedCount; });

    // [WHEN] Removing the second one
    m_model.removeAt(1);

    // [THEN] The other files remain, and the category is kept
    EXPECT_EQ(m_model.paths(), QStringList({ "/some/first.png", "/some/third.png" }));
    EXPECT_EQ(m_model.fileCategory(), FileCategory::Image);
    EXPECT_EQ(rowsRemovedCount, 1);
    EXPECT_EQ(pathsChangedCount, 1);
}

TEST_F(Project_FileListModelTest, RemoveAt_InvalidIndex_DoesNothing)
{
    // [GIVEN] A single file
    m_model.load({ "/some/file.png" }, m_config);

    // [THEN] Count the pathsChanged notifications
    int pathsChangedCount = 0;
    QObject::connect(&m_model, &FileListModel::pathsChanged, [&pathsChangedCount]() { ++pathsChangedCount; });

    // [WHEN] Removing at out-of-range indexes
    m_model.removeAt(-1);
    m_model.removeAt(1);

    // [THEN] Nothing changes
    EXPECT_EQ(m_model.rowCount(), 1);
    EXPECT_EQ(pathsChangedCount, 0);
}

TEST_F(Project_FileListModelTest, RemoveAt_LastFile_ResetsCategory)
{
    // [GIVEN] A single file
    m_model.load({ "/some/file.pdf" }, m_config);
    ASSERT_EQ(m_model.fileCategory(), FileCategory::Pdf);

    // [WHEN] Removing it
    m_model.removeAt(0);

    // [THEN] The category is reset
    EXPECT_EQ(m_model.rowCount(), 0);
    EXPECT_EQ(m_model.fileCategory(), FileCategory::Unknown);
}

TEST_F(Project_FileListModelTest, RemoveAt_UpdatesExceedsLimitsAndUsedSizeLabel)
{
    // [GIVEN] Images that exceed the combined size limit
    m_config.omr.images.maxFileSizeBytes = 1000;
    setFileSize("/some/first.png", 800);
    setFileSize("/some/second.png", 700);
    m_model.load({ "/some/first.png", "/some/second.png" }, m_config);
    ASSERT_TRUE(m_model.exceedsLimits());
    ASSERT_FALSE(m_model.usedSizeLabel().isEmpty());

    // [THEN] Count the exceedsLimitsChanged and usedSizeLabelChanged notifications
    int exceedsLimitsChangedCount = 0;
    QObject::connect(&m_model, &FileListModel::exceedsLimitsChanged, [&exceedsLimitsChangedCount]() { ++exceedsLimitsChangedCount; });
    int usedSizeLabelChangedCount = 0;
    QObject::connect(&m_model, &FileListModel::usedSizeLabelChanged, [&usedSizeLabelChangedCount]() { ++usedSizeLabelChangedCount; });

    // [WHEN] Removing the first image
    m_model.removeAt(0);

    // [THEN] The limits are no longer exceeded, and the label is hidden again
    EXPECT_FALSE(m_model.exceedsLimits());
    EXPECT_TRUE(m_model.usedSizeLabel().isEmpty());
    EXPECT_EQ(exceedsLimitsChangedCount, 1);
    EXPECT_EQ(usedSizeLabelChangedCount, 1);
}

// ==================================================
// move()
// ==================================================

TEST_F(Project_FileListModelTest, Move_Down)
{
    // [GIVEN] Three files
    m_model.load({ "/some/first.png", "/some/second.png", "/some/third.png" }, m_config);

    // [THEN] Count the pathsChanged and rowsMoved notifications
    int pathsChangedCount = 0;
    QObject::connect(&m_model, &FileListModel::pathsChanged, [&pathsChangedCount]() { ++pathsChangedCount; });
    int rowsMovedCount = 0;
    QObject::connect(&m_model, &FileListModel::rowsMoved, [&rowsMovedCount]() { ++rowsMovedCount; });

    // [WHEN] Moving the first file to the end
    m_model.move(0, 2);

    // [THEN] The files are reordered
    EXPECT_EQ(m_model.paths(), QStringList({ "/some/second.png", "/some/third.png", "/some/first.png" }));
    EXPECT_EQ(rowsMovedCount, 1);
    EXPECT_EQ(pathsChangedCount, 1);
}

TEST_F(Project_FileListModelTest, Move_Up)
{
    // [GIVEN] Three files
    m_model.load({ "/some/first.png", "/some/second.png", "/some/third.png" }, m_config);

    // [WHEN] Moving the last file to the beginning
    m_model.move(2, 0);

    // [THEN] The files are reordered
    EXPECT_EQ(m_model.paths(), QStringList({ "/some/third.png", "/some/first.png", "/some/second.png" }));
}

TEST_F(Project_FileListModelTest, Move_InvalidArguments_DoesNothing)
{
    // [GIVEN] Two files
    const QStringList paths { "/some/first.png", "/some/second.png" };
    m_model.load(paths, m_config);

    // [THEN] Count the pathsChanged notifications
    int pathsChangedCount = 0;
    QObject::connect(&m_model, &FileListModel::pathsChanged, [&pathsChangedCount]() { ++pathsChangedCount; });

    // [WHEN] Moving with out-of-range or equal indexes
    m_model.move(-1, 0);
    m_model.move(0, -1);
    m_model.move(2, 0);
    m_model.move(0, 2);
    m_model.move(1, 1);

    // [THEN] Nothing changes
    EXPECT_EQ(m_model.paths(), paths);
    EXPECT_EQ(pathsChangedCount, 0);
}
