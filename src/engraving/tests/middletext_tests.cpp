#include <gtest/gtest.h>

#include <memory>

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/text.h"
#include "engraving/dom/textline.h"
#include "engraving/rendering/iscorerenderer.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

static const String SPANNERS_DATA_DIR("spanners_data/");

class Engraving_MiddleTextTests : public ::testing::Test
{
};

TEST_F(Engraving_MiddleTextTests, middleTextRoundTrip)
{
    std::unique_ptr<MasterScore> score(compat::ScoreAccess::createMasterScore(nullptr));
    TextLine line(score->dummy());
    EXPECT_TRUE(line.middleText().empty());
    EXPECT_TRUE(line.middleTextCutThrough());
    line.setProperty(Pid::MIDDLE_TEXT, String(u"Mute"));
    line.setProperty(Pid::MIDDLE_TEXT_OFFSET, PointF(1.0, -2.0));
    line.setProperty(Pid::MIDDLE_TEXT_CUT_THROUGH, false);
    line.setProperty(Pid::MIDDLE_TEXT_MIN_LINE_LENGTH, Spatium(1.25));
    std::unique_ptr<EngravingItem> reopened(ScoreRW::writeReadElement(&line));
    ASSERT_TRUE(reopened);
    EXPECT_EQ(reopened->getProperty(Pid::MIDDLE_TEXT).value<String>(), u"Mute");
    const PointF offset = reopened->getProperty(Pid::MIDDLE_TEXT_OFFSET).value<PointF>();
    EXPECT_NEAR(offset.x(), 1.0, 1e-5);
    EXPECT_NEAR(offset.y(), -2.0, 1e-5);
    EXPECT_FALSE(reopened->getProperty(Pid::MIDDLE_TEXT_CUT_THROUGH).toBool());
    EXPECT_EQ(reopened->getProperty(Pid::MIDDLE_TEXT_MIN_LINE_LENGTH).value<Spatium>(), Spatium(1.25));
}

TEST_F(Engraving_MiddleTextTests, middleTextAcrossSystems)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(SPANNERS_DATA_DIR + u"middle-text.mscx"));
    ASSERT_TRUE(score);
    ASSERT_EQ(score->spannerMap().map().size(), 1u);
    TextLine* line = toTextLine(score->spannerMap().map().begin()->second);
    ASSERT_EQ(line->spannerSegments().size(), 2u);
    for (SpannerSegment* segment : line->spannerSegments()) {
        auto textSegment = toTextLineBaseSegment(segment);
        EXPECT_EQ(textSegment->middleText()->xmlText(), u"M");
        const auto ld = textSegment->ldata();
        EXPECT_FALSE(ld->mask().empty());
        // Skip the 90-degree hook tips to get the straight line's endpoints.
        const int lineStartIndex = textSegment->isSingleBeginType() ? 1 : 0;
        const int lineEndIndex = ld->npoints - (textSegment->isSingleEndType() ? 2 : 1);
        ASSERT_GT(lineEndIndex, lineStartIndex);
        const PointF midpoint = (ld->points[lineStartIndex] + ld->points[lineEndIndex]) * 0.5;
        const PointF textCenter = textSegment->middleText()->ldata()->bbox().translated(textSegment->middleText()->pos()).center();
        EXPECT_NEAR(textCenter.x(), midpoint.x(), 1e-6);
        EXPECT_NEAR(textCenter.y(), midpoint.y(), 1e-6);
    }

    line->setProperty(Pid::MIDDLE_TEXT_CUT_THROUGH, false);
    score->doLayout();
    for (SpannerSegment* segment : line->spannerSegments()) {
        EXPECT_TRUE(toTextLineBaseSegment(segment)->ldata()->mask().empty());
    }
}

class Engraving_MiddleTextLayoutTests : public Engraving_MiddleTextTests
{
protected:
    Engraving_MiddleTextLayoutTests()
        : m_score(compat::ScoreAccess::createMasterScore(nullptr)), m_line(m_score->dummy()), m_segment(&m_line, false)
    {
        m_line.setMiddleText(u"M");
        m_line.setMiddleTextMinLineLength(Spatium(2.0));
        m_segment.setSpannerSegmentType(SpannerSegmentType::SINGLE);
        m_segment.setPos2(PointF(10.0 * m_line.spatium(), 0.0));
    }

    const TextLineBaseSegment::LayoutData* layout()
    {
        m_segment.renderer()->layoutTextLineBaseSegment(&m_segment);
        return m_segment.ldata();
    }

