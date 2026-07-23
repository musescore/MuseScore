/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "corruptscoredevtoolsmodel.h"

#include "engraving/dom/measurebase.h"
#include "engraving/dom/score.h"

#include "notation/imasternotation.h" // IWYU pragma: keep
#include "notation/inotation.h"
#include "notation/inotationelements.h" // IWYU pragma: keep
#include "notation/inotationundostack.h" // IWYU pragma: keep

#include "project/inotationproject.h"

using namespace mu::engraving;

CorruptScoreDevToolsModel::CorruptScoreDevToolsModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void CorruptScoreDevToolsModel::corruptOpenScore()
{
    project::INotationProjectPtr project = globalContext()->currentProject();
    if (!project) {
        return;
    }

    LOGW() << "Score corruption on demand!";

    notation::INotationPtr notation = project->masterNotation()->notation();
    Score* score = notation->elements()->msScore();

    //: "Corrupt" is used as a verb here, i.e. "Make the current score corrupted" (for testing purposes).
    notation->undoStack()->transaction(TranslatableString("undoableAction", "Corrupt score"), [&](auto&) {
        for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
            Segment* firstSegment = measure->first(SegmentType::ChordRest);

            for (Segment* s = firstSegment; s; s = s->next(SegmentType::ChordRest)) {
                EngravingItem* element = s->element(0);
                if (!element) {
                    continue;
                }

                ChordRest* cr = toChordRest(element);
                cr->undoChangeProperty(Pid::DURATION, Fraction(0, 4));
            }
        }
    });
}
