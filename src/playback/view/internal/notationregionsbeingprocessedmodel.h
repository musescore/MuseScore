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

#pragma once

#include <QAbstractListModel>

#include "async/asyncable.h"
#include "modularity/ioc.h"

#include "context/iglobalcontext.h"
#include "audio/iplayback.h"
#include "iplaybackcontroller.h"
#include "iplaybackconfiguration.h"
#include "notation/inotationconfiguration.h"

namespace mu::playback {
class NotationRegionsBeingProcessedModel : public QAbstractListModel, public muse::async::Asyncable, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<mu::context::IGlobalContext> globalContext = { this };
    muse::Inject<muse::audio::IPlayback> playback = { this };
    muse::Inject<IPlaybackController> playbackController = { this };
    muse::Inject<IPlaybackConfiguration> configuration = { this };
    muse::Inject<notation::INotationConfiguration> notationConfiguration = { this };

    Q_PROPERTY(QVariant notationViewMatrix READ notationViewMatrix WRITE setNotationViewMatrix NOTIFY notationViewMatrixChanged)

    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY isEmptyChanged)

    Q_PROPERTY(QColor progressBackgroundColor READ progressBackgroundColor CONSTANT)
    Q_PROPERTY(QColor progressTextColor READ progressTextColor CONSTANT)

public:
    explicit NotationRegionsBeingProcessedModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();

    QVariant notationViewMatrix() const;

    bool isEmpty() const;

    QColor progressBackgroundColor() const;
    QColor progressTextColor() const;

public slots:
    void setNotationViewMatrix(const QVariant& matrix);

signals:
    void notationViewMatrixChanged();

    void isEmptyChanged();

private:
    enum Roles {
        RectRole = Qt::UserRole + 1,
        ProgressRole,
    };

    struct RegionInfo {
        mu::engraving::InstrumentTrackId trackId;
        QRectF rect;

        bool operator==(const RegionInfo& r) const
        {
            return trackId == r.trackId && rect == r.rect;
        }
    };

    struct TickRange {
        int tickFrom = -1;
        int tickTo = -1;

        bool operator==(const TickRange& r) const
        {
            return tickFrom == r.tickFrom && tickTo == r.tickTo;
        }
    };

    struct TrackInfo {
        std::vector<TickRange> ranges;
        int progress = 0; // 0 - 100%

        bool operator==(const TrackInfo& t) const
        {
            return ranges == t.ranges && progress == t.progress;
        }
    };

    using TracksBeingProcessed = std::unordered_map<mu::engraving::InstrumentTrackId, TrackInfo>;
    using ChunkInfoList = muse::audio::InputProcessingProgress::ChunkInfoList;

    bool isPlaying() const;

    void clear();

    void onOnlineSoundsChanged();
    void onIsPlayingChanged();

    void startListeningToProgress(const muse::audio::TrackId trackId);
    void stopListeningToProgress(const muse::audio::TrackId trackId);

    void onProgressStarted(const mu::engraving::InstrumentTrackId& instrumentTrackId);
    void onChunksReceived(const mu::engraving::InstrumentTrackId& instrumentTrackId, const ChunkInfoList& chunks);
    void onProgressChanged(const mu::engraving::InstrumentTrackId& instrumentTrackId, int progress);
    void onProgressFinished(const mu::engraving::InstrumentTrackId& instrumentTrackId);

    void initShouldShowRegions();
    void updateRegionsBeingProcessed(const TracksBeingProcessed& tracks);

    std::vector<QRectF> calculateRects(const mu::engraving::Part* part, const std::vector<TickRange>& ranges) const;
    std::vector<QRectF> calculateRects(const mu::engraving::Part* part, const mu::engraving::System* system,
                                       const std::vector<TickRange>& ranges) const;

    QVariant m_notationViewMatrix;
    std::set<muse::audio::TrackId> m_onlineSounds;
    TracksBeingProcessed m_tracksBeingProcessed;
    QList<RegionInfo> m_regions; // main model
    bool m_shouldShowRegions = true;
};
}