    std::unique_ptr<MasterScore> m_score;
    TextLine m_line;
    TextLineSegment m_segment;
};

TEST_F(Engraving_MiddleTextLayoutTests, middleTextShortSides)
{
    for (LineType style : { LineType::SOLID, LineType::DASHED, LineType::DOTTED }) {
        m_line.setLineStyle(style);
        for (HookType hook : { HookType::HOOK_90, HookType::HOOK_45, HookType::HOOK_90T,
                               HookType::ARROW, HookType::ARROW_FILLED }) {
            m_line.setBeginHookType(hook);
            m_line.setEndHookType(hook);
            for (double offset : { -2.5, 2.5 }) {
                m_line.setMiddleTextOffset(PointF(offset * m_line.spatium(), 0.0));
                const auto ld = layout();
                const bool hideBegin = offset < 0.0;
                EXPECT_EQ(ld->hideBegin, hideBegin);
                EXPECT_EQ(ld->hideEnd, !hideBegin);
                ASSERT_GE(ld->npoints, 2);
                if (hideBegin) {
                    EXPECT_NEAR(ld->points[0].x(), ld->mask().bbox().left(), 1e-6);
                } else {
                    EXPECT_NEAR(ld->points[ld->npoints - 1].x(), ld->mask().bbox().right(), 1e-6);
                }
            }
        }
    }
}

TEST_F(Engraving_MiddleTextLayoutTests, middleTextMaskPadding)
{
    for (double width : { 0.25, 2.0 }) {
        m_line.setLineWidth(Spatium(width));
        const auto ld = layout();
        const RectF textBounds = m_segment.middleText()->ldata()->bbox().translated(m_segment.middleText()->pos());
        const RectF maskBounds = ld->mask().bbox();
        const double padding = m_line.absoluteFromSpatium(m_line.gapBetweenTextAndLine());
        EXPECT_NEAR(maskBounds.left(), textBounds.left() - padding, 1e-6);
        EXPECT_NEAR(maskBounds.right(), textBounds.right() + padding, 1e-6);
        EXPECT_NEAR(maskBounds.top(), textBounds.top() - padding, 1e-6);
        EXPECT_NEAR(maskBounds.bottom(), textBounds.bottom() + padding, 1e-6);
    }
}

TEST_F(Engraving_MiddleTextLayoutTests, middleTextCutThrough)
{
    m_line.setBeginHookType(HookType::ARROW_FILLED);
    m_line.setEndHookType(HookType::ARROW_FILLED);
    m_line.setMiddleTextMinLineLength(Spatium(10.0));
    const auto ld = layout();
    EXPECT_TRUE(ld->hideBegin);
    EXPECT_TRUE(ld->hideEnd);
    EXPECT_EQ(ld->npoints, 0);

    m_line.setMiddleTextOffset(PointF(0.0, -5.0 * m_line.spatium()));
    layout();
    EXPECT_FALSE(ld->hideBegin);
    EXPECT_FALSE(ld->hideEnd);
    EXPECT_GT(ld->npoints, 0);

    m_line.setMiddleTextOffset(PointF());
    m_line.setMiddleTextCutThrough(false);
    layout();
    EXPECT_FALSE(ld->hideBegin);
    EXPECT_FALSE(ld->hideEnd);
    EXPECT_TRUE(ld->mask().empty());
    EXPECT_GT(ld->npoints, 0);
}

TEST_F(Engraving_MiddleTextLayoutTests, middleTextDiagonalLengths)
{
    m_line.setDiagonal(true);
    m_line.setLineWidth(Spatium(0.25));
    for (const PointF& end : { PointF(2.0, 20.0), PointF(0.0, 20.0), PointF(-2.0, 20.0), PointF(2.0, -20.0) }) {
        SCOPED_TRACE(::testing::Message() << "end=" << end.x() << "," << end.y());
        m_segment.setPos2(end * m_line.spatium());
        m_line.setMiddleTextMinLineLength(Spatium(2.0));
        const auto ld = layout();
        EXPECT_FALSE(ld->hideBegin);
        EXPECT_FALSE(ld->hideEnd);
        EXPECT_EQ(ld->npoints, 2);

        m_line.setMiddleTextMinLineLength(Spatium(30.0));
        layout();
        EXPECT_TRUE(ld->hideBegin);
        EXPECT_TRUE(ld->hideEnd);
        EXPECT_EQ(ld->npoints, 0);
    }
}
