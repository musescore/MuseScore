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
#include "layoutlyrics.h"

#include "style/styledef.h"

#include "libmscore/system.h"
#include "libmscore/segment.h"
#include "libmscore/score.h"
#include "libmscore/chordrest.h"
#include "libmscore/lyrics.h"

using namespace mu;
using namespace mu::engraving;
using namespace Ms;

//---------------------------------------------------------
//   findLyricsMaxY
//---------------------------------------------------------

static qreal findLyricsMaxY(Segment& s, int staffIdx)
{
    qreal yMax = 0.0;
    if (!s.isChordRestType()) {
        return yMax;
    }

    qreal lyricsMinTopDistance = s.score()->styleP(Sid::lyricsMinTopDistance);

    for (int voice = 0; voice < VOICES; ++voice) {
        ChordRest* cr = s.cr(staffIdx * VOICES + voice);
        if (cr && !cr->lyrics().empty()) {
            SkylineLine sk(true);

            for (Lyrics* l : cr->lyrics()) {
                if (l->autoplace() && l->placeBelow()) {
                    qreal yOff = l->offset().y();
                    PointF offset = l->pos() + cr->pos() + s.pos() + s.measure()->pos();
                    RectF r = l->bbox().translated(offset);
                    r.translate(0.0, -yOff);
                    sk.add(r.x(), r.top(), r.width());
                }
            }
            SysStaff* ss = s.measure()->system()->staff(staffIdx);
            for (Lyrics* l : cr->lyrics()) {
                if (l->autoplace() && l->placeBelow()) {
                    qreal y = ss->skyline().south().minDistance(sk);
                    if (y > -lyricsMinTopDistance) {
                        yMax = qMax(yMax, y + lyricsMinTopDistance);
                    }
                }
            }
        }
    }
    return yMax;
}

//---------------------------------------------------------
//   findLyricsMinY
//---------------------------------------------------------

static qreal findLyricsMinY(Segment& s, int staffIdx)
{
    qreal yMin = 0.0;
    if (!s.isChordRestType()) {
        return yMin;
    }
    qreal lyricsMinTopDistance = s.score()->styleP(Sid::lyricsMinTopDistance);
    for (int voice = 0; voice < VOICES; ++voice) {
        ChordRest* cr = s.cr(staffIdx * VOICES + voice);
        if (cr && !cr->lyrics().empty()) {
            SkylineLine sk(false);

            for (Lyrics* l : cr->lyrics()) {
                if (l->autoplace() && l->placeAbove()) {
                    qreal yOff = l->offset().y();
                    RectF r = l->bbox().translated(l->pos() + cr->pos() + s.pos() + s.measure()->pos());
                    r.translate(0.0, -yOff);
                    sk.add(r.x(), r.bottom(), r.width());
                }
            }
            SysStaff* ss = s.measure()->system()->staff(staffIdx);
            for (Lyrics* l : cr->lyrics()) {
                if (l->autoplace() && l->placeAbove()) {
                    qreal y = sk.minDistance(ss->skyline().north());
                    if (y > -lyricsMinTopDistance) {
                        yMin = qMin(yMin, -y - lyricsMinTopDistance);
                    }
                }
            }
        }
    }
    return yMin;
}

static qreal findLyricsMaxY(Measure* m, int staffIdx)
{
    qreal yMax = 0.0;
    for (Segment& s : m->segments()) {
        yMax = qMax(yMax, findLyricsMaxY(s, staffIdx));
    }
    return yMax;
}

static qreal findLyricsMinY(Measure* m, int staffIdx)
{
    qreal yMin = 0.0;
    for (Segment& s : m->segments()) {
        yMin = qMin(yMin, findLyricsMinY(s, staffIdx));
    }
    return yMin;
}

//---------------------------------------------------------
//   applyLyricsMax
//---------------------------------------------------------

static void applyLyricsMax(Segment& s, int staffIdx, qreal yMax)
{
    if (!s.isChordRestType()) {
        return;
    }
    Skyline& sk = s.measure()->system()->staff(staffIdx)->skyline();
    for (int voice = 0; voice < VOICES; ++voice) {
        ChordRest* cr = s.cr(staffIdx * VOICES + voice);
        if (cr && !cr->lyrics().empty()) {
            qreal lyricsMinBottomDistance = s.score()->styleP(Sid::lyricsMinBottomDistance);
            for (Lyrics* l : cr->lyrics()) {
                if (l->autoplace() && l->placeBelow()) {
                    l->rypos() += yMax - l->propertyDefault(Pid::OFFSET).value<PointF>().y();
                    if (l->addToSkyline()) {
                        PointF offset = l->pos() + cr->pos() + s.pos() + s.measure()->pos();
                        sk.add(l->bbox().translated(offset).adjusted(0.0, 0.0, 0.0, lyricsMinBottomDistance));
                    }
                }
            }
        }
    }
}

static void applyLyricsMax(Measure* m, int staffIdx, qreal yMax)
{
    for (Segment& s : m->segments()) {
        applyLyricsMax(s, staffIdx, yMax);
    }
}

