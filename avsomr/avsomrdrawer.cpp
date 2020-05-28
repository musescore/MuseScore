//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "avsomrdrawer.h"

#include <QPainter>

#include "libmscore/score.h"
#include "libmscore/measure.h"

#include "avslog.h"

using namespace Ms::Avs;

AvsOmrDrawer::AvsOmrDrawer()
{
}

//---------------------------------------------------------
//   makeContext
//---------------------------------------------------------

std::shared_ptr<AvsOmrDrawer::Context> AvsOmrDrawer::makeContext(QPainter* p, const Ms::Score* score) const
{
    IF_ASSERT(p) {
        return nullptr;
    }

    const Ms::MasterScore* ms = score->masterScore();
    IF_ASSERT(ms) {
        return nullptr;
    }

    std::shared_ptr<AvsOmr> omr = ms->avsOmr();
    if (!omr) {
        return nullptr;
    }

    std::shared_ptr<Context> ctx = std::make_shared<Context>();
    ctx->omr = omr;
    ctx->p = p;

    //! TODO Think about how to do it better.
    Avs::AvsOmr::Idx idx{ 0 };
    for (Ms::Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
        ctx->measureIdxs.insert(m, idx);
        ++idx;
    }

    return ctx;
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void AvsOmrDrawer::draw(std::shared_ptr<Context> ctx, const QList<const Ms::Measure*>& ml)
{
    IF_ASSERT(ctx) {
        return;
    }

    if (ctx->omr->config().isHiddenAll()) {
        return;
    }

    ctx->p->save();

    for (const Ms::Measure* m : ml) {
        ctx->ormMeasureIdx = ctx->measureIdxs.value(m);
        ctx->ormSheetNum = ctx->omr->sheetNumByMeausereIdx(ctx->ormMeasureIdx);

        MMetrics mm;
        mm.bbox = m->bbox();

        auto setMSegment = [](MSegment& ms, const Ms::Segment* s) {
                               ms.pos = s->pos();
                               ms.w = s->width();
                           };

        for (Ms::Segment* s = m->first(); s; s = s->next()) {
            if (Ms::SegmentType::Clef == s->segmentType() || Ms::SegmentType::HeaderClef == s->segmentType()) {
                setMSegment(mm.clef, s);
            } else if (Ms::SegmentType::KeySig == s->segmentType()) {
                setMSegment(mm.key, s);
            } else if (Ms::SegmentType::TimeSig == s->segmentType()) {
                setMSegment(mm.time, s);
            } else if (Ms::SegmentType::ChordRest == s->segmentType()) {
                setMSegment(mm.fchord, s);
                break;         //! NOTE The first chord and we don’t need anything else
            }
        }

        QPointF ppos = m->pagePos();
        ctx->p->translate(ppos);
        drawMeasure(ctx, mm);
        ctx->p->translate(-ppos);
    }

    ctx->p->restore();
}

//---------------------------------------------------------
//   drawMeasure
//---------------------------------------------------------

void AvsOmrDrawer::drawMeasure(std::shared_ptr<Context> ctx, const MMetrics& scoreMM) const
{
    AvsOmr::MMetrics omm = ctx->omr->mmetrics(ctx->ormSheetNum, ctx->ormMeasureIdx);

    QRectF scoreMH = scoreMM.headerBBox();
    QRectF scoreMCH = scoreMM.chordBBox();

    QRect ormMH = omm.headerBBox();
    ormMH.setRight(ormMH.right() + 30);
    QRect ormMCH = omm.chordBBox();

    int topGap = omm.ebbox.top() - omm.bbox.top();
    int bottomGap = omm.ebbox.bottom() - omm.bbox.bottom();

    drawGlyphs(ctx, ormMH, topGap, bottomGap, scoreMH);
    ctx->p->translate(scoreMCH.left(), 0);
    drawGlyphs(ctx, ormMCH, topGap, bottomGap, scoreMCH);
    ctx->p->translate(-scoreMCH.left(), 0);
}

//---------------------------------------------------------
//   drawGlyphs
//---------------------------------------------------------

void AvsOmrDrawer::drawGlyphs(std::shared_ptr<Context> ctx,
                              const QRect omrBB,
                              int topGap, int bottomGap,
                              const QRectF scoreBB) const
{
    QTransform originTransform = ctx->p->transform();

    // Scale
    qreal sx = scoreBB.width() / static_cast<qreal>(omrBB.width());
    qreal sy = scoreBB.height() / static_cast<qreal>(omrBB.height());

    QTransform transform = originTransform;
    transform.scale(sx, sy);
    ctx->p->setTransform(transform);

    QRect abb = omrBB.adjusted(0, topGap, 0, bottomGap);

    QList<AvsOmr::GlyphUsed> accepted;
    if (ctx->omr->config().isShowRecognized()) {
        accepted << AvsOmr::GlyphUsed::Used;
    }

    if (ctx->omr->config().isShowNotRecognized()) {
        accepted << AvsOmr::GlyphUsed::Free;
    }

    QList<const AvsOmr::Glyph*> glys = ctx->omr->glyphsByBBox(ctx->ormSheetNum, abb, accepted);
    for (const auto& g : glys) {
        QRect gb = g->bbox.translated(-omrBB.left(), -omrBB.top());
        ctx->p->drawImage(gb.topLeft(), g->img);
    }

    ctx->p->setTransform(originTransform);
}

//---------------------------------------------------------
//   nextColor (for debug)
//---------------------------------------------------------

QColor AvsOmrDrawer::nextColor() const
{
    static int cf = static_cast<int>(Qt::darkGray);
    static int cl = static_cast<int>(Qt::darkYellow);
    static int ci = cf;

    ++ci;
    if (ci > cl) {
        ci = cf;
    }

    return QColor(static_cast<Qt::GlobalColor>(ci));
}

//---------------------------------------------------------
//   drawBBox (for debug)
//---------------------------------------------------------

void AvsOmrDrawer::drawBBox(QPainter* p, const QRect& r, Qt::GlobalColor colr) const
{
    QPen pen;
    pen.setColor(QColor(colr));
    pen.setWidth(8);
    p->setPen(pen);
    p->drawRect(r);
}

//---------------------------------------------------------
//   drawMSegment (for debug)
//---------------------------------------------------------

void AvsOmrDrawer::drawMSegment(QPainter* p, const MSegment& s, Qt::GlobalColor colr) const
{
    if (s.isNull()) {
        return;
    }

    QPen pen;
    pen.setColor(QColor(colr));
    pen.setWidth(20);

    p->setPen(pen);
    p->drawLine(s.pos, QPoint(s.pos.x() + s.w, s.pos.y()));
}
