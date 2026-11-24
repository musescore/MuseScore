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

#include <memory>
#include <optional>

#include "global/io/buffer.h"
#include "global/tests/mocks/applicationmock.h"
#include "audio/tests/mocks/startaudiocontrollermock.h"

#include "notation/tests/mocks/notationmock.h"
#include "playback/tests/mocks/playbackcontrollermock.h"
#include "playback/tests/mocks/playbackmock.h"
#include "project/tests/mocks/projectsessionmock.h"
#include "project/tests/utils/promisetest.h"

#include "internal/abstractaudiowriter.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

using namespace muse;
using namespace muse::audio;
using namespace mu::iex::audioexport;
using namespace mu::project;

namespace mu::iex::audioexport {
class TestAudioWriter : public AbstractAudioWriter
{
public:
    using AbstractAudioWriter::AbstractAudioWriter;

protected:
    SoundTrackFormat soundTrackFormat() const override
    {
        SoundTrackFormat format;
        format.type = SoundTrackType::WAV;
        format.bitRate = 128;
        return format;
    }
};

class AbstractAudioWriterTests : public PromiseTest
{
protected:
    //! What the writer reports back
    struct Outcome {
        bool isFinished = false;
        Ret ret;
    };

    void SetUp() override
    {
        //! NOTE The writer takes the playback, the playback controller and the session from the context of the notation
        m_ctx = std::make_shared<modularity::Context>(static_cast<modularity::IoCID>(1));

        m_startAudioController = std::make_shared<NiceMock<StartAudioControllerMock> >();
        m_application = std::make_shared<NiceMock<ApplicationMock> >();
        m_playback = std::make_shared<NiceMock<PlaybackMock> >();

        //! NOTE The writer keeps the playback in the callbacks of the saving promise. Whichever of onResolve and onReject
        //! is not called, it holds the promise data (and what the callback captured), so the playback is never deleted
        testing::Mock::AllowLeak(m_playback.get());
        m_playbackController = std::make_shared<NiceMock<playback::PlaybackControllerMock> >();
        m_projectSession = std::make_shared<NiceMock<ProjectSessionMock> >();

        modularity::ioc(m_ctx)->registerExport<IPlayback>("utests", m_playback);
        modularity::ioc(m_ctx)->registerExport<playback::IPlaybackController>("utests", m_playbackController);
        modularity::ioc(m_ctx)->registerExport<IProjectSession>("utests", m_projectSession);

        m_notation = std::make_shared<NiceMock<notation::NotationMock> >();
        m_notationToRestore = std::make_shared<NiceMock<notation::NotationMock> >();
        ON_CALL(*m_notation, iocContext()).WillByDefault(ReturnRef(m_ctx));
        ON_CALL(*m_projectSession, currentNotation()).WillByDefault(Return(m_notationToRestore));

        // The audio system and the playback are ready, unless a test says otherwise
        ON_CALL(*m_startAudioController, isAudioStarted()).WillByDefault([this]() { return m_isAudioStarted; });
        ON_CALL(*m_startAudioController, isAudioStartedChanged()).WillByDefault([this]() { return m_audioStartedChanged; });
        ON_CALL(*m_playbackController, isPlaybackInited()).WillByDefault([this]() { return m_isPlaybackInited; });
        ON_CALL(*m_playbackController, playbackInitedChanged()).WillByDefault([this]() { return m_playbackInitedChanged; });

        ON_CALL(*m_playback, saveSoundTrackProgressChanged()).WillByDefault([this]() { return m_saveProgress; });

        // The sound track is saved when the test says so
        ON_CALL(*m_playback, saveSoundTrack(_, _)).WillByDefault([this](const SoundTrackFormat& format, io::IODevice&) {
            m_savedFormat = format;
            return async::make_promise<bool>([this](auto resolve, auto reject) {
                m_resolveSaving = resolve;
                m_rejectSaving = reject;
                return async::Promise<bool>::Result::unchecked();
            }, async::PromiseType::AsyncByBody);
        });

        m_writer = std::make_shared<TestAudioWriter>(m_ctx);
        m_writer->application.set(m_application);
        m_writer->startAudioController.set(m_startAudioController);

        m_buffer.open(io::IODevice::WriteOnly);
    }

