/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "notationregionsbeingprocessedmodel.h"

using namespace muse::audio;
using namespace mu::playback;
using namespace mu::engraving;

static const Segment* findSegmentFrom(const Score* score, const System* system,
                                      const int tickFrom, const staff_idx_t staffIdx)
{
    const Segment* segment = score->tick2segment(Fraction::fromTicks(tickFrom), true, SegmentType::ChordRest);
    if (!segment || segment->system() != system) {
        return nullptr;
    }

    const track_idx_t trackIdx = staff2track(staffIdx);
    const EngravingItem* item = segment->elementAt(trackIdx);
    if (!item) {
        return nullptr;
    }

    if (!item->isRest()) {
        return segment;
    }

    while (segment && item && item->isRest()) {
        segment = segment->next1(SegmentType::ChordRest);
        item = segment ? segment->elementAt(trackIdx) : nullptr;
    }

    return segment;
}

static const Segment* findSegmentTo(const Score* score, const System* system, const Segment* segmentFrom,
                                    const int tickTo, const staff_idx_t staffIdx)
{
    const Segment* segment = score->tick2segment(Fraction::fromTicks(tickTo), true, SegmentType::ChordRest);
    if (!segment || segment->system() != system) {
        return nullptr;
    }

    if (segment == segmentFrom) {
        return segment;
    }

    const track_idx_t trackIdx = staff2track(staffIdx);
    const EngravingItem* item = segment->elementAt(trackIdx);
    if (!item) {
        return nullptr;
    }

    if (!item->isRest()) {
        return segment;
    }

    while (segment && item && item->isRest()) {
        segment = segment->prev1(SegmentType::ChordRest);
        item = segment ? segment->elementAt(trackIdx) : nullptr;
    }

    return segment;
}

NotationRegionsBeingProcessedModel::NotationRegionsBeingProcessedModel(QObject* parent)
    : QAbstractListModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

QVariant NotationRegionsBeingProcessedModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const RegionInfo& region = m_regions.at(index.row());

    switch (role) {
    case RectRole: return region.rect;
    case ProgressRole: {
        auto it = m_tracksBeingProcessed.find(region.trackId);
        return it != m_tracksBeingProcessed.end() ? it->second.progress : 0;
    }
    }

    return QVariant();
}

int NotationRegionsBeingProcessedModel::rowCount(const QModelIndex&) const
{
    return m_regions.size();
}

QHash<int, QByteArray> NotationRegionsBeingProcessedModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { RectRole, "rect" },
        { ProgressRole, "progress" },
    };

    return roles;
}

void NotationRegionsBeingProcessedModel::load()
{
    clear();
    globalContext()->currentMasterNotationChanged().onNotify(this, [this]() {
        clear();
    });

    onOnlineSoundsChanged();
    playbackController()->onlineSoundsChanged().onNotify(this, [this]() {
        onOnlineSoundsChanged();
    });

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        updateRegionsBeingProcessed(m_tracksBeingProcessed);
    });

    globalContext()->playbackState()->playbackStatusChanged().onReceive(this, [this](muse::audio::PlaybackStatus) {
        onIsPlayingChanged();
    });

    configuration()->onlineSoundsShowProgressBarModeChanged().onNotify(this, [this]() {
        const bool wasEmpty = m_regions.empty();

        initShouldShowRegions();
        updateRegionsBeingProcessed(m_tracksBeingProcessed);

        if (wasEmpty != m_regions.empty()) {
            emit isEmptyChanged();
        }
    });
}

QVariant NotationRegionsBeingProcessedModel::notationViewMatrix() const
{
    return m_notationViewMatrix;
}

bool NotationRegionsBeingProcessedModel::isEmpty() const
{
    return m_regions.empty();
}

QColor NotationRegionsBeingProcessedModel::progressBackgroundColor() const
{
    return notationConfiguration()->selectionColor(VOICES); // "all voices" color
}

