/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 */

#pragma once

#include <QAbstractListModel>
#include <QStringList>
#include <qqmlintegration.h>

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

#include "audio/main/iplayback.h"
#include "iplaybackcontroller.h"

namespace mu::playback {
class MuseSoundsReassignModel : public QAbstractListModel, public muse::Contextable, public muse::async::Asyncable
{
    Q_OBJECT
    Q_PROPERTY(bool hasChoices READ hasChoices NOTIFY choicesChanged)

    QML_ELEMENT

    muse::ContextInject<context::IGlobalContext> context = { this };
    muse::ContextInject<IPlaybackController> controller = { this };
    muse::ContextInject<muse::audio::IPlayback> playback = { this };

public:
    explicit MuseSoundsReassignModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void setSelectedCandidate(int row, int candidateIndex);
    Q_INVOKABLE void apply();

    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool hasChoices() const;

signals:
    void choicesChanged();

private:
    enum Roles {
        StaffNameRole = Qt::UserRole + 1,
        CurrentSoundRole,
        CandidateTitlesRole,
        SelectedCandidateIndexRole,
        CandidateCountRole
    };

    struct Candidate {
        muse::audio::AudioResourceMeta resource;
        QString title;
        int score = 0;
    };

    struct TrackChoice {
        engraving::InstrumentTrackId instrumentTrackId;
        muse::audio::TrackId audioTrackId = muse::audio::INVALID_TRACK_ID;
        QString staffName;
        QString currentSound;
        QStringList candidateTitles;
        std::vector<Candidate> candidates;
        int selectedCandidateIndex = 0;
    };

    void buildChoices(const muse::audio::AudioResourceMetaList& availableResources);
    std::vector<Candidate> candidatesForTrack(const QString& staffName, const muse::mpe::PlaybackSetupData& setupData,
                                              const muse::audio::AudioResourceMetaList& museResources) const;
    QString trackName(const engraving::InstrumentTrackId& instrumentTrackId) const;

    std::vector<TrackChoice> m_choices;
};
}
