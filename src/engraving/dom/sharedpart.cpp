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
#include "sharedpart.h"
#include "score.h"

namespace mu::engraving {
SharedPart::SharedPart(Score* score)
    : Part(score, ElementType::SHARED_PART)
{
}

void SharedPart::addOriginPart(Part* p)
{
    DO_ASSERT(p->type() == ElementType::PART && !muse::contains(m_originParts, p));

    const std::vector<Part*>& parts = score()->parts();
    if (muse::contains(parts, p)) {
        std::unordered_map<Part*, size_t> order;
        for (size_t i = 0; i < parts.size(); ++i) {
            order[parts[i]] = i;
        }

        auto it = std::lower_bound(m_originParts.begin(), m_originParts.end(), p, [&order](Part* part1, Part* part2) {
            return order[part1] < order[part2];
        });

        m_originParts.insert(it, p);
    } else {
        // Can happen while reading files as p has not been added to the score yet
        m_originParts.push_back(p);
    }

    p->setSharedPart(this);
}

void SharedPart::removeOriginPart(Part* p)
{
    DO_ASSERT(muse::remove(m_originParts, p));

    p->setSharedPart(nullptr);
}

const SharedTrackMap& SharedPart::trackMapAtTick(const Fraction& tick) const
{
    static constexpr Fraction TICK_ZERO = Fraction(0, 1);

    IF_ASSERT_FAILED(m_trackMapsByTick.size() > 0) {
        const_cast<SharedPart*>(this)->m_trackMapsByTick[TICK_ZERO] = SharedTrackMap();
        return m_trackMapsByTick.at(TICK_ZERO);
    }

    DO_ASSERT(muse::contains(m_trackMapsByTick, TICK_ZERO));

    IF_ASSERT_FAILED(tick.positive()) {
        return m_trackMapsByTick.at(TICK_ZERO);
    }

    auto upperBound = m_trackMapsByTick.upper_bound(tick);
    DO_ASSERT(upperBound != m_trackMapsByTick.begin());

    --upperBound;

    return upperBound->second;
}

void SharedPart::setTrackMapAtTick(const SharedTrackMap& map, const Fraction& tick)
{
    m_trackMapsByTick[tick] = map;
}

void mu::engraving::SharedPart::removeMapsBetweenTicks(const Fraction& startTick, const Fraction& endTick)
{
    auto upperBound = m_trackMapsByTick.lower_bound(startTick);
    if (upperBound == m_trackMapsByTick.end()) {
        return;
    }

    for (auto iter = upperBound; iter != m_trackMapsByTick.end();) {
        if (iter->first >= endTick) {
            break;
        }
        iter = m_trackMapsByTick.erase(iter);
    }
}

mu::engraving::String mu::engraving::SharedPart::partName() const
{
    const Instrument* i = instrument();
    String fullName = i->longName();

    const String& transp = i->transposition();
    if (!transp.empty()) {
        //: For instrument transposition, e.g. Horn in F
        fullName += u" " + muse::mtrc("engraving", "in") + u" " + transp;
    }

    int firstNumber = 10000;
    int lastNumber = 0;

    for (const Part* originPart : originParts()) {
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

PropertyValue SharedPart::getProperty(Pid pid) const
{
    switch (pid) {
    case Pid::SHARED_PART_ENABLED:
        return m_enabled;
    default:
        return Part::getProperty(pid);
    }
}

PropertyValue SharedPart::propertyDefault(Pid pid) const
{
    switch (pid) {
    case Pid::SHARED_PART_ENABLED:
        return true;
    default:
        return Part::propertyDefault(pid);
    }
}

bool SharedPart::setProperty(Pid pid, const PropertyValue& v)
{
    switch (pid) {
    case Pid::SHARED_PART_ENABLED:
        m_enabled = v.toBool();
        break;
    default:
        return Part::setProperty(pid, v);
    }

    score()->setLayoutAll();
    return true;
}

bool SharedPart::enabled() const
{
    return m_enabled && style().styleB(Sid::enableStaveSharing);
}

bool SharedPart::show() const
{
    return m_show && m_enabled && style().styleB(Sid::enableStaveSharing);
}
}
