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
#include "playbacktoolbarmodel.h"

#include "types/translatablestring.h"

#include "ui/view/musicalsymbolcodes.h"
#include "playback/playbacktypes.h"
#include "playback/internal/playbackuiactions.h"

using namespace mu::playback;
using namespace mu::notation;
using namespace muse;
using namespace muse::actions;
using namespace muse::ui;
using namespace muse::uicomponents;
using namespace muse::audio;

static const ActionCode PLAY_ACTION_CODE("play");

static MusicalSymbolCodes::Code tempoDurationToNoteIcon(DurationType durationType)
{
    QMap<DurationType, MusicalSymbolCodes::Code> durationToNoteIcon {
        { DurationType::V_WHOLE, MusicalSymbolCodes::Code::SEMIBREVE },
        { DurationType::V_HALF, MusicalSymbolCodes::Code::MINIM },
        { DurationType::V_QUARTER, MusicalSymbolCodes::Code::CROTCHET },
        { DurationType::V_EIGHTH, MusicalSymbolCodes::Code::QUAVER },
        { DurationType::V_16TH, MusicalSymbolCodes::Code::SEMIQUAVER }
    };

    return durationToNoteIcon[durationType];
}

PlaybackToolBarModel::PlaybackToolBarModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

void PlaybackToolBarModel::load()
{
    AbstractMenuModel::load();
    updateActions();
    setupConnections();
}

void PlaybackToolBarModel::setupConnections()
{
    connect(this, &PlaybackToolBarModel::isToolbarFloatingChanged, this, &PlaybackToolBarModel::updateActions);

    playbackController()->isPlayAllowedChanged().onNotify(this, [this]() {
        emit isPlayAllowedChanged();
    });

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        onActionsStateChanges({ PLAY_ACTION_CODE });
    });

    playbackController()->playbackPositionChanged().onReceive(this, [this](secs_t secs) {
        updatePlayPosition(secs);
    });

    playbackController()->totalPlayTimeChanged().onNotify(this, [this]() {
        emit maxPlayTimeChanged();
        playbackController()->playbackPosition().onResolve(this, [this](secs_t pos) {
            updatePlayPosition(pos);
        });
    });

    playbackController()->currentTempoChanged().onNotify(this, [this]() {
        emit tempoChanged();
    });
}

void PlaybackToolBarModel::updateActions()
{
    MenuItemList result;
    MenuItemList settingsItems;

    for (const UiAction& action : PlaybackUiActions::settingsActions()) {
        settingsItems << makeMenuItem(action.code);
    }

    if (!m_isToolbarFloating) {
        settingsItems << makeSeparator();
    }

    //! NOTE At the moment no customization ability
    ToolConfig config = PlaybackUiActions::defaultPlaybackToolConfig();
    for (const ToolConfig::Item& item : config.items) {
        if (isAdditionalAction(item.action) && !m_isToolbarFloating) {
            //! NOTE In this case, we want to see the actions' description instead of the title
            settingsItems << makeMenuItem(item.action);
        } else {
            MenuItem* menuItem = makeMenuItem(item.action);

            if (item.action == PLAY_ACTION_CODE) {
                menuItem->setAction(playAction());
            }

            result << menuItem;
        }
    }

    MenuItem* settingsItem = makeMenu(TranslatableString("action", "Playback settings"), settingsItems);

    UiAction action = settingsItem->action();
    action.iconCode = IconCode::Code::SETTINGS_COG;
    settingsItem->setAction(action);

    result << settingsItem;

    setItems(result);
}

void PlaybackToolBarModel::onActionsStateChanges(const ActionCodeList& codes)
{
    AbstractMenuModel::onActionsStateChanges(codes);

    if (isPlayAllowed() && containsAction(codes, PLAY_ACTION_CODE)) {
        MenuItem& item = findItem(PLAY_ACTION_CODE);
        item.setAction(playAction());
    }
}

bool PlaybackToolBarModel::isAdditionalAction(const ActionCode& actionCode) const
{
    return PlaybackUiActions::loopBoundaryActions().contains(actionCode);
}

bool PlaybackToolBarModel::isPlayAllowed() const
{
    return playbackController()->isPlayAllowed();
}

bool PlaybackToolBarModel::isToolbarFloating() const
{
    return m_isToolbarFloating;
}

void PlaybackToolBarModel::setIsToolbarFloating(bool floating)
{
    if (floating == m_isToolbarFloating) {
        return;
    }

    m_isToolbarFloating = floating;
    emit isToolbarFloatingChanged(floating);
}

QDateTime PlaybackToolBarModel::maxPlayTime() const
{
    return QDateTime(QDate::currentDate(), totalPlayTime());
}

