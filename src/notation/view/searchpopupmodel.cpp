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
#include "searchpopupmodel.h"

#include "log.h"

using namespace mu::notation;

void SearchPopupModel::load()
{
    dispatcher()->reg(this, "find", [this]() {
        emit showPopupRequested();
    });
}

void SearchPopupModel::search(const QString& text)
{
    auto elements = notation()->elements()->search(text);
    if (!elements.empty() && std::find(elements.begin(), elements.end(), nullptr) == elements.end()) {
        notation()->interaction()->select(elements, elements.size() == 1 ? SelectType::SINGLE : SelectType::RANGE);
        notation()->interaction()->showItem(elements.front());
    } else {
        notation()->interaction()->clearSelection();
    }
}

INotationPtr SearchPopupModel::notation() const
{
    return globalContext()->currentNotation();
}
