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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "global/io/file.h"
#include "global/io/iodevice.h"

#include "converter/convertercodes.h"
#include "converter/internal/convertercontroller.h"

#include "notation/tests/mocks/excerptnotationmock.h"
#include "notation/tests/mocks/masternotationmock.h"
#include "notation/tests/mocks/notationelementsmock.h"
#include "notation/tests/mocks/notationmock.h"
#include "project/tests/mocks/notationprojectmock.h"
#include "project/tests/mocks/notationwritermock.h"
#include "project/tests/mocks/notationwritersregistermock.h"
#include "project/tests/mocks/projectcreatormock.h"
#include "project/tests/mocks/projectsessionmock.h"

using namespace ::testing;
using namespace mu::converter;
using namespace mu::notation;
using namespace mu::project;
using namespace muse;
using namespace muse::io;

using Options = INotationWriter::Options;
using OptionKey = INotationWriter::OptionKey;

namespace {
Ret writeToDevice(IODevice& device, const std::string& data)
{
    device.write(ByteArray(data.c_str()));
    return make_ok();
}

//! NOTE: Simulates writers (e.g. video) that ignore the device and write to the "file_path" themselves
Ret writeToFilePath(IODevice& device, const std::string& data)
{
    return File::writeFile(device.meta("file_path"), ByteArray(data.c_str()));
}

std::string readFile(const path_t& path)
{
    ByteArray data;
    if (!File::readFile(path, data)) {
        return std::string();
    }

    return std::string(data.constChar(), data.size());
}

int optionInt(const Options& options, OptionKey key)
{
    return muse::value(options, key, Val(-1)).toInt();
}
}

class Converter_ConverterControllerTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_controller = std::make_shared<ConverterController>(muse::modularity::globalCtx());

        m_projectCreator = std::make_shared<NiceMock<ProjectCreatorMock> >();
        m_writers = std::make_shared<NiceMock<NotationWritersRegisterMock> >();
        m_projectSession = std::make_shared<NiceMock<ProjectSessionMock> >();
        m_writer = std::make_shared<NiceMock<NotationWriterMock> >();
        m_project = std::make_shared<NiceMock<NotationProjectMock> >();
        m_masterNotation = std::make_shared<NiceMock<MasterNotationMock> >();
        m_notation = std::make_shared<NiceMock<NotationMock> >();

        m_controller->notationCreator.set(m_projectCreator);
        m_controller->writers.set(m_writers);
        m_controller->projectSession.set(m_projectSession);

        ON_CALL(*m_writers, writer(_)).WillByDefault(Return(m_writer));
        ON_CALL(*m_projectCreator, newProject(_)).WillByDefault(Return(m_project));
        ON_CALL(*m_project, load(_, _, _)).WillByDefault(Return(make_ok()));
        ON_CALL(*m_project, masterNotation()).WillByDefault(Return(m_masterNotation));
        ON_CALL(*m_masterNotation, notation()).WillByDefault(Return(m_notation));
    }

    void TearDown() override
    {
        for (const path_t& path : m_filePaths) {
            File::remove(path);
        }
    }

    //! NOTE: Files are created in the working directory of the test and removed in TearDown
    path_t filePath(const std::string& fileName)
    {
        path_t path("ConverterControllerTests_" + fileName);
        m_filePaths.push_back(path);
        return path;
    }

    //! NOTE: Notation with the given number of pages (the pages themselves are not accessed)
    void setPageCount(size_t count)
    {
        m_pages = PageList(count, nullptr);

        auto elements = std::make_shared<NiceMock<NotationElementsMock> >();
        ON_CALL(*elements, pages()).WillByDefault(ReturnRef(m_pages));
        ON_CALL(*m_notation, elements()).WillByDefault(Return(elements));
    }

    void setExcerpts(const std::vector<QString>& names)
    {
        m_excerpts.clear();

        for (const QString& name : names) {
            auto notation = std::make_shared<NiceMock<NotationMock> >();
            ON_CALL(*notation, name()).WillByDefault(Return(name));

            auto excerpt = std::make_shared<NiceMock<ExcerptNotationMock> >();
            ON_CALL(*excerpt, notation()).WillByDefault(Return(notation));

            m_excerpts.push_back(excerpt);
        }

        ON_CALL(*m_masterNotation, excerpts()).WillByDefault(ReturnRef(m_excerpts));
    }

    std::shared_ptr<ConverterController> m_controller;

    std::shared_ptr<ProjectCreatorMock> m_projectCreator;
    std::shared_ptr<NotationWritersRegisterMock> m_writers;
    std::shared_ptr<ProjectSessionMock> m_projectSession;
    std::shared_ptr<NotationWriterMock> m_writer;
    std::shared_ptr<NotationProjectMock> m_project;
    std::shared_ptr<MasterNotationMock> m_masterNotation;
    std::shared_ptr<NotationMock> m_notation;

    PageList m_pages;
    ExcerptNotationList m_excerpts;

    std::vector<path_t> m_filePaths;
};