QColor NotationRegionsBeingProcessedModel::progressTextColor() const
{
    return notationConfiguration()->notationColor();
}

void NotationRegionsBeingProcessedModel::setNotationViewMatrix(const QVariant& matrix)
{
    if (m_notationViewMatrix == matrix) {
        return;
    }

    m_notationViewMatrix = matrix;
    emit notationViewMatrixChanged();

    updateRegionsBeingProcessed(m_tracksBeingProcessed);
}

bool NotationRegionsBeingProcessedModel::isPlaying() const
{
    return globalContext()->playbackState()->playbackStatus() == muse::audio::PlaybackStatus::Running;
}

void NotationRegionsBeingProcessedModel::clear()
{
    m_onlineSounds.clear();
    m_tracksBeingProcessed.clear();
    initShouldShowRegions();

    if (m_regions.empty()) {
        return;
    }

    beginResetModel();
    m_regions.clear();
    endResetModel();

    emit isEmptyChanged();
}

void NotationRegionsBeingProcessedModel::onOnlineSoundsChanged()
{
    const std::set<TrackId>& newSounds = playbackController()->onlineSounds();

    for (const TrackId& trackId : newSounds) {
        if (!muse::contains(m_onlineSounds, trackId)) {
            startListeningToProgress(trackId);
        }
    }

    for (const TrackId trackId : m_onlineSounds) {
        if (!muse::contains(newSounds, trackId)) {
            stopListeningToProgress(trackId);
        }
    }

    m_onlineSounds = newSounds;
}

void NotationRegionsBeingProcessedModel::onIsPlayingChanged()
{
    const OnlineSoundsShowProgressBarMode mode = configuration()->onlineSoundsShowProgressBarMode();
    if (mode != OnlineSoundsShowProgressBarMode::DuringPlayback) {
        return;
    }

    if (!isPlaying()) {
        m_shouldShowRegions = !m_tracksBeingProcessed.empty();
        return;
    }

    m_shouldShowRegions = true;
    updateRegionsBeingProcessed(m_tracksBeingProcessed);

    if (!m_tracksBeingProcessed.empty()) {
        emit isEmptyChanged();
    }
}

void NotationRegionsBeingProcessedModel::startListeningToProgress(const TrackId trackId)
{
    const IPlaybackController::InstrumentTrackIdMap& instrumentTrackIdMap = playbackController()->instrumentTrackIdMap();
    const InstrumentTrackId instrumentTrackId = muse::key(instrumentTrackIdMap, trackId);
    if (!instrumentTrackId.isValid()) {
        return;
    }

    const TrackSequenceId sequenceId = playbackController()->currentTrackSequenceId();

    playback()->inputProcessingProgress(sequenceId, trackId)
    .onResolve(this, [this, instrumentTrackId](InputProcessingProgress inputProgress) {
        if (inputProgress.isStarted) {
            onProgressStarted(instrumentTrackId);
        }

        inputProgress.processedChannel.onReceive(this, [this, instrumentTrackId]
                                                 (const InputProcessingProgress::StatusInfo& status,
                                                  const InputProcessingProgress::ChunkInfoList& chunks,
                                                  const InputProcessingProgress::ProgressInfo& progress)
        {
            switch (status.status) {
                case InputProcessingProgress::Undefined:
                    break;
                case InputProcessingProgress::Started: {
                    onProgressStarted(instrumentTrackId);
                } break;
                case InputProcessingProgress::Processing: {
                    if (!chunks.empty()) {
                        onChunksReceived(instrumentTrackId, chunks);
                    }
                    onProgressChanged(instrumentTrackId, progress.current);
                } break;
                case InputProcessingProgress::Finished: {
                    onProgressFinished(instrumentTrackId);
                } break;
            }
        });
    });
}

