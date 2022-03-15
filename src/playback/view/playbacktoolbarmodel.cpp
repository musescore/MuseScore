/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "log.h"
#include "translation.h"

#include "ui/view/musicalsymbolcodes.h"
#include "playback/playbacktypes.h"
#include "playback/internal/playbackuiactions.h"

using namespace mu::playback;
using namespace mu::actions;
using namespace mu::ui;
using namespace mu::uicomponents;
using namespace mu::notation;
using namespace mu::audio;

static const ActionCode PLAY_ACTION_CODE("play");
static constexpr bool FORCE_END_EDIT_ELEMENT = true;

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

    playbackController()->playbackPositionChanged().onNotify(this, [this]() {
        updatePlayPosition();
    });

    playbackController()->totalPlayTimeChanged().onNotify(this, [this]() {
        emit maxPlayTimeChanged();
        updatePlayPosition();
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
        settingsItems << makeActionWithDescriptionAsTitle(action.code);
    }

    if (!m_isToolbarFloating) {
        settingsItems << makeSeparator();
    }

    //! NOTE At the moment no customization ability
    ToolConfig config = PlaybackUiActions::defaultPlaybackToolConfig();
    for (const ToolConfig::Item& item : config.items) {
        if (isAdditionalAction(item.action) && !m_isToolbarFloating) {
            //! NOTE In this case, we want to see the actions' description instead of the title
            settingsItems << makeActionWithDescriptionAsTitle(item.action);
        } else {
            result << makeMenuItem(item.action);
        }
    }

    MenuItem* settingsItem = makeMenu(qtrc("action", "Playback settings"), settingsItems);

    UiAction action = settingsItem->action();
    action.iconCode = IconCode::Code::SETTINGS_COG;
    settingsItem->setAction(action);

    result << settingsItem;

    setItems(result);

    MenuItem& playItem = findItem(PLAY_ACTION_CODE);
    playItem.setArgs(ActionData::make_arg1<bool>(FORCE_END_EDIT_ELEMENT));
}

void PlaybackToolBarModel::onActionsStateChanges(const actions::ActionCodeList& codes)
{
    AbstractMenuModel::onActionsStateChanges(codes);

    if (isPlayAllowed() && containsAction(codes, PLAY_ACTION_CODE)) {
        bool isPlaying = playbackController()->isPlaying();

        MenuItem& item = findItem(PLAY_ACTION_CODE);
        UiAction action = item.action();
        action.iconCode = isPlaying ? IconCode::Code::PAUSE : IconCode::Code::PLAY;
        item.setAction(action);
    }
}

bool PlaybackToolBarModel::isAdditionalAction(const actions::ActionCode& actionCode) const
{
    return PlaybackUiActions::loopBoundaryActions().contains(actionCode);
}

MenuItem* PlaybackToolBarModel::makeActionWithDescriptionAsTitle(const actions::ActionCode& actionCode)
{
    MenuItem* item = makeMenuItem(actionCode);

    UiAction action = item->action();
    action.title = item->action().description;
    item->setAction(action);

    return item;
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
    msecs_t totalMsecs = totalPlayTimeMilliseconds();
    if (totalMsecs == 0) {
        return 0;
    }

    msecs_t msecsDifference = totalMsecs - m_playTime.msecsTo(totalPlayTime());
    qreal position = msecsDifference / totalMsecs;

    return position;
}

void PlaybackToolBarModel::setPlayPosition(qreal position)
{
    msecs_t allMsecs = totalPlayTimeMilliseconds();
    msecs_t playPositionMsecs = allMsecs * position;

    QTime time = timeFromMilliseconds(playPositionMsecs);
    setPlayTime(QDateTime(QDate::currentDate(), time));
}

QTime PlaybackToolBarModel::totalPlayTime() const
{
    return playbackController()->totalPlayTime();
}

msecs_t PlaybackToolBarModel::totalPlayTimeMilliseconds() const
{
    return timeToMilliseconds(totalPlayTime());
}

MeasureBeat PlaybackToolBarModel::measureBeat() const
{
    return playbackController()->currentBeat();
}

void PlaybackToolBarModel::updatePlayPosition()
{
    float seconds = playbackController()->playbackPositionInSeconds();
    QTime playTime = timeFromSeconds(seconds);

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
