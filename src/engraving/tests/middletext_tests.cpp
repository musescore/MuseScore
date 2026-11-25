#include <gtest/gtest.h>

#include <memory>

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/text.h"
#include "engraving/dom/textline.h"
#include "engraving/rendering/iscorerenderer.h"
#include "engraving/rendering/single/singlerenderer.h"

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
        EXPECT_TRUE(ld->mask().empty());
        EXPECT_GE(ld->gapIndex, 0);
        // Skip the 90-degree hook tips to get the straight line's endpoints.
        const int lineStartIndex = textSegment->isSingleBeginType() ? 1 : 0;
        const int lineEndIndex = ld->npoints - (textSegment->isSingleEndType() ? 2 : 1);
        ASSERT_GT(lineEndIndex, lineStartIndex);
        const PointF midpoint = (ld->points[lineStartIndex] + ld->points[lineEndIndex]) * 0.5;
        const PointF textCenter = textSegment->middleText()->ldata()->bbox().translated(textSegment->middleText()->pos()).center();
        EXPECT_NEAR(textCenter.x(), midpoint.x(), 1e-6);
        EXPECT_NEAR(textCenter.y(), midpoint.y(), 1e-6);
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
                const RectF textBounds = m_segment.middleText()->ldata()->bbox().translated(m_segment.middleText()->pos());
                const double padding = m_line.absoluteFromSpatium(m_line.gapBetweenTextAndLine());
                if (hideBegin) {
                    EXPECT_NEAR(ld->points[0].x(), textBounds.right() + padding, 1e-6);
                } else {
                    EXPECT_NEAR(ld->points[ld->npoints - 1].x(), textBounds.left() - padding, 1e-6);
                }
            }
        }
    }
}

TEST_F(Engraving_MiddleTextLayoutTests, middleTextGapPadding)
{
    for (double width : { 0.25, 2.0 }) {
        m_line.setLineWidth(Spatium(width));
        const auto ld = layout();
        const RectF textBounds = m_segment.middleText()->ldata()->bbox().translated(m_segment.middleText()->pos());
        const double padding = m_line.absoluteFromSpatium(m_line.gapBetweenTextAndLine());
        EXPECT_TRUE(ld->mask().empty());
        ASSERT_GE(ld->gapIndex, 0);
        EXPECT_NEAR(ld->points[ld->gapIndex].x(), textBounds.left() - padding, 1e-6);
        EXPECT_NEAR(ld->points[ld->gapIndex + 1].x(), textBounds.right() + padding, 1e-6);
    }
}

TEST_F(Engraving_MiddleTextTests, middleTextHairpinMask)
{
    std::unique_ptr<MasterScore> score(compat::ScoreAccess::createMasterScore(nullptr));
    Hairpin hairpin(score->dummy());
    hairpin.setMiddleText(u"M");
    HairpinSegment segment(&hairpin);
    segment.setSpannerSegmentType(SpannerSegmentType::SINGLE);
    segment.setPos2(PointF(20.0 * hairpin.spatium(), 0.0));
    hairpin.setSpannerSegments({ &segment });
    rendering::single::SingleRenderer renderer;
    renderer.layoutItem(&hairpin);
    hairpin.setSpannerSegments({});
    EXPECT_TRUE(segment.twoLines());
    EXPECT_FALSE(segment.ldata()->mask().empty());
    EXPECT_EQ(segment.ldata()->gapIndex, -1);
    EXPECT_EQ(segment.ldata()->npoints, 4);
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
        EXPECT_EQ(ld->npoints, 4);
        EXPECT_TRUE(ld->mask().empty());
        EXPECT_EQ(ld->gapIndex, 1);

        m_line.setMiddleTextMinLineLength(Spatium(30.0));
        layout();
        EXPECT_TRUE(ld->hideBegin);
        EXPECT_TRUE(ld->hideEnd);
        EXPECT_EQ(ld->npoints, 0);
    }
}

struct MiddleTextLayoutMode {
    bool single;
    bool diagonal;
    double endY;
};