void NotationRegionsBeingProcessedModel::stopListeningToProgress(const muse::audio::TrackId trackId)
{
    const IPlaybackController::InstrumentTrackIdMap& instrumentTrackIdMap = playbackController()->instrumentTrackIdMap();
    const InstrumentTrackId instrumentTrackId = muse::key(instrumentTrackIdMap, trackId);
    if (!instrumentTrackId.isValid()) {
        return;
    }

    onProgressFinished(instrumentTrackId);
}

void NotationRegionsBeingProcessedModel::onProgressStarted(const InstrumentTrackId& instrumentTrackId)
{
    if (!muse::contains(m_tracksBeingProcessed, instrumentTrackId)) {
        m_tracksBeingProcessed[instrumentTrackId] = TrackInfo();
    }
}

void NotationRegionsBeingProcessedModel::onChunksReceived(const InstrumentTrackId& instrumentTrackId, const ChunkInfoList& chunks)
{
    const notation::IMasterNotationPtr master = globalContext()->currentMasterNotation();
    if (!master) {
        return;
    }

    auto it = m_tracksBeingProcessed.find(instrumentTrackId);
    if (it == m_tracksBeingProcessed.end()) {
        return;
    }

    const bool wasEmpty = m_regions.empty();

    TrackInfo& info = it->second;
    info.ranges.clear();

    bool shouldUpdate = false;

    for (const InputProcessingProgress::ChunkInfo& chunk : chunks) {
        TickRange range;
        range.tickFrom = master->playback()->secToPlayedTick(chunk.start);
        range.tickTo = master->playback()->secToPlayedTick(chunk.end);

        if (!muse::contains(info.ranges, range)) {
            info.ranges.push_back(range);
            shouldUpdate = true;
        }
    }

    if (shouldUpdate) {
        updateRegionsBeingProcessed({ { instrumentTrackId, info } });
    }

    if (wasEmpty != m_regions.empty()) {
        emit isEmptyChanged();
    }
}

void NotationRegionsBeingProcessedModel::onProgressChanged(const InstrumentTrackId& instrumentTrackId, int progress)
{
    auto trackIt = m_tracksBeingProcessed.find(instrumentTrackId);
    if (trackIt == m_tracksBeingProcessed.end()) {
        return;
    }

    if (trackIt->second.progress == progress) {
        return;
    }

    trackIt->second.progress = progress;

    for (int i = 0; i < m_regions.size(); ++i) {
        const RegionInfo& region = m_regions.at(i);
        if (region.trackId != instrumentTrackId) {
            continue;
        }

        QModelIndex modelIdx = index(i);
        emit dataChanged(modelIdx, modelIdx, { ProgressRole });
    }
}

void NotationRegionsBeingProcessedModel::onProgressFinished(const InstrumentTrackId& instrumentTrackId)
{
    muse::remove(m_tracksBeingProcessed, instrumentTrackId);

    const QList<RegionInfo> regionsCopy = m_regions;
    const bool wasEmpty = m_regions.empty();

    for (const RegionInfo& region : regionsCopy) {
        if (region.trackId != instrumentTrackId) {
            continue;
        }

        const int idx = m_regions.indexOf(region);
        if (idx < 0) {
            continue;
        }

        beginRemoveRows(QModelIndex(), idx, idx);
        m_regions.removeAt(idx);
        endRemoveRows();
    }

    if (wasEmpty != m_regions.empty()) {
        initShouldShowRegions();
        emit isEmptyChanged();
    }
}

void NotationRegionsBeingProcessedModel::initShouldShowRegions()
{
    switch (configuration()->onlineSoundsShowProgressBarMode()) {
    case OnlineSoundsShowProgressBarMode::Always:
        m_shouldShowRegions = true;
        break;
    case OnlineSoundsShowProgressBarMode::Never:
        m_shouldShowRegions = false;
        break;
    case OnlineSoundsShowProgressBarMode::DuringPlayback:
        m_shouldShowRegions = isPlaying();
        break;
    }
}

