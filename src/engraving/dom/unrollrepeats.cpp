/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

/**
 File handling: loading and saving.
 */

#include "style/style.h"

#include "barline.h"
#include "engravingitem.h"
#include "excerpt.h"
#include "masterscore.h"
#include "measure.h"
#include "measurebase.h"
#include "part.h"
#include "repeatlist.h"
#include "score.h"
#include "segment.h"
#include "undo.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
static void removeRepeatMarkings(Score* score)
{
    // remove bar-level repeats
    for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
        m->setRepeatStart(false);
        m->setRepeatEnd(false);
    }

    // remove volta markers
    auto smap = score->spannerMap().map();
    for (auto it = smap.cbegin(); it != smap.cend(); ++it) {
        Spanner* s = (*it).second;
        if (!s || !s->isVolta()) {
            continue;
        }
        score->removeSpanner(s);
    }

    // remove coda/fine labels and jumps
    std::vector<EngravingItem*> elems;
    score->scanElements(&elems, collectElements, false);
    for (auto e : elems) {
        if (e->isMarker() || e->isJump()) {
            score->deleteItem(e);
        } else if (e->isBarLine()) {
            toBarLine(e)->setBarLineType(BarLineType::NORMAL);
        }
    }

    // set the last bar line to end symbol
    score->lastMeasure()->setEndBarLineType(BarLineType::END, false);
    Segment* last = score->lastMeasure()->segments().last();
    if (last->segmentType() == SegmentType::EndBarLine) {
        auto els = last->elist();
        for (size_t i = 0; i < els.size(); i++) {
            if (!els[i]) {
                continue;
            }
            toBarLine(els[i])->setBarLineType(BarLineType::END);
        }
    }
}

//---------------------------------------------------------
//   createExcerpts
//    re-create all the excerpts once the master score
//    has been unrolled
//---------------------------------------------------------

static void createExcerpts(MasterScore* cs, const std::list<Excerpt*>& excerpts)
{
    // borrowed from musescore.cpp endsWith(".pdf")
    for (Excerpt* e : excerpts) {
        Score* nscore = e->masterScore()->createScore();
        e->setExcerptScore(nscore);
        nscore->style().set(Sid::createMultiMeasureRests, true);
        cs->startCmd(TranslatableString("undoableAction", "Create parts"));
        cs->undo(new AddExcerpt(e));
        Excerpt::createExcerpt(e);

        // borrowed from excerptsdialog.cpp
        // a new excerpt is created in AddExcerpt, make sure the parts are filed
        for (Excerpt* ee : e->masterScore()->excerpts()) {
            if (ee->excerptScore() == nscore && ee != e) {
                ee->parts().clear();
                ee->parts().insert(ee->parts().end(), e->parts().begin(), e->parts().end());
            }
        }

        cs->endCmd();
    }
}

//---------------------------------------------------------
//   unrollRepeats
//    unroll all the repeats
//---------------------------------------------------------

MasterScore* MasterScore::unrollRepeats()
{
    MasterScore* original = this;

    // create a copy of the original score to play with
    MasterScore* score = original->clone();

    // TODO: Give it an appropriate path/filename
    NOT_IMPLEMENTED;

    // figure out repeat structure
    original->setExpandRepeats(true);

    // if no repeats, just return the score as-is
    if (original->repeatList().size() == 1) {
        return score;
    }

    // remove excerpts for now (they are re-created after unrolling master score)
    std::list<Excerpt*> excerpts;
    for (Excerpt* e : score->excerpts()) {
        excerpts.push_back(new Excerpt(*e, false));
        score->masterScore()->deleteExcerpt(e);
    }

    // follow along with the repeatList
    bool first = true;
    for (const RepeatSegment* rs: original->repeatList()) {
        Fraction startTick = Fraction::fromTicks(rs->tick);
        Fraction endTick   = Fraction::fromTicks(rs->tick + rs->len());

        // first segment left from clone, everything past that removed
        if (first) {
            if (endTick <= score->lastMeasure()->tick()) {     // check if we actually need to remove any measures
                score->deleteMeasures(score->tick2measure(endTick), score->lastMeasure());
            }
            first = false;
        } else {  // just append this section from the original to the new score
            score->appendMeasuresFromScore(original, startTick, endTick);
        }
    }

    removeRepeatMarkings(score);

    score->setUpTempoMap();

    score->setLayoutAll();
    score->doLayout();

    // re-create excerpt parts
    if (!excerpts.empty()) {
        createExcerpts(score, excerpts);
    }

    return score;
}
}
