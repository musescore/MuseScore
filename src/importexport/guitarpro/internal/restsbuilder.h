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

#ifndef MU_IMPORTEXPORT_RESTSBUILDER_H
#define MU_IMPORTEXPORT_RESTSBUILDER_H

#include <unordered_map>
#include <array>

#include "types/fraction.h"
#include "types/types.h"

namespace mu::engraving {
class Rest;
class Segment;
class Measure;
class Score;
} // mu::engraving

namespace mu::iex::guitarpro {
/**
 * Collects rests instead of adding them directly to the score
 * Collected rests are later deleted if they won't be added to score
 *
 * Collecting rests only in 1 measure of 1 staff,
 * when moving to another staff "setStartTrack" should be called to reset the builder
 */
class RestsBuilder
{
public:

    RestsBuilder(mu::engraving::Score* score, size_t voices);
    ~RestsBuilder();

    enum class RestsStateInMeasure {
        ERROR = -1,
        COMPLETE = 0,
        UNCOMPLETE,
        SIMILE_MARK
    };

    using track_idx_t = mu::engraving::track_idx_t;

    RestsBuilder::RestsStateInMeasure addRestsToScore(track_idx_t trackIdx);

    void setMultiVoice(bool multiVoice);
    void setMeasure(mu::engraving::Measure* measure);
    void setStartTrack(track_idx_t trackIdx);

    void addRest(mu::engraving::Rest* rest, mu::engraving::Segment* segment);
    void notifyChordExists(size_t voice);
    void notifySimileMarkExists();
    void updateLastTick(size_t voice, mu::engraving::Fraction tick);

    void hideRestsInEmptyMeasures();

private:

    bool addRestsToSegments(track_idx_t trackIdx);
    void resetState();
    void clearRests();
    bool validState() const;

    mu::engraving::Score* m_score = nullptr;
    mu::engraving::Measure* m_measure = nullptr;
    bool m_simileMarkInBar = false;
    bool m_chordExistsInBar = false;
    std::vector<bool> m_chordExistsForVoice;
    std::vector<mu::engraving::Fraction> m_lastTickForVoice;

    struct RestToAdd {
        mu::engraving::Rest* rest = nullptr;
        bool added = false;
    };

    std::vector<std::unordered_map<mu::engraving::Segment*, RestToAdd> > m_restsToAdd;

    size_t m_voices = 0;
    bool m_multiVoice = false;
    track_idx_t m_startTrack = 0;
};
} // mu::iex::guitarpro
#endif // MU_IMPORTEXPORT_RESTSBUILDER_H