void NotationRegionsBeingProcessedModel::updateRegionsBeingProcessed(const TracksBeingProcessed& tracks)
{
    if (!m_shouldShowRegions || tracks.empty()) {
        return;
    }

    const notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    QList<RegionInfo> newRegions = m_regions;

    newRegions.removeIf([&tracks](const RegionInfo& r) {
        return muse::contains(tracks, r.trackId);
    });

    for (const auto& pair : tracks) {
        const Part* part = notation->parts()->part(pair.first.partId);
        if (!part) {
            continue;
        }

        const std::vector<QRectF> newRects = calculateRects(part, pair.second.ranges);

        for (const QRectF& rect : newRects) {
            RegionInfo region;
            region.trackId = pair.first;
            region.rect = rect;

            newRegions.push_back(region);
        }
    }

    if (m_regions != newRegions) {
        beginResetModel();
        m_regions = std::move(newRegions);
        endResetModel();
    }
}

std::vector<QRectF> NotationRegionsBeingProcessedModel::calculateRects(const Part* part, const std::vector<TickRange>& ranges) const
{
    std::vector<QRectF> result;

    for (const System* system : part->score()->systems()) {
        if (system->measures().empty() || system->staves().empty()) {
            continue;
        }

        std::vector<QRectF> systemRects = calculateRects(part, system, ranges);
        result.insert(result.end(), std::make_move_iterator(systemRects.begin()),
                      std::make_move_iterator(systemRects.end()));
    }

    return result;
}

std::vector<QRectF> NotationRegionsBeingProcessedModel::calculateRects(const Part* part, const System* system,
                                                                       const std::vector<TickRange>& ranges) const
{
    const staff_idx_t staffIdx = system->firstVisibleSysStaffOfPart(part);
    const SysStaff* sysStaff = system->staff(staffIdx);
    if (!sysStaff) {
        return {};
    }

    const Score* score = system->score();
    const Measure* lastMeasure = score->lastMeasure();
    const double lastMeasureEndX = lastMeasure ? lastMeasure->canvasPos().x() + lastMeasure->width() : 0.0;
    const QTransform matrix = m_notationViewMatrix.value<QTransform>();

    std::vector<QRectF> result;

    for (const TickRange& range : ranges) {
        if (system->last()->tick().ticks() < range.tickFrom) {
            continue;
        }

        if (system->first()->tick().ticks() > range.tickTo) {
            continue;
        }

        const muse::PointF systemPos = system->canvasPos();
        muse::RectF logicRect = sysStaff->bbox().translated(systemPos);

        const Segment* segmentFrom = findSegmentFrom(score, system, range.tickFrom, staffIdx);
        if (segmentFrom) {
            const double segmentFromStartX = segmentFrom->canvasPos().x();
            const double startX = std::max(segmentFromStartX, logicRect.x());
            logicRect.setLeft(startX);
        } else {
            const double firstNoteRestX = systemPos.x() + system->firstNoteRestSegmentX();
            logicRect.setLeft(firstNoteRestX);
        }

        const Segment* segmentTo = findSegmentTo(score, system, segmentFrom, range.tickTo, staffIdx);
        if (segmentTo) {
            const double segmentToEndX = segmentTo->canvasPos().x() + segmentTo->width();
            const double endX = std::min(systemPos.x() + system->width(), segmentToEndX);
            logicRect.setRight(endX);
        } else if (lastMeasure) {
            const double width = std::min(logicRect.width(), lastMeasureEndX - logicRect.x());
            logicRect.setWidth(width);
        }

        const QRectF newRect = matrix.mapRect(logicRect.toQRectF());
        bool shouldAdd = true;

        for (QRectF& rect: result) {
            if (rect.intersects(newRect)) {
                rect = rect.united(newRect);
                shouldAdd = false;
                break;
            }
        }

        if (shouldAdd) {
            result.push_back(newRect);
        }
    }

    return result;
}
