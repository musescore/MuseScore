//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: preferences.h 5660 2012-05-22 14:17:39Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

#include "globals.h"
#include "shortcut.h"
#include "mscore/importmidi_operations.h"

namespace Ms {

enum SessionStart {
      EMPTY_SESSION, LAST_SESSION, NEW_SESSION, SCORE_SESSION
      };

// midi remote control values:
enum {
      RMIDI_REWIND,
      RMIDI_TOGGLE_PLAY,
      RMIDI_PLAY,
      RMIDI_STOP,
      RMIDI_NOTE1,
      RMIDI_NOTE2,
      RMIDI_NOTE4,
      RMIDI_NOTE8,
      RMIDI_NOTE16,
      RMIDI_NOTE32,
      RMIDI_NOTE64,
      RMIDI_REST,
      RMIDI_DOT,
      RMIDI_DOTDOT,
      RMIDI_TIE,
      RMIDI_NOTE_EDIT_MODE,
      MIDI_REMOTES
      };

enum MuseScoreStyleType {
      STYLE_DARK,
      STYLE_LIGHT
      };

// MusicXML export break values
enum MusicxmlExportBreaks {
      ALL_BREAKS, MANUAL_BREAKS, NO_BREAKS
      };

//---------------------------------------------------------
//   PluginDescription
//---------------------------------------------------------

struct PluginDescription {
      QString path;
      QString version;
      QString description;
      bool load;
      Shortcut shortcut;
      QString menuPath;
      };

//---------------------------------------------------------
//   Preferences
//---------------------------------------------------------

struct Preferences {
      bool bgUseColor;
      bool fgUseColor;
      QString bgWallpaper;
      QString fgWallpaper;
      QColor fgColor;
      int iconHeight, iconWidth;
      QColor dropColor;
      bool enableMidiInput;
      bool playNotes;         // play notes on click
      QString lPort;          // audio port left
      QString rPort;          // audio port right
      bool showNavigator;
      bool showPlayPanel;
      bool showWebPanel;
      bool showStatusBar;

      bool useAlsaAudio;
      bool useJackAudio;
      bool usePortaudioAudio;
      bool usePulseAudio;
      bool useJackMidi;
      int midiPorts;
      bool rememberLastMidiConnections;

      QString alsaDevice;
      int alsaSampleRate;
      int alsaPeriodSize;
      int alsaFragments;
      int portaudioDevice;
      QString portMidiInput;

      bool antialiasedDrawing;
      SessionStart sessionStart;
      QString startScore;
      QString defaultStyleFile;
      bool showSplashScreen;

      bool useMidiRemote;
      MidiRemote midiRemote[MIDI_REMOTES];

      bool midiExpandRepeats;
      QString instrumentList1; // file path of instrument templates
      QString instrumentList2;

      bool musicxmlImportLayout;
      bool musicxmlImportBreaks;
      bool musicxmlExportLayout;
      MusicxmlExportBreaks musicxmlExportBreaks;

      bool alternateNoteEntryMethod;
      int proximity;          // proximity for selecting elements on canvas
      bool autoSave;
      int autoSaveTime;
      double pngResolution;
      bool pngTransparent;
      QString language;

      bool replaceCopyrightSymbol;
      double mag;

      //update
      int checkUpdateStartup;

      bool followSong;
      QString importCharsetOve;
      QString importCharsetGP;
      QString importStyleFile;
      int shortestNote;             // for midi input
      MidiImportOperations midiImportOperations;

      bool useOsc;
      int oscPort;
      bool singlePalette;
      QString styleName;
      int globalStyle;        // 0 - dark, 1 - light
      bool animations;

      QString myScoresPath;
      QString myStylesPath;
      QString myImagesPath;
      QString myTemplatesPath;
      QString myPluginsPath;

      QString sfPath;
      QString sfzPath;

      bool nativeDialogs;

      int exportAudioSampleRate;

      QString workspace;

      bool firstStartWeb;

      bool dirty;

      QList<PluginDescription> pluginList;

      bool readPluginList();
      void writePluginList();
      void updatePluginList();

      Preferences();
      void write();
      void read();
      void init();
      bool readDefaultStyle();
      };

//---------------------------------------------------------
//   ShortcutItem
//---------------------------------------------------------

class ShortcutItem : public QTreeWidgetItem {

      bool operator<(const QTreeWidgetItem&) const;

   public:
      ShortcutItem() : QTreeWidgetItem() {}
      };

extern Preferences preferences;
extern bool useALSA, useJACK, usePortaudio, usePulseAudio;

} // namespace Ms
#endif
