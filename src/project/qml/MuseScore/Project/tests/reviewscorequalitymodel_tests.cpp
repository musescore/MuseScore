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

#include <QCoreApplication>
#include <QUrl>

#include "global/types/uri.h"

#include "project/qml/MuseScore/Project/internal/ConvertFileToScore/reviewscorequalitymodel.h"

#include "project/tests/utils/testutils.h"
#include "project/tests/mocks/convertfiletoscoreservicemock.h"
#include "project/tests/mocks/notationprojectmock.h"
#include "context/tests/mocks/globalcontextmock.h"
#include "global/tests/mocks/interactivemock.h"

using namespace ::testing;
using namespace mu::project;
using namespace mu::context;
using namespace muse;

namespace {
constexpr int SCORE_ID = 456;

const UriQuery NOTATION_PAGE_URI(Uri("musescore://notation"));
}

namespace mu::project {
class Project_ReviewScoreQualityModelTest : public ::testing::Test
{
protected:
    enum class Event {
        Close,
        Open
    };

    void SetUp() override
    {
        m_interactive = std::make_shared<NiceMock<InteractiveMock> >();
        m_globalContext = std::make_shared<NiceMock<GlobalContextMock> >();
        m_service = std::make_shared<NiceMock<ConvertFileToScoreServiceMock> >();
        m_project = std::make_shared<NiceMock<NotationProjectMock> >();

        m_model.interactive.set(m_interactive);
        m_model.globalContext.set(m_globalContext);
        m_model.convertFileToScoreService.set(m_service);

        m_cloudInfo.sourceUrl = QUrl(QString("test://scores/%1").arg(SCORE_ID));

        ON_CALL(*m_globalContext, currentProject())
        .WillByDefault(Return(m_project));
        ON_CALL(*m_project, cloudInfo())
        .WillByDefault(ReturnRef(m_cloudInfo));

        ON_CALL(*m_interactive, open(_))
        .WillByDefault(Invoke([this](const UriQuery& uri) {
            m_events.push_back(Event::Open);
            m_openedUris.push_back(uri);

            return resolvedPromise(Val());
        }));

        QObject::connect(&m_model, &ReviewScoreQualityModel::closeRequested, [this]() { m_events.push_back(Event::Close); });
        QObject::connect(&m_model, &ReviewScoreQualityModel::stepChanged, [this]() { ++m_stepChangedCount; });
    }

    void processEvents()
    {
        QCoreApplication::processEvents();
    }

    std::vector<Event> m_events;
    std::vector<UriQuery> m_openedUris;
    int m_stepChangedCount = 0;

    CloudProjectInfo m_cloudInfo;

    ReviewScoreQualityModel m_model;

    std::shared_ptr<InteractiveMock> m_interactive;
    std::shared_ptr<GlobalContextMock> m_globalContext;
    std::shared_ptr<ConvertFileToScoreServiceMock> m_service;
    std::shared_ptr<NotationProjectMock> m_project;
};
}

// ==================================================
// Initial state
// ==================================================

TEST_F(Project_ReviewScoreQualityModelTest, InitialState_IsRatingStep)
{
    // [THEN] The model starts on the rating step
    EXPECT_EQ(m_model.step(), ReviewScoreQualityModel::Step::Rating);
}

// ==================================================
// submitGood()
// ==================================================

TEST_F(Project_ReviewScoreQualityModelTest, SubmitGood_SubmitsGoodRatingWithoutComment)
{
    // [THEN] A good rating is submitted for the current cloud score
    EXPECT_CALL(*m_service, submitReview(SCORE_ID, ReviewRating::Good, QString()))
    .Times(1);

    // [WHEN] Rating the score as good
    m_model.submitGood();

    // [THEN] The feedback step is skipped
    EXPECT_EQ(m_model.step(), ReviewScoreQualityModel::Step::Rating);
    EXPECT_EQ(m_stepChangedCount, 0);
}

TEST_F(Project_ReviewScoreQualityModelTest, SubmitGood_ClosesThenOpensNotationPage)
{
    // [WHEN] Rating the score as good
    m_model.submitGood();

    // [THEN] The panel is asked to close; the notation page is not opened yet
    EXPECT_THAT(m_events, ElementsAre(Event::Close));

    // [WHEN] The event loop spins
    processEvents();

    // [THEN] The notation page is opened
    EXPECT_THAT(m_events, ElementsAre(Event::Close, Event::Open));
    EXPECT_THAT(m_openedUris, ElementsAre(NOTATION_PAGE_URI));
}

TEST_F(Project_ReviewScoreQualityModelTest, SubmitGood_ScoreIdIsLastSegmentOfSourceUrl)
{
    // [GIVEN] The current project points to another cloud score
    m_cloudInfo.sourceUrl = QUrl("test://scores/1024");

    // [THEN] The review is submitted for that score
    EXPECT_CALL(*m_service, submitReview(1024, ReviewRating::Good, QString()))
    .Times(1);

    // [WHEN] Rating the score as good
    m_model.submitGood();
}