QDateTime PlaybackToolBarModel::playTime() const
{
    return QDateTime(QDate::currentDate(), m_playTime);
}

void PlaybackToolBarModel::setPlayTime(const QDateTime& time)
{
    QTime newTime = time.time();
    if (m_playTime == newTime || time > maxPlayTime()) {
        return;
    }

    doSetPlayTime(newTime);

    msecs_t msec = timeToMilliseconds(newTime);
    rewind(msec);
}

qreal PlaybackToolBarModel::playPosition() const
{
    QTime totalTime = totalPlayTime();
    msecs_t totalMsecs = timeToMilliseconds(totalTime);

    if (totalMsecs == 0) {
        return 0;
    }

    msecs_t msecsDifference = totalMsecs - m_playTime.msecsTo(totalTime);
    qreal position = static_cast<qreal>(msecsDifference) / static_cast<qreal>(totalMsecs);

    return position;
}

void PlaybackToolBarModel::setPlayPosition(qreal position)
{
    msecs_t totalPlayTimeMsecs = timeToMilliseconds(totalPlayTime());
    msecs_t playPositionMsecs = totalPlayTimeMsecs * position;

    QTime time = timeFromMilliseconds(playPositionMsecs);
    setPlayTime(QDateTime(QDate::currentDate(), time));
}

QTime PlaybackToolBarModel::totalPlayTime() const
{
    return playbackController()->totalPlayTime();
}

MeasureBeat PlaybackToolBarModel::measureBeat() const
{
    return playbackController()->currentBeat();
}

UiAction PlaybackToolBarModel::playAction() const
{
    UiAction action = uiActionsRegister()->action(PLAY_ACTION_CODE);

    bool isPlaying = playbackController()->isPlaying();
    action.iconCode =  isPlaying ? IconCode::Code::PAUSE : IconCode::Code::PLAY;

    return action;
}

void PlaybackToolBarModel::updatePlayPosition(secs_t secs)
{
    QTime playTime = timeFromSeconds(secs);

    if (m_playTime == playTime) {
        return;
    }

    doSetPlayTime(playTime);
}

void PlaybackToolBarModel::doSetPlayTime(const QTime& time)
{
    m_playTime = time;
    emit playPositionChanged();
}

void PlaybackToolBarModel::rewind(msecs_t milliseconds)
{
    dispatch("rewind", ActionData::make_arg1<msecs_t>(milliseconds));
}

void PlaybackToolBarModel::rewindToBeat(const MeasureBeat& beat)
{
    msecs_t msec = playbackController()->beatToMilliseconds(beat.measureIndex, beat.beatIndex);
    rewind(msec);
}

int PlaybackToolBarModel::measureNumber() const
{
    return measureBeat().measureIndex + 1;
}

void PlaybackToolBarModel::setMeasureNumber(int measureNumber)
{
    int measureIndex = measureNumber - 1;
    MeasureBeat measureBeat = this->measureBeat();

    if (measureIndex == measureBeat.measureIndex) {
        return;
    }

    measureBeat.measureIndex = measureIndex;
    rewindToBeat(measureBeat);
}

int PlaybackToolBarModel::maxMeasureNumber() const
{
    return measureBeat().maxMeasureIndex + 1;
}

int PlaybackToolBarModel::beatNumber() const
{
    return measureBeat().beatIndex + 1;
}

void PlaybackToolBarModel::setBeatNumber(int beatNumber)
{
    int beatIndex = beatNumber - 1;
    MeasureBeat measureBeat = this->measureBeat();

    if (beatIndex == measureBeat.beatIndex) {
        return;
    }

    measureBeat.beatIndex = beatIndex;
    rewindToBeat(measureBeat);
}

void PlaybackToolBarModel::setTempoMultiplier(qreal multiplier)
{
    if (multiplier == tempoMultiplier()) {
        return;
    }

    playbackController()->setTempoMultiplier(multiplier);
    emit tempoChanged();
}

int PlaybackToolBarModel::maxBeatNumber() const
{
    return measureBeat().maxBeatIndex + 1;
}

QVariant PlaybackToolBarModel::tempo() const
{
    Tempo tempo = playbackController()->currentTempo();
    MusicalSymbolCodes::Code noteIcon = tempoDurationToNoteIcon(tempo.duration);

    QVariantMap obj;
    obj["noteSymbol"] = musicalSymbolToString(noteIcon, tempo.withDot);
    obj["value"] = tempo.valueBpm;

    return obj;
}

qreal PlaybackToolBarModel::tempoMultiplier() const
{
    return playbackController()->tempoMultiplier();
}
