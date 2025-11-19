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
#include <QFile>
#include <QString>
#include <QByteArray>

#include "engraving/api/v1/util.h"
#include "global/modularity/ioc.h"

using namespace mu::engraving::apiv1;

class Extensions_UtilTests : public ::testing::Test
{
public:
    QTemporaryDir tempDir;

    void SetUp() override
    {
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

//---------------------------------------------------------
//   Binary I/O Tests
//   These tests verify the low-level binary data handling
//   used by FileIO::writeBinary()
//---------------------------------------------------------

//---------------------------------------------------------
//   testWriteBinary
//   Test that binary data can be written using toLatin1() method
//   This verifies the approach used by FileIO::writeBinary()
//---------------------------------------------------------

TEST_F(Extensions_UtilTests, writeBinary)
{
    QString testFile = tempDir.filePath("test_binary.bin");

    // Create test data with all byte values 0-255
    QString binaryData;
    for (int i = 0; i < 256; i++) {
        binaryData += QChar(i);
    }

    // Write using the same approach as FileIO::writeBinary()
    QFile file(testFile);
    ASSERT_TRUE(file.open(QFile::WriteOnly | QFile::Truncate));
    QByteArray bytes = binaryData.toLatin1();
    qint64 written = file.write(bytes);
    file.close();

    EXPECT_EQ(written, 256);

    // Verify file was created
    EXPECT_TRUE(QFile::exists(testFile));

    // Read back and verify all bytes preserved
    QFile readFile(testFile);
    ASSERT_TRUE(readFile.open(QIODevice::ReadOnly));
    QByteArray readData = readFile.readAll();
    readFile.close();

    EXPECT_EQ(readData.size(), 256);

    // Verify each byte value
    for (int i = 0; i < 256; i++) {
        EXPECT_EQ((unsigned char)readData[i], i)
            << "Byte at position " << i << " was corrupted";
    }
}

//---------------------------------------------------------
//   testWriteBinaryWithHighBytes
//   Test that bytes > 127 are preserved correctly
//---------------------------------------------------------

TEST_F(Extensions_UtilTests, writeBinaryWithHighBytes)
{
    QString testFile = tempDir.filePath("test_high_bytes.bin");

    // Test specifically high bytes that often get corrupted in text encoding
    QString binaryData;
    unsigned char testBytes[] = { 0x00, 0x7F, 0x80, 0xFF, 0xFE, 0x01, 0x02 };
    for (size_t i = 0; i < sizeof(testBytes); i++) {
        binaryData += QChar(testBytes[i]);
    }

    // Write using the same approach as FileIO::writeBinary()
    QFile file(testFile);
    ASSERT_TRUE(file.open(QFile::WriteOnly | QFile::Truncate));
    QByteArray bytes = binaryData.toLatin1();
    file.write(bytes);
    file.close();

    // Read back and verify
    QFile readFile(testFile);
    ASSERT_TRUE(readFile.open(QIODevice::ReadOnly));
    QByteArray readData = readFile.readAll();
    readFile.close();

    EXPECT_EQ(readData.size(), sizeof(testBytes));

    for (size_t i = 0; i < sizeof(testBytes); i++) {
        EXPECT_EQ((unsigned char)readData[i], testBytes[i]);
    }
}

//---------------------------------------------------------
//   testWriteTextStillWorks
//   Test that UTF-8 text writing works correctly
//   This verifies normal text file operations are not affected
//---------------------------------------------------------

TEST_F(Extensions_UtilTests, writeTextStillWorks)
{
    QString testFile = tempDir.filePath("test_text.txt");
    QString textData = "Hello, World!\nThis is a test.";

    // Write using normal text approach
    QFile file(testFile);
    ASSERT_TRUE(file.open(QFile::WriteOnly | QFile::Text));
    file.write(textData.toUtf8());
    file.close();

    // Verify file was created
    EXPECT_TRUE(QFile::exists(testFile));

    // Read back and verify
    QFile readFile(testFile);
    ASSERT_TRUE(readFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString readData = QString::fromUtf8(readFile.readAll());
    readFile.close();

    EXPECT_EQ(readData, textData);
}

//---------------------------------------------------------
//   testTextEncodingCorruptsBinary
//   Test that using UTF-8 encoding corrupts binary data
//   This demonstrates why writeBinary() is necessary
//---------------------------------------------------------

TEST_F(Extensions_UtilTests, textEncodingCorruptsBinary)
{
    QString testFile = tempDir.filePath("test_corrupted.bin");

    // Create binary data with high bytes that get corrupted in UTF-8
    QString binaryData;
    unsigned char testBytes[] = { 0x80, 0xFF, 0xFE, 0xC0, 0xE0 };
    for (size_t i = 0; i < sizeof(testBytes); i++) {
        binaryData += QChar(testBytes[i]);
    }

    // Write using UTF-8 text encoding (wrong approach for binary)
    QFile file(testFile);
    ASSERT_TRUE(file.open(QFile::WriteOnly | QFile::Text));
    file.write(binaryData.toUtf8());  // This corrupts the data!
    file.close();

    // Read back as binary
    QFile readFile(testFile);
    ASSERT_TRUE(readFile.open(QIODevice::ReadOnly));
    QByteArray readData = readFile.readAll();
    readFile.close();

    // Verify data was corrupted (different from original)
    EXPECT_NE(readData.size(), sizeof(testBytes))
        << "UTF-8 encoding changed the data size, demonstrating corruption";

    // If by chance the size matches, verify at least some bytes are different
    if (readData.size() == sizeof(testBytes)) {
        bool foundDifference = false;
        for (size_t i = 0; i < sizeof(testBytes); i++) {
            if ((unsigned char)readData[i] != testBytes[i]) {
                foundDifference = true;
                break;
            }
        }
        EXPECT_TRUE(foundDifference)
            << "At least some bytes should be corrupted by UTF-8 encoding";
    }
}
