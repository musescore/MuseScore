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

#include <map>
#include <set>

#include "project/internal/exportprojectscenario.h"

#include "engraving/compat/scoreaccess.h"

#include "global/tests/mocks/filesystemmock.h"
#include "interactive/tests/mocks/interactivemock.h"
#include "interactive/tests/mocks/platforminteractivemock.h"
#include "notation/tests/mocks/excerptnotationmock.h"
#include "notation/tests/mocks/masternotationmock.h"
#include "notation/tests/mocks/notationelementsmock.h"
#include "notation/tests/mocks/notationmock.h"
#include "notation/tests/mocks/notationpaintingmock.h"

#include "mocks/notationwritermock.h"
#include "mocks/notationwritersregistermock.h"
#include "mocks/projectsessionmock.h"
#include "utils/promisetest.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

using namespace muse;
using namespace mu::project;

namespace mu::project {
class ExportProjectScenarioTests : public PromiseTest
{
protected:
    //! A score or a part, with what the export reads from it and does to it
    struct Score {
        std::shared_ptr<notation::NotationMock> notation;
        std::shared_ptr<notation::NotationElementsMock> elements;
        std::shared_ptr<notation::NotationPaintingMock> painting;

        //! NOTE The export lays out the score, and counts its pages, so it has to be a real one
        engraving::MasterScore* msScore = nullptr;
        notation::PageList pages;

        std::shared_ptr<notation::ViewMode> viewMode = std::make_shared<notation::ViewMode>(notation::ViewMode::LINE);
    };

    using ScorePtr = std::shared_ptr<Score>;

    //! What the writer was asked to write, and where
    struct WrittenFile {
        io::path_t path;
        notation::INotationPtrList notations;
        INotationWriter::Options options;
        notation::ViewMode viewMode = notation::ViewMode::LINE;
    };

    //! NOTE Mirrors the button ids that exportprojectscenario.cpp assigns to its custom buttons.
    static constexpr int REPLACE_BTN = int(IInteractive::Button::CustomButton) + 1;
    static constexpr int REPLACE_ALL_BTN = int(IInteractive::Button::CustomButton) + 2;
    static constexpr int SKIP_BTN = int(IInteractive::Button::CustomButton) + 3;
    static constexpr int SKIP_ALL_BTN = int(IInteractive::Button::CustomButton) + 4;

    void SetUp() override
    {
        //! NOTE The file system is the global one: the scenario takes it from there, and so does the file stream it writes with
        m_fileSystem = std::make_shared<NiceMock<io::FileSystemMock> >();
        m_realFileSystem = modularity::globalIoc()->resolve<io::IFileSystem>("utests");
        modularity::globalIoc()->unregister<io::IFileSystem>("utests");
        modularity::globalIoc()->registerExport<io::IFileSystem>("utests", m_fileSystem);

        m_scenario = std::make_shared<ExportProjectScenario>(modularity::globalCtx());

        m_writers = std::make_shared<NiceMock<NotationWritersRegisterMock> >();
        m_interactive = std::make_shared<NiceMock<InteractiveMock> >();
        m_platformInteractive = std::make_shared<NiceMock<PlatformInteractiveMock> >();
        m_projectSession = std::make_shared<NiceMock<ProjectSessionMock> >();

        m_scenario->writers.set(m_writers);
        m_scenario->interactive.set(m_interactive);
        m_scenario->platformInteractive.set(m_platformInteractive);
        m_scenario->context.set(m_projectSession);

        m_writer = std::make_shared<NiceMock<NotationWriterMock> >();
        m_masterNotation = std::make_shared<NiceMock<notation::MasterNotationMock> >();

        // An opened score of one page, with no parts waiting to be initialized.
        m_main = makeScore("Score");
        ON_CALL(*m_projectSession, masterNotation()).WillByDefault(Return(m_masterNotation));
        ON_CALL(*m_masterNotation, notation()).WillByDefault(Return(m_main->notation));
        ON_CALL(*m_masterNotation, potentialExcerpts()).WillByDefault(ReturnRef(m_potentialExcerpts));

        // A writer for any format, which writes successfully and reports no progress, unless a test says otherwise.
        ON_CALL(*m_writers, writer(_)).WillByDefault(Return(m_writer));
        ON_CALL(*m_writer, supportsUnitType(_)).WillByDefault(Return(true));
        ON_CALL(*m_writer, progress()).WillByDefault(Return(nullptr));
        ON_CALL(*m_writer, writeSync(_, _, _))
        .WillByDefault([this](notation::INotationPtr notation, io::IODevice& device, const INotationWriter::Options& options) {
            return writeFile({ notation }, device, options);
        });
        ON_CALL(*m_writer, writeListSync(_, _, _))
        .WillByDefault([this](const notation::INotationPtrList& notations, io::IODevice& device, const INotationWriter::Options& options) {
            return writeFile(notations, device, options);
        });

        // Only the files a test puts there exist, and they can be removed.
        ON_CALL(*m_fileSystem, exists(_)).WillByDefault([this](const io::path_t& path) {
            return m_existingPaths.count(path) ? make_ok() : make_ret(Ret::Code::UnknownError);
        });
        ON_CALL(*m_fileSystem, remove(_, _)).WillByDefault(Return(make_ok()));

        // The files are written to memory, unless their directory is missing.
        ON_CALL(*m_fileSystem, openStream(_, _)).WillByDefault([this](const io::path_t& path, io::OpenMode) {
            if (m_missingDirs.count(io::dirpath(path))) {
                return RetVal<io::StreamId>(make_ret(Ret::Code::UnknownError));
            }

            io::StreamId id = io::StreamId(m_openedStreams.size());
            m_openedStreams.push_back(path);
            m_writtenData[path] = ByteArray();
            return RetVal<io::StreamId>::make_ok(id);
        });
        ON_CALL(*m_fileSystem, writeToStream(_, _, _)).WillByDefault([this](io::StreamId id, const ByteArray& data, uint64_t offset) {
            EXPECT_EQ(offset, io::STREAM_POS_CURRENT) << "the writers here only append";
            m_writtenData[m_openedStreams.at(id)].push_back(data);
            return make_ok();
        });
        ON_CALL(*m_fileSystem, closeStream(_)).WillByDefault(Return(make_ok()));

        ON_CALL(*m_interactive, buttonData(_)).WillByDefault([](IInteractive::Button btn) {
            return IInteractive::ButtonData(btn, "");
        });
    }

    void TearDown() override
    {
        // Let whatever the flow queued after its result run, so that nothing outlives the mocks
        drainDeferredCalls();

        for (const ScorePtr& score : m_scores) {
            // The pages are fake, the score only has to know how many there are
            score->msScore->pages().clear();
            delete score->msScore;

            release(score->notation);
            release(score->elements);
            release(score->painting);
        }

        modularity::globalIoc()->unregister<io::IFileSystem>("utests");
        if (m_realFileSystem) {
            modularity::globalIoc()->registerExport<io::IFileSystem>("utests", m_realFileSystem);
        }

        release(m_projectSession);
        release(m_masterNotation);
        release(m_writer);
    }

