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
#include <QFile>
#include <QString>
#include <QByteArray>

class Engraving_FileIOTests : public ::testing::Test
{
public:
    QTemporaryDir tempDir;
};

//---------------------------------------------------------
//   testWriteBinary
//   Test that binary data can be written using toLatin1() method
//   This verifies the approach used by FileIO::writeBinary()
//---------------------------------------------------------

TEST_F(Engraving_FileIOTests, writeBinary)
{
    ASSERT_TRUE(tempDir.isValid());

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

TEST_F(Engraving_FileIOTests, writeBinaryWithHighBytes)
{
    ASSERT_TRUE(tempDir.isValid());

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

TEST_F(Engraving_FileIOTests, writeTextStillWorks)
{
    ASSERT_TRUE(tempDir.isValid());

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

TEST_F(Engraving_FileIOTests, textEncodingCorruptsBinary)
{
    ASSERT_TRUE(tempDir.isValid());

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