//---------------------------------------------------------
//   applyLyricsMin
//---------------------------------------------------------

static void applyLyricsMin(ChordRest* cr, int staffIdx, qreal yMin)
{
    Skyline& sk = cr->measure()->system()->staff(staffIdx)->skyline();
    for (Lyrics* l : cr->lyrics()) {
        if (l->autoplace() && l->placeAbove()) {
            l->rypos() += yMin - l->propertyDefault(Pid::OFFSET).value<PointF>().y();
            if (l->addToSkyline()) {
                PointF offset = l->pos() + cr->pos() + cr->segment()->pos() + cr->segment()->measure()->pos();
                sk.add(l->bbox().translated(offset));
            }
        }
    }
}

static void applyLyricsMin(Measure* m, int staffIdx, qreal yMin)
{
    for (Segment& s : m->segments()) {
        if (s.isChordRestType()) {
            for (int voice = 0; voice < VOICES; ++voice) {
                ChordRest* cr = s.cr(staffIdx * VOICES + voice);
                if (cr) {
                    applyLyricsMin(cr, staffIdx, yMin);
                }
            }
        }
    }
}

//---------------------------------------------------------
//   layoutLyrics
//
//    vertical align lyrics
//
//---------------------------------------------------------

void LayoutLyrics::layoutLyrics(const LayoutOptions& options, const Score* score, System* system)
{
    std::vector<int> visibleStaves;
    for (int staffIdx = system->firstVisibleStaff(); staffIdx < score->nstaves(); staffIdx = system->nextVisibleStaff(staffIdx)) {
        visibleStaves.push_back(staffIdx);
    }

    //int nAbove[nstaves()];
    std::vector<int> VnAbove(score->nstaves());

    for (int staffIdx : visibleStaves) {
        VnAbove[staffIdx] = 0;
        for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure()) {
                continue;
            }
            Measure* m = toMeasure(mb);
            for (Segment& s : m->segments()) {
                if (s.isChordRestType()) {
                    for (int voice = 0; voice < VOICES; ++voice) {
                        ChordRest* cr = s.cr(staffIdx * VOICES + voice);
                        if (cr) {
                            int nA = 0;
                            for (Lyrics* l : cr->lyrics()) {
                                // user adjusted offset can possibly change placement
                                if (l->offsetChanged() != OffsetChange::NONE) {
                                    Placement p = l->placement();
                                    l->rebaseOffset();
                                    if (l->placement() != p) {
                                        l->undoResetProperty(Pid::AUTOPLACE);
                                        //l->undoResetProperty(Pid::OFFSET);
                                        //l->layout();
                                    }
                                }
                                l->setOffsetChanged(false);
                                if (l->placeAbove()) {
                                    ++nA;
                                }
                            }
                            VnAbove[staffIdx] = qMax(VnAbove[staffIdx], nA);
                        }
                    }
                }
            }
        }
    }

    for (int staffIdx : visibleStaves) {
        for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure()) {
                continue;
            }
            Measure* m = toMeasure(mb);
            for (Segment& s : m->segments()) {
                if (s.isChordRestType()) {
                    for (int voice = 0; voice < VOICES; ++voice) {
                        ChordRest* cr = s.cr(staffIdx * VOICES + voice);
                        if (cr) {
                            for (Lyrics* l : cr->lyrics()) {
                                l->layout2(VnAbove[staffIdx]);
                            }
                        }
                    }
                }
            }
        }
    }

    switch (options.verticalAlignRange) {
    case VerticalAlignRange::MEASURE:
        for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure()) {
                continue;
            }
            Measure* m = toMeasure(mb);
            for (int staffIdx : visibleStaves) {
                qreal yMax = findLyricsMaxY(m, staffIdx);
                applyLyricsMax(m, staffIdx, yMax);
            }
        }
        break;
    case VerticalAlignRange::SYSTEM:
        for (int staffIdx : visibleStaves) {
            qreal yMax = 0.0;
            qreal yMin = 0.0;
            for (MeasureBase* mb : system->measures()) {
                if (!mb->isMeasure()) {
                    continue;
                }
                yMax = qMax<qreal>(yMax, findLyricsMaxY(toMeasure(mb), staffIdx));
                yMin = qMin(yMin, findLyricsMinY(toMeasure(mb), staffIdx));
            }
            for (MeasureBase* mb : system->measures()) {
                if (!mb->isMeasure()) {
                    continue;
                }
                applyLyricsMax(toMeasure(mb), staffIdx, yMax);
                applyLyricsMin(toMeasure(mb), staffIdx, yMin);
            }
        }
        break;
    case VerticalAlignRange::SEGMENT:
        for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure()) {
                continue;
            }
            Measure* m = toMeasure(mb);
            for (int staffIdx : visibleStaves) {
                for (Segment& s : m->segments()) {
                    qreal yMax = findLyricsMaxY(s, staffIdx);
                    applyLyricsMax(s, staffIdx, yMax);
                }
            }
        }
        break;
    }
}