class Engraving_MiddleTextOverflowTests : public Engraving_MiddleTextLayoutTests, public ::testing::WithParamInterface<MiddleTextLayoutMode>
{
protected:
    const TextLineBaseSegment::LayoutData* layout()
    {
        if (GetParam().single) {
            rendering::single::SingleRenderer renderer;
            m_line.setSpannerSegments({ &m_segment });
            renderer.layoutItem(&m_line);
            m_line.setSpannerSegments({});
            return m_segment.ldata();
        }
        return Engraving_MiddleTextLayoutTests::layout();
    }
};

TEST_F(Engraving_MiddleTextLayoutTests, middleTextSingleHiddenLine)
{
    m_line.setBeginText(u"Begin");
    m_line.setEndText(u"End");
    m_line.setBeginTextAlign({ AlignH::LEFT, AlignV::VCENTER });
    m_line.setEndTextAlign({ AlignH::LEFT, AlignV::VCENTER });
    rendering::single::SingleRenderer renderer;
    for (HookType hook : { HookType::NONE, HookType::HOOK_45, HookType::HOOK_90T, HookType::ARROW_FILLED }) {
        SCOPED_TRACE(static_cast<int>(hook));
        m_line.setBeginHookType(hook);
        m_line.setEndHookType(hook);
        m_line.setLineVisible(true);
        m_segment.setPos2(PointF(30.0 * m_line.spatium(), 0.0));
        m_line.setSpannerSegments({ &m_segment });
        renderer.layoutItem(&m_line);
        m_line.setSpannerSegments({});
        ASSERT_GT(m_segment.ldata()->npoints, 0);

        m_line.setLineVisible(false);
        for (double length : { 30.0, 1.0 }) {
            SCOPED_TRACE(length);
            m_segment.setPos2(PointF(length * m_line.spatium(), 0.0));
            m_line.setSpannerSegments({ &m_segment });
            renderer.layoutItem(&m_line);
            m_line.setSpannerSegments({});
            const auto ld = m_segment.ldata();
            const RectF beginBounds = m_segment.text()->ldata()->bbox().translated(m_segment.text()->pos());
            const RectF middleBounds = m_segment.middleText()->ldata()->bbox().translated(m_segment.middleText()->pos());
            const RectF endBounds = m_segment.endText()->ldata()->bbox().translated(m_segment.endText()->pos());
            EXPECT_EQ(ld->npoints, 0);
            EXPECT_EQ(ld->shape().size(), 3u);
            for (const RectF& bounds : { beginBounds, middleBounds, endBounds }) {
                EXPECT_TRUE(ld->shape().bbox().contains(bounds));
            }
            if (length == 30.0) {
                EXPECT_NEAR(middleBounds.center().x(), m_segment.pos2().x() * 0.5, 1e-6);
                EXPECT_NEAR(middleBounds.center().y(), 0.0, 1e-6);
            } else {
                const double gap = m_line.absoluteFromSpatium(m_line.gapBetweenTextAndLine());
                EXPECT_NEAR(middleBounds.left() - beginBounds.right(), gap, 1e-6);
                EXPECT_NEAR(endBounds.left() - middleBounds.right(), gap, 1e-6);
            }
        }
    }
}

