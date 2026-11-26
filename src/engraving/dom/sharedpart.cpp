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
#include "staff.h"

#include <algorithm>
#include <limits>
#include <sstream>
#include <utility>

namespace mu::engraving {
std::string dump(const SharedTrackMap& map)
{
    std::stringstream ss;
    ss << "SharedTrackMap size: " << map.size() << " {";
    for (auto it = map.begin(); it != map.end(); ++it) {
        if (it != map.begin()) {
            ss << ", ";
        }
        ss << it->first << ": " << it->second;
    }
    ss << "}";
    return ss.str();
}

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

    computeIsSameInstruments();
}

void SharedPart::removeOriginPart(Part* p)
{
    DO_ASSERT(muse::remove(m_originParts, p));

    p->setSharedPart(nullptr);

    computeIsSameInstruments();
}

void SharedPart::sortOriginParts()
{
    const std::vector<Part*>& parts = score()->parts();
    std::unordered_map<Part*, size_t> order;
    for (size_t i = 0; i < parts.size(); ++i) {
        order[parts[i]] = i;
    }

    std::sort(m_originParts.begin(), m_originParts.end(), [&order](Part* part1, Part* part2) {
        return order.at(part1) < order.at(part2);
    });
}

Part* SharedPart::mappedOriginPart(track_idx_t sharedTrack, const Fraction& tick) const
{
    const SharedTrackMap& trackMap = staveSharingStateAtTick(tick.negative() ? Fraction(0, 1) : tick).sharedTrackMap();

    track_idx_t originTrack = muse::key(trackMap, sharedTrack, muse::nidx);

    const Staff* originStaff = originTrack != muse::nidx ? score()->staff(track2staff(originTrack)) : nullptr;
    return originStaff ? originStaff->part() : nullptr;
}

const Part* SharedPart::originPart(track_idx_t sharedTrack, const Fraction& tick) const
{
    if (sharedTrack != muse::nidx) {
        if (const Part* p = mappedOriginPart(sharedTrack, tick)) {
            return p;
        }
    }

    return m_originParts.empty() ? this : m_originParts.front();
}

Part* SharedPart::originPartForStaff(staff_idx_t sharedStaffIdx, const Fraction& tick) const
{
    return mappedOriginPart(staff2track(sharedStaffIdx, 0), tick);
}

const StaveSharingState& SharedPart::staveSharingStateAtTick(const Fraction& tick) const
{
    static constexpr Fraction TICK_ZERO = Fraction(0, 1);

    IF_ASSERT_FAILED(m_staveSharingStateByTick.size() > 0) {
        const_cast<SharedPart*>(this)->m_staveSharingStateByTick[TICK_ZERO] = StaveSharingState();
        return m_staveSharingStateByTick.at(TICK_ZERO);
    }

    DO_ASSERT(muse::contains(m_staveSharingStateByTick, TICK_ZERO));

    IF_ASSERT_FAILED(tick.positive()) {
        return m_staveSharingStateByTick.at(TICK_ZERO);
    }

    auto upperBound = m_staveSharingStateByTick.upper_bound(tick);
    DO_ASSERT(upperBound != m_staveSharingStateByTick.begin());

    --upperBound;

    return upperBound->second;
}

StaveSharingState& SharedPart::staveSharingStateAtTick(const Fraction& tick)
{
    return const_cast<StaveSharingState&>(std::as_const(*this).staveSharingStateAtTick(tick));
}

std::optional<StaveSharingState> SharedPart::exactStaveSharingStateAtTick(const Fraction& tick) const
{
    auto it = m_staveSharingStateByTick.find(tick);
    if (it == m_staveSharingStateByTick.end()) {
        return std::nullopt;
    }

    return it->second;
}

void SharedPart::setStaveSharingStateAtTick(const StaveSharingState& state, const Fraction& tick)
{
    m_staveSharingStateByTick[tick] = state;
}

void SharedPart::removeStaveSharingStateAtTick(const Fraction& tick)
{
    m_staveSharingStateByTick.erase(tick);

    // We must always have a state at the start of the score
    if (tick == Fraction(0, 1)) {
        setStaveSharingStateAtTick(StaveSharingState(), tick);
    }
}

void SharedPart::removeStaveSharingStatesBetweenTicks(const Fraction& startTick, const Fraction& endTick, bool removeUserChanges)
{
    auto lowerBound = m_staveSharingStateByTick.lower_bound(startTick);
    if (lowerBound == m_staveSharingStateByTick.end()) {
        return;
    }

    for (auto iter = lowerBound; iter != m_staveSharingStateByTick.end();) {
        if (iter->first >= endTick) {
            break;
        }
        if (iter->second.isUserModified() && !removeUserChanges) {
            ++iter;
            continue;
        }
        iter = m_staveSharingStateByTick.erase(iter);
    }
}

