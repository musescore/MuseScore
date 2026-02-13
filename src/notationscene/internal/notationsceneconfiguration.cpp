/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "notationsceneconfiguration.h"

#include "settings.h"

using namespace mu::notation;
using namespace muse;
using namespace muse::async;

static const std::string module_name("notation");

static const Settings::Key PLAYBACK_SMOOTH_PANNING(module_name, "application/playback/smoothPan");

static const Settings::Key IS_LIMIT_CANVAS_SCROLL_AREA_KEY(module_name, "ui/canvas/scroll/limitScrollArea");

static const Settings::Key PIANO_KEYBOARD_NUMBER_OF_KEYS(module_name,  "pianoKeyboard/numberOfKeys");

static const Settings::Key USE_NEW_PERCUSSION_PANEL_KEY(module_name,  "ui/useNewPercussionPanel");
static const Settings::Key PERCUSSION_PANEL_USE_NOTATION_PREVIEW_KEY(module_name,  "ui/percussionPanelUseNotationPreview");
static const Settings::Key PERCUSSION_PANEL_AUTO_SHOW_MODE_KEY(module_name,  "ui/percussionPanelAutoShowMode");
static const Settings::Key AUTO_CLOSE_PERCUSSION_PANEL_KEY(module_name, "ui/autoClosePercussionPanel");
static const Settings::Key SHOW_PERCUSSION_PANEL_SWAP_DIALOG(module_name,  "ui/showPercussionPanelPadSwapDialog");
static const Settings::Key PERCUSSION_PANEL_MOVE_MIDI_NOTES_AND_SHORTCUTS(module_name,  "ui/percussionPanelMoveMidiNotesAndShortcuts");

NotationSceneConfiguration::NotationSceneConfiguration(const muse::modularity::ContextPtr& ctx)
    : Contextable(ctx)
{
}

void NotationSceneConfiguration::init()
{
    settings()->setDefaultValue(PLAYBACK_SMOOTH_PANNING, Val(false));
    settings()->setDescription(PLAYBACK_SMOOTH_PANNING, muse::trc("notation", "Smooth panning"));
    settings()->setCanBeManuallyEdited(PLAYBACK_SMOOTH_PANNING, true);
    settings()->valueChanged(PLAYBACK_SMOOTH_PANNING).onReceive(this, [this](const Val&) {
        m_isSmoothPanningChanged.notify();
    });

    settings()->setDefaultValue(IS_LIMIT_CANVAS_SCROLL_AREA_KEY, Val(false));
    settings()->valueChanged(IS_LIMIT_CANVAS_SCROLL_AREA_KEY).onReceive(this, [this](const Val&) {
        m_isLimitCanvasScrollAreaChanged.notify();
    });

    settings()->setDefaultValue(PIANO_KEYBOARD_NUMBER_OF_KEYS, Val(88));
    m_pianoKeyboardNumberOfKeys.val = settings()->value(PIANO_KEYBOARD_NUMBER_OF_KEYS).toInt();
    settings()->valueChanged(PIANO_KEYBOARD_NUMBER_OF_KEYS).onReceive(this, [this](const Val& val) {
        m_pianoKeyboardNumberOfKeys.set(val.toInt());
    });

    settings()->setDefaultValue(USE_NEW_PERCUSSION_PANEL_KEY, Val(true));
    settings()->valueChanged(USE_NEW_PERCUSSION_PANEL_KEY).onReceive(this, [this](const Val&) {
        m_useNewPercussionPanelChanged.notify();
    });

    settings()->setDefaultValue(PERCUSSION_PANEL_USE_NOTATION_PREVIEW_KEY, Val(false));
    settings()->valueChanged(PERCUSSION_PANEL_USE_NOTATION_PREVIEW_KEY).onReceive(this, [this](const Val&) {
        m_percussionPanelUseNotationPreviewChanged.notify();
    });

    settings()->setDefaultValue(PERCUSSION_PANEL_AUTO_SHOW_MODE_KEY, Val(PercussionPanelAutoShowMode::UNPITCHED_STAFF));
    settings()->valueChanged(PERCUSSION_PANEL_AUTO_SHOW_MODE_KEY).onReceive(this, [this](const Val&) {
        m_percussionPanelAutoShowModeChanged.notify();
    });

    settings()->setDefaultValue(AUTO_CLOSE_PERCUSSION_PANEL_KEY, Val(true));
    settings()->valueChanged(AUTO_CLOSE_PERCUSSION_PANEL_KEY).onReceive(this, [this](const Val&) {
        m_autoClosePercussionPanelChanged.notify();
    });

    settings()->setDefaultValue(SHOW_PERCUSSION_PANEL_SWAP_DIALOG, Val(true));
    settings()->valueChanged(SHOW_PERCUSSION_PANEL_SWAP_DIALOG).onReceive(this, [this](const Val&) {
        m_showPercussionPanelPadSwapDialogChanged.notify();
    });

    settings()->setDefaultValue(PERCUSSION_PANEL_MOVE_MIDI_NOTES_AND_SHORTCUTS, Val(true));
    settings()->valueChanged(PERCUSSION_PANEL_MOVE_MIDI_NOTES_AND_SHORTCUTS).onReceive(this, [this](const Val&) {
        m_percussionPanelMoveMidiNotesAndShortcutsChanged.notify();
    });

    context()->currentProjectChanged().onNotify(this, [this]() {
        resetStyleDialogPageIndices();
    });
}

bool NotationSceneConfiguration::isSmoothPanning() const
{
    return settings()->value(PLAYBACK_SMOOTH_PANNING).toBool();
}

