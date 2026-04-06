/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include <cstdint>
#include <cstring>
#include <string>

#include "io/file.h"
#include "io/filestream.h"
#include "io/path.h"
#include "types/bytearray.h"

using namespace muse;
using namespace muse::io;

class Global_IO_FileStreamTests : public ::testing::Test
{
public:
    void TearDown() override
    {
        File::remove(m_filePath);
    }

protected:
    path_t m_filePath;
};

static ByteArray toByteArray(const std::string& s)
{
    return ByteArray(reinterpret_cast<const uint8_t*>(s.c_str()), s.size());
}

static ByteArray readAll(const path_t& filePath)
{
    ByteArray data;
    Ret ret = File::readFile(filePath, data);
    EXPECT_TRUE(ret) << "Failed to read test file: " << filePath.toStdString();
    return data;
}

TEST_F(Global_IO_FileStreamTests, WriteOnly_WritesContentToDisk)
{
    m_filePath = path_t("FileStreamTests_WriteOnly.txt");
    std::string ref = "Hello World!";

    {
        //! GIVEN A new FileStream
        FileStream fs(m_filePath);

        //! DO Open for writing
        EXPECT_TRUE(fs.open(IODevice::WriteOnly));

        //! DO Write data
        size_t written = fs.write(toByteArray(ref));

        //! CHECK all bytes written
        EXPECT_EQ(written, ref.size());
    }

    //! CHECK content on disk matches
    EXPECT_EQ(readAll(m_filePath), toByteArray(ref));
}

TEST_F(Global_IO_FileStreamTests, WriteOnly_MultipleChunks_AllChunksOnDisk)
{
    m_filePath = path_t("FileStreamTests_WriteOnly_MultiChunk.txt");
    std::string chunk1 = "Hello";
    std::string chunk2 = " ";
    std::string chunk3 = "World!";

    {
        //! GIVEN A new FileStream
        FileStream fs(m_filePath);

        //! DO Open for writing
        EXPECT_TRUE(fs.open(IODevice::WriteOnly));

        //! DO Write chunks one by one
        EXPECT_EQ(fs.write(toByteArray(chunk1)), chunk1.size());
        EXPECT_EQ(fs.write(toByteArray(chunk2)), chunk2.size());
        EXPECT_EQ(fs.write(toByteArray(chunk3)), chunk3.size());
    }

    //! CHECK all chunks are concatenated on disk
    EXPECT_EQ(readAll(m_filePath), toByteArray(chunk1 + chunk2 + chunk3));
}

TEST_F(Global_IO_FileStreamTests, WriteOnly_OverwritesExistingFile)
{
    m_filePath = path_t("FileStreamTests_WriteOnly_Overwrite.txt");

    //! GIVEN A file with existing content
    {
        FileStream fs(m_filePath);
        EXPECT_TRUE(fs.open(IODevice::WriteOnly));
        fs.write(toByteArray("Old content here"));
    }

    //! DO Open the same file again with WriteOnly
    {
        FileStream fs(m_filePath);
        EXPECT_TRUE(fs.open(IODevice::WriteOnly));

        //! CHECK size is 0 (file truncated on open)
        EXPECT_EQ(fs.size(), 0);

        fs.write(toByteArray("New"));
    }

    //! CHECK only new content is on disk
    EXPECT_EQ(readAll(m_filePath), toByteArray("New"));
}

TEST_F(Global_IO_FileStreamTests, Append_AppendsToExistingFile)
{
    m_filePath = path_t("FileStreamTests_Append.txt");

    //! GIVEN A file with initial content
    {
        FileStream fs(m_filePath);
        EXPECT_TRUE(fs.open(IODevice::WriteOnly));
        fs.write(toByteArray("Hello World!"));
    }

    {
        //! GIVEN Open same file in Append mode
        FileStream fs(m_filePath);
        EXPECT_TRUE(fs.open(IODevice::Append));

        //! CHECK size reflects existing file length
        EXPECT_EQ(fs.size(), 12);

        //! DO Append data
        size_t written = fs.write(toByteArray("mimi"));
        EXPECT_EQ(written, 4);
    }

    //! CHECK appended content follows original
    EXPECT_EQ(readAll(m_filePath), toByteArray("Hello World!mimi"));
}

TEST_F(Global_IO_FileStreamTests, Append_MultipleChunks_AllAppended)
{
    m_filePath = path_t("FileStreamTests_Append_MultiChunk.txt");

    //! GIVEN A file with initial content
    {
        FileStream fs(m_filePath);
        EXPECT_TRUE(fs.open(IODevice::WriteOnly));
        fs.write(toByteArray("Base"));
    }

    {
        //! GIVEN Open in Append mode
        FileStream fs(m_filePath);
        EXPECT_TRUE(fs.open(IODevice::Append));

        //! DO Append several chunks
        EXPECT_EQ(fs.write(toByteArray("-A")), 2);
        EXPECT_EQ(fs.write(toByteArray("-B")), 2);
        EXPECT_EQ(fs.write(toByteArray("-C")), 2);
    }

    //! CHECK all chunks appended in order
    EXPECT_EQ(readAll(m_filePath), toByteArray("Base-A-B-C"));
}

TEST_F(Global_IO_FileStreamTests, Append_CreatesFileIfNotExists)
{
    m_filePath = path_t("FileStreamTests_Append_NewFile.txt");

    // Make sure file doesn't exist
    File::remove(m_filePath);

    {
        //! GIVEN Open a non-existent file in Append mode
        FileStream fs(m_filePath);
        EXPECT_TRUE(fs.open(IODevice::Append));

        //! DO Write data
        EXPECT_EQ(fs.write(toByteArray("Created")), 7);
    }

    //! CHECK file was created with correct content
    EXPECT_EQ(readAll(m_filePath), toByteArray("Created"));
}

TEST_F(Global_IO_FileStreamTests, WriteOnly_SizeTracksWrittenBytes)
{
    m_filePath = path_t("FileStreamTests_Size.txt");

    FileStream fs(m_filePath);
    EXPECT_TRUE(fs.open(IODevice::WriteOnly));

    EXPECT_EQ(fs.size(), 0);

    fs.write(toByteArray("abc"));
    EXPECT_EQ(fs.size(), 3);

    fs.write(toByteArray("de"));
    EXPECT_EQ(fs.size(), 5);
}
