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
using namespace mu::notation;

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
        emit maxPlayTimeChanged();
        updatePlayTime();
        emit isPlayAllowedChanged();
    });

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        onActionsStateChanges({ PLAY_ACTION_CODE });
    });

    playbackController()->playbackPositionChanged().onNotify(this, [this]() {
        updatePlayTime();
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

    MenuItem settingsItem = makeMenu(qtrc("action", "Playback settings"), settingsItems);
    settingsItem.iconCode = IconCode::Code::SETTINGS_COG;
    result << settingsItem;

    setItems(result);
}

void PlaybackToolBarModel::onActionsStateChanges(const actions::ActionCodeList& codes)
{
    AbstractMenuModel::onActionsStateChanges(codes);

    if (isPlayAllowed() && containsAction(codes, PLAY_ACTION_CODE)) {
        bool isPlaying = playbackController()->isPlaying();
        findItem(PLAY_ACTION_CODE).iconCode = isPlaying ? IconCode::Code::PAUSE : IconCode::Code::PLAY;
    }

    emit dataChanged(index(0), index(rowCount() - 1));
}

bool PlaybackToolBarModel::isAdditionalAction(const actions::ActionCode& actionCode) const
{
    return PlaybackUiActions::loopBoundaryActions().contains(actionCode);
}

MenuItem PlaybackToolBarModel::makeActionWithDescriptionAsTitle(const actions::ActionCode& actionCode) const
{
    MenuItem item = makeMenuItem(actionCode);
    item.title = item.description;
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

    uint64_t msec = timeToMilliseconds(newTime);
    rewind(msec);
}

qreal PlaybackToolBarModel::playPosition() const
{
    qreal allMsecs = totalPlayTimeMilliseconds();
    qreal msecsDifference = allMsecs - m_playTime.msecsTo(totalPlayTime());

    qreal position = msecsDifference / allMsecs;
    return position;
}

void PlaybackToolBarModel::setPlayPosition(qreal position)
{
    uint64_t allMsecs = totalPlayTimeMilliseconds();
    uint64_t playPositionMsecs = allMsecs * position;

    QTime time = timeFromMilliseconds(playPositionMsecs);
    setPlayTime(QDateTime(QDate::currentDate(), time));
}

QTime PlaybackToolBarModel::totalPlayTime() const
{
    return playbackController()->totalPlayTime();
}

uint64_t PlaybackToolBarModel::totalPlayTimeMilliseconds() const
{
    return timeToMilliseconds(totalPlayTime());
}

MeasureBeat PlaybackToolBarModel::measureBeat() const
{
    return playbackController()->currentBeat();
}

void PlaybackToolBarModel::updatePlayTime()
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
    emit playTimeChanged();
}

void PlaybackToolBarModel::rewind(uint64_t milliseconds)
{
    dispatch("rewind", ActionData::make_arg1<uint64_t>(milliseconds));
}

void PlaybackToolBarModel::rewindToBeat(const MeasureBeat& beat)
{
    uint64_t msec = playbackController()->beatToMilliseconds(beat.measureIndex, beat.beatIndex);
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
