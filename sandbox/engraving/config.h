//=============================================================================
//  MusE
//  Linux Music Editor
//
//  Copyright (C) 2002-2010 by Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

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
#define INSTALL_NAME           "mscore-4.0/"
#define INSTPREFIX             "/home/igor/Dev/MuseScore/build.debug/install"
#define VERSION                "4.0.0"
#define VERSION_LABEL          "Development"
#define BUILD_NUMBER           ""
#define SPARKLE_APPCAST_URL    ""

#define BUILD_TELEMETRY_MODULE
#define TELEMETRY_TRACK_ID ""

#define BUILD_SHORTCUTS_MODULE
#define BUILD_SYSTEM_MODULE
#define BUILD_NETWORK_MODULE
#define BUILD_AUDIO_MODULE
#define BUILD_USERSCORES_MODULE
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
#define BUILD_VST

/* #undef ENGRAVING_BUILD_ACCESSIBLE_TREE */

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
