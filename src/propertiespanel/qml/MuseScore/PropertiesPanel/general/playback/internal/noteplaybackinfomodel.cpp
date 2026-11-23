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

#include "translation.h"

#include "engraving/automation/dynamicvalues.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/textbase.h"
#include "engraving/dom/utils.h"
#include "engraving/types/typesconv.h"

#include "notation/imasternotation.h"
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
    if (item->isDynamic()) {
        const Dynamic* dynamic = toDynamic(item);

        if (dynamic->dynamicType() != mu::engraving::DynamicType::OTHER) {
            return muse::qtrc("propertiespanel", "Dynamic %1")
                   .arg(QString(TConv::toXml(dynamic->dynamicType()).toQLatin1String()));
        }
    }

    if (item->isTempoText()) {
        const TempoText* tt = toTempoText(item);

        return muse::qtrc("propertiespanel", "Tempo marking (%1 BPM)")
               .arg(formatBpm(tt->tempo().toBPM().val));
    }

    QString name = item->translatedSubtypeUserName().toQString();

    if (name.isEmpty()) {
        name = item->translatedTypeUserName().toQString();
    }

    if (item->isTextBase()) {
        const QString text = toTextBase(item)->plainText().toQString().simplified();

        if (!text.isEmpty() && text != name) {
            name += QStringLiteral(" “") + text + QStringLiteral("”");
        }
    }

    return name;
}

QString describeSources(const std::vector<NotePlaybackSource>& sources)
{
    QStringList lines;

    for (const NotePlaybackSource& source : sources) {
        if (!source.item) {
            lines << muse::qtrc("propertiespanel", "Point drawn in the automation lane");
            continue;
        }

        QString line = itemName(source.item);
        const QString ref = measureRef(source.item);

        if (!ref.isEmpty()) {
            line += QStringLiteral(" (") + ref + QStringLiteral(")");
        }

        if (source.editedInAutomation) {
            line += QStringLiteral(" — ") + muse::qtrc("propertiespanel", "edited in the automation lane");
        }

        lines << line;
    }

    return lines.join(QLatin1Char('\n'));
}

QVariantMap passToMap(const NotePlaybackPass& pass, double tempoMultiplier)
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

    map["tempoSource"] = pass.isDefaultTempo
                         ? muse::qtrc("propertiespanel", "Default tempo (no tempo marking before this note)")
                         : describeSources(pass.tempoSources);

    // --- Dynamic
    const int percent = static_cast<int>(std::round(pass.dynamicValue * 100));

    if (pass.isDefaultDynamic) {
        map["dynamic"] = muse::qtrc("propertiespanel", "Default");
        map["dynamicSource"] = muse::qtrc("propertiespanel", "No dynamic applies to this note yet");
    } else {
        const mu::engraving::DynamicType nearest = nearestOrdinaryDynamic(pass.dynamicValue);
        const QString name = QString(TConv::toXml(nearest).toQLatin1String());

        const auto it = ORDINARY_DYNAMIC_VALUES.find(nearest);
        const bool exact = it != ORDINARY_DYNAMIC_VALUES.end() && std::abs(it->second.raw() - pass.dynamicValue) < 0.005;

        map["dynamic"] = exact ? name : QStringLiteral("≈ ") + name;
        map["dynamicSource"] = describeSources(pass.dynamicSources);
    }

    map["dynamicDetail"] = muse::qtrc("propertiespanel", "%1% level").arg(percent);

    return map;
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
        [this](
            const InstrumentTrackId&,
            const notation::INotationSoloMuteState::SoloMuteState&
        ) {
        reload();
    }, Mode::SetReplace
        );

    // Fires after the playback data is re-rendered, which includes automation lane edits
    notation->masterNotation()->playback()->tracksDataChanged().onReceive(
        this,
        [this](const InstrumentTrackIdSet&) {
        reload();
    }, Mode::SetReplace
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
    m_isTiedContinuation = false;

    const Note* note = selectedNote();
    const notation::INotationPtr notation = currentNotation();

    if (note && notation) {
        const notation::INotationPlaybackPtr playback = notation->masterNotation()->playback();
        const NotePlaybackInfo info = playback->notePlaybackInfo(note);

        if (info.isValid()) {
            m_velocityOverride = info.userVelocity;
            m_isTiedContinuation = info.attackNote != info.note;

            const double multiplier = playback->tempoMultiplier();

            // Merge passes that play identically
            std::vector<std::pair<QVariantMap, QList<int> > > groups;
            for (const NotePlaybackPass& pass : info.passes) {
                const QVariantMap map = passToMap(pass, multiplier);
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