    void TearDown() override
    {
        //! NOTE A promise that is not resolved keeps what its callbacks hold (e.g. the notation) forever,
        //! so the writing, which a test left unfinished, is finished here.
        //! What the test expected has been checked by now.
        testing::Mock::VerifyAndClearExpectations(m_playback.get());
        testing::Mock::VerifyAndClearExpectations(m_playbackController.get());

        m_isAudioStarted = true;
        m_audioStartedChanged.send(true);
        m_isPlaybackInited = true;
        m_playbackInitedChanged.send(true);
        drainDeferredCalls();

        if (m_resolveSaving && !m_isSavingDone) {
            (void)(*m_resolveSaving)(true);
            drainDeferredCalls();
        }

        m_resolveSaving.reset();
        m_rejectSaving.reset();
        m_writer.reset();

        modularity::ioc(m_ctx)->unregister<IPlayback>("utests");
        modularity::ioc(m_ctx)->unregister<playback::IPlaybackController>("utests");
        modularity::ioc(m_ctx)->unregister<IProjectSession>("utests");
        modularity::removeIoC(m_ctx);
    }

    std::shared_ptr<Outcome> startWriting(const INotationWriter::Options& options = {})
    {
        auto outcome = std::make_shared<Outcome>();

        m_writer->write(m_notation, m_buffer, options).onResolve(this, [outcome](const Ret& ret) {
            outcome->isFinished = true;
            outcome->ret = ret;
        });

        drainDeferredCalls();
        return outcome;
    }

    void finishSaving()
    {
        ASSERT_TRUE(m_resolveSaving.has_value());
        m_isSavingDone = true;
        (void)(*m_resolveSaving)(true);
        drainDeferredCalls();
    }

    void failSaving(int code, const std::string& msg)
    {
        ASSERT_TRUE(m_rejectSaving.has_value());
        m_isSavingDone = true;
        (void)(*m_rejectSaving)(code, msg);
        drainDeferredCalls();
    }

    modularity::ContextPtr m_ctx;

    std::shared_ptr<NiceMock<StartAudioControllerMock> > m_startAudioController;
    std::shared_ptr<NiceMock<ApplicationMock> > m_application;
    std::shared_ptr<NiceMock<PlaybackMock> > m_playback;
    std::shared_ptr<NiceMock<playback::PlaybackControllerMock> > m_playbackController;
    std::shared_ptr<NiceMock<ProjectSessionMock> > m_projectSession;

    std::shared_ptr<NiceMock<notation::NotationMock> > m_notation;
    std::shared_ptr<NiceMock<notation::NotationMock> > m_notationToRestore;

    bool m_isAudioStarted = true;
    async::Channel<bool> m_audioStartedChanged;
    bool m_isPlaybackInited = true;
    async::Channel<bool> m_playbackInitedChanged;
    SaveSoundTrackProgress m_saveProgress;

    SoundTrackFormat m_savedFormat;
    std::optional<async::Promise<bool>::Resolve> m_resolveSaving;
    std::optional<async::Promise<bool>::Reject> m_rejectSaving;
    bool m_isSavingDone = false;