TEST_P(Engraving_MiddleTextOverflowTests, middleTextAfterOverlongBeginning)
{
    m_line.setDiagonal(GetParam().diagonal);
    const Align middleAlign = m_segment.middleText()->align();
    m_segment.setPos2(PointF(5.0, GetParam().endY) * m_line.spatium());
    m_line.setBeginText(u"BeginTextBeingMuchTooLongForTheAvailableLine");
    m_line.setEndText(u"End");
    m_line.setBeginTextAlign({ AlignH::LEFT, AlignV::BASELINE });
    m_line.setEndTextAlign({ AlignH::LEFT, AlignV::BASELINE });
    for (HookType hook : { HookType::NONE, HookType::HOOK_45, HookType::ARROW_FILLED }) {
        SCOPED_TRACE(static_cast<int>(hook));
        m_line.setBeginHookType(hook);
        m_line.setEndHookType(hook);
        const auto ld = layout();
        const RectF beginBounds = m_segment.text()->ldata()->bbox().translated(m_segment.text()->pos());
        const RectF middleBounds = m_segment.middleText()->ldata()->bbox().translated(m_segment.middleText()->pos());
        const RectF endBounds = m_segment.endText()->ldata()->bbox().translated(m_segment.endText()->pos());
        const double gap = m_line.absoluteFromSpatium(m_line.gapBetweenTextAndLine());
        EXPECT_GT(beginBounds.right(), m_segment.pos2().x());
        EXPECT_NEAR(middleBounds.left() - beginBounds.right(), gap, 1e-6);
        EXPECT_NEAR(endBounds.left() - middleBounds.right(), gap, 1e-6);
        EXPECT_EQ(m_segment.middleText()->align(), middleAlign);
        EXPECT_NEAR(m_segment.text()->ldata()->blocks.front().y(), 0.0, 1e-6);
        EXPECT_NEAR(m_segment.endText()->ldata()->blocks.front().y(), 0.0, 1e-6);
        EXPECT_NEAR(middleBounds.center().y(), 0.0, 1e-6);
        for (const RectF& bounds : { beginBounds, middleBounds, endBounds }) {
            EXPECT_TRUE(ld->shape().bbox().contains(bounds));
        }
        ASSERT_EQ(ld->npoints, 0);
        EXPECT_TRUE(ld->mask().empty());
    }
}

TEST_P(Engraving_MiddleTextOverflowTests, middleTextConsumesRemainingLine)
{
    m_line.setDiagonal(GetParam().diagonal);
    m_line.setBeginText(u"Begin");
    m_line.setMiddleText(u"Mid");
    m_line.setGapBetweenTextAndLine(Spatium(0.1));
    m_line.setEndText(u"End");
    m_line.setBeginTextAlign({ AlignH::LEFT, AlignV::VCENTER });
    m_line.setEndTextAlign({ AlignH::LEFT, AlignV::VCENTER });
    const Align middleAlign = m_segment.middleText()->align();
    const double endY = GetParam().endY * m_line.spatium();
    m_segment.setPos2(PointF(30.0 * m_line.spatium(), endY));
    ASSERT_GT(layout()->npoints, 0);
    const double gap = m_line.absoluteFromSpatium(m_line.gapBetweenTextAndLine());
    const double beginWidth = m_segment.text()->ldata()->bbox().width();
    const double middleWidth = m_segment.middleText()->ldata()->bbox().width();
    m_segment.setPos2(PointF(beginWidth + 2.0 * gap + middleWidth * 0.5, endY));
    const auto ld = layout();
    const RectF beginBounds = m_segment.text()->ldata()->bbox().translated(m_segment.text()->pos());
    const RectF middleBounds = m_segment.middleText()->ldata()->bbox().translated(m_segment.middleText()->pos());
    const RectF endBounds = m_segment.endText()->ldata()->bbox().translated(m_segment.endText()->pos());
    EXPECT_NEAR(middleBounds.left() - beginBounds.right(), gap, 1e-6);
    EXPECT_NEAR(endBounds.left() - middleBounds.right(), gap, 1e-6);
    EXPECT_NEAR(middleBounds.center().y(), beginBounds.center().y(), 1e-6);
    EXPECT_NEAR(middleBounds.center().y(), endBounds.center().y(), 1e-6);
    EXPECT_EQ(m_segment.middleText()->align(), middleAlign);
    EXPECT_EQ(ld->npoints, 0);
    EXPECT_TRUE(ld->mask().empty());
    for (const RectF& bounds : { beginBounds, middleBounds, endBounds }) {
        EXPECT_TRUE(ld->shape().bbox().contains(bounds));
    }
    m_segment.setPos2(PointF(30.0 * m_line.spatium(), endY));
    EXPECT_GT(layout()->npoints, 0);
}

INSTANTIATE_TEST_SUITE_P(ScoreAndSingle, Engraving_MiddleTextOverflowTests, ::testing::Values(
                             MiddleTextLayoutMode { false, false, 0.0 }, MiddleTextLayoutMode { true, false, 0.0 },
                             MiddleTextLayoutMode { false, true, -2.0 }, MiddleTextLayoutMode { true, true, -2.0 },
                             MiddleTextLayoutMode { false, true, 2.0 }, MiddleTextLayoutMode { true, true, 2.0 }));