    Ret exportScores(const notation::INotationPtrList& notations, const io::path_t& destinationPath,
                     INotationWriter::UnitType unitType = INotationWriter::UnitType::PER_PART,
                     bool openDestinationFolderOnExport = false)
    {
        return await(m_scenario->exportScores(notations, destinationPath, unitType, openDestinationFolderOnExport));
    }

    ScorePtr makeScore(const QString& name, size_t pageCount = 1)
    {
        auto score = std::make_shared<Score>();
        score->notation = std::make_shared<NiceMock<notation::NotationMock> >();
        score->elements = std::make_shared<NiceMock<notation::NotationElementsMock> >();
        score->painting = std::make_shared<NiceMock<notation::NotationPaintingMock> >();

        score->msScore = engraving::compat::ScoreAccess::createMasterScore(modularity::globalCtx());
        score->msScore->setIsOpen(true); // already laid out
        for (size_t i = 0; i < pageCount; ++i) {
            score->msScore->pages().push_back(nullptr);
            score->pages.push_back(nullptr);
        }

        ON_CALL(*score->notation, name()).WillByDefault(Return(name));
        ON_CALL(*score->notation, elements()).WillByDefault(Return(score->elements));
        ON_CALL(*score->notation, painting()).WillByDefault(Return(score->painting));
        ON_CALL(*score->elements, msScore()).WillByDefault(Return(score->msScore));
        ON_CALL(*score->elements, pages()).WillByDefault(ReturnRef(score->pages));

        std::shared_ptr<notation::ViewMode> viewMode = score->viewMode;
        ON_CALL(*score->painting, viewMode()).WillByDefault([viewMode]() { return *viewMode; });
        ON_CALL(*score->painting, setViewMode(_)).WillByDefault([viewMode](const notation::ViewMode& vm) { *viewMode = vm; });

        m_scores.push_back(score);
        return score;
    }

    //! A part that has been opened before, so it is initialized already
    ScorePtr makePart(const QString& name, size_t pageCount = 1)
    {
        return makeScore(name, pageCount);
    }

    //! A part that has never been opened, so it is not initialized yet
    ScorePtr makePotentialPart(const QString& name, size_t pageCount = 1)
    {
        ScorePtr part = makeScore(name, pageCount);

        auto excerpt = std::make_shared<NiceMock<notation::ExcerptNotationMock> >();
        ON_CALL(*excerpt, notation()).WillByDefault(Return(part->notation));
        m_potentialExcerpts.push_back(excerpt);

        return part;
    }

    Ret writeFile(const notation::INotationPtrList& notations, io::IODevice& device, const INotationWriter::Options& options)
    {
        WrittenFile file;
        file.path = device.meta("file_path");
        file.notations = notations;
        file.options = options;
        file.viewMode = viewModeOf(notations.front());
        m_writtenFiles.push_back(file);

        device.write(ByteArray("data"));
        return make_ok();
    }

    notation::ViewMode viewModeOf(const notation::INotationPtr& notation) const
    {
        for (const ScorePtr& score : m_scores) {
            if (score->notation == notation) {
                return *score->viewMode;
            }
        }

        return notation::ViewMode::LINE;
    }

    io::path_t path(const QString& fileName) const
    {
        return io::path_t("/Exports/" + fileName);
    }

    std::vector<io::path_t> writtenPaths() const
    {
        std::vector<io::path_t> paths;
        for (const WrittenFile& file : m_writtenFiles) {
            paths.push_back(file.path);
        }

        return paths;
    }

    static std::vector<int> buttonIds(const IInteractive::ButtonDatas& buttons)
    {
        std::vector<int> ids;
        for (const IInteractive::ButtonData& button : buttons) {
            ids.push_back(button.btn);
        }

        return ids;
    }

    static ::testing::Matcher<const IInteractive::Text&> IsReplaceQuestion()
    {
        return ::testing::Truly([](const IInteractive::Text& text) {
            return text.text.find("already exists") != std::string::npos;
        });
    }

    static ::testing::Matcher<const IInteractive::Text&> IsRetryQuestion()
    {
        return ::testing::Truly([](const IInteractive::Text& text) {
            return text.text.find("error occurred") != std::string::npos;
        });
    }

    void givenFileExists(const io::path_t& path)
    {
        m_existingPaths.insert(path);
    }

    void givenDirectoryDoesNotExist(const io::path_t& dirPath)
    {
        m_missingDirs.insert(dirPath);
    }

    //! The user answers the question about replacing an existing file with `btn`.
    void givenReplaceQuestionAnsweredWith(int btn)
    {
        ON_CALL(*m_interactive, question(_, IsReplaceQuestion(), _, _, _, _)).WillByDefault([btn] { return dialogResult(btn); });
    }

    //! The user answers the question about retrying a failed write with `btn`.
    void givenRetryQuestionAnsweredWith(IInteractive::Button btn)
    {
        ON_CALL(*m_interactive, question(_, IsRetryQuestion(), _, _, _, _)).WillByDefault([btn] { return dialogResult(btn); });
    }

    //! Writing fails with `ret` every time.
    void givenWritingFailsWith(const Ret& ret)
    {
        ON_CALL(*m_writer, writeSync(_, _, _)).WillByDefault(Return(ret));
    }

    //! Writing fails with `ret` the first time, and succeeds afterwards.
    void givenWritingFailsOnceWith(const Ret& ret)
    {
        auto failed = std::make_shared<bool>(false);
        ON_CALL(*m_writer, writeSync(_, _, _))
        .WillByDefault([this, ret, failed](notation::INotationPtr notation, io::IODevice& device, const INotationWriter::Options& options) {
            if (!*failed) {
                *failed = true;
                return ret;
            }

            return writeFile({ notation }, device, options);
        });
    }

    //! The writing is cancelled (e.g. by the user in the progress dialog), after the file has been created.
    void givenWritingIsCancelledAfterCreatingTheFile()
    {
        ON_CALL(*m_writer, writeSync(_, _, _)).WillByDefault([this](notation::INotationPtr, io::IODevice& device,
                                                                    const INotationWriter::Options&) {
            givenFileExists(device.meta("file_path"));
            return make_ret(Ret::Code::Cancel);
        });
    }

    //! Removing the old file fails the first time, and succeeds afterwards.
    void givenRemovingFailsOnce()
    {
        auto failed = std::make_shared<bool>(false);
        ON_CALL(*m_fileSystem, remove(_, _)).WillByDefault([failed](const io::path_t&, bool) {
            if (!*failed) {
                *failed = true;
                return make_ret(Ret::Code::UnknownError);
            }

            return make_ok();
        });
    }