// ==================================================
// submitBad()
// ==================================================

TEST_F(Project_ReviewScoreQualityModelTest, SubmitBad_SwitchesToFeedbackStep)
{
    // [THEN] Nothing is submitted yet
    EXPECT_CALL(*m_service, submitReview(_, _, _))
    .Times(0);

    // [WHEN] Rating the score as bad
    m_model.submitBad();
    processEvents();

    // [THEN] The model asks for feedback
    EXPECT_EQ(m_model.step(), ReviewScoreQualityModel::Step::Feedback);
    EXPECT_EQ(m_stepChangedCount, 1);

    // [THEN] The panel stays open
    EXPECT_TRUE(m_events.empty());
}

// ==================================================
// submitFeedback()
// ==================================================

TEST_F(Project_ReviewScoreQualityModelTest, SubmitFeedback_SubmitsBadRatingWithComment)
{
    // [GIVEN] The score was rated as bad
    m_model.submitBad();

    // [THEN] The bad rating is submitted together with the comment
    const QString feedback = "Wrong notes";
    EXPECT_CALL(*m_service, submitReview(SCORE_ID, ReviewRating::Bad, feedback))
    .Times(1);

    // [WHEN] Submitting the feedback
    m_model.submitFeedback(feedback);
}

TEST_F(Project_ReviewScoreQualityModelTest, SubmitFeedback_ClosesThenOpensNotationPage)
{
    // [GIVEN] The score was rated as bad
    m_model.submitBad();

    // [WHEN] Submitting the feedback
    m_model.submitFeedback("Wrong notes");

    // [THEN] The panel is asked to close and the model is back on the rating step
    EXPECT_THAT(m_events, ElementsAre(Event::Close));
    EXPECT_EQ(m_model.step(), ReviewScoreQualityModel::Step::Rating);

    // [WHEN] The event loop spins
    processEvents();

    // [THEN] The notation page is opened
    EXPECT_THAT(m_events, ElementsAre(Event::Close, Event::Open));
    EXPECT_THAT(m_openedUris, ElementsAre(NOTATION_PAGE_URI));
}

TEST_F(Project_ReviewScoreQualityModelTest, SubmitFeedback_WithoutRating_SubmitsNothing)
{
    // [THEN] Nothing is submitted, as the score was not rated
    EXPECT_CALL(*m_service, submitReview(_, _, _))
    .Times(0);

    // [WHEN] Submitting the feedback
    m_model.submitFeedback("Wrong notes");
    processEvents();

    // [THEN] The panel is still closed and the notation page opened
    EXPECT_THAT(m_events, ElementsAre(Event::Close, Event::Open));
}

// ==================================================
// skipFeedback()
// ==================================================

TEST_F(Project_ReviewScoreQualityModelTest, SkipFeedback_SubmitsBadRatingWithoutComment)
{
    // [GIVEN] The score was rated as bad
    m_model.submitBad();

    // [THEN] Only the bad rating is submitted
    EXPECT_CALL(*m_service, submitReview(SCORE_ID, ReviewRating::Bad, QString()))
    .Times(1);

    // [WHEN] Skipping the feedback
    m_model.skipFeedback();
}

TEST_F(Project_ReviewScoreQualityModelTest, SkipFeedback_ClosesThenOpensNotationPage)
{
    // [GIVEN] The score was rated as bad
    m_model.submitBad();

    // [WHEN] Skipping the feedback
    m_model.skipFeedback();

    // [THEN] The panel is asked to close and the model is back on the rating step
    EXPECT_THAT(m_events, ElementsAre(Event::Close));
    EXPECT_EQ(m_model.step(), ReviewScoreQualityModel::Step::Rating);

    // [WHEN] The event loop spins
    processEvents();

    // [THEN] The notation page is opened
    EXPECT_THAT(m_events, ElementsAre(Event::Close, Event::Open));
    EXPECT_THAT(m_openedUris, ElementsAre(NOTATION_PAGE_URI));
}

TEST_F(Project_ReviewScoreQualityModelTest, SkipFeedback_WithoutRating_SubmitsNothing)
{
    // [THEN] Nothing is submitted, as the score was not rated
    EXPECT_CALL(*m_service, submitReview(_, _, _))
    .Times(0);

    // [WHEN] Skipping the feedback
    m_model.skipFeedback();
    processEvents();

    // [THEN] The panel is still closed and the notation page opened
    EXPECT_THAT(m_events, ElementsAre(Event::Close, Event::Open));
}

// ==================================================
// Reset after closing
// ==================================================

TEST_F(Project_ReviewScoreQualityModelTest, Close_ResetsRating)
{
    // [THEN] The bad rating is submitted only once
    EXPECT_CALL(*m_service, submitReview(SCORE_ID, ReviewRating::Bad, QString()))
    .Times(1);

    // [GIVEN] A finished review
    m_model.submitBad();
    m_model.skipFeedback();

    // [WHEN] Submitting feedback again without rating the score
    m_model.submitFeedback("Late comment");
}
