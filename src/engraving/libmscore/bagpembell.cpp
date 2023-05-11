/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "bagpembell.h"

#include "draw/types/pen.h"

#include "types/typesconv.h"
#include "layout/tlayout.h"
#include "iengravingfont.h"

#include "score.h"
#include "log.h"

using namespace mu;

namespace mu::engraving {
// Staff line and pitch for every bagpipe note
BagpipeNoteInfo BagpipeEmbellishment::BagpipeNoteInfoList[] = {
    { "LG",  6,  65 },
    { "LA",  5,  67 },
    { "B",   4,  69 },
    { "C",   3,  71 },  // actually C#
    { "D",   2,  72 },
    { "E",   1,  74 },
    { "F",   0,  76 },  // actually F#
    { "HG", -1,  77 },
    { "HA", -2,  79 }
};

//---------------------------------------------------------
//   getNoteList
//     return notes as list of indices in BagpipeNoteInfoList
//---------------------------------------------------------

noteList BagpipeEmbellishment::getNoteList() const
{
    noteList nl;

    StringList notes = TConv::embellishmentNotes(_embelType);
    int noteInfoSize = sizeof(BagpipeNoteInfoList) / sizeof(*BagpipeNoteInfoList);
    for (const String& note : notes) {
        // search for note in BagpipeNoteInfoList
        for (int i = 0; i < noteInfoSize; ++i) {
            if (String::fromAscii(BagpipeNoteInfoList[i].name.ascii()) == note) {
                // found it, append to list
                nl.push_back(i);
                break;
            }
        }
    }

    return nl;
}

//---------------------------------------------------------
//   BEDrawingDataX
//      BagpipeEmbellishment drawing data in the x direction
//      shared between ::draw() and ::layout()
//---------------------------------------------------------

BagpipeEmbellishment::BEDrawingDataX::BEDrawingDataX(SymId hs, SymId fs, const double m, const double s, const int nn)
    : headsym(hs), flagsym(fs), mags(0.75 * m), spatium(s), lw(0.1 * s), xcorr(0.1 * s)
{
    double w = Score::paletteScore()->engravingFont()->width(hs, mags);
    headw = 1.2 * w;         // using 1.0 the stem xpos is off
    headp = 1.6 * w;
    xl    = (1 - 1.6 * (nn - 1)) * w / 2;
}

//---------------------------------------------------------
//   BEDrawingDataY
//      BagpipeEmbellishment drawing data in the y direction
//      shared between ::draw() and ::layout()
//---------------------------------------------------------

BagpipeEmbellishment::BEDrawingDataY::BEDrawingDataY(const int l, const double s)
    : y1b(-8 * s / 2),
    y1f((l - 6) * s / 2),
    y2(l * s / 2),
    ycorr(0.8 * s),
    bw(0.3 * s) {}

//---------------------------------------------------------
//   debug support (disabled)
//---------------------------------------------------------

/*
static void printBBox(const char* name, const RectF b)
      {
      LOGD("bbox%s left %f bot %f right %f top %f",
             name,
             b.left(),
             b.bottom(),
             b.right(),
             b.top());
      }

static void symMetrics(const char* name, const Sym& headsym)
      {
      LOGD("%s", name);
      LOGD("bbox left %f bot %f right %f top %f",
             headsym.getBbox().left(),
             headsym.getBbox().bottom(),
             headsym.getBbox().right(),
             headsym.getBbox().top());
      LOGD("attach x %f y %f",
             headsym.getAttach().x(),
             headsym.getAttach().y());
      }
*/

//---------------------------------------------------------
//   mag
//      return fixed magnification
//---------------------------------------------------------

double BagpipeEmbellishment::mag() const
{
    return 0.7;
}

//---------------------------------------------------------
<<<<<<< HEAD
=======
//   layout
//      calculate and set bounding box
//---------------------------------------------------------

void BagpipeEmbellishment::layout()
{
    UNREACHABLE;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
}

//---------------------------------------------------------
>>>>>>> 4f8a1b6dd0... [engraving] replaced item->layout() to TLayout::layout
//   drawBeams
//      draw the beams
//      x1,y is one side of the top beam
//      x2,y is the other side of the top beam
//---------------------------------------------------------

static void drawBeams(mu::draw::Painter* painter, const double spatium,
                      const double x1, const double x2, double y)
{
    // draw the beams
    painter->drawLine(mu::LineF(x1, y, x2, y));
    y += spatium / 1.5;
    painter->drawLine(mu::LineF(x1, y, x2, y));
    y += spatium / 1.5;
    painter->drawLine(mu::LineF(x1, y, x2, y));
}

//---------------------------------------------------------
//   drawGraceNote
//      draw a single grace note in a palette cell
//      x,y1 is the top of the stem
//      x,y2 is the bottom of the stem
//---------------------------------------------------------

void BagpipeEmbellishment::drawGraceNote(mu::draw::Painter* painter,
                                         const BEDrawingDataX& dx,
                                         const BEDrawingDataY& dy,
                                         SymId flagsym, const double x, const bool drawFlag) const
{
    // draw head
    drawSymbol(dx.headsym, painter, mu::PointF(x - dx.headw, dy.y2));
    // draw stem
    double y1 =  drawFlag ? dy.y1f : dy.y1b;            // top of stems actually used
    painter->drawLine(mu::LineF(x - dx.lw * .5, y1, x - dx.lw * .5, dy.y2));
    if (drawFlag) {
        // draw flag
        drawSymbol(flagsym, painter, mu::PointF(x - dx.lw * .5 + dx.xcorr, y1 + dy.ycorr));
    }
}

//---------------------------------------------------------
//   draw
//      draw the embellishment centered in a palette cell
//      x = 0 is horizontal cell center
//      y = 0 is the top staff line
//---------------------------------------------------------

void BagpipeEmbellishment::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    using namespace mu::draw;
    SymId headsym = SymId::noteheadBlack;
    SymId flagsym = SymId::flag32ndUp;

    noteList nl = getNoteList();
    BEDrawingDataX dx(headsym, flagsym, magS(), score()->spatium(), static_cast<int>(nl.size()));

    Pen pen(curColor(), dx.lw, PenStyle::SolidLine, PenCapStyle::FlatCap);
    painter->setPen(pen);

    bool drawBeam = nl.size() > 1;
    bool drawFlag = nl.size() == 1;

    // draw the notes including stem, (optional) flag and (optional) ledger line
    double x = dx.xl;
    for (int note : nl) {
        int line = BagpipeNoteInfoList[note].line;
        BEDrawingDataY dy(line, score()->spatium());
        drawGraceNote(painter, dx, dy, flagsym, x, drawFlag);

        // draw the ledger line for high A
        if (line == -2) {
            painter->drawLine(mu::LineF(x - dx.headw * 1.5 - dx.lw * .5, dy.y2, x + dx.headw * .5 - dx.lw * .5, dy.y2));
        }

        // move x to next note x position
        x += dx.headp;
    }

    if (drawBeam) {
        // beam drawing setup
        BEDrawingDataY dy(0, score()->spatium());
        Pen beamPen(curColor(), dy.bw, PenStyle::SolidLine, PenCapStyle::FlatCap);
        painter->setPen(beamPen);
        // draw the beams
        drawBeams(painter, dx.spatium, dx.xl - dx.lw * .5, x - dx.headp - dx.lw * .5, dy.y1b);
    }
}
}