    //! The writer reports its progress to `progress`, and the progress shown to the user is handed to `shown`.
    void givenWriterReportsProgress(Progress* progress, Progress* shown)
    {
        ON_CALL(*m_writer, progress()).WillByDefault(Return(progress));
        ON_CALL(*m_interactive, showProgress(_, _)).WillByDefault([shown](const std::string&, Progress p) {
            *shown = p;
        });
    }

    std::shared_ptr<ExportProjectScenario> m_scenario;

    std::shared_ptr<io::FileSystemMock> m_fileSystem;
    std::shared_ptr<io::IFileSystem> m_realFileSystem;
    std::shared_ptr<NotationWritersRegisterMock> m_writers;
    std::shared_ptr<InteractiveMock> m_interactive;
    std::shared_ptr<PlatformInteractiveMock> m_platformInteractive;
    std::shared_ptr<ProjectSessionMock> m_projectSession;

    std::shared_ptr<NotationWriterMock> m_writer;
    std::shared_ptr<notation::MasterNotationMock> m_masterNotation;

    std::vector<ScorePtr> m_scores;
    ScorePtr m_main;
    notation::ExcerptNotationList m_potentialExcerpts;

    std::set<io::path_t> m_existingPaths;
    std::vector<WrittenFile> m_writtenFiles;

