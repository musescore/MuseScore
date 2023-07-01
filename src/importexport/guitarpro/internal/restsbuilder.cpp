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
#include "restsbuilder.h"

#include "libmscore/rest.h"
#include "libmscore/segment.h"
#include "libmscore/score.h"
#include "libmscore/measure.h"

using namespace mu::engraving;

namespace mu::iex::guitarpro {
RestsBuilder::RestsBuilder(Score* score, size_t voices)
    : m_score(score), m_voices(voices)
{
    resetState();

    m_chordExistsForVoice.resize(m_voices);
    m_lastTickForVoice.resize(m_voices);
    m_restsToAdd.resize(m_voices);
}

RestsBuilder::~RestsBuilder()
{
    clearRests();
}

bool RestsBuilder::validState() const
{
    return m_score && m_measure && (m_startTrack < m_score->nstaves() * m_voices);
}

void RestsBuilder::clearRests()
{
    /// here releasing memory for rests which will not be added to score
    /// the memory was allocated outside of this class (bad design),
    /// that's because we assume by default that every rest will be added to score
    for (auto& restsToAdd : m_restsToAdd) {
        for (auto& [segment, restToAdd] : restsToAdd) {
            auto& [rest, added] = restToAdd;
            if (!added && rest) {
                delete rest;
                rest = nullptr;
            }
        }
    }

    m_restsToAdd.clear();
}

void RestsBuilder::resetState()
{
    clearRests();
    m_restsToAdd.resize(m_voices);
    std::fill(m_chordExistsForVoice.begin(), m_chordExistsForVoice.end(), false);
    std::fill(m_lastTickForVoice.begin(), m_lastTickForVoice.end(), Fraction());
    m_simileMarkInBar = false;
    m_chordExistsInBar = false;
}

void RestsBuilder::setMeasure(mu::engraving::Measure* measure)
{
    m_measure = measure;
}

void RestsBuilder::setMultiVoice(bool multiVoice)
{
    m_multiVoice = multiVoice;
}

RestsBuilder::RestsStateInMeasure RestsBuilder::addRestsToScore(track_idx_t trackIdx)
{
    if (!validState()) {
        LOGE() << "no score or measure specified";
        return RestsStateInMeasure::ERROR;
    }

    size_t voice = trackIdx % m_voices;

    if (m_simileMarkInBar) {
        return RestsStateInMeasure::SIMILE_MARK;
    }

    Fraction offsetFromLastChord;
    bool restsAdded = addRestsToSegments(voice);

    // if this voice has chord or rest, then offset to fill uncomplete measure should be updated,
    // otherwise the offset should be kept in the beginning of the measure
    if (restsAdded || m_chordExistsForVoice.at(voice)) {
        offsetFromLastChord = m_lastTickForVoice.at(voice);
    } else {
        offsetFromLastChord = m_measure->tick();
    }

    int tickOffset = m_measure->ticks().ticks() + m_measure->tick().ticks() - offsetFromLastChord.ticks();
    if (tickOffset > 0) {
        m_score->setRest(offsetFromLastChord, trackIdx, Fraction::fromTicks(tickOffset), true, nullptr);
        return RestsStateInMeasure::UNCOMPLETE;
    }

    return RestsStateInMeasure::COMPLETE;
}

void RestsBuilder::setStartTrack(track_idx_t trackIdx)
{
    m_startTrack = trackIdx;
    resetState();
}

bool RestsBuilder::addRestsToSegments(size_t voice)
{
    IF_ASSERT_FAILED(voice < m_chordExistsForVoice.size() && m_chordExistsForVoice.size() == m_restsToAdd.size()) {
        return false;
    }

    if (!m_chordExistsForVoice[voice] || m_restsToAdd[voice].empty()) {
        return false;
    }

    for (auto& [segment, restToAdd] : m_restsToAdd[voice]) {
        auto& [rest, added] = restToAdd;
        segment->add(rest);
        added = true;
    }

    return true;
}

void RestsBuilder::hideRestsInEmptyMeasures()
{
    for (Segment* segment = m_measure->first(SegmentType::ChordRest); segment; segment = segment->next(SegmentType::ChordRest)) {
        for (track_idx_t trackIdx = m_startTrack; trackIdx < m_startTrack + m_voices; trackIdx++) {
            EngravingItem* element = segment->element(trackIdx);
            if (!element || !element->isRest()) {
                continue;
            }

            Rest* rest = toRest(element);
            size_t voice = trackIdx % m_voices;
            bool mainVoice = (voice == 0);

            // hiding rests in secondary voices for measures without any chords
            if (!m_chordExistsInBar) {
                rest->setGap(!mainVoice);
                continue;
            }

            // hiding rests in voices without chords, but leaving voice 0 for "single-voice" mode
            if (!m_chordExistsForVoice[voice]) {
                rest->setGap(!mainVoice || m_multiVoice);
            }
        }
    }
}

void RestsBuilder::addRest(Rest* rest, Segment* segment)
{
    m_restsToAdd[rest->track() % m_voices][segment] = { rest, false };
}

void RestsBuilder::notifyChordExists(size_t voice)
{
    assert(voice < m_voices);
    m_chordExistsForVoice[voice] = true;
    m_chordExistsInBar = true;
}

void RestsBuilder::notifySimileMarkExists()
{
    m_simileMarkInBar = true;
}

void RestsBuilder::updateLastTick(size_t voice, Fraction tick)
{
    m_lastTickForVoice[voice] = tick;
}
} // namespace mu::iex::guitarpro