TEST_F(Converter_ConverterControllerTests, FileConvert_WriterWritesToDevice)
{
    // [GIVEN] Writer that writes the data to the device
    const path_t out = filePath("out.pdf");

    EXPECT_CALL(*m_writer, write(_, _, _))
    .WillOnce(Invoke([out](INotationPtr, IODevice& device, const Options&) {
        EXPECT_EQ(device.meta("file_path"), out.toStdString());
        return writeToDevice(device, "data");
    }));

    // [WHEN] Convert the file
    Ret ret = m_controller->fileConvert("in.mscz", out);

    // [THEN] The written data is in the output file
    EXPECT_TRUE(ret);
    EXPECT_EQ(readFile(out), "data");
}

TEST_F(Converter_ConverterControllerTests, FileConvert_WriterWritesToFilePath_FileIsNotOverwritten)
{
    // [GIVEN] Writer that writes to the "file_path" itself and leaves the device empty
    const path_t out = filePath("out.mp4");

    EXPECT_CALL(*m_writer, write(_, _, _))
    .WillOnce(Invoke([](INotationPtr, IODevice& device, const Options&) {
        return writeToFilePath(device, "data");
    }));

    // [WHEN] Convert the file
    Ret ret = m_controller->fileConvert("in.mscz", out);

    // [THEN] The written data is not overwritten by the empty device
    EXPECT_TRUE(ret);
    EXPECT_EQ(readFile(out), "data");
}

TEST_F(Converter_ConverterControllerTests, FileConvert_WriterFails_OutputFileIsRemoved)
{
    // [GIVEN] Writer that writes part of the data and then fails
    const path_t out = filePath("out.pdf");

    EXPECT_CALL(*m_writer, write(_, _, _))
    .WillOnce(Invoke([](INotationPtr, IODevice& device, const Options&) {
        writeToDevice(device, "data");
        return make_ret(Ret::Code::UnknownError);
    }));

    // [WHEN] Convert the file
    Ret ret = m_controller->fileConvert("in.mscz", out);

    // [THEN] The convert fails and no partial file is left
    EXPECT_EQ(ret.code(), static_cast<int>(Err::OutFileFailedWrite));
    EXPECT_FALSE(File::exists(out));
}

TEST_F(Converter_ConverterControllerTests, FileConvert_OutputFileCannotBeOpened)
{
    // [GIVEN] Output path in a directory that does not exist
    const path_t out = filePath("not_existing_dir/out.pdf");

    // [THEN] The writer is not called
    EXPECT_CALL(*m_writer, write(_, _, _)).Times(0);

    // [WHEN] Convert the file
    Ret ret = m_controller->fileConvert("in.mscz", out);

    // [THEN] The convert fails
    EXPECT_EQ(ret.code(), static_cast<int>(Err::OutFileFailedOpen));
}

TEST_F(Converter_ConverterControllerTests, FileConvert_UnknownSuffix)
{
    // [GIVEN] No writer for the suffix
    EXPECT_CALL(*m_writers, writer("unknown")).WillOnce(Return(nullptr));

    // [THEN] The project is not created
    EXPECT_CALL(*m_projectCreator, newProject(_)).Times(0);

    // [WHEN] Convert the file
    Ret ret = m_controller->fileConvert("in.mscz", filePath("out.unknown"));

    // [THEN] The convert fails
    EXPECT_EQ(ret.code(), static_cast<int>(Err::ConvertTypeUnknown));
}

TEST_F(Converter_ConverterControllerTests, FileConvert_LoadFails)
{
    // [GIVEN] The input file cannot be loaded
    const path_t out = filePath("out.pdf");

    EXPECT_CALL(*m_project, load(path_t("in.mscz"), _, _)).WillOnce(Return(make_ret(Ret::Code::UnknownError)));

    // [THEN] Nothing is written
    EXPECT_CALL(*m_writer, write(_, _, _)).Times(0);

    // [WHEN] Convert the file
    Ret ret = m_controller->fileConvert("in.mscz", out);

    // [THEN] The convert fails and no file is created
    EXPECT_EQ(ret.code(), static_cast<int>(Err::InFileFailedLoad));
    EXPECT_FALSE(File::exists(out));
}

TEST_F(Converter_ConverterControllerTests, FileConvert_ProjectIsSetToSessionDuringConvert)
{
    // [THEN] The project is set to the session before writing and reset after
    {
        InSequence seq;

        EXPECT_CALL(*m_projectSession, setProject(INotationProjectPtr(m_project)));
        EXPECT_CALL(*m_writer, write(_, _, _)).WillOnce(Return(make_ok()));
        EXPECT_CALL(*m_projectSession, setProject(INotationProjectPtr()));
    }

    // [WHEN] Convert the file
    Ret ret = m_controller->fileConvert("in.mscz", filePath("out.pdf"));

    EXPECT_TRUE(ret);
}

