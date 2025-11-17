/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include <QTemporaryDir>
#include <QDir>

#include "extensions/api/v1/util.h"
#include "global/modularity/ioc.h"

using namespace muse::extensions::apiv1;

class Extensions_UtilTests : public ::testing::Test
{
public:
    QTemporaryDir tempDir;

    void SetUp() override {
        ASSERT_TRUE(tempDir.isValid());
    }
};

//---------------------------------------------------------
//   testPathTraversalResolution
//   Test that path traversal patterns are resolved correctly
//---------------------------------------------------------

TEST_F(Extensions_UtilTests, pathTraversalResolution)
{
    // Test QDir::cleanPath resolves path traversal
    // Note: QDir::cleanPath doesn't resolve beyond root, so /tmp/../../.. becomes /
    QString path1 = "/tmp/../../../etc/foo";
    QString clean1 = QDir::cleanPath(path1);
    EXPECT_EQ(clean1.toStdString(), "/../../etc/foo")
        << "Path traversal beyond root should be normalized but not resolve beyond /";

    QString path2 = "/tmp/foo/../bar/../file.txt";
    QString clean2 = QDir::cleanPath(path2);
    EXPECT_EQ(clean2.toStdString(), "/tmp/file.txt")
        << "Nested path traversal should resolve correctly";

    QString path3 = "/tmp/./subdir/../file.txt";
    QString clean3 = QDir::cleanPath(path3);
    EXPECT_EQ(clean3.toStdString(), "/tmp/file.txt")
        << "Mixed . and .. should resolve correctly";
}

//---------------------------------------------------------
//   testAllowedPaths
//   Test that allowed paths are correctly identified
//---------------------------------------------------------

TEST_F(Extensions_UtilTests, allowedPaths)
{
    FileIO fileIO;

    // Test temp directory is allowed
    QString tempFile = tempDir.filePath("test.txt");
    fileIO.setSource(tempFile);
    bool resultTemp = fileIO.write("test data");
    EXPECT_TRUE(resultTemp) << "Write to temp directory should succeed";

    // Cleanup
    QFile::remove(tempFile);
}

//---------------------------------------------------------
//   testBlockedSystemPaths
//   Test that system paths are blocked
//---------------------------------------------------------

TEST_F(Extensions_UtilTests, blockedSystemPaths)
{
    FileIO fileIO;

    // Test system directories are blocked
    QStringList blockedPaths = {
        "/etc/foo",
        "/usr/bin/footest",
        "/bin/foosh"
    };

    for (const QString& path : blockedPaths) {
        fileIO.setSource(path);
        bool result = fileIO.write("malicious");
        EXPECT_FALSE(result) << "Write to " << path.toStdString() << " should be blocked";
    }
}

//---------------------------------------------------------
//   testPathTraversalBlocked
//   Test that path traversal attacks are blocked
//---------------------------------------------------------

TEST_F(Extensions_UtilTests, pathTraversalBlocked)
{
    FileIO fileIO;

    // Test various path traversal patterns
    QStringList traversalPaths = {
        "/tmp/../../../etc/bazpasswd",
        "/tmp/foo/../../../../../../etc/foo",
        tempDir.path() + "/../../../etc/bar"
    };

    for (const QString& path : traversalPaths) {
        fileIO.setSource(path);
        bool result = fileIO.write("malicious");
        EXPECT_FALSE(result) << "Path traversal " << path.toStdString() << " should be blocked";
    }
}

//---------------------------------------------------------
//   testRelativePathResolution
//   Test that straightforward paths within allowed directories work
//---------------------------------------------------------

TEST_F(Extensions_UtilTests, relativePathResolution)
{
    FileIO fileIO;

    // Test straightforward path within temp directory
    QString testPath = tempDir.path() + "/test.txt";
    fileIO.setSource(testPath);
    bool result = fileIO.write("test data");
    EXPECT_TRUE(result) << "Write to temp directory should succeed";

    // Verify file was created
    EXPECT_TRUE(QFile::exists(testPath));

    // Cleanup
    QFile::remove(testPath);
}
