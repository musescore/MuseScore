/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_PLAYBACK_PLAYBACKTOOLBARMODEL_H
#define MU_PLAYBACK_PLAYBACKTOOLBARMODEL_H

#include "uicomponents/view/abstractmenumodel.h"

#include "modularity/ioc.h"
#include "iplaybackcontroller.h"
#include "notation/inotationconfiguration.h"
#include "context/iglobalcontext.h"

namespace mu::playback {
class PlaybackToolBarModel : public muse::uicomponents::AbstractMenuModel
{
    Q_OBJECT

    Q_PROPERTY(bool isToolbarFloating READ isToolbarFloating WRITE setIsToolbarFloating NOTIFY isToolbarFloatingChanged)
    Q_PROPERTY(bool isPlayAllowed READ isPlayAllowed NOTIFY isPlayAllowedChanged)

    Q_PROPERTY(QDateTime maxPlayTime READ maxPlayTime NOTIFY maxPlayTimeChanged)

    Q_PROPERTY(QDateTime playTime READ playTime WRITE setPlayTime NOTIFY playPositionChanged)
    Q_PROPERTY(qreal playPosition READ playPosition WRITE setPlayPosition NOTIFY playPositionChanged)
    Q_PROPERTY(int measureNumber READ measureNumber WRITE setMeasureNumber NOTIFY playPositionChanged)
    Q_PROPERTY(int maxMeasureNumber READ maxMeasureNumber NOTIFY playPositionChanged)
    Q_PROPERTY(int beatNumber READ beatNumber WRITE setBeatNumber NOTIFY playPositionChanged)
    Q_PROPERTY(int maxBeatNumber READ maxBeatNumber NOTIFY playPositionChanged)

    Q_PROPERTY(QVariant tempo READ tempo NOTIFY tempoChanged)
    Q_PROPERTY(qreal tempoMultiplier READ tempoMultiplier WRITE setTempoMultiplier NOTIFY tempoChanged)

    muse::Inject<IPlaybackController> playbackController;
    muse::Inject<context::IGlobalContext> globalContext;
    muse::Inject<notation::INotationConfiguration> notationConfiguration = { this };

public:
    explicit PlaybackToolBarModel(QObject* parent = nullptr);

    bool isToolbarFloating() const;
    bool isPlayAllowed() const;

    QDateTime maxPlayTime() const;
    QDateTime playTime() const;
    qreal playPosition() const;

    int measureNumber() const;
    int maxMeasureNumber() const;
    int beatNumber() const;
    int maxBeatNumber() const;

    QVariant tempo() const;
    qreal tempoMultiplier() const;

    Q_INVOKABLE void load() override;

public slots:
    void setIsToolbarFloating(bool floating);
    void setPlayPosition(qreal position);
    void setPlayTime(const QDateTime& time);
    void setMeasureNumber(int measureNumber);
    void setBeatNumber(int beatNumber);
    void setTempoMultiplier(qreal multiplier);

signals:
    void isToolbarFloatingChanged(bool floating);
    void isPlayAllowedChanged();
    void maxPlayTimeChanged();
    void playPositionChanged();
    void tempoChanged();

private:
    void setupConnections();
    muse::uicomponents::MenuItem* makeInputPitchMenu();

    void updateActions();
    void onActionsStateChanges(const muse::actions::ActionCodeList& codes) override;

    bool isAdditionalAction(const muse::actions::ActionCode& actionCode) const;

    QTime totalPlayTime() const;
    notation::MeasureBeat measureBeat() const;

    muse::ui::UiAction playAction() const;

    void updatePlayPosition(muse::audio::secs_t secs);
    void doSetPlayTime(const QTime& time);

    void rewind(muse::audio::secs_t secs);
    void rewindToBeat(const notation::MeasureBeat& beat);

    bool m_isToolbarFloating = false;
    QTime m_playTime;
};
}

#endif // MU_PLAYBACK_PLAYBACKTOOLBARMODEL_H