TEST_F(Converter_ConverterControllerTests, FileConvert_MsczIsSavedByProject)
{
    // [GIVEN] Output with the project's own format
    const path_t out = filePath("out.mscz");

    // [THEN] The project saves itself, the writer is not used
    EXPECT_CALL(*m_project, save(out, _, _)).WillOnce(Return(make_ok()));
    EXPECT_CALL(*m_writer, write(_, _, _)).Times(0);

    // [WHEN] Convert the file
    Ret ret = m_controller->fileConvert("in.mscz", out);

    EXPECT_TRUE(ret);
}

TEST_F(Converter_ConverterControllerTests, FileConvert_PageByPage)
{
    // [GIVEN] Notation with 2 pages and the page by page format
    setPageCount(2);

    const path_t out = filePath("out.png");

    EXPECT_CALL(*m_writer, write(_, _, _))
    .Times(2)
    .WillRepeatedly(Invoke([out](INotationPtr, IODevice& device, const Options& options) {
        EXPECT_EQ(device.meta("dir_path"), out.toStdString());
        return writeToDevice(device, "page " + std::to_string(optionInt(options, OptionKey::PAGE_NUMBER)));
    }));

    // [WHEN] Convert the file
    Ret ret = m_controller->fileConvert("in.mscz", out);

    // [THEN] Each page is written to its own file
    EXPECT_TRUE(ret);
    EXPECT_EQ(readFile(filePath("out-1.png")), "page 0");
    EXPECT_EQ(readFile(filePath("out-2.png")), "page 1");
    EXPECT_FALSE(File::exists(out));
}

TEST_F(Converter_ConverterControllerTests, FileConvert_PageTarget)
{
    // [GIVEN] Target page
    const path_t out = filePath("out.pdf");

    EXPECT_CALL(*m_writer, write(_, _, _))
    .WillOnce(Invoke([](INotationPtr, IODevice& device, const Options& options) {
        return writeToDevice(device, "page " + std::to_string(optionInt(options, OptionKey::PAGE_NUMBER)));
    }));

    // [WHEN] Convert the file
    Ret ret = m_controller->fileConvert("in.mscz", out, {}, {}, {}, {}, {}, ConvertTarget(page_num_t(1)));

    // [THEN] Only the target page is written to the output file
    EXPECT_TRUE(ret);
    EXPECT_EQ(readFile(out), "page 1");
}

TEST_F(Converter_ConverterControllerTests, FileConvert_Parts)
{
    // [GIVEN] Notation with 2 parts and output with the part name placeholder
    setExcerpts({ "A", "B" });

    EXPECT_CALL(*m_writer, write(_, _, _))
    .Times(2)
    .WillRepeatedly(Invoke([](INotationPtr notation, IODevice& device, const Options& options) {
        EXPECT_EQ(muse::value(options, OptionKey::UNIT_TYPE, Val()).toEnum<INotationWriter::UnitType>(),
                  INotationWriter::UnitType::PER_PART);
        return writeToDevice(device, notation->name().toStdString());
    }));

    // [WHEN] Convert the file
    Ret ret = m_controller->fileConvert("in.mscz", filePath("out-*.pdf"));

    // [THEN] Each part is written to the file with its name
    EXPECT_TRUE(ret);
    EXPECT_EQ(readFile(filePath("out-A.pdf")), "A");
    EXPECT_EQ(readFile(filePath("out-B.pdf")), "B");
}

TEST_F(Converter_ConverterControllerTests, FileConvert_Parts_WriterFails)
{
    // [GIVEN] Notation with 2 parts and writer that fails on the first one
    setExcerpts({ "A", "B" });

    EXPECT_CALL(*m_writer, write(_, _, _))
    .WillOnce(Return(make_ret(Ret::Code::UnknownError)));

    // [WHEN] Convert the file
    Ret ret = m_controller->fileConvert("in.mscz", filePath("out-*.mp3"));

    // [THEN] The convert stops with an error and no files are left
    EXPECT_EQ(ret.code(), static_cast<int>(Err::OutFileFailedWrite));
    EXPECT_FALSE(File::exists(filePath("out-A.mp3")));
    EXPECT_FALSE(File::exists(filePath("out-B.mp3")));
}

TEST_F(Converter_ConverterControllerTests, ExportScoreVideo_WriterWritesToFilePath_FileIsNotOverwritten)
{
    // [GIVEN] Writer that writes to the "file_path" itself, like the video writer
    const path_t out = filePath("out.mp4");

    EXPECT_CALL(*m_writer, write(_, _, _))
    .WillOnce(Invoke([](INotationPtr, IODevice& device, const Options& options) {
        EXPECT_FALSE(muse::value(options, OptionKey::WITH_AUDIO, Val(true)).toBool());
        return writeToFilePath(device, "data");
    }));

    // [WHEN] Export the video without audio
    Ret ret = m_controller->exportScoreVideo("in.mscz", out, {}, false);

    // [THEN] The written data is not overwritten by the empty device
    EXPECT_TRUE(ret);
    EXPECT_EQ(readFile(out), "data");
}
