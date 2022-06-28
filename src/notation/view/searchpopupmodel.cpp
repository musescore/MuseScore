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
#include "searchpopupmodel.h"

#include "log.h"

using namespace mu::notation;

void SearchPopupModel::load()
{
    dispatcher()->reg(this, "find", [this]() {
        emit showPopupRequested();
    });
}

//maybe add a reset function that jumps back to the noteInput cursor if the user presses escape

void SearchPopupModel::cancelPreview()
{
    notation()->interaction()->clearSelection();
}

void SearchPopupModel::finalize()
{
    notation()->interaction()->noteInput()->endNoteInput();
}

void SearchPopupModel::cancelSearch()
{
    if (notation()->interaction()->noteInput()->isNoteInputMode()) {
        notation()->interaction()->showItem(notation()->elements()->msScore()->inputState().segment()->findMeasureBase());
    }
}

bool SearchPopupModel::search(const QString& text)
{
    mu::engraving::EngravingItem* element = notation()->elements()->search(text.toStdString());
    if (element) {
        mu::engraving::EngravingItem* measure=nullptr;
        if (element->isPage()) {
            mu::engraving::Page* p = static_cast<mu::engraving::Page*>(element);
            measure = p->firstMeasure();
            if (!measure && !p->systems().empty() && !p->systems().front()->measures().empty()) {
                measure = p->systems().front()->measures().front();
            }
        } else if (element->isRehearsalMark()) {
            mu::engraving::RehearsalMark* r = static_cast<mu::engraving::RehearsalMark*>(element);
            measure = r->findMeasure();
        }
        if (!measure) {
            measure = element;
        }
        notation()->interaction()->select({ measure }, SelectType::SINGLE);
        notation()->interaction()->showItem(element);
        return true;
    }
    return false;
}

INotationPtr SearchPopupModel::notation() const
{
    return globalContext()->currentNotation();
}
