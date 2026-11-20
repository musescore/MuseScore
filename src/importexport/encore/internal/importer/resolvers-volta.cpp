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

// Post-pass: put every volta bracket back on the measures it covers.

#include "resolvers.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/volta.h"

using namespace mu::engraving;

namespace mu::iex::enc {
void resolveVoltas(BuildCtx& ctx)
{
    if (ctx.pendingVoltas.empty()) {
        return;
    }

    MasterScore* score = ctx.score;

    // Measures still change length after a bracket is built, and every later tick moves with them, so the
    // ticks are read off the measures now, when none can change again.
    std::vector<Volta*> toRemove;
    for (const PendingVolta& pv : ctx.pendingVoltas) {
        if (!pv.volta) {
            continue;
        }
        if (!pv.firstMeasure || !pv.lastMeasure) {
            toRemove.push_back(pv.volta);
            continue;
        }
        pv.volta->setTick(pv.firstMeasure->tick());
        pv.volta->setTick2(pv.lastMeasure->endTick());
        pv.volta->computeStartElement();
        pv.volta->computeEndElement();
        // Without both ends the bracket cannot be laid out or written, so drop it rather than
        // leave a half-anchored spanner in the score.
        if (!pv.volta->startElement() || !pv.volta->endElement()) {
            toRemove.push_back(pv.volta);
        }
    }
    for (Volta* v : toRemove) {
        score->removeElement(v);
    }
}
} // namespace mu::iex::enc
