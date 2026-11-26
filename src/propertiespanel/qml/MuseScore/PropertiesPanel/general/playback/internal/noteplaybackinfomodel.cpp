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
#include "noteplaybackinfomodel.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <map>

#include <QFont>
#include <QRawFont>
#include <QRegularExpression>

#include "translation.h"

#include "engraving/automation/dynamicvalues.h"
#include "engraving/dom/arpeggio.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/breath.h"
#include "engraving/dom/capo.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/fermata.h"
#include "engraving/dom/glissando.h"
#include "engraving/dom/gradualtempochange.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/soundflag.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/textbase.h"
#include "engraving/dom/textlinebase.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/tremolotwochord.h"
#include "engraving/dom/trill.h"
#include "engraving/dom/utils.h"
#include "engraving/editing/navigation.h"
#include "engraving/infrastructure/smufl.h"
#include "engraving/types/symnames.h"
#include "engraving/types/typesconv.h"

#include "mpe/internal/articulationstringutils.h"

#include "notation/imasternotation.h"
#include "notation/inotationelements.h"
#include "notation/inotationinteraction.h"
#include "notation/inotationplayback.h"
#include "notation/inotationsolomutestate.h"

using namespace mu::propertiespanel;
using namespace mu::engraving;

namespace noteplaybackinfomodel_detail {
QString formatBpm(double bpm)
{
    const double rounded = std::round(bpm);

    if (std::abs(bpm - rounded) < 0.05) {
        return QString::number(static_cast<int>(rounded));
    }

    return QString::number(bpm, 'f', 1);
}

QString measureRef(const EngravingItem* item)
{
    const Score* score = item->score();
    const Measure* startMeasure = score->tick2measure(item->tick());

    if (!startMeasure) {
        return QString();
    }

    const int start = startMeasure->measureNumber() + 1;

    if (item->isSpanner()) {
        const Spanner* spanner = toSpanner(item);
        const Fraction lastTick = spanner->tick2() - Fraction::fromTicks(1);

        const Measure* endMeasure = lastTick > spanner->tick()
                                    ? score->tick2measure(lastTick)
                                    : nullptr;

        const int end = endMeasure
                        ? endMeasure->measureNumber() + 1
                        : start;

        if (end != start) {
            return muse::qtrc("propertiespanel", "m. %1–%2").arg(start).arg(end);
        }
    }

    return muse::qtrc("propertiespanel", "m. %1").arg(start);
}

QString itemName(const EngravingItem* item)
{
    if (item->isNote()) {
        return muse::qtrc("propertiespanel", "Notehead / symbols on the note");
    }

    // A slur's or tie's subtype is only its line style ("Solid", "Dashed", ...)
    const bool subtypeIsStyle = item->isSlurTie() || item->isSlurTieSegment();

    QString name = subtypeIsStyle
                   ? QString()
                   : item->translatedSubtypeUserName().toQString();

    if (name.isEmpty()) {
        name = item->translatedTypeUserName().toQString();
    }

    return name;
}

//! Index of the item in the panel's item list, adding it if it isn't there yet
int registerItem(std::vector<const EngravingItem*>& items, const EngravingItem* item)
{
    const auto it = std::ranges::find(items, item);

    if (it != items.end()) {
        return static_cast<int>(std::distance(items.begin(), it));
    }

    items.push_back(item);

    return static_cast<int>(items.size()) - 1;
}

//! A piece of a row's label: plain text, or SMuFL symbols drawn in the music font
QVariantMap labelPart(const QString& text, bool isSymbol)
{
    QVariantMap part;

    part["text"] = text;
    part["isSymbol"] = isSymbol;

    return part;
}

QString symbolText(SymId id)
{
    const char32_t code = Smufl::smuflCode(id);

    return (code != 0)
           ? QString::fromUcs4(&code, 1)
           : QString();
}

//! Score text as label parts: <sym> tags become symbols, other formatting is dropped
QVariantList partsFromXml(const String& xml)
{
    static const QRegularExpression whitespace(QStringLiteral("\\s+"));
    static const QString symOpen = QStringLiteral("<sym>");
    static const QString symClose = QStringLiteral("</sym>");

    const QString source = xml.toQString();

    QVariantList parts;
    QString pending;
    bool pendingIsSymbol = false;

    const auto append = [&](const QString& text, bool isSymbol) {
        if (text.isEmpty()) {
            return;
        }

        if (!pending.isEmpty() && (pendingIsSymbol != isSymbol)) {
            parts << labelPart(pending, pendingIsSymbol);
            pending.clear();
        }

        pending += text;
        pendingIsSymbol = isSymbol;
    };

    QString plain;

    const auto flushPlain = [&]() {
        append(TextBase::unEscape(String::fromQString(plain)).toQString().replace(whitespace, QStringLiteral(" ")), false);
        plain.clear();
    };

    for (qsizetype i = 0; i < source.size();) {
        if (source.at(i) != QLatin1Char('<')) {
            plain += source.at(i++);
            continue;
        }

        if (source.mid(i, symOpen.size()) == symOpen) {
            const qsizetype nameStart = i + symOpen.size();
            const qsizetype nameEnd = source.indexOf(symClose, nameStart);

            if (nameEnd < 0) {
                break;
            }

            flushPlain();

            const String name = String::fromQString(source.mid(nameStart, nameEnd - nameStart));
            append(symbolText(SymNames::symIdByName(name)), true);

            i = nameEnd + symClose.size();
            continue;
        }

        // Any other tag (font, bold, italic, ...)
        const qsizetype tagEnd = source.indexOf(QLatin1Char('>'), i);

        if (tagEnd < 0) {
            break;
        }

        i = tagEnd + 1;
    }

    flushPlain();

    if (!pending.isEmpty()) {
        parts << labelPart(pending, pendingIsSymbol);
    }

    // No space before the first part or after the last
    if (!parts.isEmpty()) {
        QVariantMap first = parts.first().toMap();

        if (!first.value("isSymbol").toBool()) {
            const QString text = first.value("text").toString();
            const qsizetype start = std::ranges::find_if_not(text, [](QChar ch) { return ch.isSpace(); }) - text.begin();

            first["text"] = text.mid(start);
            parts.first() = first;
        }

        QVariantMap last = parts.last().toMap();

        if (!last.value("isSymbol").toBool()) {
            QString text = last.value("text").toString();

            while (!text.isEmpty() && text.back().isSpace()) {
                text.chop(1);
            }

            last["text"] = text;
            parts.last() = last;
        }
    }

    return parts;
}

//! As the Properties panel's "Easing method" dropdown names it
QString easingMethodName(ChangeMethod method)
{
    switch (method) {
    case ChangeMethod::NORMAL:
        return muse::qtrc("propertiespanel", "Normal", "gradual tempo change method");
    case ChangeMethod::EASE_IN:
        return muse::qtrc("propertiespanel", "Ease in", "gradual tempo change method");
    case ChangeMethod::EASE_OUT:
        return muse::qtrc("propertiespanel", "Ease out", "gradual tempo change method");
    case ChangeMethod::EASE_IN_OUT:
        return muse::qtrc("propertiespanel", "Ease in and out", "gradual tempo change method");
    case ChangeMethod::EXPONENTIAL:
        return muse::qtrc("propertiespanel", "Exponential", "gradual tempo change method");
    }

    return QString();
}

int toPercent(double factor)
{
    return static_cast<int>(std::round(factor * 100));
}

//! The settings that decide what the item does to the note's tempo, e.g. " (Normal, 60%)" for a rit., or
//! " (200% stretch)" for a fermata; empty for other items
QString playbackSettingsText(const EngravingItem* item)
{
    if (item->isGradualTempoChange()) {
        const GradualTempoChange* change = toGradualTempoChange(item);

        return QStringLiteral(" (%1, %2%)")
               .arg(easingMethodName(change->easingMethod()))
               .arg(toPercent(change->tempoChangeFactor()));
    }

    if (item->isFermata()) {
        const double stretch = item->getProperty(Pid::TIME_STRETCH).toDouble();

        return QStringLiteral(" (") + muse::qtrc("propertiespanel", "%1% stretch").arg(toPercent(stretch)) + QStringLiteral(")");
    }

    return QString();
}

//! The item as it's written in the score where possible (dynamic, hairpin and articulation symbols, "rit.",
//! "♩ = 120", "pizz."), otherwise its name
QVariantList writtenParts(const EngravingItem* item)
{
    if (item->isHairpin() && !toHairpin(item)->isLineType()) {
        const SymId id = toHairpin(item)->isCrescendo()
                         ? SymId::dynamicCrescendoHairpin
                         : SymId::dynamicDiminuendoHairpin;

        const QString symbol = symbolText(id);

        if (!symbol.isEmpty()) {
            return { labelPart(symbol, true) };
        }
    }

    if (item->isTextLineBase()) {
        const QVariantList parts = partsFromXml(static_cast<const TextLineBase*>(item)->beginText());

        if (!parts.isEmpty()) {
            return parts;
        }
    }

    SymId id = SymId::noSym;

    if (item->isArticulationFamily()) {
        id = static_cast<const Articulation*>(item)->symId();
    } else if (item->isFermata()) {
        id = toFermata(item)->symId();
    } else if (item->isBreath()) {
        id = toBreath(item)->symId();
    }

    if (const QString symbol = symbolText(id); !symbol.isEmpty()) {
        return { labelPart(symbol, true) };
    }

    if (item->isTextBase()) {
        QVariantList parts = partsFromXml(toTextBase(item)->xmlText());

        // Text that's only a direction ("pizz.", "con sord.") is quoted, so it doesn't read as the panel's own words
        const bool isQuoted = !(item->isDynamic() || item->isTempoText() || item->isCapo());

        if (!parts.isEmpty() && isQuoted) {
            parts.prepend(labelPart(QStringLiteral("“"), false));
            parts.append(labelPart(QStringLiteral("”"), false));
        }

        if (!parts.isEmpty()) {
            return parts;
        }
    }

    return { labelPart(itemName(item), false) };
}

//! writtenParts, then the item's playback settings where they matter (see playbackSettingsText)
QVariantList itemParts(const EngravingItem* item)
{
    QVariantList parts = writtenParts(item);

    if (const QString settings = playbackSettingsText(item); !settings.isEmpty()) {
        parts << labelPart(settings, false);
    }

    return parts;
}

//! "m. 3" or "m. 3–4"; withStaff adds the staff: "m. 3, s. 2"
QString itemLocation(const EngravingItem* item, bool withStaff)
{
    QString location = measureRef(item);

    if (withStaff) {
        const QString staff = muse::qtrc("propertiespanel", "s. %1").arg(item->staffIdx() + 1);

        location = location.isEmpty()
                   ? staff
                   : (location + QStringLiteral(", ") + staff);
    }

    return location;
}

//! "SlideOutDown" -> "Slide out down"
QString articulationName(muse::mpe::ArticulationType type)
{
    const QString raw = muse::mpe::articulationTypeToString(type);
    QString result;

    for (qsizetype i = 0; i < raw.size(); ++i) {
        const QChar ch = raw.at(i);

        if (i > 0 && ch.isUpper()) {
            result += QLatin1Char(' ');
            result += ch.toLower();
        } else {
            result += ch;
        }
    }

    return result;
}

//! True if every word of effect appears in name, e.g. "Arpeggio up" in "Up arpeggio", "Marcato" in "Marcato above"
bool nameSaysEffect(const QString& name, const QString& effect)
{
    const QStringList nameWords = name.toLower().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    const QStringList effectWords = effect.toLower().split(QLatin1Char(' '), Qt::SkipEmptyParts);

    return std::ranges::all_of(effectWords, [&nameWords](const QString& word) {
        return nameWords.contains(word);
    });
}

//! What playback does because of the marking, where its name doesn't already say so
//! (e.g. a slur → "Legato", a tremolo → "Tremolo 16th", the notehead → "Ghost note")
QString markingEffects(const NotePlaybackMarking& marking)
{
    const QString name = itemName(marking.item);
    QStringList effects;

    for (muse::mpe::ArticulationType type : marking.effects) {
        const QString effect = articulationName(type);

        if (!nameSaysEffect(name, effect)) {
            effects << effect;
        }
    }

    return effects.join(QStringLiteral(", "));
}

//! What a listed row shows: the item as written, or text instead if set. item == nullptr: a plain text row.
struct RowSpec {
    const EngravingItem* item = nullptr;

