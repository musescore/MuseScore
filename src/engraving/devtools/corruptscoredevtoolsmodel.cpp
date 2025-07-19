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

#include "corruptscoredevtoolsmodel.h"

using namespace mu::engraving;

CorruptScoreDevToolsModel::CorruptScoreDevToolsModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void CorruptScoreDevToolsModel::corruptOpenScore()
{
    project::INotationProjectPtr project = globalContext()->currentProject();
    if (!project) {
        return;
    }

    LOGW() << "Score corruption on demand!";

    mu::notation::INotationPtr notation = project->masterNotation()->notation();

    //: "Corrupt" is used as a verb here, i.e. "Make the current score corrupted" (for testing purposes).
    notation->undoStack()->prepareChanges(TranslatableString("undoableAction", "Corrupt score"));

    for (engraving::System* system : notation->elements()->msScore()->systems()) {
        for (engraving::MeasureBase* measureBase : system->measures()) {
            if (!measureBase->isMeasure()) {
                continue;
            }

            mu::engraving::Measure* measure = mu::engraving::toMeasure(measureBase);
            mu::engraving::Segment* firstSegment = measure->first(mu::engraving::SegmentType::ChordRest);

            for (mu::engraving::Segment* s = firstSegment; s; s = s->next(mu::engraving::SegmentType::ChordRest)) {
                mu::engraving::EngravingItem* element = s->element(0);
                if (!element) {
                    continue;
                }

                mu::engraving::ChordRest* cr = toChordRest(element);
                cr->undoChangeProperty(mu::engraving::Pid::DURATION, mu::engraving::Fraction::fromString(u"0/4"));
            }
        }
    }

    notation->undoStack()->commitChanges();
}
