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
#include <set>

#include <QRegularExpression>

#include "translation.h"

#include "engraving/automation/dynamicvalues.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/breath.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/fermata.h"
#include "engraving/dom/gradualtempochange.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/textbase.h"
#include "engraving/dom/textlinebase.h"
#include "engraving/dom/utils.h"
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
        const bool isQuoted = !(item->isDynamic() || item->isTempoText());

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

    QString text;
    QString prefix;
    QString detail;
};

//! One row per spec: { parts, prefix, location, detail, itemIndex }; itemIndex is -1 when there's no item to show.
//! Rows that would read the same (e.g. a hairpin on each staff of a piano) get the staff in their location.
QVariantList makeRows(const std::vector<RowSpec>& specs, std::vector<const EngravingItem*>& items)
{
    std::vector<QVariantList> parts;
    QStringList keys;

    for (const RowSpec& spec : specs) {
        parts.push_back((spec.item && spec.text.isEmpty())
                        ? itemParts(spec.item)
                        : QVariantList { labelPart(spec.text, false) }
                        );

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

    map["tempo"] = muse::qtrc("propertiespanel", "%1 BPM").arg(formatBpm(bpm));

    if (std::abs(tempoMultiplier - 1.0) > 1e-6) {
        map["tempoDetail"] = muse::qtrc("propertiespanel", "Plays at %1 BPM (%2% playback speed)")
                             .arg(formatBpm(pass.multipliedTempo.toBPM().val))
                             .arg(static_cast<int>(std::round(tempoMultiplier * 100)));
    } else {
        map["tempoDetail"] = QString();
    }

    map["tempoRows"] = pass.isDefaultTempo
                       ? textRows(muse::qtrc("propertiespanel", "Default tempo (no tempo marking before this note)"), items)
                       : sourceRows(pass.tempoSources, pass.tempoOrigins, items);

    // --- Dynamic: the most recent ordinary dynamic's symbol, "≈" when the level is between two of them
    map["dynamicPercent"] = muse::qtrc("propertiespanel", "%1%").arg(static_cast<int>(std::round(pass.dynamicValue * 100)));

    if (pass.isDefaultDynamic) {
        map["dynamicText"] = muse::qtrc("propertiespanel", "Default");
        map["dynamicSymbol"] = QString();
        map["dynamicApprox"] = false;
        map["dynamicRows"] = textRows(muse::qtrc("propertiespanel", "No dynamic applies to this note yet"), items);
    } else {
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
        map["dynamicRows"] = sourceRows(pass.dynamicSources, pass.dynamicOrigins, items);
    }

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

//! The markings each filter accepts, filter by filter; claimed records which markings were taken
std::vector<const NotePlaybackMarking*> filterMarkings(
    const std::vector<NotePlaybackMarking>& markings,
    const std::vector<MarkingFilter>& filters,
    std::set<const NotePlaybackMarking*>& claimed)
{
    std::vector<const NotePlaybackMarking*> result;

    for (const MarkingFilter& filter : filters) {
        for (const NotePlaybackMarking& marking : markings) {
            const bool isTaken = std::ranges::find(result, &marking) != result.end();

            if (!isTaken && filter(marking)) {
                result.push_back(&marking);
                claimed.insert(&marking);
            }
        }
    }

    return result;
}

//! For markings better named by what they do than by what they are: a slur → "Legato", a tremolo → "32nd Tremolo"
QString effectLabel(const NotePlaybackMarking& marking)
{
    using muse::mpe::ArticulationType;

    if (marking.item->isSlur() && hasEffect(marking, { ArticulationType::Legato })) {
        return muse::qtrc("propertiespanel", "Legato");
    }

    static const std::vector<std::pair<ArticulationType, const char*> > TREMOLO_SPEEDS {
        { ArticulationType::Tremolo8th,  QT_TRANSLATE_NOOP("propertiespanel", "8th") },
        { ArticulationType::Tremolo16th, QT_TRANSLATE_NOOP("propertiespanel", "16th") },
        { ArticulationType::Tremolo32nd, QT_TRANSLATE_NOOP("propertiespanel", "32nd") },
        { ArticulationType::Tremolo64th, QT_TRANSLATE_NOOP("propertiespanel", "64th") },
        { ArticulationType::TremoloBuzz, QT_TRANSLATE_NOOP("propertiespanel", "Buzz") },
    };

    for (const auto& [type, speed] : TREMOLO_SPEEDS) {
        if (hasEffect(marking, { type })) {
            //: %1 is the tremolo's speed, e.g. "32nd" or "Buzz"
            return muse::qtrc("propertiespanel", "%1 Tremolo").arg(muse::qtrc("propertiespanel", speed));
        }
    }

    return QString();
}

QVariantList markingRows(const std::vector<const NotePlaybackMarking*>& markings, std::vector<const EngravingItem*>& items)
{
    std::vector<RowSpec> specs;

    for (const NotePlaybackMarking* marking : markings) {
        const QString label = effectLabel(*marking);

        // The label already says what the marking does
        const QString detail = label.isEmpty()
                               ? markingEffects(*marking)
                               : QString();

        specs.push_back({ marking->item, label, QString(), detail });
    }

    return makeRows(specs, items);
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

            // At this point m_items holds only the tempo and dynamic sources, which aren't listed again
            std::vector<NotePlaybackMarking> markings;

            for (const NotePlaybackMarking& marking : info.markings) {
                if (std::ranges::find(m_items, marking.item) == m_items.end()) {
                    markings.push_back(marking);
                }
            }

            std::set<const NotePlaybackMarking*> claimed;

            m_tempoMarkings = markingRows(filterMarkings(markings, tempoMarkingFilters(), claimed), m_items);
            m_dynamicMarkings = markingRows(filterMarkings(markings, dynamicMarkingFilters(), claimed), m_items);

            std::vector<const NotePlaybackMarking*> otherMarkings;

            for (const NotePlaybackMarking& marking : markings) {
                if (!claimed.contains(&marking)) {
                    otherMarkings.push_back(&marking);
                }
            }

            m_otherMarkings = markingRows(otherMarkings, m_items);
        }

        updateMuteState(info.note ? info.note : note);
    }

    emit infoChanged();
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

    m_pinnedIndex = itemIndex;
    emit focusChanged();

    applyFocus();
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
