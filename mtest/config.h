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

#define MSCORE_UNSTABLE

#define HAS_MIDI
#define STATIC_SCRIPT_BINDINGS
#define BUILD_SCRIPTGEN
#define HAS_AUDIOFILE

#define INSTALL_NAME      "mscore-2.0/"
#define INSTPREFIX        "/usr/local"
#define VERSION           "2.0.0"

/* #undef OPENGL */
#define SOUNDFONT3

/* #undef Q_OS_UIKIT */

#define USE_BSP         true
#define SCRIPT_INTERFACE true

#endif /* MUSESCORE_CONFIG_H */