void NotationSceneConfiguration::setIsSmoothPanning(bool value)
{
    settings()->setSharedValue(PLAYBACK_SMOOTH_PANNING, Val(value));
}

Notification NotationSceneConfiguration::isSmoothPanningChanged() const
{
    return m_isSmoothPanningChanged;
}

bool NotationSceneConfiguration::isLimitCanvasScrollArea() const
{
    return settings()->value(IS_LIMIT_CANVAS_SCROLL_AREA_KEY).toBool();
}

void NotationSceneConfiguration::setIsLimitCanvasScrollArea(bool limited)
{
    settings()->setSharedValue(IS_LIMIT_CANVAS_SCROLL_AREA_KEY, Val(limited));
}

Notification NotationSceneConfiguration::isLimitCanvasScrollAreaChanged() const
{
    return m_isLimitCanvasScrollAreaChanged;
}

ValCh<int> NotationSceneConfiguration::pianoKeyboardNumberOfKeys() const
{
    return m_pianoKeyboardNumberOfKeys;
}

void NotationSceneConfiguration::setPianoKeyboardNumberOfKeys(int number)
{
    settings()->setSharedValue(PIANO_KEYBOARD_NUMBER_OF_KEYS, Val(number));
}

bool NotationSceneConfiguration::useNewPercussionPanel() const
{
    return settings()->value(USE_NEW_PERCUSSION_PANEL_KEY).toBool();
}

void NotationSceneConfiguration::setUseNewPercussionPanel(bool use)
{
    settings()->setSharedValue(USE_NEW_PERCUSSION_PANEL_KEY, Val(use));
}

Notification NotationSceneConfiguration::useNewPercussionPanelChanged() const
{
    return m_useNewPercussionPanelChanged;
}

bool NotationSceneConfiguration::percussionPanelUseNotationPreview() const
{
    return settings()->value(PERCUSSION_PANEL_USE_NOTATION_PREVIEW_KEY).toBool();
}

void NotationSceneConfiguration::setPercussionPanelUseNotationPreview(bool use)
{
    settings()->setSharedValue(PERCUSSION_PANEL_USE_NOTATION_PREVIEW_KEY, Val(use));
}

Notification NotationSceneConfiguration::percussionPanelUseNotationPreviewChanged() const
{
    return m_percussionPanelUseNotationPreviewChanged;
}

PercussionPanelAutoShowMode NotationSceneConfiguration::percussionPanelAutoShowMode() const
{
    return settings()->value(PERCUSSION_PANEL_AUTO_SHOW_MODE_KEY).toEnum<PercussionPanelAutoShowMode>();
}

void NotationSceneConfiguration::setPercussionPanelAutoShowMode(PercussionPanelAutoShowMode autoShowMode)
{
    settings()->setSharedValue(PERCUSSION_PANEL_AUTO_SHOW_MODE_KEY, Val(autoShowMode));
}

Notification NotationSceneConfiguration::percussionPanelAutoShowModeChanged() const
{
    return m_percussionPanelAutoShowModeChanged;
}

bool NotationSceneConfiguration::autoClosePercussionPanel() const
{
    return settings()->value(AUTO_CLOSE_PERCUSSION_PANEL_KEY).toBool();
}

void NotationSceneConfiguration::setAutoClosePercussionPanel(bool autoClose)
{
    settings()->setSharedValue(AUTO_CLOSE_PERCUSSION_PANEL_KEY, Val(autoClose));
}

Notification NotationSceneConfiguration::autoClosePercussionPanelChanged() const
{
    return m_autoClosePercussionPanelChanged;
}

bool NotationSceneConfiguration::showPercussionPanelPadSwapDialog() const
{
    return settings()->value(SHOW_PERCUSSION_PANEL_SWAP_DIALOG).toBool();
}

void NotationSceneConfiguration::setShowPercussionPanelPadSwapDialog(bool show)
{
    settings()->setSharedValue(SHOW_PERCUSSION_PANEL_SWAP_DIALOG, Val(show));
}

Notification NotationSceneConfiguration::showPercussionPanelPadSwapDialogChanged() const
{
    return m_showPercussionPanelPadSwapDialogChanged;
}

bool NotationSceneConfiguration::percussionPanelMoveMidiNotesAndShortcuts() const
{
    return settings()->value(PERCUSSION_PANEL_MOVE_MIDI_NOTES_AND_SHORTCUTS).toBool();
}

void NotationSceneConfiguration::setPercussionPanelMoveMidiNotesAndShortcuts(bool move)
{
    settings()->setSharedValue(PERCUSSION_PANEL_MOVE_MIDI_NOTES_AND_SHORTCUTS, Val(move));
}

Notification NotationSceneConfiguration::percussionPanelMoveMidiNotesAndShortcutsChanged() const
{
    return m_percussionPanelMoveMidiNotesAndShortcutsChanged;
}

int NotationSceneConfiguration::styleDialogLastPageIndex() const
{
    return m_styleDialogLastPageIndex;
}

void NotationSceneConfiguration::setStyleDialogLastPageIndex(int value)
{
    m_styleDialogLastPageIndex = value;
}

int NotationSceneConfiguration::styleDialogLastSubPageIndex() const
{
    return m_styleDialogLastSubPageIndex;
}

void NotationSceneConfiguration::setStyleDialogLastSubPageIndex(int value)
{
    m_styleDialogLastSubPageIndex = value;
}

void NotationSceneConfiguration::resetStyleDialogPageIndices()
{
    setStyleDialogLastPageIndex(0);
    setStyleDialogLastSubPageIndex(0);
}
