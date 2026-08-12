/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) MuseScore Limited and others
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

 #include "rcommand/commandtypes.h"

namespace mu::appshell {
inline static const muse::rcommand::Command APP_QUIT_COMMAND("command://app/quit");
inline static const muse::rcommand::Command APP_RESTART_COMMAND("command://app/restart");
inline static const muse::rcommand::Command APP_FULLSCREEN_COMMAND("command://app/fullscreen");
inline static const muse::rcommand::Command APP_ABOUT_MUSESCORE_COMMAND("command://app/about-musescore");
inline static const muse::rcommand::Command APP_ABOUT_QT_COMMAND("command://app/about-qt");
inline static const muse::rcommand::Command APP_ABOUT_MUSICXML_COMMAND("command://app/about-musicxml");
inline static const muse::rcommand::Command APP_ONLINE_HANDBOOK_COMMAND("command://app/online-handbook");
inline static const muse::rcommand::Command APP_ASK_HELP_COMMAND("command://app/ask-help");
inline static const muse::rcommand::Command APP_ACCESSIBILITY_STATEMENT_COMMAND("command://app/accessibility-statement");
inline static const muse::rcommand::Command APP_PREFERENCES_COMMAND("command://app/preferences");
inline static const muse::rcommand::Command APP_REVERT_TO_FACTORY_COMMAND("command://app/revert-to-factory");
inline static const muse::rcommand::Command APP_EXTENSIONS_COMMAND("command://app/extensions");

// docks commands
inline static const muse::rcommand::Command DOCK_TOGGLE_PLAYBACK_COMMAND("command://app/dock/toggle-playback");
inline static const muse::rcommand::Command DOCK_TOGGLE_NOTEINPUT_COMMAND("command://app/dock/toggle-noteinput");
inline static const muse::rcommand::Command DOCK_TOGGLE_PALETTES_COMMAND("command://app/dock/toggle-palettes");
inline static const muse::rcommand::Command DOCK_TOGGLE_INSTRUMENTS_COMMAND("command://app/dock/toggle-instruments");
inline static const muse::rcommand::Command DOCK_TOGGLE_PROPERTIES_COMMAND("command://app/dock/toggle-properties");
inline static const muse::rcommand::Command DOCK_TOGGLE_SELECTION_FILTER_COMMAND("command://app/dock/toggle-selection-filter");
inline static const muse::rcommand::Command DOCK_TOGGLE_UNDO_HISTORY_COMMAND("command://app/dock/toggle-undo-history");
inline static const muse::rcommand::Command DOCK_TOGGLE_NAVIGATOR_COMMAND("command://app/dock/toggle-navigator");
inline static const muse::rcommand::Command DOCK_TOGGLE_BRAILLE_COMMAND("command://app/dock/toggle-braille");
inline static const muse::rcommand::Command DOCK_TOGGLE_TIMELINE_COMMAND("command://app/dock/toggle-timeline");
inline static const muse::rcommand::Command DOCK_TOGGLE_MIXER_COMMAND("command://app/dock/toggle-mixer");
inline static const muse::rcommand::Command DOCK_TOGGLE_PIANO_KEYBOARD_COMMAND("command://app/dock/toggle-piano-keyboard");
inline static const muse::rcommand::Command DOCK_TOGGLE_PERCUSSION_COMMAND("command://app/dock/toggle-percussion");
inline static const muse::rcommand::Command DOCK_TOGGLE_STATUSBAR_COMMAND("command://app/dock/toggle-statusbar");
}
