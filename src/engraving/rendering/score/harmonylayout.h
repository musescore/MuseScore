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
#pragma once

#include "layoutcontext.h"
#include "dom/harmony.h"

namespace mu::engraving {
class Segment;
class System;
}

namespace mu::engraving::rendering::score {
class HarmonyLayout
{
public:
    static void layoutHarmony(Harmony* item, Harmony::LayoutData* ldata, const LayoutContext& ctx);

private:

    struct HarmonyRenderCtx {
        PointF pos = PointF();
        std::vector<TextSegment*> textList;

        // Reset every single chord
        bool hAlign = true;
        HarmonyInfo* info = nullptr;

        // Reset every render() call
        std::stack<PointF> stack;
        int tpc = Tpc::TPC_INVALID;
        NoteSpellingType noteSpelling = NoteSpellingType::STANDARD;
        NoteCaseType noteCase = NoteCaseType::AUTO;
        double scale = 1.0;

        double x() const { return pos.x(); }
        double y() const { return pos.y(); }

        void setx(double v) { pos.setX(v); }
        void movex(double v) { pos.setX(pos.x() + v); }
        void sety(double v) { pos.setY(v); }
        void movey(double v) { pos.setY(pos.y() + v); }
    };

    static void render(Harmony* item, Harmony::LayoutData* ldata, const LayoutContext& ctx);
    static void renderSingleHarmony(Harmony* item, Harmony::LayoutData* ldata, HarmonyRenderCtx& harmonyCtx, const LayoutContext& ctx);
    static void renderRomanNumeral(Harmony* item, Harmony::LayoutData* ldata, const LayoutContext& ctx);
    static void render(Harmony* item, Harmony::LayoutData* ldata, const String& str, HarmonyRenderCtx& harmonyCtx);
    static void render(Harmony* item, Harmony::LayoutData* ldata, SymId sym, HarmonyRenderCtx& harmonyCtx, const LayoutContext& ctx);
    static void render(Harmony* item, Harmony::LayoutData* ldata, const std::list<RenderActionPtr>& renderList, HarmonyRenderCtx& ctx,
                       int tpc, NoteSpellingType noteSpelling = NoteSpellingType::STANDARD, NoteCaseType noteCase = NoteCaseType::AUTO,
                       double noteMag = 1.0);

    static void renderAction(Harmony* item, Harmony::LayoutData* ldata, const RenderActionPtr& a, HarmonyRenderCtx& harmonyCtx);
    static void renderActionSet(Harmony* item, Harmony::LayoutData* ldata, const RenderActionSetPtr& a, HarmonyRenderCtx& harmonyCtx);
    static void renderActionMove(Harmony* item, const RenderActionMovePtr& a, HarmonyRenderCtx& harmonyCtx);
    static void renderActionMoveXHeight(Harmony* item, const RenderActionMoveXHeightPtr& a, HarmonyRenderCtx& harmonyCtx);
    static void renderActionPush(HarmonyRenderCtx& harmonyCtx);
    static void renderActionPop(const RenderActionPopPtr& a, HarmonyRenderCtx& harmonyCtx);
    static void renderActionNote(Harmony* item, Harmony::LayoutData* ldata, HarmonyRenderCtx& harmonyCtx);
    static void renderActionAcc(Harmony* item, Harmony::LayoutData* ldata, HarmonyRenderCtx& harmonyCtx);
    static void renderActionAlign(HarmonyRenderCtx& harmonyCtx);
    static void renderActionScale(const RenderActionScalePtr& a, HarmonyRenderCtx& harmonyCtx);
};
}
