/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <vector>

#include "global/containers.h"
#include "types/translatablestring.h"

#include "ui/view/musicalsymbolcodes.h"
#include "playback/playbacktypes.h"
#include "playback/playbackcommands.h"

#include "ui/toolconfig.h"

using namespace mu::playback;
using namespace mu::engraving;
using namespace muse;
using namespace muse::actions;
using namespace muse::ui;
using namespace muse::uicomponents;
using namespace muse::audio;

static const QString PLAY_TOGGLE_ID("play-toggle");

static const ToolConfig& defaultPlaybackToolConfig()
{
    static ToolConfig config;
    if (!config.isValid()) {
        config.items = {
            { REWIND_COMMAND, true },
            { "play-toggle", true }, // virtual code
            { LOOP_TOGGLE_COMMAND, true },
            { LOOP_IN_COMMAND, true },
            { LOOP_OUT_COMMAND, true },
            { METRONOME_TOGGLE_COMMAND, true },
        };
    }
    return config;
}

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
    playbackController()->isPlayAllowedChanged().onReceive(this, [this](bool) {
        emit isPlayAllowedChanged();
    });

    globalContext()->playbackState()->playbackStatusChanged().onReceive(this, [this](audio::PlaybackStatus) {
        updatePlayItem();
    });

    globalContext()->playbackState()->playbackPositionChanged().onReceive(this, [this](secs_t secs) {
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
    MenuItemList settingsItems = {
        makeMenuItem(MIDI_TOGGLE_COMMAND),
        makeInputPitchMenu(),
        makeSeparator(),
        makeMenuItem(REPEATS_TOGGLE_COMMAND),
        makeMenuItem(CHORDSYMBOLS_TOGGLE_COMMAND),
        makeMenuItem(HEAR_PLAYBACK_WHEN_EDITING_TOGGLE_COMMAND),
        makeMenuItem(PAN_TOGGLE_COMMAND),
        makeMenuItem(COUNTIN_TOGGLE_COMMAND),
    };

    if (!m_isToolbarFloating) {
        settingsItems << makeSeparator();
    }

    //! NOTE At the moment no customization ability
    ToolConfig config = defaultPlaybackToolConfig();
    for (const ToolConfig::Item& item : config.items) {
        if (item.intent == PLAY_TOGGLE_ID) {
            result << makePlayItem();
        } else {
            const rcommand::Command command(item.intent);
            if (isAdditionalCommand(command) && !m_isToolbarFloating) {
                //! NOTE In this case, we want to see the actions' description instead of the title
                settingsItems << makeMenuItem(command);
            } else {
                result << makeMenuItem(command);
            }
        }
    }

    MenuItem* settingsItem = makeMenu(TranslatableString("action", "Playback settings"), settingsItems);
    settingsItem->setIcon(ui::IconCode::Code::SETTINGS_COG);
    result << settingsItem;

    setItems(result);
}

MenuItem* PlaybackToolBarModel::makeInputPitchMenu()
{
    MenuItemList items = {
        makeMenuItem(MIDI_INPUT_WRITTEN_PITCH_COMMAND),
        makeMenuItem(MIDI_INPUT_SOUNDING_PITCH_COMMAND),
    };

    MenuItem* menu = makeMenu(muse::TranslatableString("playback", "MIDI input pitch"), items);
    menu->setIcon(ui::IconCode::Code::MUSIC_NOTES);
    return menu;
}

bool PlaybackToolBarModel::isAdditionalCommand(const muse::rcommand::Command& command) const
{
    static const std::vector<muse::rcommand::Command> additionalCommands = {
        LOOP_IN_COMMAND,
        LOOP_OUT_COMMAND,
    };

    return muse::contains(additionalCommands, command);
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

muse::uicomponents::MenuItem* PlaybackToolBarModel::makePlayItem()
{
    rcommand::Command command = globalContext()->playbackState()->isPlaying() ? PAUSE_COMMAND : PLAY_COMMAND;
    MenuItem* item = makeMenuItem(command);
    item->setId(PLAY_TOGGLE_ID);
    return item;
}

void PlaybackToolBarModel::updatePlayItem()
{
    MenuItem& item = findItem(PLAY_TOGGLE_ID);
    if (item.isValid()) {
        rcommand::Command command = globalContext()->playbackState()->isPlaying() ? PAUSE_COMMAND : PLAY_COMMAND;
        item.setCommandInfo(commandsRegister()->commandInfo(command));
        item.setId(PLAY_TOGGLE_ID);
    }
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

    dispatch(rcommand::make_query(REWIND_COMMAND, { { "position", Val(secs) } }));
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

    const notation::Tempo& tempo = playbackController()->currentTempo();
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
