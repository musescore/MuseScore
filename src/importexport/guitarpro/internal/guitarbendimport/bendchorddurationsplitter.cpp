/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
#include "bendchorddurationsplitter.h"

#include "log.h"

using namespace mu::engraving;

namespace mu::iex::guitarpro {
constexpr int MAX_DENOMINATOR = 64;
constexpr Fraction ONE_DOT_DURATION_MULTIPLIER(3, 2);
constexpr Fraction TWO_DOTS_DURATION_MULTIPLIER(7, 4);

static Fraction findClosestDisplayableDuration(const Fraction& totalDuration, const Fraction& targetDuration, int maxDenominator,
                                               bool skipFirstBest);
static std::vector<Fraction> fillDurationsByProportions(const Fraction& totalDuration, const std::vector<Fraction>& proportions,
                                                        int maxDenominator, bool skipFirstBest);
static bool canBeRepresentedAsDottedNote(const Fraction& duration, int maxDenominator);
static std::vector<Fraction> bruteForceSplit(const Fraction& totalDuration, const std::vector<Fraction>& proportions, int maxDenominator);
static std::vector<Fraction> bruteForceSplitInTwo(const Fraction& target, const std::vector<Fraction>& proportions,
                                                  const std::vector<Fraction>& durations);
static std::vector<Fraction> bruteForceSplitInThree(const Fraction& target, const std::vector<Fraction>& durations);

static std::vector<Fraction> generateDurations(int maxDenominator);

std::vector<Fraction> BendChordDurationSplitter::findValidNoteSplit(const Fraction& totalDuration, const std::vector<Fraction>& proportions,
                                                                    int maxDenominator)
{
    if (proportions.size() == 1) {
        std::vector<Fraction> unchangedDurations;
        unchangedDurations.push_back(totalDuration);
        return unchangedDurations;
    }

    bool splitReached = false;
    bool skipFirstBest = false;
    std::vector<Fraction> durations;

    do {
        durations = fillDurationsByProportions(totalDuration, proportions, maxDenominator, skipFirstBest);

        Fraction accumulatedDuration = std::accumulate(durations.begin(), durations.end(), Fraction(0, 1));
        Fraction lastDuration = totalDuration - accumulatedDuration;

        if (lastDuration.isNotZero() && !lastDuration.negative() && canBeRepresentedAsDottedNote(lastDuration, maxDenominator)) {
            durations.push_back(lastDuration);
            splitReached = true;
        } else if (!skipFirstBest) {
            skipFirstBest = true;
        } else {
            durations = bruteForceSplit(totalDuration, proportions, maxDenominator);
            splitReached = true;
        }
    } while (!splitReached);

    return durations;
}

static std::vector<Fraction> fillDurationsByProportions(const Fraction& totalDuration, const std::vector<Fraction>& proportions,
                                                        int maxDenominator, bool skipFirstBest)
{
    std::vector<Fraction> durations;

    for (size_t i = 0; i < proportions.size() - 1; i++) {
        Fraction targetDuration = totalDuration * proportions[i];
        Fraction closestDuration = findClosestDisplayableDuration(totalDuration, targetDuration, maxDenominator,
                                                                  i == 0 ? skipFirstBest : false);
        durations.push_back(closestDuration);
    }

    return durations;
}

static std::vector<Fraction> bruteForceSplit(const Fraction& totalDuration, const std::vector<Fraction>& proportions, int maxDenominator)
{
    auto durations = generateDurations(maxDenominator);
    std::sort(durations.begin(), durations.end());

    if (proportions.size() == 2) {
        return bruteForceSplitInTwo(totalDuration, proportions, durations);
    }

    return bruteForceSplitInThree(totalDuration, durations);
}

static std::vector<Fraction> bruteForceSplitInTwo(const Fraction& target, const std::vector<Fraction>& proportions,
                                                  const std::vector<Fraction>& durations)
{
    if (proportions.front() < proportions.back()) {
        for (auto it1 = durations.begin(); it1 != durations.end(); ++it1) {
            for (auto it2 = it1; it2 != durations.end(); ++it2) {
                if (*it1 + *it2 == target) {
                    return { *it1, *it2 };
                }
            }
        }
    } else {
        for (auto it1 = durations.rbegin(); it1 != durations.rend(); ++it1) {
            for (auto it2 = it1; it2 != durations.rend(); ++it2) {
                if (*it1 + *it2 == target) {
                    return { *it1, *it2 };
                }
            }
        }
    }

    return {};
}

static std::vector<Fraction> bruteForceSplitInThree(const Fraction& target, const std::vector<Fraction>& durations)
{
    for (auto it1 = durations.begin(); it1 != durations.end(); ++it1) {
        for (auto it2 = it1; it2 != durations.end(); ++it2) {
            for (auto it3 = it2; it3 != durations.end(); ++it3) {
                if (*it1 + *it2 + *it3 == target) {
                    return { *it1, *it2, *it3 };
                }
            }
        }
    }

    return {};
}

static std::vector<Fraction> generateDurations(int maxDenominator)
{
    std::vector<Fraction> durations;
    for (int denom = 1; denom <= std::min(maxDenominator, MAX_DENOMINATOR); denom *= 2) {
        durations.emplace_back(1, denom);
        durations.emplace_back(ONE_DOT_DURATION_MULTIPLIER * denom);
        durations.emplace_back(TWO_DOTS_DURATION_MULTIPLIER * denom);
    }
    return durations;
}

static Fraction findClosestDisplayableDuration(const Fraction& totalDuration, const Fraction& targetDuration, int requestedDenominator,
                                               bool skipFirstBest)
{
    Fraction closestFraction = Fraction(0, 1);
    double minDifference = std::numeric_limits<double>::max();
    int numerator = targetDuration.numerator();
    int denominator = targetDuration.denominator();
    const Fraction smallestFraction(1, requestedDenominator);

    bool firstBestSkipped = false;

    for (int candidateNumerator = 1; candidateNumerator < requestedDenominator; candidateNumerator++) {
        double originalFraction = static_cast<double>(numerator) / denominator;
        double candidateFraction = static_cast<double>(candidateNumerator) / requestedDenominator;

        double difference = std::fabs(originalFraction - candidateFraction);

        if (difference < minDifference) {
            Fraction resultCandidateFraction(candidateNumerator, requestedDenominator);
            resultCandidateFraction = std::max(resultCandidateFraction, smallestFraction);
            resultCandidateFraction = std::min(resultCandidateFraction, totalDuration - smallestFraction);

            if (canBeRepresentedAsDottedNote(resultCandidateFraction, requestedDenominator)) {
                if (skipFirstBest && !firstBestSkipped) {
                    firstBestSkipped = true;
                } else {
                    minDifference = difference;
                    closestFraction = resultCandidateFraction;
                }
            }
        }
    }

    return closestFraction.reduced();
}

static bool canBeRepresentedAsDottedNote(const Fraction& duration, int maxDenominator)
{
    int denom = 1;
    while (denom <= std::min(maxDenominator, MAX_DENOMINATOR)) {
        Fraction base(1, denom);
        if (duration == base || duration == base * ONE_DOT_DURATION_MULTIPLIER || duration == base * TWO_DOTS_DURATION_MULTIPLIER) {
            return true;
        }

        denom *= 2;
    }

    return false;
}
} // namespace mu::iex::guitarpro
