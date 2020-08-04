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

#ifndef AVS_AVSOMRDRAWER_H
#define AVS_AVSOMRDRAWER_H

#include "avsomr.h"

namespace Ms {
class Score;
class Measure;

namespace Avs {
class AvsOmrDrawer
{
public:
    AvsOmrDrawer();

    struct Context {
        QPainter* p{ nullptr };
        std::shared_ptr<AvsOmr> omr;
        QHash<const Ms::Measure*, AvsOmr::Idx> measureIdxs;
        AvsOmr::Num ormSheetNum{ 0 };
        AvsOmr::Idx ormMeasureIdx{ 0 };
    };

    std::shared_ptr<Context> makeContext(QPainter* p, const Ms::Score* score) const;

    void draw(std::shared_ptr<Context> ctx, const QList<const Ms::Measure*>& ml);

private:

    struct MSegment {
        QPointF pos;
        qreal w{ 0 };
        bool isNull() const { return qFuzzyIsNull(w); }
    };

    struct MMetrics {
        QRectF bbox;
        MSegment clef;
        MSegment key;
        MSegment time;
        MSegment fchord;

        qreal headerW() const
        {
            return clef.w + key.w + time.w;
        }

        QRectF headerBBox() const
        {
            QRectF h = bbox;
            h.setRight(headerW());
            return h;
        }

        QRectF chordBBox() const
        {
            QRectF c = bbox;
            c.setLeft(headerW());
            return c;
        }
    };

    void drawMeasure(std::shared_ptr<Context> ctx, const MMetrics& scoreMM) const;
    void drawGlyphs(std::shared_ptr<Context> ctx,const QRect omrBB,int topGap, int bottomGap,const QRectF scoreBB) const;

    // debug
    QColor nextColor() const;
    void drawBBox(QPainter* p, const QRect& r, Qt::GlobalColor colr) const;
    void drawMSegment(QPainter* p, const MSegment& s, Qt::GlobalColor colr) const;
};
} // Avs
} // Ms

#endif // AVS_AVSOMRDRAWER_H
