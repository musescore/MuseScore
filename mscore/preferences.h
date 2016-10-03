//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: preferences.h 5660 2012-05-22 14:17:39Z wschweer $
//
//  Copyright (C) 2002-2016 Werner Schweer and others
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
#include "mscore/importmidi/importmidi_operations.h"

namespace Ms {

extern QString mscoreGlobalShare;

enum class SessionStart : char {
      EMPTY, LAST, NEW, SCORE
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
      RMIDI_UNDO,
      RMIDI_NOTE_EDIT_MODE,
      RMIDI_REALTIME_ADVANCE,
      MIDI_REMOTES
      };

enum class MuseScoreStyleType : char {
      DARK_OXYGEN = 0,
      LIGHT_OXYGEN,
      DARK_FUSION,
      LIGHT_FUSION
      };

// MusicXML export break values
enum class MusicxmlExportBreaks : char {
      ALL, MANUAL, NO
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
      QColor pianoHlColor;
      bool enableMidiInput;
      int realtimeDelay;
      bool playNotes;         // play notes on click
      bool playChordOnAddNote;
      bool showNavigator;
      bool showPlayPanel;
      bool showStatusBar;

      bool useAlsaAudio;
      bool useJackAudio;
      bool usePortaudioAudio;
      bool usePulseAudio;
      bool useJackMidi;
      bool useJackTransport;
      bool jackTimebaseMaster;
      bool rememberLastConnections;

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
      bool showStartcenter;

      bool useMidiRemote;
      MidiRemote midiRemote[MIDI_REMOTES];
      bool advanceOnRelease;

      bool midiExpandRepeats;
      bool midiExportRPNs;
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

      double mag;
      bool showMidiControls;

      //update
      bool checkUpdateStartup;

      bool followSong;
      QString importCharsetOve;
      QString importCharsetGP;
      QString importStyleFile;
      int shortestNote;             // for midi input
      MidiOperations::Data midiImportOperations;

      bool useOsc;
      int oscPort;
      bool singlePalette;
      //QString styleName;
      MuseScoreStyleType globalStyle;
      bool animations;

      QString myScoresPath;
      QString myStylesPath;
      QString myImagesPath;
      QString myTemplatesPath;
      QString myPluginsPath;
      QString mySoundfontsPath;

      bool nativeDialogs;

      int exportAudioSampleRate;

      QString workspace;
      int exportPdfDpi;

      bool dirty;

      QList<PluginDescription> pluginList;

      bool readPluginList();
      void writePluginList();
      void updatePluginList();

      Preferences();
      void write();
      void read();
      QColor readColor(QString key, QColor def);
      void init();
      bool readDefaultStyle();
      bool isThemeDark() { return globalStyle == MuseScoreStyleType::DARK_OXYGEN || globalStyle == MuseScoreStyleType::DARK_FUSION;}
      bool isOxygen() { return globalStyle == MuseScoreStyleType::DARK_OXYGEN || globalStyle == MuseScoreStyleType::LIGHT_OXYGEN;}
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
