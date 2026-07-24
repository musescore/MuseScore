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

// Match Encore instrument names and MIDI programs to MuseScore instrument templates.

#include "mappers.h"

#include <algorithm>

#include <QRegularExpression>

#include "global/stringutils.h"

#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/part.h"

using namespace mu::engraving;

namespace mu::iex::enc {
// Strip trailing ordinal numbers from instrument names ("Bandurria 1ª" -> "Bandurria"; standalone "ª"/"º" also removed).
QString normalizeEncoreInstrName(const QString& name)
{
    QString s = name.trimmed();
    // Remove:  optional separator (space, - _ .) + digits + optional ordinal (ª º °).
    // The separator is optional so an attached part number is also stripped:
    // "Voz 1", "Voz-1", "Voz.1", "Bandurria 2ª", "Bandurria1", "Violin2" -> base name.
    static const QRegularExpression trailingNum(QStringLiteral("[\\s\\-_.]*\\d+[\xaa\xb0\xba]*$"));
    // Remove:  trailing ordinals with no preceding digit
    static const QRegularExpression trailingOrd(QStringLiteral("[\xaa\xb0\xba]+$"));
    // Only strip when a non-empty stem remains (so "1234" or "2ª" are left untouched).
    const QRegularExpressionMatch numMatch = trailingNum.match(s);
    if (numMatch.hasMatch() && numMatch.capturedStart() > 0) {
        s = s.left(numMatch.capturedStart()).trimmed();
    }
    s.remove(trailingOrd);
    return s.trimmed();
}

// Lowercase + accent-strip so "Laúd" matches "Laud" and "Percusión" matches "Percusion".
static QString normalizeForCompare(const QString& s)
{
    const QString d = s.normalized(QString::NormalizationForm_D);
    QString out;
    out.reserve(d.size());
    for (const QChar& ch : d) {
        if (ch.category() != QChar::Mark_NonSpacing) {
            out.append(ch.toLower());
        }
    }
    return out;
}

// Transposition compatibility: octave-only (chromatic%12==0) always passes (handled by pickStaffClef);
// non-octave requires matching mod-12 with encKey; rejects when Encore says C-instrument (encKey%12==0).
static bool transpCompatibleWith(int tmplChromatic, int encKeySemitones)
{
    if (tmplChromatic % 12 == 0) {
        return true;
    }
    if (encKeySemitones % 12 == 0) {
        return false;
    }
    const auto mod12 = [](int x) { return ((x % 12) + 12) % 12; };
    return mod12(tmplChromatic) == mod12(encKeySemitones);
}

// Minimum instrument name length for template search. Single-letter SATB labels
// ("S","A","T","B") match too broadly with substring scoring; skip them.
static constexpr int kMinInstrNameLen = 4;

// Instrument template matching scores (see findEncoreInstrumentTemplate).
static constexpr int kScoreTrackExact    = 4;  // trackName == needle
static constexpr int kScoreTrackContains = 2;  // trackName contains needle
static constexpr int kScoreLongExact     = 2;  // longName == needle
static constexpr int kScoreLongContains  = 1;  // longName contains needle
static constexpr int kScoreShortExact    = 1;  // shortName == needle
static constexpr int kScoreMidiMatch     = 6;  // MIDI program matches
static constexpr int kScoreCommonGenre   = 1;  // "common" genre tag

// Find best non-drumset template by name+MIDI score (weights above). With the encKeySemitones filter,
// prefers a transposition-compatible match; falls back to best name+MIDI match when none is compatible.
const InstrumentTemplate* findEncoreInstrumentTemplate(const QString& encName, int encMidiProgram,
                                                       int encKeySemitones, bool* outExactName,
                                                       bool* outUniqueName)
{
    if (outExactName) {
        *outExactName = false;
    }
    if (outUniqueName) {
        *outUniqueName = false;
    }
    if (encName.isEmpty()) {
        return nullptr;
    }

    if (encName.trimmed().size() < static_cast<qsizetype>(kMinInstrNameLen)) {
        return nullptr;
    }

    const QString norm = normalizeEncoreInstrName(encName);

    QStringList needles;
    auto addNeedle = [&](const QString& s) {
        const QString n = normalizeForCompare(s);
        if (!n.isEmpty() && !needles.contains(n)) {
            needles << n;
        }
    };
    // Add a needle plus its Romance singular stems (-s and -es); template matching picks the one
    // that exists, so a stem matching no template is simply never scored. Language-agnostic.
    auto addNeedleWithStems = [&](const QString& s) {
        addNeedle(s);
        const QString n = normalizeForCompare(s);
        if (n.endsWith(u's') && n.length() >= 5) {
            addNeedle(n.left(n.length() - 1));
        }
        if (n.endsWith(QStringLiteral("es")) && n.length() >= 6) {
            addNeedle(n.left(n.length() - 2));
        }
    };
    addNeedle(encName);
    addNeedleWithStems(norm);
    // Split words on any non-alphanumeric separator (space, - _ . and punctuation), keeping
    // accented letters within words. So "Voz-1"/"Vln.2"/"Sax_tenor" tokenize correctly.
    static const QRegularExpression wordSplit(QStringLiteral("[^\\p{L}\\p{N}]+"));
    for (const QString& word : norm.split(wordSplit, Qt::SkipEmptyParts)) {
        if (word.length() >= 4) {
            addNeedleWithStems(word);
        }
    }
    if (needles.isEmpty()) {
        return nullptr;
    }

    const bool filterTransp = (encKeySemitones != ENC_KEY_NO_FILTER);
    const InstrumentTemplate* best = nullptr;
    const InstrumentTemplate* bestCompatible = nullptr;
    int bestScore = 0;
    int bestCompatibleScore = 0;
    int bestNameStrength = 0;
    int bestCompatibleNameStrength = 0;
    bool bestExact = false;
    bool bestCompatibleExact = false;
    for (const InstrumentGroup* g : instrumentGroups) {
        for (const InstrumentTemplate* it : g->instrumentTemplates) {
            if (it->useDrumset) {
                continue;
            }
            const QString nt = normalizeForCompare(it->trackName.toQString());
            const QString nl = normalizeForCompare(it->instrumentName.longName().toQString());
            const QString ns = normalizeForCompare(it->instrumentName.shortName().toQString());
            int nameStrength = 0;
            bool exactHit = false;
            for (const QString& needle : needles) {
                int s = 0;
                if (nt == needle) {
                    s += kScoreTrackExact;
                    exactHit = true;
                } else if (nt.contains(needle)) {
                    s += kScoreTrackContains;
                }
                if (nl == needle) {
                    s += kScoreLongExact;
                    exactHit = true;
                } else if (nl.contains(needle)) {
                    s += kScoreLongContains;
                }
                if (ns == needle) {
                    s += kScoreShortExact;
                    exactHit = true;
                }
                if (s > nameStrength) {
                    nameStrength = s;
                }
            }
            if (nameStrength == 0) {
                continue;
            }
            int midiBonus = 0;
            if (encMidiProgram >= 0) {
                for (const InstrChannel& ch : it->channel) {
                    if (ch.program() == encMidiProgram) {
                        midiBonus = kScoreMidiMatch;
                        break;
                    }
                }
            }
            // "common" genre tag breaks ties between same-score templates (e.g. guitar-nylon vs soprano-guitar).
            int commonBonus = 0;
            for (const InstrumentGenre* gen : it->genres) {
                if (gen && gen->id == "common") {
                    commonBonus = kScoreCommonGenre;
                    break;
                }
            }
            const int score = nameStrength + midiBonus + commonBonus;
            if (score > bestScore
                || (score == bestScore && nameStrength > bestNameStrength)) {
                bestScore = score;
                bestNameStrength = nameStrength;
                best = it;
                bestExact = exactHit;
            }
            if (filterTransp && transpCompatibleWith(it->transpose.chromatic, encKeySemitones)) {
                if (score > bestCompatibleScore
                    || (score == bestCompatibleScore && nameStrength > bestCompatibleNameStrength)) {
                    bestCompatibleScore = score;
                    bestCompatibleNameStrength = nameStrength;
                    bestCompatible = it;
                    bestCompatibleExact = exactHit;
                }
            }
        }
    }
    // Last resort when neither exact nor substring matched: an edit-distance (Levenshtein) match
    // for typos and close inflections (>= 75% similar, <= 2 edits, both words >= 5 chars). Catches
    // near roots but not distant cross-language roots. A fuzzy hit unique to one template is as
    // distinctive as a unique substring (reported unique below so MIDI won't override); a shared
    // fuzzy hit stays non-unique and defers to MIDI.
    bool fuzzyUnique = false;
    if (!best) {
        static const QRegularExpression fuzzyWordSplit(QStringLiteral("[^\\p{L}\\p{N}]+"));
        constexpr double kFuzzyMinSim = 0.75;
        constexpr int kFuzzyMaxDist = 2;
        constexpr int kFuzzyMinLen = 5;
        double bestSim = 0.0;
        int fuzzyHits = 0;   // distinct templates reaching kFuzzyMinSim
        for (const InstrumentGroup* g : instrumentGroups) {
            for (const InstrumentTemplate* it : g->instrumentTemplates) {
                if (it->useDrumset) {
                    continue;
                }
                const QString nt = normalizeForCompare(it->trackName.toQString());
                const QString nl = normalizeForCompare(it->instrumentName.longName().toQString());
                double sim = 0.0;
                for (const QString& needle : needles) {
                    if (needle.length() < kFuzzyMinLen) {
                        continue;
                    }
                    for (const QString& field : { nt, nl }) {
                        for (const QString& w : field.split(fuzzyWordSplit, Qt::SkipEmptyParts)) {
                            if (w.length() < kFuzzyMinLen) {
                                continue;
                            }
                            const int dist = static_cast<int>(
                                muse::strings::levenshteinDistance(needle.toStdString(), w.toStdString()));
                            if (dist < 1 || dist > kFuzzyMaxDist) {
                                continue;
                            }
                            const double s = 1.0 - static_cast<double>(dist)
                                             / std::max(needle.length(), w.length());
                            sim = std::max(sim, s);
                        }
                    }
                }
                if (sim >= kFuzzyMinSim) {
                    ++fuzzyHits;
                }
                if (sim > bestSim) {
                    bestSim = sim;
                    best = it;
                    bestExact = false;
                }
            }
        }
        if (bestSim < kFuzzyMinSim) {
            best = nullptr;
        } else {
            fuzzyUnique = (fuzzyHits == 1);
        }
    }

    const bool useCompatible = filterTransp && bestCompatible;
    if (outExactName) {
        *outExactName = useCompatible ? bestCompatibleExact : bestExact;
    }

    const InstrumentTemplate* result = useCompatible ? bestCompatible : best;

    // A fuzzy match unique above the similarity threshold is as distinctive as a unique substring.
    if (outUniqueName && fuzzyUnique && result == best) {
        *outUniqueName = true;
    }

    // A contains-only match is weak in general, but a needle matched by exactly one template is
    // as confident as an exact match; report it so callers do not let MIDI override it. Ambiguous
    // needles that hit many templates stay weak and still defer to MIDI.
    if (outUniqueName && result) {
        auto needleMatches = [](const QString& needle, const InstrumentTemplate* it) {
            const QString nt = normalizeForCompare(it->trackName.toQString());
            const QString nl = normalizeForCompare(it->instrumentName.longName().toQString());
            const QString ns = normalizeForCompare(it->instrumentName.shortName().toQString());
            return nt == needle || nt.contains(needle)
                   || nl == needle || nl.contains(needle)
                   || ns == needle;
        };
        for (const QString& needle : needles) {
            if (!needleMatches(needle, result)) {
                continue;
            }
            int freq = 0;
            for (const InstrumentGroup* g : instrumentGroups) {
                for (const InstrumentTemplate* it : g->instrumentTemplates) {
                    if (!it->useDrumset && needleMatches(needle, it) && ++freq > 1) {
                        break;
                    }
                }
                if (freq > 1) {
                    break;
                }
            }
            if (freq == 1) {
                *outUniqueName = true;
                break;
            }
        }
    }

    return result;
}

// Find best drumset template by name score (exact match only, no substring).
const InstrumentTemplate* findDrumsetTemplate(const QString& encName)
{
    if (encName.trimmed().size() < static_cast<qsizetype>(kMinInstrNameLen)) {
        return nullptr;
    }

    const QString norm = normalizeEncoreInstrName(encName);
    QStringList needles;
    auto addNeedle = [&](const QString& s) {
        const QString n = normalizeForCompare(s);
        if (!n.isEmpty() && !needles.contains(n)) {
            needles << n;
        }
    };
    addNeedle(encName);
    addNeedle(norm);
    static const QRegularExpression wordSplit(QStringLiteral("[^\\p{L}\\p{N}]+"));
    for (const QString& word : norm.split(wordSplit, Qt::SkipEmptyParts)) {
        if (word.length() >= 4) {
            addNeedle(word);
        }
    }
    if (needles.isEmpty()) {
        return nullptr;
    }

    const InstrumentTemplate* best = nullptr;
    int bestScore = 0;
    for (const InstrumentGroup* g : instrumentGroups) {
        for (const InstrumentTemplate* it : g->instrumentTemplates) {
            if (!it->useDrumset) {
                continue;
            }
            const QString nt = normalizeForCompare(it->trackName.toQString());
            const QString nl = normalizeForCompare(
                it->instrumentName.longName().toQString());
            const QString ns = normalizeForCompare(
                it->instrumentName.shortName().toQString());
            int score = 0;
            for (const QString& needle : needles) {
                if (nt == needle) {
                    score += 4;
                }
                if (nl == needle) {
                    score += 2;
                }
                if (ns == needle) {
                    score += 1;
                }
            }
            if (score > bestScore) {
                bestScore = score;
                best = it;
            }
        }
    }
    return best;
}

// Strip a trailing parenthetical variant suffix: "Classical Guitar (tablature)" -> "Classical Guitar".
static QString stripVariantSuffix(const QString& trackName)
{
    QString s = trackName.trimmed();
    const int paren = s.lastIndexOf(u'(');
    if (paren > 0) {
        s = s.left(paren).trimmed();
    }
    return normalizeForCompare(s);
}

const InstrumentTemplate* findInstrumentVariant(const InstrumentTemplate* base, bool wantTab)
{
    if (!base) {
        return nullptr;
    }
    const bool baseIsTab = (base->staffGroup == StaffGroup::TAB);
    if (baseIsTab == wantTab) {
        return base;   // already the requested kind
    }
    const String baseXmlId = base->musicXmlId;
    const QString baseName = stripVariantSuffix(base->trackName.toQString());

    const InstrumentTemplate* best = nullptr;
    bool bestIsCommon = false;
    for (const InstrumentGroup* g : instrumentGroups) {
        for (const InstrumentTemplate* it : g->instrumentTemplates) {
            if (it->useDrumset) {
                continue;
            }
            if ((it->staffGroup == StaffGroup::TAB) != wantTab) {
                continue;
            }
            const bool sameByXmlId = !baseXmlId.isEmpty() && it->musicXmlId == baseXmlId;
            const bool sameByName = !baseName.isEmpty()
                                    && stripVariantSuffix(it->trackName.toQString()) == baseName;
            if (!sameByXmlId && !sameByName) {
                continue;
            }
            bool isCommon = false;
            for (const InstrumentGenre* gen : it->genres) {
                if (gen && gen->id == "common") {
                    isCommon = true;
                    break;
                }
            }
            if (!best || (isCommon && !bestIsCommon)) {
                best = it;
                bestIsCommon = isCommon;
            }
        }
    }
    return best;
}

const InstrumentTemplate* findTemplateByMidi(int encMidiProgram0indexed)
{
    if (encMidiProgram0indexed < 0) {
        return nullptr;
    }
    const InstrumentTemplate* best = nullptr;
    bool bestIsCommon = false;
    for (const InstrumentGroup* g : instrumentGroups) {
        for (const InstrumentTemplate* it : g->instrumentTemplates) {
            if (it->useDrumset || it->channel.empty()) {
                continue;
            }
            // Match only the first channel of each instrument. The first channel is the
            // instrument's primary sound; additional channels (tremolo, pizzicato, mute…)
            // are articulation variants that share programs across many instruments and
            // would produce false matches if included.
            if (it->channel.front().program() != encMidiProgram0indexed) {
                continue;
            }
            bool isCommon = false;
            for (const InstrumentGenre* gen : it->genres) {
                if (gen && gen->id == "common") {
                    isCommon = true;
                    break;
                }
            }
            if (!best || (isCommon && !bestIsCommon)) {
                best = it;
                bestIsCommon = isCommon;
            }
        }
    }
    return best;
}

const InstrumentTemplate* findTemplateByMidiFamily(int encMidiProgram0indexed)
{
    if (encMidiProgram0indexed < 0) {
        return nullptr;
    }
    // General MIDI groups its 128 programs into 16 families of 8. When no template has the exact
    // program as its primary sound, fall back to the nearest template within the same family so the
    // part keeps its instrument category instead of collapsing to Grand Piano.
    const int family = encMidiProgram0indexed / 8;
    const int familyFirst = family * 8;
    const int familyLast = familyFirst + 7;
    const InstrumentTemplate* best = nullptr;
    int bestDist = 1000;
    bool bestIsCommon = false;
    for (const InstrumentGroup* g : instrumentGroups) {
        for (const InstrumentTemplate* it : g->instrumentTemplates) {
            if (it->useDrumset || it->channel.empty()) {
                continue;
            }
            const int prog = it->channel.front().program();
            if (prog < familyFirst || prog > familyLast) {
                continue;
            }
            bool isCommon = false;
            for (const InstrumentGenre* gen : it->genres) {
                if (gen && gen->id == "common") {
                    isCommon = true;
                    break;
                }
            }
            const int dist = prog > encMidiProgram0indexed
                             ? prog - encMidiProgram0indexed : encMidiProgram0indexed - prog;
            // Prefer the nearest program; break ties toward the "common" genre.
            if (!best || dist < bestDist || (dist == bestDist && isCommon && !bestIsCommon)) {
                best = it;
                bestDist = dist;
                bestIsCommon = isCommon;
            }
        }
    }
    return best;
}
} // namespace mu::iex::enc
