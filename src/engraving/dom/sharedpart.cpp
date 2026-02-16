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
#include "sharedpart.h"
#include "score.h"

namespace mu::engraving {
SharedPart::SharedPart(Score* score)
    : Part(score, ElementType::SHARED_PART)
{
    setHideStavesWhenIndividuallyEmpty(true);
}

void SharedPart::addOriginPart(Part* p)
{
    DO_ASSERT(p->type() == ElementType::PART && !muse::contains(m_originParts, p));

    m_originParts.push_back(p);

    const std::vector<Part*>& parts = score()->parts();
    std::sort(m_originParts.begin(), m_originParts.end(), [&parts](Part* p1, Part* p2) {
        return muse::indexOf(parts, p1) < muse::indexOf(parts, p2);
    });

    p->m_sharedPart = this;
}

void SharedPart::removeOriginPart(Part* p)
{
    DO_ASSERT(muse::remove(m_originParts, p));

    p->m_sharedPart = nullptr;
}

mu::engraving::String mu::engraving::SharedPart::partName() const
{
    const Instrument* i = instrument();
    String fullName = i->longName();

    const String& transp = i->transposition();
    if (!transp.empty()) {
        //: For instrument transposition, e.g. Horn in F
        fullName += u" " + muse::mtrc("notation", "in") + u" " + transp;
    }

    int firstNumber = 10000;
    int lastNumber = 0;

    for (Part* originPart : m_originParts) {
        int number = originPart->instrument()->number();
        firstNumber = std::min(firstNumber, number);
        lastNumber = std::max(lastNumber, number);
    }

    if (firstNumber == lastNumber) {
        return fullName;
    }

    fullName += u" " + String::number(firstNumber) + u"-" + String::number(lastNumber);

    return fullName;
}
}
