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

#ifndef MUSESCORE_CONFIG_H
#define MUSESCORE_CONFIG_H

/* #undef FOR_WINSTORE */

#define MSCORE_UNSTABLE

/* #undef HAS_MIDI */
#define SCRIPT_INTERFACE
/* #undef HAS_AUDIOFILE */

#define CRASH_REPORT_URL       ""
#define MUSESCORE_NAME_VERSION "MuseScore 4 (4.0.0 unstable)"
#define MUSESCORE_REVISION     ""
#define INSTALL_NAME           "Contents/Resources/"
#define INSTPREFIX             "/Users/fer200/Desktop/MuseScore/MuseScore/my_install_dir"
#define VERSION                "4.0.0"
#define VERSION_LABEL          "Development"
#define BUILD_NUMBER           ""
#define SPARKLE_APPCAST_URL    ""

#define YOUTUBE_API_KEY ""

#define LOGGER_DEBUGLEVEL_ENABLED

#define BUILD_SHORTCUTS_MODULE
#define BUILD_SYSTEM_MODULE
#define BUILD_NETWORK_MODULE
#define BUILD_AUDIO_MODULE
#define BUILD_LEARN_MODULE
#define BUILD_WORKSPACE_MODULE
#define BUILD_CLOUD_MODULE
#define BUILD_EXTENSIONS_MODULE
#define BUILD_LANGUAGES_MODULE
#define BUILD_PLUGINS_MODULE
#define BUILD_PLAYBACK_MODULE
#define BUILD_PALETTE_MODULE
#define BUILD_INSTRUMENTSSCENE_MODULE
#define BUILD_INSPECTOR_MODULE
#define BUILD_AUTOBOT_MODULE
#define BUILD_MULTIINSTANCES_MODULE
/* #undef BUILD_VST */
#define BUILD_DIAGNOSTICS

/* #undef ENGRAVING_PAINT_DEBUGGER_ENABLED */
#define ENGRAVING_COMPAT_WRITESTYLE_302
#define ENGRAVING_COMPAT_WRITEEXCERPTS_302

/* #undef UI_DISABLE_MODALITY */

/* #undef ACCESSIBILITY_LOGGING_ENABLED */

/* #undef SPARKLE_ENABLED */
/* #undef OPENGL */
#define SOUNDFONT3
/* #undef WIN_PORTABLE */

/* #undef QML_LOAD_FROM_SOURCE */
/* #undef TRACE_DRAW_OBJ_ENABLED */

/* #undef Q_WS_UIKIT */

#define APP_UPDATABLE

#define USE_BSP     true

// does not work on windows/mac:
//#define USE_GLYPHS  true

#endif /* MUSESCORE_CONFIG_H */