    io::Buffer m_buffer;
    std::shared_ptr<TestAudioWriter> m_writer;
};

TEST_F(AbstractAudioWriterTests, StartsSavingAtOnce_WhenEverythingIsReady)
{
    //! [GIVEN] The audio system is started and the playback is inited

    //! [THEN] The playback is switched to the export mode for the notation, and the sound track is saved
    EXPECT_CALL(*m_playbackController, setNotation(notation::INotationPtr(m_notation))).Times(1);
    EXPECT_CALL(*m_playbackController, setIsExportingAudio(true)).Times(1);
    EXPECT_CALL(*m_playback, saveSoundTrack(_, _)).Times(1);

    //! [WHEN] Write
    auto outcome = startWriting();

    //! [THEN] It is not finished until the saving is
    EXPECT_FALSE(outcome->isFinished);
}

TEST_F(AbstractAudioWriterTests, WaitsForAudioSystem)
{
    //! [GIVEN] The audio system is not started yet
    m_isAudioStarted = false;

    EXPECT_CALL(*m_playback, saveSoundTrack(_, _)).Times(0);

    //! [WHEN] Write
    startWriting();

    //! [THEN] Nothing is saved until the audio system is started
    testing::Mock::VerifyAndClearExpectations(m_playback.get());
    EXPECT_CALL(*m_playback, saveSoundTrack(_, _)).Times(1);

    //! [WHEN] The audio system is started
    m_isAudioStarted = true;
    m_audioStartedChanged.send(true);
    drainDeferredCalls();
}

TEST_F(AbstractAudioWriterTests, WaitsForPlayback)
{
    //! [GIVEN] The playback (tracks and duration) is not inited yet
    m_isPlaybackInited = false;

    EXPECT_CALL(*m_playback, saveSoundTrack(_, _)).Times(0);

    //! [WHEN] Write
    startWriting();

    //! [THEN] Nothing is saved until the playback is inited
    testing::Mock::VerifyAndClearExpectations(m_playback.get());
    EXPECT_CALL(*m_playback, saveSoundTrack(_, _)).Times(1);

    //! [WHEN] The playback is inited
    m_isPlaybackInited = true;
    m_playbackInitedChanged.send(true);
    drainDeferredCalls();
}

TEST_F(AbstractAudioWriterTests, IgnoresNotificationsWhileNotReady)
{
    //! [GIVEN] The audio system is not started yet
    m_isAudioStarted = false;

    startWriting();

    //! [THEN] The notification that doesn't make it ready doesn't start the saving
    EXPECT_CALL(*m_playback, saveSoundTrack(_, _)).Times(0);

    //! [WHEN] The channel notifies, but the audio system is still not started
    m_audioStartedChanged.send(false);
    drainDeferredCalls();
}

TEST_F(AbstractAudioWriterTests, SavesWithTheFormatOfTheWriter_AndTheSilenceFromTheOptions)
{
    //! [GIVEN] The options with the silence
    INotationWriter::Options options;
    options[INotationWriter::OptionKey::LEADING_SILENCE_SEC] = Val(2.0);
    options[INotationWriter::OptionKey::TRAILING_SILENCE_SEC] = Val(3.0);

    //! [WHEN] Write
    startWriting(options);

    //! [THEN] The format is the one of the writer, with the silence
    EXPECT_EQ(m_savedFormat.type, SoundTrackType::WAV);
    EXPECT_EQ(m_savedFormat.bitRate, 128);
    EXPECT_EQ(m_savedFormat.leadingSilenceDuration, secs_t(2.0));
    EXPECT_EQ(m_savedFormat.trailingSilenceDuration, secs_t(3.0));
}

TEST_F(AbstractAudioWriterTests, SavesWithTheFractionalSilence)
{
    //! [GIVEN] The options with the silence that is not a whole number of seconds
    INotationWriter::Options options;
    options[INotationWriter::OptionKey::LEADING_SILENCE_SEC] = Val(0.5);
    options[INotationWriter::OptionKey::TRAILING_SILENCE_SEC] = Val(1.25);

    //! [WHEN] Write
    startWriting(options);

    //! [THEN] The fractions of seconds are kept
    EXPECT_EQ(m_savedFormat.leadingSilenceDuration, secs_t(0.5));
    EXPECT_EQ(m_savedFormat.trailingSilenceDuration, secs_t(1.25));
}

TEST_F(AbstractAudioWriterTests, SavesWithoutSilence_ByDefault)
{
    //! [WHEN] Write without the options
    startWriting();

    //! [THEN] There is no silence
    EXPECT_EQ(m_savedFormat.leadingSilenceDuration, secs_t(0.0));
    EXPECT_EQ(m_savedFormat.trailingSilenceDuration, secs_t(0.0));
}

TEST_F(AbstractAudioWriterTests, IgnoresNotFiniteSilence)
{
    //! [GIVEN] The options with the silence that is not a number
    INotationWriter::Options options;
    options[INotationWriter::OptionKey::LEADING_SILENCE_SEC] = Val(std::numeric_limits<double>::quiet_NaN());
    options[INotationWriter::OptionKey::TRAILING_SILENCE_SEC] = Val(std::numeric_limits<double>::infinity());

    //! [WHEN] Write
    startWriting(options);

    //! [THEN] There is no silence
    EXPECT_EQ(m_savedFormat.leadingSilenceDuration, secs_t(0.0));
    EXPECT_EQ(m_savedFormat.trailingSilenceDuration, secs_t(0.0));
}

TEST_F(AbstractAudioWriterTests, Finishes_WhenSaved)
{
    //! [GIVEN] The saving is started
    auto outcome = startWriting();
    ASSERT_FALSE(outcome->isFinished);

    //! [THEN] The playback is restored
    EXPECT_CALL(*m_playbackController, setIsExportingAudio(false)).Times(1);
    EXPECT_CALL(*m_playbackController, setNotation(notation::INotationPtr(m_notationToRestore))).Times(1);

    //! [WHEN] The sound track is saved
    finishSaving();

    //! [THEN] The writing is finished successfully
    EXPECT_TRUE(outcome->isFinished);
    EXPECT_TRUE(outcome->ret);
}

TEST_F(AbstractAudioWriterTests, FinishesWithError_WhenSavingFailed)
{
    //! [GIVEN] The saving is started
    auto outcome = startWriting();

    //! [THEN] The playback is restored
    EXPECT_CALL(*m_playbackController, setIsExportingAudio(false)).Times(1);
    EXPECT_CALL(*m_playbackController, setNotation(notation::INotationPtr(m_notationToRestore))).Times(1);

    //! [WHEN] The saving is failed
    failSaving(static_cast<int>(Ret::Code::InternalError), "failed");

    //! [THEN] The writing is finished with this error
    EXPECT_TRUE(outcome->isFinished);
    EXPECT_FALSE(outcome->ret);
    EXPECT_EQ(outcome->ret.code(), static_cast<int>(Ret::Code::InternalError));
}

TEST_F(AbstractAudioWriterTests, FinishesRightAfterTheStart_WhenNotWaitingForCompletion)
{
    //! [GIVEN] The options not to wait for the completion (e.g. the video export)
    INotationWriter::Options options;
    options[INotationWriter::OptionKey::WAIT_FOR_COMPLETION] = Val(false);

    //! [WHEN] Write
    EXPECT_CALL(*m_playback, saveSoundTrack(_, _)).Times(1);
    auto outcome = startWriting(options);

    //! [THEN] It is finished though the sound track is not saved yet
    EXPECT_TRUE(outcome->isFinished);
    EXPECT_TRUE(outcome->ret);
}

TEST_F(AbstractAudioWriterTests, ReportsProgress)
{
    //! [GIVEN] The saving is started
    startWriting();

    struct Progress {
        int64_t current = 0;
        int64_t total = 0;
        std::string title;
    };
    std::vector<Progress> progress;
    m_writer->progress()->progressChanged().onReceive(this, [&progress](int64_t current, int64_t total, const std::string& title) {
        progress.push_back({ current, total, title });
    });

    //! [WHEN] The playback reports the progress
    m_saveProgress.send(10, 100, SaveSoundTrackStage::WritingSoundTrack);
    m_saveProgress.send(20, 100, SaveSoundTrackStage::ProcessingOnlineSounds);
    drainDeferredCalls();

    //! [THEN] It is passed on, and the title is only for the online sounds
    ASSERT_EQ(progress.size(), 2u);
    EXPECT_EQ(progress[0].current, 10);
    EXPECT_EQ(progress[0].total, 100);
    EXPECT_TRUE(progress[0].title.empty());
    EXPECT_EQ(progress[1].current, 20);
    EXPECT_FALSE(progress[1].title.empty());
}

TEST_F(AbstractAudioWriterTests, StopsReportingProgress_WhenSaved)
{
    //! [GIVEN] The saving is finished
    startWriting();
    finishSaving();

    int count = 0;
    m_writer->progress()->progressChanged().onReceive(this, [&count](int64_t, int64_t, const std::string&) {
        ++count;
    });

    //! [WHEN] The playback reports the progress
    m_saveProgress.send(10, 100, SaveSoundTrackStage::WritingSoundTrack);
    drainDeferredCalls();

    //! [THEN] It is not passed on
    EXPECT_EQ(count, 0);
}

TEST_F(AbstractAudioWriterTests, Abort_CancelsSaving)
{
    //! [GIVEN] The saving is started
    auto outcome = startWriting();
    ASSERT_FALSE(outcome->isFinished);

    //! [THEN] All the saving is aborted
    EXPECT_CALL(*m_playback, abortSavingAllSoundTracks()).Times(1);

    //! [WHEN] Abort
    m_writer->abort();
    drainDeferredCalls();

    //! [THEN] The writing is finished as canceled
    EXPECT_TRUE(outcome->isFinished);
    EXPECT_EQ(outcome->ret.code(), static_cast<int>(Ret::Code::Cancel));
}

TEST_F(AbstractAudioWriterTests, Abort_WhileWaitingForAudioSystem_CancelsAndDoesNotStart)
{
    //! [GIVEN] The audio system is not started yet, so the writing is waiting
    m_isAudioStarted = false;
    auto outcome = startWriting();
    ASSERT_FALSE(outcome->isFinished);

    //! [WHEN] Abort
    m_writer->abort();
    drainDeferredCalls();

    //! [THEN] The writing is finished as canceled
    EXPECT_TRUE(outcome->isFinished);
    EXPECT_EQ(outcome->ret.code(), static_cast<int>(Ret::Code::Cancel));

    //! [THEN] It doesn't start when the audio system is started later
    EXPECT_CALL(*m_playback, saveSoundTrack(_, _)).Times(0);

    m_isAudioStarted = true;
    m_audioStartedChanged.send(true);
    drainDeferredCalls();
}

TEST_F(AbstractAudioWriterTests, Abort_WhileWaitingForPlayback_CancelsAndDoesNotStart)
{
    //! [GIVEN] The playback is not inited yet, so the writing is waiting
    m_isPlaybackInited = false;
    auto outcome = startWriting();
    ASSERT_FALSE(outcome->isFinished);

    //! [WHEN] Abort
    m_writer->abort();
    drainDeferredCalls();

    //! [THEN] The writing is finished as canceled
    EXPECT_TRUE(outcome->isFinished);
    EXPECT_EQ(outcome->ret.code(), static_cast<int>(Ret::Code::Cancel));

    //! [THEN] It doesn't start when the playback is inited later
    EXPECT_CALL(*m_playback, saveSoundTrack(_, _)).Times(0);

    m_isPlaybackInited = true;
    m_playbackInitedChanged.send(true);
    drainDeferredCalls();
}

TEST_F(AbstractAudioWriterTests, Abort_WhileSaving_RestoresThePlaybackAndFinishesOnce)
{
    //! [GIVEN] The saving is started
    auto outcome = startWriting();
    ASSERT_FALSE(outcome->isFinished);

    //! [THEN] The playback is restored by the abort, and only once
    EXPECT_CALL(*m_playbackController, setIsExportingAudio(false)).Times(1);
    EXPECT_CALL(*m_playbackController, setNotation(notation::INotationPtr(m_notationToRestore))).Times(1);

    //! [WHEN] Abort, and then the engine rejects the aborted saving
    m_writer->abort();
    drainDeferredCalls();
    failSaving(static_cast<int>(Ret::Code::Cancel), "aborted");

    //! [THEN] The writing is finished as canceled
    EXPECT_TRUE(outcome->isFinished);
    EXPECT_EQ(outcome->ret.code(), static_cast<int>(Ret::Code::Cancel));
}

TEST_F(AbstractAudioWriterTests, AbortedSaving_Rejected_DoesNotFinishTheNextWrite)
{
    //! [GIVEN] The first write is aborted while saving
    auto first = startWriting();
    ASSERT_FALSE(first->isFinished);

    auto staleReject = m_rejectSaving;
    ASSERT_TRUE(staleReject.has_value());

    m_writer->abort();
    drainDeferredCalls();
    ASSERT_TRUE(first->isFinished);

    //! [GIVEN] The second write is started and saving
    auto second = startWriting();
    ASSERT_FALSE(second->isFinished);

    //! [WHEN] The engine rejects the aborted saving of the first write
    (void)(*staleReject)(static_cast<int>(Ret::Code::Cancel), "aborted");
    drainDeferredCalls();

    //! [THEN] It doesn't finish the second write
    EXPECT_FALSE(second->isFinished);

    //! [WHEN] The second saving is finished
    finishSaving();

    //! [THEN] The second write is finished successfully
    EXPECT_TRUE(second->isFinished);
    EXPECT_TRUE(second->ret);
}

TEST_F(AbstractAudioWriterTests, WriteBlocksUntilSaved)
{
    //! [GIVEN] The saving finishes while the events are being processed
    int processed = 0;
    ON_CALL(*m_application, processEvents()).WillByDefault([this, &processed]() {
        ++processed;
        if (processed == 3) {
            finishSaving();
        }
    });

    //! [WHEN] Write (blocking)
    Ret ret = m_writer->writeSync(m_notation, m_buffer);

    //! [THEN] It returns the result of the saving, after the events were processed
    EXPECT_TRUE(ret);
    EXPECT_GE(processed, 3);
}

TEST_F(AbstractAudioWriterTests, WriteReturnsError_WhenSavingFailed)
{
    //! [GIVEN] The saving fails while the events are being processed
    ON_CALL(*m_application, processEvents()).WillByDefault([this]() {
        failSaving(static_cast<int>(Ret::Code::InternalError), "failed");
    });

    //! [WHEN] Write (blocking)
    Ret ret = m_writer->writeSync(m_notation, m_buffer);

    //! [THEN] It returns this error
    EXPECT_FALSE(ret);
    EXPECT_EQ(ret.code(), static_cast<int>(Ret::Code::InternalError));
}
}
