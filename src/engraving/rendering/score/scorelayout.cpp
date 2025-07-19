/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include "scorelayout.h"

#include "dom/score.h"
#include "dom/masterscore.h"
#include "dom/system.h"
#include "dom/page.h"

#include "layoutcontext.h"

#include "pagelayout.h"
#include "scorepageviewlayout.h"
#include "scorehorizontalviewlayout.h"
#include "scoreverticalviewlayout.h"

#include "dumplayoutdata.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

class CmdStateLocker
{
    Score* m_score = nullptr;
public:
    CmdStateLocker(Score* s)
        : m_score(s) { m_score->cmdState().lock(); }
    ~CmdStateLocker() { m_score->cmdState().unlock(); }
};

void ScoreLayout::layoutRange(Score* score, const Fraction& st, const Fraction& et)
{
    TRACEFUNC;

    CmdStateLocker cmdStateLocker(score);
    LayoutContext ctx(score);

    Fraction stick(st);
    Fraction etick(et);
    assert(!(stick == Fraction(-1, 1) && etick == Fraction(-1, 1)));

    // Check empty score
    if (!score->last() || (ctx.conf().isLinearMode() && !score->firstMeasure())) {
        LOGD() << "empty score";
        muse::DeleteAll(score->systems());
        score->systems().clear();
        muse::DeleteAll(score->pages());
        score->pages().clear();
        PageLayout::getNextPage(ctx);
        return;
    }

    // Check range
    bool isLayoutAll = stick <= Fraction(0, 1) && (etick < Fraction(0, 1) || etick >= score->masterScore()->last()->endTick());
    if (stick < Fraction(0, 1)) {
        stick = Fraction(0, 1);
    }
    if (etick < Fraction(0, 1)) {
        etick = score->last()->endTick();
    }

    ctx.mutState().setIsLayoutAll(isLayoutAll);

    // Init context and layout
    switch (ctx.conf().viewMode()) {
    case LayoutMode::PAGE:
    case LayoutMode::FLOAT:
        ScorePageViewLayout::layoutPageView(score, ctx, stick, etick);
        break;
    case LayoutMode::LINE:
    case LayoutMode::HORIZONTAL_FIXED:
        ScoreHorizontalViewLayout::layoutHorizontalView(score, ctx, stick, etick);
        break;
    case LayoutMode::SYSTEM:
        ScoreVerticalViewLayout::layoutVerticalView(score, ctx, stick, etick);
        break;
    }

    //LOGDA() << DumpLayoutData::dump(score);
}