    std::set<io::path_t> m_missingDirs;
    std::vector<io::path_t> m_openedStreams;
    std::map<io::path_t, ByteArray> m_writtenData;
};

// ─── Which writer ────────────────────────────────────────────────────────────

TEST_F(ExportProjectScenarioTests, ExportScores_WriterIsChosenBySuffix)
{
    //! [THEN] The writer is looked up by the suffix of the destination
    EXPECT_CALL(*m_writers, writer("pdf")).WillOnce(Return(m_writer));

    //! [WHEN] Exporting to a pdf...
    Ret ret = exportScores({ m_main->notation }, path("Export.pdf"));

    //! [THEN] ...succeeds
    EXPECT_TRUE(ret);
}

TEST_F(ExportProjectScenarioTests, ExportScores_NoWriterForSuffix_WritesNothing)
{
    //! [GIVEN] There is no writer for the format
    ON_CALL(*m_writers, writer(_)).WillByDefault(Return(nullptr));

    //! [WHEN] Exporting to it...
    Ret ret = exportScores({ m_main->notation }, path("Export.xyz"));

    //! [THEN] ...fails, and nothing is written
    EXPECT_FALSE(ret);
    EXPECT_EQ(ret.code(), int(Ret::Code::NotSupported));
    EXPECT_TRUE(m_writtenFiles.empty());
}

// ─── Where the files go ──────────────────────────────────────────────────────

TEST_F(ExportProjectScenarioTests, PerPart_OneScore_WritesExactlyWhereTheUserSaid)
{
    //! [WHEN] Exporting the score alone...
    EXPECT_TRUE(exportScores({ m_main->notation }, path("Export.pdf")));

    //! [THEN] ...writes it to the chosen path, and tells the writer how
    ASSERT_EQ(m_writtenFiles.size(), 1);
    EXPECT_EQ(m_writtenFiles[0].path, path("Export.pdf"));
    EXPECT_EQ(m_writtenFiles[0].notations, notation::INotationPtrList { m_main->notation });
    EXPECT_EQ(m_writtenFiles[0].options.at(INotationWriter::OptionKey::UNIT_TYPE).toInt(), int(INotationWriter::UnitType::PER_PART));
    EXPECT_EQ(m_writtenFiles[0].options.count(INotationWriter::OptionKey::PAGE_NUMBER), 0);

    //! [THEN] What the writer wrote really ends up in the file
    EXPECT_EQ(m_writtenData[path("Export.pdf")], ByteArray("data"));
}

TEST_F(ExportProjectScenarioTests, PerPart_OnePart_WritesExactlyWhereTheUserSaid)
{
    //! [GIVEN] A part
    ScorePtr violin = makePart("Violin");

    //! [WHEN] Exporting the part alone...
    EXPECT_TRUE(exportScores({ violin->notation }, path("Export.pdf")));

    //! [THEN] ...does not add its name, the user has named the file already
    EXPECT_EQ(writtenPaths(), std::vector<io::path_t> { path("Export.pdf") });
}

TEST_F(ExportProjectScenarioTests, PerPart_ScoreAndParts_NamesThePartFilesAfterTheParts)
{
    //! [GIVEN] Two parts
    ScorePtr violin = makePart("Violin");
    ScorePtr flute = makePart("Flute");

    //! [WHEN] Exporting the score with the parts...
    EXPECT_TRUE(exportScores({ m_main->notation, violin->notation, flute->notation }, path("Export.pdf")));

    //! [THEN] ...writes the score to the chosen path, and each part next to it, under its name
    std::vector<io::path_t> expected = { path("Export.pdf"), path("Export-Violin.pdf"), path("Export-Flute.pdf") };
    EXPECT_EQ(writtenPaths(), expected);

    ASSERT_EQ(m_writtenFiles.size(), 3);
    EXPECT_EQ(m_writtenFiles[1].notations, notation::INotationPtrList { violin->notation });
    EXPECT_EQ(m_writtenFiles[2].notations, notation::INotationPtrList { flute->notation });
}

TEST_F(ExportProjectScenarioTests, PerPart_PartNameNotValidForAFile_IsEscaped)
{
    //! [GIVEN] A part whose name has characters a file name cannot have
    ScorePtr violin = makePart("Violin 1/2");

    //! [WHEN] Exporting the score with the part...
    EXPECT_TRUE(exportScores({ m_main->notation, violin->notation }, path("Export.pdf")));

    //! [THEN] ...escapes the name of the part
    ASSERT_EQ(m_writtenFiles.size(), 2);
    EXPECT_EQ(m_writtenFiles[1].path, path("Export-" + io::escapeFileName("Violin 1/2").toQString() + ".pdf"));
}

TEST_F(ExportProjectScenarioTests, PerPage_OnePage_WritesExactlyWhereTheUserSaid)
{
    //! [WHEN] Exporting a score of one page, page by page...
    EXPECT_TRUE(exportScores({ m_main->notation }, path("Export.png"), INotationWriter::UnitType::PER_PAGE));

    //! [THEN] ...writes the page to the chosen path
    ASSERT_EQ(m_writtenFiles.size(), 1);
    EXPECT_EQ(m_writtenFiles[0].path, path("Export.png"));
    EXPECT_EQ(m_writtenFiles[0].options.at(INotationWriter::OptionKey::UNIT_TYPE).toInt(), int(INotationWriter::UnitType::PER_PAGE));
    EXPECT_EQ(m_writtenFiles[0].options.at(INotationWriter::OptionKey::PAGE_NUMBER).toInt(), 0);
}

TEST_F(ExportProjectScenarioTests, PerPage_SeveralPages_NumbersThePageFiles)
{
    //! [GIVEN] A score of three pages
    ScorePtr score = makeScore("Score", 3);
    ON_CALL(*m_masterNotation, notation()).WillByDefault(Return(score->notation));

    //! [WHEN] Exporting it page by page...
    EXPECT_TRUE(exportScores({ score->notation }, path("Export.png"), INotationWriter::UnitType::PER_PAGE));

    //! [THEN] ...writes each page to a file of its own, numbered from one
    std::vector<io::path_t> expected = { path("Export-1.png"), path("Export-2.png"), path("Export-3.png") };
    EXPECT_EQ(writtenPaths(), expected);

    //! [THEN] The writer is asked for the pages one by one, numbered from zero
    ASSERT_EQ(m_writtenFiles.size(), 3);
    for (size_t i = 0; i < m_writtenFiles.size(); ++i) {
        EXPECT_EQ(m_writtenFiles[i].options.at(INotationWriter::OptionKey::PAGE_NUMBER).toInt(), int(i));
    }
}

TEST_F(ExportProjectScenarioTests, PerPage_ScoreAndParts_NumbersThePagesOfEachSeparately)
{
    //! [GIVEN] A part of two pages
    ScorePtr violin = makePart("Violin", 2);

    //! [WHEN] Exporting the score with the part, page by page...
    EXPECT_TRUE(exportScores({ m_main->notation, violin->notation }, path("Export.png"), INotationWriter::UnitType::PER_PAGE));

    //! [THEN] ...numbers the pages of the score and of the part separately
    std::vector<io::path_t> expected = { path("Export-1.png"), path("Export-Violin-1.png"), path("Export-Violin-2.png") };
    EXPECT_EQ(writtenPaths(), expected);
}

TEST_F(ExportProjectScenarioTests, PerPage_OnePartNotInitializedYet_NumbersItsPage)
{
    //! [GIVEN] A part that is not initialized yet, so how many pages it has is not known in advance
    ScorePtr violin = makePotentialPart("Violin", 1);

    //! [WHEN] Exporting it page by page...
    EXPECT_TRUE(exportScores({ violin->notation }, path("Export.png"), INotationWriter::UnitType::PER_PAGE));

    //! [THEN] ...numbers its page, the same way as when the path was suggested
    EXPECT_EQ(writtenPaths(), std::vector<io::path_t> { path("Export-1.png") });
}

TEST_F(ExportProjectScenarioTests, MultiPart_WritesAllTheScoresToOneFile)
{
    //! [GIVEN] Two parts
    ScorePtr violin = makePart("Violin");
    ScorePtr flute = makePart("Flute");
    notation::INotationPtrList notations = { m_main->notation, violin->notation, flute->notation };

    //! [THEN] They are written all together, not one by one
    EXPECT_CALL(*m_writer, writeSync(_, _, _)).Times(0);

    //! [WHEN] Exporting the score with the parts to one file...
    EXPECT_TRUE(exportScores(notations, path("Export.mxl"), INotationWriter::UnitType::MULTI_PART));

    //! [THEN] ...writes them to the chosen path
    ASSERT_EQ(m_writtenFiles.size(), 1);
    EXPECT_EQ(m_writtenFiles[0].path, path("Export.mxl"));
    EXPECT_EQ(m_writtenFiles[0].notations, notations);
    EXPECT_EQ(m_writtenFiles[0].options.at(INotationWriter::OptionKey::UNIT_TYPE).toInt(),
              int(INotationWriter::UnitType::MULTI_PART));
}

// ─── Files that are there already ────────────────────────────────────────────

TEST_F(ExportProjectScenarioTests, ExistingFile_OnlyOneFile_IsReplacedWithoutAsking)
{
    //! [GIVEN] The chosen file exists; the file dialog has asked about it already
    givenFileExists(path("Export.pdf"));

    //! [THEN] Nothing is asked again, the old file is removed, and the new one takes its place
    EXPECT_CALL(*m_interactive, question(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(*m_fileSystem, remove(path("Export.pdf"), _)).Times(1);

    //! [WHEN] Exporting...
    EXPECT_TRUE(exportScores({ m_main->notation }, path("Export.pdf")));

    EXPECT_EQ(writtenPaths(), std::vector<io::path_t> { path("Export.pdf") });
}

TEST_F(ExportProjectScenarioTests, ExistingFile_SeveralFiles_AsksOnlyAboutTheExistingOnes)
{
    //! [GIVEN] Of the three files, only the one of the first part exists, and the user agrees to replace it
    ScorePtr violin = makePart("Violin");
    ScorePtr flute = makePart("Flute");
    givenFileExists(path("Export-Violin.pdf"));
    givenReplaceQuestionAnsweredWith(REPLACE_BTN);

    //! [THEN] The user is asked once, about that file only
    EXPECT_CALL(*m_interactive, question(_, IsReplaceQuestion(), _, _, _, _))
    .WillOnce([](const std::string&, const IInteractive::Text& text, const IInteractive::ButtonDatas& buttons, int,
                 const IInteractive::Options&, const std::string&) {
        //! [THEN] The question names the file, and offers to replace or to skip it, or all of them
        EXPECT_NE(text.text.find("Export-Violin.pdf"), std::string::npos);
        EXPECT_EQ(buttonIds(buttons), (std::vector<int> { REPLACE_BTN, REPLACE_ALL_BTN, SKIP_BTN, SKIP_ALL_BTN }));

        return dialogResult(REPLACE_BTN);
    });

    //! [WHEN] Exporting the score with the parts...
    EXPECT_TRUE(exportScores({ m_main->notation, violin->notation, flute->notation }, path("Export.pdf")));

    //! [THEN] ...writes all of them
    std::vector<io::path_t> expected = { path("Export.pdf"), path("Export-Violin.pdf"), path("Export-Flute.pdf") };
    EXPECT_EQ(writtenPaths(), expected);
}

TEST_F(ExportProjectScenarioTests, ExistingFile_QuestionNotAnsweredYet_WaitsForTheAnswer)
{
    //! [GIVEN] The file of the score exists, and the user has not answered the question about it yet
    ScorePtr violin = makePart("Violin");
    givenFileExists(path("Export.pdf"));
    ON_CALL(*m_interactive, question(_, IsReplaceQuestion(), _, _, _, _)).WillByDefault([] {
        return async::make_promise<IInteractive::Result>([](auto, auto) {
            return async::Promise<IInteractive::Result>::dummy_result();
        });
    });

    //! [THEN] Nothing is written in the meantime, the files come in the chosen order
    EXPECT_CALL(*m_writer, writeSync(_, _, _)).Times(0);

    //! [WHEN] Exporting the score with the part...
    bool resolved = false;
    m_scenario->exportScores({ m_main->notation, violin->notation }, path("Export.pdf"))
    .onResolve(this, [&resolved](const Ret&) {
        resolved = true;
    });
    drainDeferredCalls();

    //! [THEN] ...has not finished, it is still waiting for the user
    EXPECT_FALSE(resolved);
}

TEST_F(ExportProjectScenarioTests, ExistingFile_Replace_AsksAgainAboutTheNextOne)
{
    //! [GIVEN] All the files exist, and the user agrees to replace each
    ScorePtr violin = makePart("Violin");
    givenFileExists(path("Export.pdf"));
    givenFileExists(path("Export-Violin.pdf"));
    givenReplaceQuestionAnsweredWith(REPLACE_BTN);

    //! [THEN] Replacing one file does not answer for the others
    EXPECT_CALL(*m_interactive, question(_, IsReplaceQuestion(), _, _, _, _)).Times(2);

    //! [WHEN] Exporting the score with the part...
    EXPECT_TRUE(exportScores({ m_main->notation, violin->notation }, path("Export.pdf")));

    //! [THEN] ...replaces both
    std::vector<io::path_t> expected = { path("Export.pdf"), path("Export-Violin.pdf") };
    EXPECT_EQ(writtenPaths(), expected);
}

TEST_F(ExportProjectScenarioTests, ExistingFile_Skip_LeavesItAndGoesOn)
{
    //! [GIVEN] The file of the score exists, and the user does not want to replace it
    ScorePtr violin = makePart("Violin");
    givenFileExists(path("Export.pdf"));
    givenReplaceQuestionAnsweredWith(SKIP_BTN);

    //! [THEN] The existing file is left alone
    EXPECT_CALL(*m_fileSystem, remove(_, _)).Times(AnyNumber());
    EXPECT_CALL(*m_fileSystem, remove(path("Export.pdf"), _)).Times(0);

    //! [WHEN] Exporting the score with the part...
    Ret ret = exportScores({ m_main->notation, violin->notation }, path("Export.pdf"));

    //! [THEN] ...goes on, and writes the part only
    EXPECT_TRUE(ret);
    EXPECT_EQ(writtenPaths(), std::vector<io::path_t> { path("Export-Violin.pdf") });
}

TEST_F(ExportProjectScenarioTests, ExistingFile_QuestionDismissed_IsSkipped)
{
    //! [GIVEN] The file of the score exists, and the user closes the question without choosing
    ScorePtr violin = makePart("Violin");
    givenFileExists(path("Export.pdf"));
    givenReplaceQuestionAnsweredWith(int(IInteractive::Button::Cancel));

    //! [WHEN] Exporting the score with the part...
    Ret ret = exportScores({ m_main->notation, violin->notation }, path("Export.pdf"));

    //! [THEN] ...skips the file
    EXPECT_TRUE(ret);
    EXPECT_EQ(writtenPaths(), std::vector<io::path_t> { path("Export-Violin.pdf") });
}

TEST_F(ExportProjectScenarioTests, ExistingFile_ReplaceAll_ReplacesTheRestWithoutAsking)
{
    //! [GIVEN] All the files exist, and the user wants to replace all of them
    ScorePtr violin = makePart("Violin");
    ScorePtr flute = makePart("Flute");
    givenFileExists(path("Export.pdf"));
    givenFileExists(path("Export-Violin.pdf"));
    givenFileExists(path("Export-Flute.pdf"));
    givenReplaceQuestionAnsweredWith(REPLACE_ALL_BTN);

    //! [THEN] The user is asked once
    EXPECT_CALL(*m_interactive, question(_, IsReplaceQuestion(), _, _, _, _)).Times(1);

    //! [WHEN] Exporting the score with the parts...
    EXPECT_TRUE(exportScores({ m_main->notation, violin->notation, flute->notation }, path("Export.pdf")));

    //! [THEN] ...replaces all of them
    std::vector<io::path_t> expected = { path("Export.pdf"), path("Export-Violin.pdf"), path("Export-Flute.pdf") };
    EXPECT_EQ(writtenPaths(), expected);
}

TEST_F(ExportProjectScenarioTests, ExistingFile_SkipAll_SkipsTheRestWithoutAsking)
{
    //! [GIVEN] The files of the score and of the second part exist, and the user wants to skip all of them
    ScorePtr violin = makePart("Violin");
    ScorePtr flute = makePart("Flute");
    givenFileExists(path("Export.pdf"));
    givenFileExists(path("Export-Flute.pdf"));
    givenReplaceQuestionAnsweredWith(SKIP_ALL_BTN);

    //! [THEN] The user is asked once
    EXPECT_CALL(*m_interactive, question(_, IsReplaceQuestion(), _, _, _, _)).Times(1);

    //! [WHEN] Exporting the score with the parts...
    Ret ret = exportScores({ m_main->notation, violin->notation, flute->notation }, path("Export.pdf"));

    //! [THEN] ...writes the new file only
    EXPECT_TRUE(ret);
    EXPECT_EQ(writtenPaths(), std::vector<io::path_t> { path("Export-Violin.pdf") });
}

TEST_F(ExportProjectScenarioTests, ExistingFile_TheAnswerDoesNotOutliveTheExport)
{
    //! [GIVEN] The file of the score exists, and the user wants to skip all
    ScorePtr violin = makePart("Violin");
    givenFileExists(path("Export.pdf"));
    givenReplaceQuestionAnsweredWith(SKIP_ALL_BTN);

    //! [THEN] The user is asked again by the next export
    EXPECT_CALL(*m_interactive, question(_, IsReplaceQuestion(), _, _, _, _)).Times(2);

    //! [WHEN] Exporting twice...
    EXPECT_TRUE(exportScores({ m_main->notation, violin->notation }, path("Export.pdf")));
    EXPECT_TRUE(exportScores({ m_main->notation, violin->notation }, path("Export.pdf")));
}

// ─── When writing fails ──────────────────────────────────────────────────────

TEST_F(ExportProjectScenarioTests, WriteFails_Retry_WritesAgain)
{
    //! [GIVEN] Writing fails the first time, and the user wants to retry
    givenWritingFailsOnceWith(make_ret(Ret::Code::UnknownError));
    givenRetryQuestionAnsweredWith(IInteractive::Button::Retry);

    //! [THEN] The user is asked once
    EXPECT_CALL(*m_interactive, question(_, IsRetryQuestion(), _, _, _, _))
    .WillOnce([](const std::string&, const IInteractive::Text& text, const IInteractive::ButtonDatas& buttons, int,
                 const IInteractive::Options&, const std::string&) {
        //! [THEN] The question names the file, and offers to retry or to abort
        EXPECT_NE(text.text.find("Export.pdf"), std::string::npos);
        EXPECT_EQ(buttonIds(buttons), (std::vector<int> { int(IInteractive::Button::Retry), int(IInteractive::Button::Abort) }));

        return dialogResult(IInteractive::Button::Retry);
    });

    //! [WHEN] Exporting...
    Ret ret = exportScores({ m_main->notation }, path("Export.pdf"));

    //! [THEN] ...writes the file the second time
    EXPECT_TRUE(ret);
    EXPECT_EQ(writtenPaths(), std::vector<io::path_t> { path("Export.pdf") });
}

TEST_F(ExportProjectScenarioTests, WriteFails_RetryAReplacedFile_DoesNotAskAboutReplacingAgain)
{
    //! [GIVEN] The file of the score exists, and the user agrees to replace it
    ScorePtr violin = makePart("Violin");
    givenFileExists(path("Export.pdf"));
    givenReplaceQuestionAnsweredWith(REPLACE_BTN);

    //! [GIVEN] Writing it fails the first time, and the user wants to retry
    givenWritingFailsOnceWith(make_ret(Ret::Code::UnknownError));
    givenRetryQuestionAnsweredWith(IInteractive::Button::Retry);

    //! [THEN] The user is asked about replacing it once
    EXPECT_CALL(*m_interactive, question(_, IsReplaceQuestion(), _, _, _, _)).Times(1);
    EXPECT_CALL(*m_interactive, question(_, IsRetryQuestion(), _, _, _, _)).Times(1);

    //! [WHEN] Exporting the score with the part...
    EXPECT_TRUE(exportScores({ m_main->notation, violin->notation }, path("Export.pdf")));

    //! [THEN] ...writes both
    std::vector<io::path_t> expected = { path("Export.pdf"), path("Export-Violin.pdf") };
    EXPECT_EQ(writtenPaths(), expected);
}

TEST_F(ExportProjectScenarioTests, WriteFails_Abort_StopsTheWholeExport)
{
    //! [GIVEN] Writing fails, and the user does not want to retry
    ScorePtr violin = makePart("Violin");
    givenWritingFailsWith(make_ret(Ret::Code::UnknownError));
    givenRetryQuestionAnsweredWith(IInteractive::Button::Abort);

    //! [THEN] The part is not even tried
    EXPECT_CALL(*m_writer, writeSync(_, _, _)).Times(1);

    //! [WHEN] Exporting the score with the part...
    Ret ret = exportScores({ m_main->notation, violin->notation }, path("Export.pdf"));

    //! [THEN] ...is cancelled
    EXPECT_FALSE(ret);
    EXPECT_EQ(ret.code(), int(Ret::Code::Cancel));
}

TEST_F(ExportProjectScenarioTests, RemoveFails_Retry_RemovesAgain)
{
    //! [GIVEN] The old file cannot be removed the first time, and the user wants to retry
    givenRemovingFailsOnce();
    givenRetryQuestionAnsweredWith(IInteractive::Button::Retry);

    //! [THEN] The user is asked once
    EXPECT_CALL(*m_interactive, question(_, IsRetryQuestion(), _, _, _, _)).Times(1);

    //! [WHEN] Exporting...
    Ret ret = exportScores({ m_main->notation }, path("Export.pdf"));

    //! [THEN] ...writes the file after the retry
    EXPECT_TRUE(ret);
    EXPECT_EQ(writtenPaths(), std::vector<io::path_t> { path("Export.pdf") });
}

TEST_F(ExportProjectScenarioTests, RemoveFails_Abort_WritesNothing)
{
    //! [GIVEN] The old file cannot be removed, and the user does not want to retry
    ON_CALL(*m_fileSystem, remove(_, _)).WillByDefault(Return(make_ret(Ret::Code::UnknownError)));
    givenRetryQuestionAnsweredWith(IInteractive::Button::Abort);

    //! [THEN] Nothing is written over it
    EXPECT_CALL(*m_writer, writeSync(_, _, _)).Times(0);

    //! [WHEN] Exporting...
    Ret ret = exportScores({ m_main->notation }, path("Export.pdf"));

    //! [THEN] ...is cancelled
    EXPECT_FALSE(ret);
    EXPECT_EQ(ret.code(), int(Ret::Code::Cancel));
}

TEST_F(ExportProjectScenarioTests, FileCannotBeOpened_Retry_OpensAgain)
{
    //! [GIVEN] The chosen directory does not exist
    givenDirectoryDoesNotExist(path("NotYet"));
    const io::path_t filePath = path("NotYet/Export.pdf");

    //! [GIVEN] The user creates it, and retries
    ON_CALL(*m_interactive, question(_, IsRetryQuestion(), _, _, _, _))
    .WillByDefault([this](const std::string&, const IInteractive::Text&, const IInteractive::ButtonDatas&, int,
                          const IInteractive::Options&, const std::string&) {
        m_missingDirs.erase(path("NotYet"));
        return dialogResult(IInteractive::Button::Retry);
    });

    //! [THEN] The user is asked once
    EXPECT_CALL(*m_interactive, question(_, IsRetryQuestion(), _, _, _, _)).Times(1);

    //! [WHEN] Exporting there...
    Ret ret = exportScores({ m_main->notation }, filePath);

    //! [THEN] ...writes the file after the retry
    EXPECT_TRUE(ret);
    EXPECT_EQ(writtenPaths(), std::vector<io::path_t> { filePath });
}

TEST_F(ExportProjectScenarioTests, FileCannotBeOpened_Abort_WritesNothing)
{
    //! [GIVEN] The chosen directory does not exist, and the user does not want to retry
    givenDirectoryDoesNotExist(path("NotExisting"));
    givenRetryQuestionAnsweredWith(IInteractive::Button::Abort);

    //! [THEN] The writer is not called
    EXPECT_CALL(*m_writer, writeSync(_, _, _)).Times(0);

    //! [WHEN] Exporting there...
    Ret ret = exportScores({ m_main->notation }, path("NotExisting/Export.pdf"));

    //! [THEN] ...is cancelled
    EXPECT_FALSE(ret);
    EXPECT_EQ(ret.code(), int(Ret::Code::Cancel));
}

// ─── When writing is cancelled ───────────────────────────────────────────────

TEST_F(ExportProjectScenarioTests, WriteCancelled_StopsTheWholeExportWithoutAsking)
{
    //! [GIVEN] The writing is cancelled (e.g. by the user in the progress dialog)
    ScorePtr violin = makePart("Violin");
    givenWritingFailsWith(make_ret(Ret::Code::Cancel));

    //! [THEN] Nothing is asked, and the part is not even tried
    EXPECT_CALL(*m_interactive, question(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(*m_writer, writeSync(_, _, _)).Times(1);

    //! [WHEN] Exporting the score with the part...
    Ret ret = exportScores({ m_main->notation, violin->notation }, path("Export.pdf"));

    //! [THEN] ...is cancelled
    EXPECT_FALSE(ret);
    EXPECT_EQ(ret.code(), int(Ret::Code::Cancel));
}

TEST_F(ExportProjectScenarioTests, WriteCancelled_RemovesThePartlyWrittenFile)
{
    //! [GIVEN] The writing is cancelled after the file has been created
    givenWritingIsCancelledAfterCreatingTheFile();

    //! [THEN] The file is removed before writing, and what has been written of it after the cancellation
    EXPECT_CALL(*m_fileSystem, remove(path("Export.pdf"), _)).Times(2);

    //! [WHEN] Exporting...
    EXPECT_FALSE(exportScores({ m_main->notation }, path("Export.pdf")));
}

TEST_F(ExportProjectScenarioTests, WriteCancelled_PartlyWrittenFileIsLocked_RemovingIsRetried)
{
    //! [GIVEN] The writing is cancelled after the file has been created
    givenWritingIsCancelledAfterCreatingTheFile();

    //! [THEN] Removing what has been written is retried while the file is still locked (e.g. on Windows)
    EXPECT_CALL(*m_fileSystem, remove(path("Export.pdf"), _))
    .WillOnce(Return(make_ok()))                          // before writing
    .WillOnce(Return(make_ret(Ret::Code::UnknownError)))  // still locked
    .WillOnce(Return(make_ok()));

    //! [WHEN] Exporting...
    EXPECT_FALSE(exportScores({ m_main->notation }, path("Export.pdf")));
}

TEST_F(ExportProjectScenarioTests, WriteCancelled_NoFileAtThePath_RemovesNothingAfterwards)
{
    //! [GIVEN] The writing is cancelled, and there is no file at the path (e.g. the writer writes a directory)
    givenWritingFailsWith(make_ret(Ret::Code::Cancel));

    //! [THEN] The only removal is the one before writing
    EXPECT_CALL(*m_fileSystem, remove(path("Export.pdf"), _)).Times(1);

    //! [WHEN] Exporting...
    EXPECT_FALSE(exportScores({ m_main->notation }, path("Export.pdf")));
}

// ─── After the export ────────────────────────────────────────────────────────

TEST_F(ExportProjectScenarioTests, OpenDestinationFolder_Asked_IsOpened)
{
    //! [THEN] The destination is shown in the file browser
    EXPECT_CALL(*m_platformInteractive, revealInFileBrowser(path("Export.pdf"))).Times(1);

    //! [WHEN] Exporting, with the folder to be opened...
    EXPECT_TRUE(exportScores({ m_main->notation }, path("Export.pdf"), INotationWriter::UnitType::PER_PART, true));
}

TEST_F(ExportProjectScenarioTests, OpenDestinationFolder_NotAsked_IsNotOpened)
{
    //! [THEN] Nothing is shown
    EXPECT_CALL(*m_platformInteractive, revealInFileBrowser(_)).Times(0);

    //! [WHEN] Exporting...
    EXPECT_TRUE(exportScores({ m_main->notation }, path("Export.pdf")));
}

TEST_F(ExportProjectScenarioTests, OpenDestinationFolder_ExportCancelled_IsNotOpened)
{
    //! [GIVEN] The writing is cancelled
    givenWritingFailsWith(make_ret(Ret::Code::Cancel));

    //! [THEN] Nothing is shown
    EXPECT_CALL(*m_platformInteractive, revealInFileBrowser(_)).Times(0);

    //! [WHEN] Exporting, with the folder to be opened...
    EXPECT_FALSE(exportScores({ m_main->notation }, path("Export.pdf"), INotationWriter::UnitType::PER_PART, true));
}

// ─── View modes ──────────────────────────────────────────────────────────────

TEST_F(ExportProjectScenarioTests, ViewModes_ArePageWhileWriting)
{
    //! [GIVEN] The score and the part are shown in other view modes
    ScorePtr violin = makePart("Violin");
    *m_main->viewMode = notation::ViewMode::LINE;
    *violin->viewMode = notation::ViewMode::FLOAT;

    //! [WHEN] Exporting the score with the part...
    EXPECT_TRUE(exportScores({ m_main->notation, violin->notation }, path("Export.pdf")));

    //! [THEN] ...writes them laid out in pages
    ASSERT_EQ(m_writtenFiles.size(), 2);
    EXPECT_EQ(m_writtenFiles[0].viewMode, notation::ViewMode::PAGE);
    EXPECT_EQ(m_writtenFiles[1].viewMode, notation::ViewMode::PAGE);
}

TEST_F(ExportProjectScenarioTests, ViewModes_AreRestoredAfterwards)
{
    //! [GIVEN] The score and the part are shown in other view modes
    ScorePtr violin = makePart("Violin");
    *m_main->viewMode = notation::ViewMode::LINE;
    *violin->viewMode = notation::ViewMode::FLOAT;

    //! [WHEN] Exporting the score with the part...
    EXPECT_TRUE(exportScores({ m_main->notation, violin->notation }, path("Export.pdf")));

    //! [THEN] ...gives them their view modes back
    EXPECT_EQ(*m_main->viewMode, notation::ViewMode::LINE);
    EXPECT_EQ(*violin->viewMode, notation::ViewMode::FLOAT);
}

TEST_F(ExportProjectScenarioTests, ViewModes_ExportCancelled_AreRestoredAnyway)
{
    //! [GIVEN] The score is shown continuously, and the writing is cancelled
    *m_main->viewMode = notation::ViewMode::LINE;
    givenWritingFailsWith(make_ret(Ret::Code::Cancel));

    //! [WHEN] Exporting...
    EXPECT_FALSE(exportScores({ m_main->notation }, path("Export.pdf")));

    //! [THEN] ...gives it its view mode back
    EXPECT_EQ(*m_main->viewMode, notation::ViewMode::LINE);
}

// ─── Several exports at once ─────────────────────────────────────────────────

TEST_F(ExportProjectScenarioTests, SeveralExports_GoOneAfterAnother)
{
    //! [GIVEN] The score is shown continuously
    *m_main->viewMode = notation::ViewMode::LINE;

    //! [WHEN] Exporting twice, before the first export is over (e.g. by a plugin)...
    Ret first;
    Ret second;
    bool firstResolved = false;
    bool secondResolved = false;

    m_scenario->exportScores({ m_main->notation }, path("Export.pdf"), INotationWriter::UnitType::PER_PART, false)
    .onResolve(this, [&first, &firstResolved](const Ret& ret) {
        first = ret;
        firstResolved = true;
    });

    m_scenario->exportScores({ m_main->notation }, path("Export.png"), INotationWriter::UnitType::PER_PAGE, false)
    .onResolve(this, [&second, &secondResolved](const Ret& ret) {
        second = ret;
        secondResolved = true;
    });

    drainDeferredCalls();

    //! [THEN] ...both succeed
    EXPECT_TRUE(firstResolved);
    EXPECT_TRUE(secondResolved);
    EXPECT_TRUE(first);
    EXPECT_TRUE(second);

    //! [THEN] The second one is written after the first one, both in the page view mode
    ASSERT_EQ(writtenPaths(), (std::vector<io::path_t> { path("Export.pdf"), path("Export.png") }));
    EXPECT_EQ(m_writtenFiles[0].viewMode, notation::ViewMode::PAGE);
    EXPECT_EQ(m_writtenFiles[1].viewMode, notation::ViewMode::PAGE);

    //! [THEN] The score is given its own view mode back, not the one of the first export
    EXPECT_EQ(*m_main->viewMode, notation::ViewMode::LINE);
}

TEST_F(ExportProjectScenarioTests, SeveralExports_TheFirstFails_TheNextOneGoesOn)
{
    //! [GIVEN] There is no writer for the first format
    ON_CALL(*m_writers, writer("xyz")).WillByDefault(Return(nullptr));

    //! [WHEN] Exporting to it, and then to a pdf, before the first export is over...
    Ret first;
    Ret second;

    m_scenario->exportScores({ m_main->notation }, path("Export.xyz"), INotationWriter::UnitType::PER_PART, false)
    .onResolve(this, [&first](const Ret& ret) {
        first = ret;
    });

    m_scenario->exportScores({ m_main->notation }, path("Export.pdf"), INotationWriter::UnitType::PER_PART, false)
    .onResolve(this, [&second](const Ret& ret) {
        second = ret;
    });

    drainDeferredCalls();

    //! [THEN] ...the first one fails, and the second one is written anyway
    EXPECT_EQ(first.code(), int(Ret::Code::NotSupported));
    EXPECT_TRUE(second);
    EXPECT_EQ(writtenPaths(), std::vector<io::path_t> { path("Export.pdf") });
}

// ─── Parts not initialized yet ───────────────────────────────────────────────

TEST_F(ExportProjectScenarioTests, PotentialParts_OnlyTheExportedOnesAreInitialized)
{
    //! [GIVEN] Two parts that are not initialized yet
    ScorePtr violin = makePotentialPart("Violin");
    ScorePtr flute = makePotentialPart("Flute");

    //! [THEN] Only the exported one is initialized, the score is not changed without need
    EXPECT_CALL(*m_masterNotation, initExcerpts(notation::ExcerptNotationList { m_potentialExcerpts[0] })).Times(1);

    //! [WHEN] Exporting the score with the first part...
    EXPECT_TRUE(exportScores({ m_main->notation, violin->notation }, path("Export.pdf")));
}

TEST_F(ExportProjectScenarioTests, PotentialParts_NoneExported_NoneIsInitialized)
{
    //! [GIVEN] A part that is not initialized yet
    makePotentialPart("Violin");

    //! [THEN] It is left as it is
    EXPECT_CALL(*m_masterNotation, initExcerpts(notation::ExcerptNotationList {})).Times(1);

    //! [WHEN] Exporting the score alone...
    EXPECT_TRUE(exportScores({ m_main->notation }, path("Export.pdf")));
}

// ─── Progress ────────────────────────────────────────────────────────────────

TEST_F(ExportProjectScenarioTests, Progress_WriterReportsNone_NoneIsShown)
{
    //! [THEN] No progress is shown
    EXPECT_CALL(*m_interactive, showProgress(_, _)).Times(0);

    //! [WHEN] Exporting with a writer that reports no progress...
    EXPECT_TRUE(exportScores({ m_main->notation }, path("Export.pdf")));
}

TEST_F(ExportProjectScenarioTests, Progress_AudioExport_IsShownAsAudioExport)
{
    //! [GIVEN] A writer that reports its progress
    Progress writerProgress;
    Progress shownProgress;
    givenWriterReportsProgress(&writerProgress, &shownProgress);

    //! [THEN] The progress of exporting audio is shown
    EXPECT_CALL(*m_interactive, showProgress(::testing::HasSubstr("audio"), _)).Times(1);

    //! [WHEN] Exporting audio...
    EXPECT_TRUE(exportScores({ m_main->notation }, path("Export.mp3")));
}

TEST_F(ExportProjectScenarioTests, Progress_SeveralFiles_AddsUpToOne)
{
    //! [GIVEN] A writer that reports its progress
    ScorePtr violin = makePart("Violin");
    Progress writerProgress;
    Progress shownProgress;
    givenWriterReportsProgress(&writerProgress, &shownProgress);

    //! [GIVEN] The progress shown to the user is watched from the start of the writing
    std::vector<std::pair<int64_t, int64_t> > reported;
    bool finished = false;

    //! [GIVEN] The first file is reported half written, and the second one 60 % written
    int64_t current = 50;
    ON_CALL(*m_writer, writeSync(_, _, _))
    .WillByDefault([&](notation::INotationPtr notation, io::IODevice& device, const INotationWriter::Options& options) {
        if (current == 50) {
            shownProgress.progressChanged().onReceive(this, [&reported](int64_t cur, int64_t total, const std::string&) {
                reported.push_back({ cur, total });
            });
            shownProgress.finished().onReceive(this, [&finished](const ProgressResult&) {
                finished = true;
            });
        }

        writerProgress.progress(current, 100);
        current += 10;
        return writeFile({ notation }, device, options);
    });

    //! [WHEN] Exporting the score with the part as audio...
    EXPECT_TRUE(exportScores({ m_main->notation, violin->notation }, path("Export.mp3")));

    //! [THEN] ...reports the progress of each file as a share of the whole
    std::vector<std::pair<int64_t, int64_t> > expected = { { 50, 200 }, { 160, 200 } };
    EXPECT_EQ(reported, expected);

    //! [THEN] ...and finishes the progress
    EXPECT_TRUE(finished);
}

TEST_F(ExportProjectScenarioTests, Progress_CancelledBetweenFiles_DoesNotWriteTheNextFile)
{
    //! [GIVEN] A writer that reports its progress, and two files to write
    ScorePtr violin = makePart("Violin");
    Progress writerProgress;
    Progress shownProgress;
    givenWriterReportsProgress(&writerProgress, &shownProgress);

    //! [GIVEN] The user cancels the progress when the first file is written, and there is nothing to abort
    ON_CALL(*m_writer, writeSync(_, _, _)).WillByDefault([&shownProgress](notation::INotationPtr, io::IODevice&,
                                                                          const INotationWriter::Options&) {
        shownProgress.cancel();
        return make_ok();
    });

    //! [THEN] The second file is not written
    EXPECT_CALL(*m_writer, writeSync(_, _, _)).Times(1);

    //! [WHEN] Exporting the score with the part as audio...
    Ret ret = exportScores({ m_main->notation, violin->notation }, path("Export.mp3"));

    //! [THEN] ...is cancelled
    EXPECT_FALSE(ret);
    EXPECT_EQ(ret.code(), int(Ret::Code::Cancel));
}

TEST_F(ExportProjectScenarioTests, Progress_CancelledByTheUser_AbortsTheWriter)
{
    //! [GIVEN] A writer that reports its progress
    Progress writerProgress;
    Progress shownProgress;
    givenWriterReportsProgress(&writerProgress, &shownProgress);

    //! [GIVEN] The user cancels the progress while the file is being written
    ON_CALL(*m_writer, writeSync(_, _, _)).WillByDefault([&shownProgress](notation::INotationPtr, io::IODevice&,
                                                                          const INotationWriter::Options&) {
        shownProgress.cancel();
        return make_ret(Ret::Code::Cancel);
    });

    //! [THEN] The writer is told to stop
    EXPECT_CALL(*m_writer, abort()).Times(1);

    //! [WHEN] Exporting audio...
    EXPECT_FALSE(exportScores({ m_main->notation }, path("Export.mp3")));
}
}
