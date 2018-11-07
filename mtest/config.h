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

#define USE_ALSA
#define USE_JACK
/* #undef USE_PORTAUDIO */
/* #undef USE_PORTMIDI */

#define MSCORE_UNSTABLE

#define HAS_MIDI
#define STATIC_SCRIPT_BINDINGS
#define BUILD_SCRIPTGEN
#define HAS_AUDIOFILE
/* #undef USE_SSE */

#define INSTALL_NAME      "mscore-2.0/"
#define INSTPREFIX        "/usr/local"
#define VERSION           "2.0.0"

#define AEOLUS
/* #undef OMR */
/* #undef OCR */
#define OSC
/* #undef OPENGL */
#define SOUNDFONT3

/* #undef Q_OS_UIKIT */

#define USE_BSP         true
#define SCRIPT_INTERFACE true


