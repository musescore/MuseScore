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

PlaybackToolBarModel::PlaybackToolBarModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

void PlaybackToolBarModel::load()
{
    AbstractMenuModel::load();
    updateActions();

    connect(this, &PlaybackToolBarModel::isToolbarFloatingChanged, this, &PlaybackToolBarModel::updateActions);

    setupConnections();
}

void PlaybackToolBarModel::setupConnections()
{
    playbackController()->isPlayAllowedChanged().onNotify(this, [this]() {
        emit isPlayAllowedChanged();
    });

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        onActionsStateChanges({ PLAY_ACTION_CODE });
    });

    playbackController()->currentPlaybackPositionChanged().onReceive(this, [this](secs_t secs, midi::tick_t) {
        updatePlayPosition(secs);
    });

    playbackController()->totalPlayTimeChanged().onNotify(this, [this]() {
        emit maxPlayTimeChanged();
        secs_t pos = globalContext()->playbackState()->playbackPosition();
        updatePlayPosition(pos);
    });

    playbackController()->currentTempoChanged().onNotify(this, [this]() {
        emit tempoChanged();
    });
}

void PlaybackToolBarModel::updateActions()
{
    MenuItemList result;
    MenuItemList settingsItems;

    for (const UiAction& action : PlaybackUiActions::midiInputActions()) {
        settingsItems << makeMenuItem(action.code);
    }

    settingsItems << makeInputPitchMenu();
    settingsItems << makeSeparator();

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

MenuItem* PlaybackToolBarModel::makeInputPitchMenu()
{
    MenuItemList items;
    items.reserve(PlaybackUiActions::midiInputPitchActions().size());

    for (const UiAction& action : PlaybackUiActions::midiInputPitchActions()) {
        items << makeMenuItem(action.code);
    }

    MenuItem* menu = makeMenu(muse::TranslatableString("notation", "MIDI input pitch"), items);
    UiAction action = menu->action();
    action.iconCode = IconCode::Code::MUSIC_NOTES;
    menu->setAction(action);

    return menu;
}

void PlaybackToolBarModel::onActionsStateChanges(const ActionCodeList& codes)
{
    AbstractMenuModel::onActionsStateChanges(codes);

    if (isPlayAllowed() && containsAction(codes, PLAY_ACTION_CODE)) {
        MenuItem& item = findItem(PLAY_ACTION_CODE);
        item.setAction(playAction());
    }
}

bool PlaybackToolBarModel::isAdditionalAction(const ActionCode& code) const
{
    return muse::contains_if(PlaybackUiActions::loopBoundaryActions(), [code](const UiAction& a) {
        return a.code == code;
    });
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

QTime PlaybackToolBarModel::maxPlayTime() const
{
    return timeFromSeconds(totalPlayTime());
}

QTime PlaybackToolBarModel::playTime() const
{
    return timeFromSeconds(m_playbackPositionSecs);
}

void PlaybackToolBarModel::setPlayTime(const QTime& time)
{
    rewind(timeToSeconds(time));
}

qreal PlaybackToolBarModel::playPosition() const
{
    const secs_t totalSecs = totalPlayTime();
    if (totalSecs.is_zero()) {
        return 0.0;
    }

    const qreal position = static_cast<qreal>(m_playbackPositionSecs) / static_cast<qreal>(totalSecs);
    return position;
}

void PlaybackToolBarModel::setPlayPosition(qreal position)
{
    rewind(totalPlayTime() * position);
}

secs_t PlaybackToolBarModel::totalPlayTime() const
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
    if (m_playbackPositionSecs == secs) {
        return;
    }

    m_playbackPositionSecs = secs;
    emit playPositionChanged();
}

void PlaybackToolBarModel::rewind(secs_t secs)
{
    if (m_playbackPositionSecs == secs || secs > totalPlayTime()) {
        return;
    }

    dispatch("rewind", ActionData::make_arg1<secs_t>(secs));
}

void PlaybackToolBarModel::rewindToBeat(const MeasureBeat& beat)
{
    secs_t secs = playbackController()->beatToSecs(beat.measureIndex, static_cast<int>(beat.beat));
    rewind(secs);
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
    return static_cast<int>(measureBeat().beat) + 1;
}

void PlaybackToolBarModel::setBeatNumber(int beatNumber)
{
    int beatIndex = beatNumber - 1;
    MeasureBeat measureBeat = this->measureBeat();

    if (beatIndex == static_cast<int>(measureBeat.beat)) {
        return;
    }

    measureBeat.beat = beatIndex;
    rewindToBeat(measureBeat);
}

void PlaybackToolBarModel::setTempoMultiplier(qreal multiplier)
{
    if (muse::RealIsEqual(multiplier, tempoMultiplier())) {
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
    static const std::unordered_map<DurationType, MusicalSymbolCodes::Code> DURATION_TO_ICON {
        { DurationType::V_WHOLE, MusicalSymbolCodes::Code::SEMIBREVE },
        { DurationType::V_HALF, MusicalSymbolCodes::Code::MINIM },
        { DurationType::V_QUARTER, MusicalSymbolCodes::Code::CROTCHET },
        { DurationType::V_EIGHTH, MusicalSymbolCodes::Code::QUAVER },
        { DurationType::V_16TH, MusicalSymbolCodes::Code::SEMIQUAVER },
    };

    const Tempo& tempo = playbackController()->currentTempo();
    const MusicalSymbolCodes::Code noteIcon = muse::value(DURATION_TO_ICON, tempo.duration,
                                                          MusicalSymbolCodes::Code::CROTCHET);

    QVariantMap obj;
    obj["noteSymbol"] = musicalSymbolToString(noteIcon, tempo.withDot);
    obj["value"] = tempo.valueBpm;

    return obj;
}

qreal PlaybackToolBarModel::tempoMultiplier() const
{
    return playbackController()->tempoMultiplier();
}