std::map<Fraction, StaveSharingState> SharedPart::staveSharingStatesBetweenTicks(const Fraction& startTick,
                                                                                 const Fraction& endTick) const
{
    std::map<Fraction, StaveSharingState> result;

    auto lowerBound = m_staveSharingStateByTick.lower_bound(startTick);
    for (auto iter = lowerBound; iter != m_staveSharingStateByTick.end() && iter->first < endTick; ++iter) {
        result.emplace(iter->first, iter->second);
    }

    return result;
}

Fraction SharedPart::nextUserModifiedStaveSharingTick(const Fraction& tick) const
{
    for (auto iter = m_staveSharingStateByTick.upper_bound(tick); iter != m_staveSharingStateByTick.end(); ++iter) {
        if (iter->second.isUserModified()) {
            return iter->first;
        }
    }

    return score()->endTick();
}

bool SharedPart::isSameInstrumentsAtTick(const Fraction& tick)
{
    if (m_originParts.empty()) {
        return false;
    }

    const Instrument* instr = m_originParts.front()->instrument(tick);
    for (Part* p : m_originParts) {
        if (p->instrument(tick)->id() != instr->id()) {
            return false;
        }
    }

    return true;
}

void SharedPart::computeIsSameInstruments()
{
    m_isSameInstruments = isSameInstrumentsAtTick(Fraction(0, 1));
}

static std::pair<String, int> splitTrailingNumber(const String& name)
{
    // "Trumpet 2" -> { "Trumpet", 2 }; "Flutes" -> { "Flutes", 0 }
    size_t start = name.size();
    while (start > 0 && name.at(start - 1).isDigit()) {
        --start;
    }

    if (start == name.size()) {
        return { name, 0 };
    }

    return { name.left(start).trimmed(), name.mid(start).toInt() };
}

String SharedPart::partName() const
{
    String commonName;
    bool hasMixedNames = false;
    int firstNumber = std::numeric_limits<int>::max();
    int lastNumber = std::numeric_limits<int>::min();

    for (const Part* originPart : originParts()) {
        const auto [name, nameNumber] = splitTrailingNumber(originPart->partName().simplified());

        // The instrument's number; else one in its name (e.g. "Violins 1" from a template)
        const int instrumentNumber = originPart->instrument()->number();
        const int number = instrumentNumber > 0 ? instrumentNumber : nameNumber;

        if (commonName.empty()) {
            commonName = name;
        } else if (name != commonName) {
            hasMixedNames = true;
        }

        if (number > 0) {
            firstNumber = std::min(firstNumber, number);
            lastNumber = std::max(lastNumber, number);
        }
    }

    // no numbers, a single number, or different instruments
    if (hasMixedNames || commonName.empty() || firstNumber >= lastNumber) {
        return instrumentName();
    }

    // e.g. "Trumpet 1–3"
    return commonName + u" " + String::number(firstNumber) + u"–" + String::number(lastNumber);
}

String SharedPart::instrumentName() const
{
    const Instrument* i = instrument();
    String name = i->longName();

    const String& transp = i->transposition();
    if (!transp.empty()) {
        //: For instrument transposition, e.g. Horn in F
        name += u" " + muse::mtrc("engraving", "in") + u" " + transp;
    }

    return name;
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

void SharedPart::insertTime(const Fraction& tick, const Fraction& len)
{
    if (len.isZero()) {
        return;
    }

    if (len < Fraction(0, 1)) {
        m_staveSharingStateByTick.erase(m_staveSharingStateByTick.lower_bound(tick), m_staveSharingStateByTick.lower_bound(tick - len));
    }

    std::map<Fraction, StaveSharingState> newSharedStaves;

    for (auto s = m_staveSharingStateByTick.lower_bound(tick); s != m_staveSharingStateByTick.end();) {
        StaveSharingState state = s->second;
        Fraction t = s->first;
        m_staveSharingStateByTick.erase(s++);
        newSharedStaves[t + len] = state;
    }
    m_staveSharingStateByTick.insert(newSharedStaves.begin(), newSharedStaves.end());

    if (!muse::contains(m_staveSharingStateByTick, Fraction(0, 1))) {
        m_staveSharingStateByTick.emplace(Fraction(0, 1), StaveSharingState());
    }

    Part::insertTime(tick, len);
}
}