    QString text {};
    QString prefix {};
    QString detail {};
    QString suffix {}; // added after the item or text, e.g. " (+12)"

    QVariantList parts {}; // if set, shown instead of the item or text
    QVariantList lines {}; // listed under the label, each a list of parts like it
};

//! One row per spec: { parts, prefix, location, detail, lines, itemIndex }; itemIndex is -1 when there's no item to show.
//! Rows that would read the same (e.g. a hairpin on each staff of a piano) get the staff in their location.
QVariantList makeRows(const std::vector<RowSpec>& specs, std::vector<const EngravingItem*>& items)
{
    std::vector<QVariantList> parts;
    QStringList keys;

    for (const RowSpec& spec : specs) {
        if (!spec.parts.isEmpty()) {
            parts.push_back(spec.parts);
        } else if (spec.item && spec.text.isEmpty()) {
            parts.push_back(itemParts(spec.item));
        } else {
            parts.push_back(QVariantList { labelPart(spec.text, false) });
        }

        if (!spec.suffix.isEmpty()) {
            parts.back() << labelPart(spec.suffix, false);
        }

        QString key = spec.prefix;

        for (const QVariant& part : parts.back()) {
            key += part.toMap().value("text").toString();
        }

        if (spec.item) {
            key += QLatin1Char('|') + itemLocation(spec.item, false);
        }

        keys << key;
    }

    QVariantList rows;

    for (size_t i = 0; i < specs.size(); ++i) {
        const RowSpec& spec = specs.at(i);
        const qsizetype keyIndex = static_cast<qsizetype>(i);

        QVariantMap row;

        row["parts"] = parts.at(i);
        row["prefix"] = spec.prefix;
        row["detail"] = spec.detail;
        row["lines"] = spec.lines;

        if (spec.item) {
            const bool isAmbiguous = keys.count(keys.at(keyIndex)) > 1;

            row["location"] = itemLocation(spec.item, isAmbiguous);
            row["itemIndex"] = registerItem(items, spec.item);
        } else {
            row["location"] = QString();
            row["itemIndex"] = -1;
        }

        rows << row;
    }

    return rows;
}

RowSpec sourceSpec(const NotePlaybackSource& source, const QString& prefix = QString())
{
    if (!source.item) {
        return { nullptr, muse::qtrc("propertiespanel", "Point drawn in the automation lane"), prefix, QString() };
    }

    const QString detail = source.editedInAutomation
                           ? muse::qtrc("propertiespanel", "Edited in the automation lane")
                           : QString();

    return { source.item, QString(), prefix, detail };
}

//! The sources, then what their gradual changes start from (prefixed "Orig:")
QVariantList sourceRows(
    const std::vector<NotePlaybackSource>& sources,
    const std::vector<NotePlaybackSource>& origins,
    std::vector<const EngravingItem*>& items)
{
    std::vector<RowSpec> specs;

    for (const NotePlaybackSource& source : sources) {
        specs.push_back(sourceSpec(source));
    }

    for (const NotePlaybackSource& origin : origins) {
        const bool isListed = std::ranges::find(sources, origin) != sources.end();

        if (!isListed) {
            specs.push_back(sourceSpec(origin, muse::qtrc("propertiespanel", "Orig:", "the marking a gradual change starts from")));
        }
    }

    return makeRows(specs, items);
}

QVariantList textRows(const QString& text, std::vector<const EngravingItem*>& items)
{
    return makeRows({ RowSpec { nullptr, text, QString(), QString() } }, items);
}

QVariantMap passToMap(const NotePlaybackPass& pass, double tempoMultiplier, std::vector<const EngravingItem*>& items)
{
    QVariantMap map;

    // --- Tempo
    const double bpm = pass.tempoValue.toBPM().val;

    // With no tempo marking before the note, the default tempo applies
    map["tempo"] = pass.isDefaultTempo
                   ? muse::qtrc("propertiespanel", "%1 BPM (default)").arg(formatBpm(bpm))
                   : muse::qtrc("propertiespanel", "%1 BPM").arg(formatBpm(bpm));

    if (std::abs(tempoMultiplier - 1.0) > 1e-6) {
        map["tempoDetail"] = muse::qtrc("propertiespanel", "Plays at %1 BPM (%2% playback speed)")
                             .arg(formatBpm(pass.multipliedTempo.toBPM().val))
                             .arg(static_cast<int>(std::round(tempoMultiplier * 100)));
    } else {
        map["tempoDetail"] = QString();
    }

    map["tempoRows"] = pass.isDefaultTempo
                       ? QVariantList()
                       : sourceRows(pass.tempoSources, pass.tempoOrigins, items);

    // --- Dynamic: the most recent ordinary dynamic's symbol, "≈" when the level is between two of them
    map["dynamicPercent"] = muse::qtrc("propertiespanel", "%1%").arg(static_cast<int>(std::round(pass.dynamicValue * 100)));

    // With no dynamic marking, the default level still shows as its nearest dynamic, with "Default" below it
    const mu::engraving::DynamicType nearest = nearestOrdinaryDynamic(pass.dynamicValue);

    const auto it = ORDINARY_DYNAMIC_VALUES.find(nearest);
    const bool exact = (it != ORDINARY_DYNAMIC_VALUES.end())
                       && (std::abs(it->second.raw() - pass.dynamicValue) < 0.005);

    QString symbol;

    for (const QVariant& part : partsFromXml(Dynamic::dynamicText(nearest))) {
        if (part.toMap().value("isSymbol").toBool()) {
            symbol += part.toMap().value("text").toString();
        }
    }

    map["dynamicText"] = symbol.isEmpty()
                         ? QString(TConv::toXml(nearest).toQLatin1String())
                         : QString();
    map["dynamicSymbol"] = symbol;
    map["dynamicApprox"] = !exact;
    map["dynamicRows"] = pass.isDefaultDynamic
                         ? textRows(muse::qtrc("propertiespanel", "Default"), items)
                         : sourceRows(pass.dynamicSources, pass.dynamicOrigins, items);

    return map;
}

bool hasEffect(const NotePlaybackMarking& marking, std::initializer_list<muse::mpe::ArticulationType> types)
{
    return std::ranges::any_of(types, [&marking](muse::mpe::ArticulationType type) {
        return std::ranges::find(marking.effects, type) != marking.effects.end();
    });
}

using MarkingFilter = std::function<bool (const NotePlaybackMarking&)>;

//! Markings that change how long the note lasts, in order of how much they can change it
const std::vector<MarkingFilter>& tempoMarkingFilters()
{
    using muse::mpe::ArticulationType;

    static const std::vector<MarkingFilter> filters {
        [](const NotePlaybackMarking& m) {
            return m.item->isTie() || hasEffect(m, { ArticulationType::LaissezVibrer });
        },
        [](const NotePlaybackMarking& m) {
            return m.item->isFermata() || m.item->isBreath() || hasEffect(m, { ArticulationType::Breath });
        },
        [](const NotePlaybackMarking& m) {
            return hasEffect(m, { ArticulationType::Staccato, ArticulationType::Staccatissimo, ArticulationType::Tenuto });
        },
        [](const NotePlaybackMarking& m) {
            return hasEffect(m, { ArticulationType::Legato });
        },
    };

    return filters;
}

//! Markings that make the note louder than its dynamic (they aren't part of the dynamic's level)
const std::vector<MarkingFilter>& dynamicMarkingFilters()
{
    using muse::mpe::ArticulationType;

    static const std::vector<MarkingFilter> filters {
        [](const NotePlaybackMarking& m) {
            return hasEffect(m, { ArticulationType::Marcato });
        },
        [](const NotePlaybackMarking& m) {
            return hasEffect(m, { ArticulationType::Accent, ArticulationType::SoftAccent });
        },
    };

    return filters;
}

//! Where filters rank the marking: the index of the first filter that accepts it, or filters.size() if none does
size_t markingRank(const NotePlaybackMarking& marking, const std::vector<MarkingFilter>& filters)
{
    const auto it = std::ranges::find_if(filters, [&marking](const MarkingFilter& filter) {
        return filter(marking);
    });

    return static_cast<size_t>(std::distance(filters.begin(), it));
}

//! One line of a marking's row: its glyph, then its name; either may be missing
struct MarkingLine {
    SymId glyph = SymId::noSym;
    const char* name = nullptr; // untranslated, in the "propertiespanel" context
};

//! The line's glyph without its name
constexpr MarkingLine glyphOnly(const MarkingLine& line)
{
    return { line.glyph, nullptr };
}

//! What a marking shows in each section of the panel. A section's first line is the marking's row, the others are
//! listed under it. A marking with no lines in any section isn't shown.
struct MarkingDescription {
    std::vector<MarkingLine> tempo {};
    std::vector<MarkingLine> dynamic {};
    std::vector<MarkingLine> other {};
};

//! The panel's strings for each marking. Symbols that come in "Above" and "Below" versions are listed by their
//! "Above" version (see aboveVersion).
namespace markingstrings {
// --- Articulations
constexpr MarkingLine ACCENT { SymId::articAccentAbove, QT_TRANSLATE_NOOP("propertiespanel", "Accent") };
constexpr MarkingLine ACCENT_STACCATO { SymId::articAccentStaccatoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Accent-Staccato") };
constexpr MarkingLine LAISSEZ_VIBRER { SymId::articLaissezVibrerAbove, QT_TRANSLATE_NOOP("propertiespanel", "Laissez Vibrer (l.v.)") };
constexpr MarkingLine MARCATO { SymId::articMarcatoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Marcato") };
constexpr MarkingLine MARCATO_STACCATO { SymId::articMarcatoStaccatoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Marcato-Staccato") };
constexpr MarkingLine MARCATO_TENUTO { SymId::articMarcatoTenutoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Marcato-Tenuto") };
constexpr MarkingLine SOFT_ACCENT { SymId::articSoftAccentAbove, QT_TRANSLATE_NOOP("propertiespanel", "Accent (soft)") };
constexpr MarkingLine SOFT_ACCENT_STACCATO {
    SymId::articSoftAccentStaccatoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Accent-Staccato (soft)")
};
constexpr MarkingLine SOFT_ACCENT_TENUTO {
    SymId::articSoftAccentTenutoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Accent-Tenuto (soft)")
};
constexpr MarkingLine SOFT_ACCENT_TENUTO_STACCATO {
    SymId::articSoftAccentTenutoStaccatoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Accent-Tenuto-Staccato (soft)")
};
constexpr MarkingLine STACCATISSIMO { SymId::articStaccatissimoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Staccatissimo") };
constexpr MarkingLine STACCATISSIMO_STROKE {
    SymId::articStaccatissimoStrokeAbove, QT_TRANSLATE_NOOP("propertiespanel", "Staccatissimo Stroke")
};
constexpr MarkingLine STACCATISSIMO_WEDGE {
    SymId::articStaccatissimoWedgeAbove, QT_TRANSLATE_NOOP("propertiespanel", "Staccatissimo Wedge")
};
constexpr MarkingLine STACCATO { SymId::articStaccatoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Staccato") };
constexpr MarkingLine TENUTO { SymId::articTenutoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Tenuto") };
constexpr MarkingLine TENUTO_STACCATO { SymId::articTenutoStaccatoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Tenuto-Staccato") };
constexpr MarkingLine TENUTO_ACCENT { SymId::articTenutoAccentAbove, QT_TRANSLATE_NOOP("propertiespanel", "Tenuto-Accent") };

// --- Dynamics inside symbols
constexpr MarkingLine SFORZANDO { SymId::dynamicSforzando, QT_TRANSLATE_NOOP("propertiespanel", "Sforzando (s)") };
constexpr MarkingLine SFORZANDO_1 { SymId::dynamicSforzando1, QT_TRANSLATE_NOOP("propertiespanel", "Sforzando (sf)") };
constexpr MarkingLine SFORZANDO_PIANISSIMO {
    SymId::dynamicSforzandoPianissimo, QT_TRANSLATE_NOOP("propertiespanel", "Sforzando-Pianissimo (sfpp)")
};
constexpr MarkingLine SFORZANDO_PIANO { SymId::dynamicSforzandoPiano, QT_TRANSLATE_NOOP("propertiespanel", "Sforzando-Piano (sfp)") };
constexpr MarkingLine SFORZATO { SymId::dynamicSforzato, QT_TRANSLATE_NOOP("propertiespanel", "Sforzato (sfz)") };
constexpr MarkingLine SFORZATO_FF { SymId::dynamicSforzatoFF, QT_TRANSLATE_NOOP("propertiespanel", "Sforzato-FF (sffz)") };
constexpr MarkingLine SFORZATO_PIANO { SymId::dynamicSforzatoPiano, QT_TRANSLATE_NOOP("propertiespanel", "Sforzato-Piano (sfzp)") };
constexpr MarkingLine CRESC_HAIRPIN { SymId::dynamicCrescendoHairpin, QT_TRANSLATE_NOOP("propertiespanel", "Cresc. Hairpin") };
constexpr MarkingLine DIM_HAIRPIN { SymId::dynamicDiminuendoHairpin, QT_TRANSLATE_NOOP("propertiespanel", "Dim. Hairpin") };

// --- Notes under a marking's row
constexpr MarkingLine STRINGS_ONLY { SymId::noSym, QT_TRANSLATE_NOOP("propertiespanel", "(Strings only)") };
constexpr MarkingLine BOWED_ONLY { SymId::noSym, QT_TRANSLATE_NOOP("propertiespanel", "(Bowed only)") };
constexpr MarkingLine DRUMS_ONLY { SymId::noSym, QT_TRANSLATE_NOOP("propertiespanel", "(Drums only)") };
//! Playback caps tremolos at 64ths (TremoloMetaParser)
constexpr MarkingLine PLAYS_AS_64THS { SymId::noSym, QT_TRANSLATE_NOOP("propertiespanel", "(Plays as 64ths)") };

// --- Spanners
constexpr MarkingLine LEGATO { SymId::noSym, QT_TRANSLATE_NOOP("propertiespanel", "Legato") };
constexpr MarkingLine PEDAL { SymId::keyboardPedalPed, QT_TRANSLATE_NOOP("propertiespanel", "Pedal") };

//! Shown in the Other section, with note (if any) under it
MarkingDescription inOther(const MarkingLine& line, const MarkingLine& note = {})
{
    if (!note.name) {
        return { .other = { line } };
    }

    return { .other = { line, note } };
}

//! A symbol's entry: shown in the Other section, with note (if any) under it
std::pair<const SymId, MarkingDescription> symbolInOther(const MarkingLine& line, const MarkingLine& note = {})
{
    return { line.glyph, inOther(line, note) };
}

//! A symbol's entry: its glyph alone in the Dynamic section
std::pair<const SymId, MarkingDescription> symbolInDynamic(const MarkingLine& line)
{
    return { line.glyph, { .dynamic = { glyphOnly(line) } } };
}

const std::map<SymId, MarkingDescription>& symbols()
{
    static const std::map<SymId, MarkingDescription> table {
        // --- Articulations
        { ACCENT.glyph, { .dynamic = { ACCENT } } },
        { ACCENT_STACCATO.glyph, { .tempo = { ACCENT_STACCATO, STACCATO }, .dynamic = { ACCENT_STACCATO, ACCENT } } },
        { LAISSEZ_VIBRER.glyph, { .tempo = { LAISSEZ_VIBRER } } },
        { MARCATO.glyph, { .dynamic = { MARCATO } } },
        { MARCATO_STACCATO.glyph, { .tempo = { MARCATO_STACCATO, STACCATO }, .dynamic = { MARCATO_STACCATO, MARCATO } } },
        { MARCATO_TENUTO.glyph, { .tempo = { MARCATO_TENUTO, TENUTO }, .dynamic = { MARCATO_TENUTO, MARCATO } } },
        { SOFT_ACCENT.glyph, { .dynamic = { SOFT_ACCENT } } },
        {
            SOFT_ACCENT_STACCATO.glyph,
            { .tempo = { SOFT_ACCENT_STACCATO, STACCATO }, .dynamic = { SOFT_ACCENT_STACCATO, SOFT_ACCENT } }
        },
        {
            SOFT_ACCENT_TENUTO.glyph,
            { .tempo = { SOFT_ACCENT_TENUTO, TENUTO }, .dynamic = { SOFT_ACCENT_TENUTO, SOFT_ACCENT } }
        },
        {
            SOFT_ACCENT_TENUTO_STACCATO.glyph,
            {
                .tempo = { SOFT_ACCENT_TENUTO_STACCATO, TENUTO, STACCATO },
                .dynamic = { SOFT_ACCENT_TENUTO_STACCATO, SOFT_ACCENT }
            }
        },
        { STACCATISSIMO.glyph, { .tempo = { STACCATISSIMO } } },
        { STACCATISSIMO_STROKE.glyph, { .tempo = { STACCATISSIMO_STROKE } } },
        { STACCATISSIMO_WEDGE.glyph, { .tempo = { STACCATISSIMO_WEDGE } } },
        { STACCATO.glyph, { .tempo = { STACCATO } } },
        { TENUTO.glyph, { .tempo = { TENUTO } } },
        { TENUTO_STACCATO.glyph, { .tempo = { TENUTO_STACCATO, TENUTO, STACCATO } } },
        { TENUTO_ACCENT.glyph, { .tempo = { TENUTO_ACCENT, TENUTO }, .dynamic = { TENUTO_ACCENT, ACCENT } } },

        // --- Dynamics inside symbols
        symbolInDynamic(SFORZANDO),
        symbolInDynamic(SFORZANDO_1),
        symbolInDynamic(SFORZANDO_PIANISSIMO),
        symbolInDynamic(SFORZANDO_PIANO),
        symbolInDynamic(SFORZATO),
        symbolInDynamic(SFORZATO_FF),
        symbolInDynamic(SFORZATO_PIANO),
        symbolInDynamic(CRESC_HAIRPIN),
        symbolInDynamic(DIM_HAIRPIN),

        // --- Other symbols
        symbolInOther({ SymId::pluckedSnapPizzicatoAbove, QT_TRANSLATE_NOOP("propertiespanel", "Snap Pizzicato") }, STRINGS_ONLY),
        symbolInOther({ SymId::stringsDownBow, QT_TRANSLATE_NOOP("propertiespanel", "Down Bow") }, BOWED_ONLY),
        symbolInOther({ SymId::stringsHarmonic, QT_TRANSLATE_NOOP("propertiespanel", "Harmonic") }, STRINGS_ONLY),
        symbolInOther({ SymId::stringsUpBow, QT_TRANSLATE_NOOP("propertiespanel", "Up Bow") }, BOWED_ONLY),

        // --- Ornaments
        symbolInOther({ SymId::ornamentDownMordent, QT_TRANSLATE_NOOP("propertiespanel", "Down Mordent") }),
        symbolInOther({ SymId::ornamentHaydn, QT_TRANSLATE_NOOP("propertiespanel", "Haydn Ornament") }),
        symbolInOther({ SymId::ornamentLinePrall, QT_TRANSLATE_NOOP("propertiespanel", "Line Prall") }),
        symbolInOther({ SymId::ornamentMordent, QT_TRANSLATE_NOOP("propertiespanel", "Mordent") }),
        symbolInOther({ SymId::ornamentPinceCouperin, QT_TRANSLATE_NOOP("propertiespanel", "Pincé (Couperin)") }),
        symbolInOther({ SymId::ornamentPrallDown, QT_TRANSLATE_NOOP("propertiespanel", "Prall Down") }),
        symbolInOther({ SymId::ornamentPrallMordent, QT_TRANSLATE_NOOP("propertiespanel", "Prall Mordent") }),
        symbolInOther({ SymId::ornamentPrallUp, QT_TRANSLATE_NOOP("propertiespanel", "Prall Up") }),
        symbolInOther({ SymId::ornamentShake3, QT_TRANSLATE_NOOP("propertiespanel", "Shake") }),
        symbolInOther({ SymId::ornamentShakeMuffat1, QT_TRANSLATE_NOOP("propertiespanel", "Shake (Muffat)") }),
        symbolInOther({ SymId::ornamentShortTrill, QT_TRANSLATE_NOOP("propertiespanel", "Trill (short)") }),
        symbolInOther({ SymId::ornamentTremblement, QT_TRANSLATE_NOOP("propertiespanel", "Tremblement") }),
        symbolInOther({ SymId::ornamentTremblementCouperin, QT_TRANSLATE_NOOP("propertiespanel", "Tremblement appuyé (Couperin)") }),
        symbolInOther({ SymId::ornamentTrill, QT_TRANSLATE_NOOP("propertiespanel", "Trill") }),
        symbolInOther({ SymId::ornamentTurn, QT_TRANSLATE_NOOP("propertiespanel", "Turn") }),
        symbolInOther({ SymId::ornamentTurnInverted, QT_TRANSLATE_NOOP("propertiespanel", "Turn (inverted)") }),
        symbolInOther({ SymId::ornamentTurnSlash, QT_TRANSLATE_NOOP("propertiespanel", "Turn (w/ slash)") }),
        symbolInOther({ SymId::ornamentTurnUp, QT_TRANSLATE_NOOP("propertiespanel", "Turn (up)") }),
        symbolInOther({ SymId::ornamentTurnUpS, QT_TRANSLATE_NOOP("propertiespanel", "Turn (inverted, up)") }),
        symbolInOther({ SymId::ornamentUpMordent, QT_TRANSLATE_NOOP("propertiespanel", "Up Mordent") }),
        symbolInOther({ SymId::ornamentUpPrall, QT_TRANSLATE_NOOP("propertiespanel", "Up Prall") }),
    };

    return table;
}

//! SMuFL's strokes stop at 5 (128ths), so 256ths share them
const std::map<TremoloType, MarkingDescription>& tremolos()
{
    static const std::map<TremoloType, MarkingDescription> table {
        { TremoloType::BUZZ_ROLL, inOther({ SymId::buzzRoll, QT_TRANSLATE_NOOP("propertiespanel", "Buzz Roll") }, DRUMS_ONLY) },
        { TremoloType::R8, inOther({ SymId::tremolo1, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (8ths, 1 note)") }) },
        { TremoloType::R16, inOther({ SymId::tremolo2, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (16ths, 1 note)") }) },
        { TremoloType::R32, inOther({ SymId::tremolo3, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (32nds, 1 note)") }) },
        { TremoloType::R64, inOther({ SymId::tremolo4, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (64ths, 1 note)") }) },
        {
            TremoloType::R128,
            inOther({ SymId::tremolo5, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (128ths, 1 note)") }, PLAYS_AS_64THS)
        },
        {
            TremoloType::R256,
            inOther({ SymId::tremolo5, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (256ths, 1 note)") }, PLAYS_AS_64THS)
        },
        { TremoloType::C8, inOther({ SymId::tremolo1, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (8ths, 2 notes)") }) },
        { TremoloType::C16, inOther({ SymId::tremolo2, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (16ths, 2 notes)") }) },
        { TremoloType::C32, inOther({ SymId::tremolo3, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (32nds, 2 notes)") }) },
        { TremoloType::C64, inOther({ SymId::tremolo4, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (64ths, 2 notes)") }) },
        {
            TremoloType::C128,
            inOther({ SymId::tremolo5, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (128ths, 2 notes)") }, PLAYS_AS_64THS)
        },
        {
            TremoloType::C256,
            inOther({ SymId::tremolo5, QT_TRANSLATE_NOOP("propertiespanel", "Tremolo (256ths, 2 notes)") }, PLAYS_AS_64THS)
        },
    };

    return table;
}

//! The plain arpeggio has no glyph of its own (the score draws it from wiggle segments turned on their side)
const std::map<ArpeggioType, MarkingDescription>& arpeggios()
{
    static const std::map<ArpeggioType, MarkingDescription> table {
        { ArpeggioType::NORMAL, inOther({ SymId::noSym, QT_TRANSLATE_NOOP("propertiespanel", "Arpeggio") }) },
        { ArpeggioType::UP, inOther({ SymId::arpeggiatoUp, QT_TRANSLATE_NOOP("propertiespanel", "Arpeggio (up)") }) },
        { ArpeggioType::DOWN, inOther({ SymId::arpeggiatoDown, QT_TRANSLATE_NOOP("propertiespanel", "Arpeggio (down)") }) },
        {
            ArpeggioType::UP_STRAIGHT,
            inOther({ SymId::arrowBlackUp, QT_TRANSLATE_NOOP("propertiespanel", "Arpeggio (up, straight)") })
        },
        {
            ArpeggioType::DOWN_STRAIGHT,
            inOther({ SymId::arrowBlackDown, QT_TRANSLATE_NOOP("propertiespanel", "Arpeggio (down, straight)") })
        },
    };

    return table;
}

const std::map<TrillType, MarkingDescription>& trills()
{
    static const std::map<TrillType, MarkingDescription> table {
        { TrillType::TRILL_LINE, inOther({ SymId::ornamentTrill, QT_TRANSLATE_NOOP("propertiespanel", "Trill Line") }) },
        { TrillType::UPPRALL_LINE, inOther({ SymId::ornamentUpPrall, QT_TRANSLATE_NOOP("propertiespanel", "Up Prall") }) },
        {
            TrillType::DOWNPRALL_LINE,
            inOther({ SymId::ornamentPrecompMordentUpperPrefix, QT_TRANSLATE_NOOP("propertiespanel", "Down Prall") })
        },
        { TrillType::PRALLPRALL_LINE, inOther({ SymId::ornamentLinePrall, QT_TRANSLATE_NOOP("propertiespanel", "Line Prall") }) },
    };

    return table;
}

//! Neither kind has a glyph that reads as a glissando
const std::map<GlissandoType, MarkingDescription>& glissandos()
{
    static const std::map<GlissandoType, MarkingDescription> table {
        { GlissandoType::WAVY, inOther({ SymId::noSym, QT_TRANSLATE_NOOP("propertiespanel", "Glissando") }) },
        { GlissandoType::STRAIGHT, inOther({ SymId::noSym, QT_TRANSLATE_NOOP("propertiespanel", "Glissando (straight)") }) },
    };

    return table;
}

const std::map<NoteType, MarkingDescription>& graceNotes()
{
    constexpr SymId APPOGGIATURA = SymId::graceNoteAppoggiaturaStemUp;

    static const std::map<NoteType, MarkingDescription> table {
        {
            NoteType::ACCIACCATURA,
            inOther({ SymId::graceNoteAcciaccaturaStemUp, QT_TRANSLATE_NOOP("propertiespanel", "Acciaccatura") })
        },
        { NoteType::APPOGGIATURA, inOther({ APPOGGIATURA, QT_TRANSLATE_NOOP("propertiespanel", "Appoggiatura") }) },
        { NoteType::GRACE4, inOther({ APPOGGIATURA, QT_TRANSLATE_NOOP("propertiespanel", "Appoggiatura (4th, leading)") }) },
        { NoteType::GRACE16, inOther({ APPOGGIATURA, QT_TRANSLATE_NOOP("propertiespanel", "Appoggiatura (16th, leading)") }) },
        { NoteType::GRACE32, inOther({ APPOGGIATURA, QT_TRANSLATE_NOOP("propertiespanel", "Appoggiatura (32nd, leading)") }) },
        { NoteType::GRACE8_AFTER, inOther({ APPOGGIATURA, QT_TRANSLATE_NOOP("propertiespanel", "Appoggiatura (8th, trailing)") }) },
        {
            NoteType::GRACE16_AFTER,
            inOther({ APPOGGIATURA, QT_TRANSLATE_NOOP("propertiespanel", "Appoggiatura (16th, trailing)") })
        },
        {
            NoteType::GRACE32_AFTER,
            inOther({ APPOGGIATURA, QT_TRANSLATE_NOOP("propertiespanel", "Appoggiatura (32nd, trailing)") })
        },
    };

    return table;
}

//! For kinds of items the table covers but doesn't list
const MarkingDescription HIDDEN;

template<typename Key>
const MarkingDescription* lookUp(const std::map<Key, MarkingDescription>& table, Key key)
{
    const auto it = table.find(key);

    return (it != table.end())
           ? &it->second
           : &HIDDEN;
}
} // namespace markingstrings

//! The "Above" version of a "…Below" symbol: the panel shows both the same, as some "Below" glyphs sit badly in a line
SymId aboveVersion(SymId id)
{
    static const QLatin1StringView below("Below");

    const QLatin1StringView name = SymNames::nameForSymId(id).toQLatin1String();

    if (!name.endsWith(below)) {
        return id;
    }

    const QString aboveName = QString(name.chopped(below.size())) + QStringLiteral("Above");

    return SymNames::symIdByName(String::fromQString(aboveName), id);
}

//! How the strings table describes the item: nullptr for kinds of items it doesn't cover (they're listed by what they
//! do), a description with no lines for those it hides (notehead changes, and kinds left out for now)
const MarkingDescription* markingDescription(const EngravingItem* item)
{
    if (item->isArticulationFamily()) {
        const SymId symbol = aboveVersion(static_cast<const Articulation*>(item)->symId());

        return markingstrings::lookUp(markingstrings::symbols(), symbol);
    }

    if (item->type() == ElementType::TREMOLO_SINGLECHORD) {
        return markingstrings::lookUp(markingstrings::tremolos(), item_cast<const TremoloSingleChord*>(item)->tremoloType());
    }

    if (item->type() == ElementType::TREMOLO_TWOCHORD) {
        return markingstrings::lookUp(markingstrings::tremolos(), item_cast<const TremoloTwoChord*>(item)->tremoloType());
    }

    if (item->isArpeggio()) {
        return markingstrings::lookUp(markingstrings::arpeggios(), toArpeggio(item)->arpeggioType());
    }

    if (item->isTrill()) {
        return markingstrings::lookUp(markingstrings::trills(), toTrill(item)->trillType());
    }

    if (item->isGlissando()) {
        return markingstrings::lookUp(markingstrings::glissandos(), toGlissando(item)->glissandoType());
    }

    if (item->isChord() && toChord(item)->isGrace()) {
        return markingstrings::lookUp(markingstrings::graceNotes(), toChord(item)->noteType());
    }

    if (item->isSlur()) {
        static const MarkingDescription slur { .tempo = { markingstrings::LEGATO } };
        return &slur;
    }

    if (item->isPedal()) {
        static const MarkingDescription pedal = markingstrings::inOther(markingstrings::PEDAL);
        return &pedal;
    }

    const bool isHidden = item->isNote() || item->isChordLine() || item->isVibrato() || item->isLetRing()
                          || item->isHammerOnPullOff() || item->isPalmMute() || item->isGuitarBend();

    return isHidden
           ? &markingstrings::HIDDEN
           : nullptr;
}

//! The line's glyph, then its name
QVariantList lineParts(const MarkingLine& line)
{
    QVariantList parts;

    if (line.glyph != SymId::noSym) {
        if (const QString symbol = symbolText(line.glyph); !symbol.isEmpty()) {
            parts << labelPart(symbol, true);
        }
    }

    if (line.name) {
        const QString name = muse::qtrc("propertiespanel", line.name);

        parts << labelPart(parts.isEmpty() ? name : (QStringLiteral(" ") + name), false);
    }

    return parts;
}

//! A row for the item: the first line as its label, the others under it
RowSpec describedSpec(const EngravingItem* item, const std::vector<MarkingLine>& lines)
{
    RowSpec spec;

    spec.item = item;
    spec.parts = lineParts(lines.front());

    for (size_t i = 1; i < lines.size(); ++i) {
        spec.lines.push_back(QVariant(lineParts(lines.at(i))));
    }

    return spec;
}

//! The rows of each section's markings
struct MarkingRows {
    QVariantList tempo;
    QVariantList dynamic;
    QVariantList other;
};

//! Each marking in the sections the strings table gives it, or (if the table doesn't cover it) in the first section
//! whose filters take it, with what it does under it. The Tempo and Dynamic sections list them in their filters' order.
MarkingRows markingRows(const std::vector<NotePlaybackMarking>& markings, std::vector<const EngravingItem*>& items)
{
    struct RankedSpec {
        size_t rank = 0;
        RowSpec spec;
    };

    const std::vector<MarkingFilter>& tempoFilters = tempoMarkingFilters();
    const std::vector<MarkingFilter>& dynamicFilters = dynamicMarkingFilters();

    std::vector<RankedSpec> tempo;
    std::vector<RankedSpec> dynamic;
    std::vector<RowSpec> other;

    for (const NotePlaybackMarking& marking : markings) {
        const size_t tempoRank = markingRank(marking, tempoFilters);
        const size_t dynamicRank = markingRank(marking, dynamicFilters);

        if (const MarkingDescription* description = markingDescription(marking.item)) {
            if (!description->tempo.empty()) {
                tempo.push_back({ tempoRank, describedSpec(marking.item, description->tempo) });
            }

            if (!description->dynamic.empty()) {
                dynamic.push_back({ dynamicRank, describedSpec(marking.item, description->dynamic) });
            }

            if (!description->other.empty()) {
                other.push_back(describedSpec(marking.item, description->other));
            }

            continue;
        }

        const RowSpec spec { marking.item, QString(), QString(), markingEffects(marking) };

        if (tempoRank < tempoFilters.size()) {
            tempo.push_back({ tempoRank, spec });
        } else if (dynamicRank < dynamicFilters.size()) {
            dynamic.push_back({ dynamicRank, spec });
        } else {
            other.push_back(spec);
        }
    }

    const auto rankedRows = [&items](std::vector<RankedSpec>& ranked) {
        std::ranges::stable_sort(ranked, {}, &RankedSpec::rank);

        std::vector<RowSpec> specs;

        for (RankedSpec& rankedSpec : ranked) {
            specs.push_back(std::move(rankedSpec.spec));
        }

        return makeRows(specs, items);
    };

    return { rankedRows(tempo), rankedRows(dynamic), makeRows(other, items) };
}

//! "+12", "−12"
QString semitonesText(int semitones)
{
    return (semitones > 0)
           ? QStringLiteral("+%1").arg(semitones)
           : QStringLiteral("−%1").arg(-semitones);
}

//! The ottava and capo, each with how far it moves the pitch
QVariantList pitchShiftRows(const std::vector<NotePlaybackPitchShift>& shifts, std::vector<const EngravingItem*>& items)
{
    std::vector<RowSpec> specs;

    for (const NotePlaybackPitchShift& shift : shifts) {
        RowSpec spec;

        spec.item = shift.item;
        spec.suffix = QStringLiteral(" (") + semitonesText(shift.semitones) + QStringLiteral(")");

        specs.push_back(spec);
    }

    return makeRows(specs, items);
}

//! "Muse Sounds: Mute, pizz": what the staff text's sound flag asks Muse Sounds for
QString soundFlagDetail(const EngravingItem* item)
{
    if (!item->isStaffText()) {
        return QString();
    }

    const SoundFlag* flag = toStaffText(item)->soundFlag();

    if (!flag) {
        return QString();
    }

    QStringList requests;

    for (const String& preset : flag->soundPresets()) {
        if (!preset.empty()) {
            requests << preset.toQString();
        }
    }

    if (!flag->playingTechnique().empty()) {
        requests << flag->playingTechnique().toQString();
    }

    if (requests.isEmpty()) {
        return QString();
    }

    return muse::qtrc("propertiespanel", "Muse Sounds: %1").arg(requests.join(QStringLiteral(", ")));
}

//! A text part shown brighter and bold (emphasis 1) or dimmed (emphasis -1)
QVariantMap emphasisPart(const QString& text, int emphasis)
{
    QVariantMap part = labelPart(text, false);

    part["emphasis"] = emphasis;

    return part;
}

//! The whole word a syllable belongs to, e.g. “Re-mem-ber” with "Re" bold. Follows the syllables' hyphens, so it's
//! only as good as the lyrics' entry: a syllable that doesn't end with "-" ends its word.
QVariantList lyricWordParts(const Lyrics* syllable)
{
    constexpr size_t maxSyllables = 32;

    auto continuesToNext = [](const Lyrics* lyrics) {
        return (lyrics->syllabic() == LyricsSyllabic::BEGIN) || (lyrics->syllabic() == LyricsSyllabic::MIDDLE);
    };

    auto continuesFromPrev = [](const Lyrics* lyrics) {
        return (lyrics->syllabic() == LyricsSyllabic::MIDDLE) || (lyrics->syllabic() == LyricsSyllabic::END);
    };

    std::vector<const Lyrics*> word { syllable };

    while (continuesFromPrev(word.front()) && (word.size() < maxSyllables)) {
        const Lyrics* prev = Navigation::prevLyrics(word.front());
        if (!prev || !continuesToNext(prev)) {
            break;
        }

        word.insert(word.begin(), prev);
    }

    while (continuesToNext(word.back()) && (word.size() < maxSyllables)) {
        const Lyrics* next = Navigation::nextLyrics(word.back());
        if (!next || !continuesFromPrev(next)) {
            break;
        }

        word.push_back(next);
    }

    QVariantList parts { emphasisPart(QStringLiteral("“"), -1) };

    for (size_t i = 0; i < word.size(); ++i) {
        if (i > 0) {
            parts << emphasisPart(QStringLiteral("-"), -1);
        }

        parts << emphasisPart(word.at(i)->plainText().toQString(), (word.at(i) == syllable) ? 1 : -1);
    }

    parts << emphasisPart(QStringLiteral("”"), -1);

    return parts;
}

//! The playing technique text, the sound flags, then the sung syllable
QVariantList soundSourceRows(const NotePlaybackSound& sound, std::vector<const EngravingItem*>& items)
{
    std::vector<RowSpec> specs;

    if (sound.playingTechnique) {
        RowSpec spec;

        spec.item = sound.playingTechnique;
        spec.detail = soundFlagDetail(sound.playingTechnique);

        specs.push_back(spec);
    }

    for (const EngravingItem* flag : sound.soundFlags) {
        if (flag == sound.playingTechnique) {
            continue;
        }

        RowSpec spec;

        spec.item = flag;
        spec.detail = soundFlagDetail(flag);

        specs.push_back(spec);
    }

    if (sound.syllable) {
        RowSpec spec;

        spec.item = sound.syllable;
        spec.detail = sound.syllableStartsHere
                      ? muse::qtrc("propertiespanel", "Lyric")
                      : muse::qtrc("propertiespanel", "Lyric (held)");

        if (sound.syllable->isLyrics()) {
            spec.parts = lyricWordParts(toLyrics(sound.syllable));
        }

        specs.push_back(spec);
    }

    return makeRows(specs, items);
}

//! The music font the score uses for symbols its own font lacks (EngravingFontsProvider's fallback)
constexpr const char* FALLBACK_MUSIC_FONT = "Bravura";

//! Gives each symbol part musicFont can't draw a "fontFamily" of FALLBACK_MUSIC_FONT, as the score does. Without it,
//! Qt draws whatever another font has at that code point (e.g. an arrow for Leland's missing staccatissimo stroke).
void addPartFallbacks(QVariantList& parts, const QRawFont& musicFont)
{
    for (QVariant& partValue : parts) {
        QVariantMap part = partValue.toMap();

        if (!part.value("isSymbol").toBool()) {
            continue;
        }

        const QList<uint> codes = part.value("text").toString().toUcs4();

        const bool isDrawable = std::ranges::all_of(codes, [&musicFont](uint code) {
            return musicFont.supportsCharacter(code);
        });

        if (!isDrawable) {
            part["fontFamily"] = QString::fromLatin1(FALLBACK_MUSIC_FONT);
            partValue = part;
        }
    }
}

//! addPartFallbacks for each row's label and the lines under it
void addSymbolFallbacks(QVariantList& rows, const QRawFont& musicFont)
{
    for (QVariant& rowValue : rows) {
        QVariantMap row = rowValue.toMap();

        QVariantList parts = row.value("parts").toList();
        addPartFallbacks(parts, musicFont);
        row["parts"] = parts;

        QVariantList lines = row.value("lines").toList();

        for (QVariant& line : lines) {
            QVariantList partsOfLine = line.toList();
            addPartFallbacks(partsOfLine, musicFont);
            line = partsOfLine;
        }

        row["lines"] = lines;
        rowValue = row;
    }
}
} // namespace noteplaybackinfomodel_detail

using namespace noteplaybackinfomodel_detail;

NotePlaybackInfoModel::NotePlaybackInfoModel(
    QObject* parent,
    const muse::modularity::ContextPtr& iocCtx,
    IElementRepositoryService* repository)
    : PropertiesPanelAbstractModel(parent, iocCtx, repository)
{
    setTitle(muse::qtrc("propertiespanel", "Info"));
    setModelType(PropertiesPanelModelType::TYPE_NOTE_PLAYBACK_INFO);

    createProperties();
}

void NotePlaybackInfoModel::requestElements()
{
    // The selection changed: whatever was hovered or pinned belonged to the previous note
    clearFocus();

    m_elementList = m_repository->findElementsByType(ElementType::NOTEHEAD);
}

void NotePlaybackInfoModel::onCurrentNotationChanged()
{
    PropertiesPanelAbstractModel::onCurrentNotationChanged();

    const notation::INotationPtr notation = currentNotation();

    if (!notation) {
        return;
    }

    notation->soloMuteState()->trackSoloMuteStateChanged().onReceive(
        this,
        [this](const InstrumentTrackId&, const notation::INotationSoloMuteState::SoloMuteState&) { reload(); },
        Mode::SetReplace
        );

    // Fires after the playback data is re-rendered, which includes automation lane edits
    notation->masterNotation()->playback()->tracksDataChanged().onReceive(
        this,
        [this](const InstrumentTrackIdSet&) { reload(); },
        Mode::SetReplace
        );
}

void NotePlaybackInfoModel::onNotationChanged(const PropertyIdSet&, const StyleIdSet&)
{
    reload();
}

void NotePlaybackInfoModel::reload()
{
    if (isEmpty()) {
        return;
    }

    loadProperties();
}

const Note* NotePlaybackInfoModel::selectedNote() const
{
    if (m_elementList.size() != 1 || !m_elementList.front()->isNote()) {
        return nullptr;
    }

    return toNote(m_elementList.front());
}

void NotePlaybackInfoModel::loadProperties()
{
    m_passes.clear();
    m_isMuted = false;
    m_muteReason.clear();
    m_velocityOverride = 0;
    m_tempoMarkings.clear();
    m_dynamicMarkings.clear();
    m_otherMarkings.clear();
    m_isPlayed = true;
    m_pitch.clear();
    m_pitchRows.clear();
    m_soundRows.clear();

    // Items may have been deleted or replaced by the score change that triggered this reload
    clearFocus();
    m_items.clear();

    const Note* note = selectedNote();
    const notation::INotationPtr notation = currentNotation();

    if (note && notation) {
        const notation::INotationPlaybackPtr playback = notation->masterNotation()->playback();
        const NotePlaybackInfo info = playback->notePlaybackInfo(note);

        if (info.isValid()) {
            m_velocityOverride = info.userVelocity;
            m_isPlayed = info.isPlayed;

            // --- Pitch and sound (listed before the markings, so the markings don't repeat them)
            m_pitch = tpcUserName(info.soundingTpc, info.soundingPitch, false /*explicitAccidental*/).toQString();
            m_pitchRows = pitchShiftRows(info.pitchShifts, m_items);
            m_soundRows = soundSourceRows(info.sound, m_items);

            const double multiplier = playback->tempoMultiplier();

            // Merge passes that play identically
            std::vector<std::pair<QVariantMap, QList<int> > > groups;

            for (const NotePlaybackPass& pass : info.passes) {
                const QVariantMap map = passToMap(pass, multiplier, m_items);
                auto it = std::find_if(groups.begin(), groups.end(), [&map](const auto& g) { return g.first == map; });

                if (it == groups.end()) {
                    groups.push_back({ map, { pass.passNumber } });
                } else {
                    it->second << pass.passNumber;
                }
            }

            for (auto& [map, numbers] : groups) {
                if (groups.size() > 1 || info.passes.size() > 1) {
                    QStringList nums;
                    for (int n : numbers) {
                        nums << QString::number(n);
                    }
                    if (groups.size() == 1) {
                        map["label"] = muse::qtrc("propertiespanel", "All %1 repeat passes").arg(numbers.size());
                    } else if (numbers.size() == 1) {
                        map["label"] = muse::qtrc("propertiespanel", "Repeat pass %1").arg(numbers.front());
                    } else {
                        map["label"] = muse::qtrc("propertiespanel", "Repeat passes %1").arg(nums.join(QStringLiteral(", ")));
                    }
                } else {
                    map["label"] = QString();
                }

                m_passes << map;
            }

            // At this point m_items holds only the pitch, tempo, dynamic and sound sources, which aren't listed again
            std::vector<NotePlaybackMarking> markings;

            for (const NotePlaybackMarking& marking : info.markings) {
                if (std::ranges::find(m_items, marking.item) == m_items.end()) {
                    markings.push_back(marking);
                }
            }

            MarkingRows rows = markingRows(markings, m_items);

            m_tempoMarkings = std::move(rows.tempo);
            m_dynamicMarkings = std::move(rows.dynamic);
            m_otherMarkings = std::move(rows.other);
        }

        updateMuteState(info.note ? info.note : note);
    }

    addSymbolFallbacks();

    emit infoChanged();
}

void NotePlaybackInfoModel::addSymbolFallbacks()
{
    // QRawFont is the font itself; QFontMetrics would also count glyphs from Qt's own fallback fonts
    const QRawFont musicFont = QRawFont::fromFont(QFont(QString::fromStdString(uiConfiguration()->musicalFontFamily())));

    for (QVariantList* rows : { &m_pitchRows, &m_soundRows, &m_tempoMarkings, &m_dynamicMarkings, &m_otherMarkings }) {
        noteplaybackinfomodel_detail::addSymbolFallbacks(*rows, musicFont);
    }

    for (QVariant& passValue : m_passes) {
        QVariantMap pass = passValue.toMap();

        for (const char* key : { "tempoRows", "dynamicRows" }) {
            QVariantList rows = pass.value(key).toList();
            noteplaybackinfomodel_detail::addSymbolFallbacks(rows, musicFont);
            pass[key] = rows;
        }

        passValue = pass;
    }
}

void NotePlaybackInfoModel::updateMuteState(const Note* note)
{
    const notation::INotationPtr notation = currentNotation();
    if (!notation || !note) {
        return;
    }

    const notation::INotationSoloMuteStatePtr soloMute = notation->soloMuteState();
    const notation::INotationPlaybackPtr playback = notation->masterNotation()->playback();

    const InstrumentTrackId trackId = makeInstrumentTrackId(note);
    const bool exists = soloMute->trackSoloMuteStateExists(trackId);
    const bool isSolo = exists && soloMute->trackSoloMuteState(trackId).solo;

    // Same rule as PlaybackController::updateSoloMuteStates (range-selection muting is temporary, so ignored)
    bool otherIsSolo = false;
    for (const InstrumentTrackId& id : playback->existingTrackIdSet()) {
        if (id == playback->metronomeTrackId() || id == trackId) {
            continue;
        }
        if (soloMute->trackSoloMuteStateExists(id) && soloMute->trackSoloMuteState(id).solo) {
            otherIsSolo = true;
            break;
        }
    }

    if (exists && soloMute->trackSoloMuteState(trackId).mute) {
        m_isMuted = true;
        m_muteReason = muse::qtrc("propertiespanel", "Muted in the Mixer");
    } else if (otherIsSolo && !isSolo) {
        m_isMuted = true;
        m_muteReason = muse::qtrc("propertiespanel", "Muted because another instrument is soloed in the Mixer");
    }
}

const EngravingItem* NotePlaybackInfoModel::itemInCurrentScore(const EngravingItem* item) const
{
    const notation::INotationPtr notation = currentNotation();

    if (!item || !notation) {
        return nullptr;
    }

    const Score* score = notation->elements()->msScore();

    if (item->score() == score) {
        return item;
    }

    return item->findLinkedInScore(score);
}

void NotePlaybackInfoModel::hoverItem(int itemIndex)
{
    if (itemIndex < 0 || itemIndex >= static_cast<int>(m_items.size()) || itemIndex == m_hoveredIndex) {
        return;
    }

    m_hoveredIndex = itemIndex;
    applyFocus();
}

void NotePlaybackInfoModel::unhoverItem(int itemIndex)
{
    // Entering the next row can arrive before leaving this one
    if (itemIndex != m_hoveredIndex) {
        return;
    }

    m_hoveredIndex = -1;
    applyFocus();
}

void NotePlaybackInfoModel::clickItem(int itemIndex)
{
    if (itemIndex < 0 || itemIndex >= static_cast<int>(m_items.size())) {
        return;
    }

    if (itemIndex == m_pinnedIndex) {
        // Unpinned: nothing is hovered either until the mouse enters a row again, so the view goes back now
        m_pinnedIndex = -1;
        m_hoveredIndex = -1;
    } else {
        m_pinnedIndex = itemIndex;
    }

    emit focusChanged();

    applyFocus();
}

void NotePlaybackInfoModel::showSelectedNote()
{
    const notation::INotationPtr notation = currentNotation();

    if (!notation) {
        return;
    }

    m_hoveredIndex = -1;
    m_pinnedIndex = -1;
    emit focusChanged();

    const notation::INotationInteractionPtr interaction = notation->interaction();

    interaction->setPlaybackInfluenceHighlight({});

    // Back to where the view was before it followed the listed items, then to the note if that isn't it
    interaction->endCentering(true);

    if (const EngravingItem* note = itemInCurrentScore(selectedNote())) {
        interaction->centerOnItem(note);

        // This is now where the view belongs; a later hover mustn't bring back the place it just left
        interaction->endCentering(false);
    }
}

void NotePlaybackInfoModel::doubleClickItem(int itemIndex)
{
    const notation::INotationPtr notation = currentNotation();

    if (!notation || itemIndex < 0 || itemIndex >= static_cast<int>(m_items.size())) {
        return;
    }

    const EngravingItem* item = itemInCurrentScore(m_items.at(itemIndex));

    if (!item) {
        return;
    }

    // A spanner is selected through one of its segments
    if (item->isSpanner() && !toSpanner(item)->spannerSegments().empty()) {
        item = toSpanner(item)->frontSegment();
    }

    clearFocus();

    notation->interaction()->select({ const_cast<EngravingItem*>(item) }, SelectType::SINGLE);
}

void NotePlaybackInfoModel::applyFocus()
{
    const notation::INotationPtr notation = currentNotation();

    if (!notation) {
        return;
    }

    const notation::INotationInteractionPtr interaction = notation->interaction();

    const int index = (m_hoveredIndex >= 0)
                      ? m_hoveredIndex
                      : m_pinnedIndex;

    if (index < 0) {
        interaction->setPlaybackInfluenceHighlight({});
        interaction->endCentering(true);

        return;
    }

    const EngravingItem* item = m_items.at(index);

    interaction->setPlaybackInfluenceHighlight({ item });

    if (const EngravingItem* shown = itemInCurrentScore(item)) {
        interaction->centerOnItem(shown);
    }
}

void NotePlaybackInfoModel::clearFocus()
{
    const bool hadFocus = (m_hoveredIndex >= 0) || (m_pinnedIndex >= 0);

    m_hoveredIndex = -1;
    m_pinnedIndex = -1;

    if (!hadFocus) {
        return;
    }

    emit focusChanged();

    if (const notation::INotationPtr notation = currentNotation()) {
        notation->interaction()->setPlaybackInfluenceHighlight({});

        // Forget where the view was without scrolling back: the selection is changing
        notation->interaction()->endCentering(false);
    }
}
