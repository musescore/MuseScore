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

/*
 * HOW TO ADD A NEW PREFERENCE
 * - Add a new define to the list of defines below
 * - Add the preference to the _allPreferences map in the init() function in preferences.cpp
 *   and specify the default value for this preference
 * - That's it. The preference is stored and retrieved automatically and can be read
 *   using getString(), getInt(), etc., and changed using setPreference()
 */

#include "globals.h"

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
      DARK_FUSION = 0,
      LIGHT_FUSION
      };

// MusicXML export break values
enum class MusicxmlExportBreaks : char {
      ALL, MANUAL, NO
      };

//
// Defines for all preferences
// Every preference should have a define to ease the usage of the preference
// Make sure the string key has a sensible grouping - use / for grouping
//
#define PREF_APP_AUTOSAVE_AUTOSAVETIME                      "application/autosave/autosaveTime"
#define PREF_APP_AUTOSAVE_USEAUTOSAVE                       "application/autosave/useAutosave"
#define PREF_APP_KEYBOARDLAYOUT                             "application/keyboardLayout"
// file path of instrument templates
#define PREF_APP_PATHS_INSTRUMENTLIST1                      "application/paths/instrumentList1"
#define PREF_APP_PATHS_INSTRUMENTLIST2                      "application/paths/instrumentList2"
#define PREF_APP_PATHS_MYIMAGES                             "application/paths/myImages"
#define PREF_APP_PATHS_MYPLUGINS                            "application/paths/myPlugins"
#define PREF_APP_PATHS_MYSCORES                             "application/paths/myScores"
#define PREF_APP_PATHS_MYSHORTCUTS                          "application/paths/myShortcuts"
#define PREF_APP_PATHS_MYSOUNDFONTS                         "application/paths/mySoundfonts"
#define PREF_APP_PATHS_MYSTYLES                             "application/paths/myStyles"
#define PREF_APP_PATHS_MYTEMPLATES                          "application/paths/myTemplates"
#define PREF_APP_PLAYBACK_FOLLOWSONG                        "application/playback/followSong"
#define PREF_APP_PLAYBACK_PANPLAYBACK                       "application/playback/panPlayback"
#define PREF_APP_PLAYBACK_PLAYREPEATS                       "application/playback/playRepeats"
#define PREF_APP_USESINGLEPALETTE                           "application/useSinglePalette"
#define PREF_APP_STARTUP_FIRSTSTART                         "application/startup/firstStart"
#define PREF_APP_STARTUP_SESSIONSTART                       "application/startup/sessionStart"
#define PREF_APP_STARTUP_STARTSCORE                         "application/startup/startScore"
#define PREF_APP_WORKSPACE                                  "application/workspace"
#define PREF_EXPORT_AUDIO_SAMPLERATE                        "export/audio/sampleRate"
#define PREF_EXPORT_MP3_BITRATE                             "export/mp3/bitRate"
#define PREF_EXPORT_MUSICXML_EXPORTLAYOUT                   "export/musicXML/exportLayout"
#define PREF_EXPORT_MUSICXML_EXPORTBREAKS                   "export/musicXML/exportBreaks"
#define PREF_EXPORT_PDF_DPI                                 "export/pdf/dpi"
#define PREF_EXPORT_PNG_RESOLUTION                          "export/png/resolution"
#define PREF_EXPORT_PNG_USETRANSPARENCY                     "export/png/useTransparency"
#define PREF_IMPORT_GUITARPRO_CHARSET                       "import/guitarpro/charset"
#define PREF_IMPORT_MUSICXML_IMPORTBREAKS                   "import/musicXML/importBreaks"
#define PREF_IMPORT_MUSICXML_IMPORTLAYOUT                   "import/musicXML/importLayout"
#define PREF_IMPORT_OVERTURE_CHARSET                        "import/overture/charset"
#define PREF_IMPORT_STYLE_STYLEFILE                         "import/style/styleFile"
#define PREF_IO_ALSA_DEVICE                                 "io/alsa/device"
#define PREF_IO_ALSA_FRAGMENTS                              "io/alsa/fragments"
#define PREF_IO_ALSA_PERIODSIZE                             "io/alsa/periodSize"
#define PREF_IO_ALSA_SAMPLERATE                             "io/alsa/sampleRate"
#define PREF_IO_ALSA_USEALSAAUDIO                           "io/alsa/useAlsaAudio"
#define PREF_IO_JACK_REMEMBERLASTCONNECTIONS                "io/jack/rememberLastConnections"
#define PREF_IO_JACK_TIMEBASEMASTER                         "io/jack/timebaseMaster"
#define PREF_IO_JACK_USEJACKAUDIO                           "io/jack/useJackAudio"
#define PREF_IO_JACK_USEJACKMIDI                            "io/jack/useJackMIDI"
#define PREF_IO_JACK_USEJACKTRANSPORT                       "io/jack/useJackTransport"
#define PREF_IO_MIDI_ADVANCEONRELEASE                       "io/midi/advanceOnRelease"
#define PREF_IO_MIDI_ENABLEINPUT                            "io/midi/enableInput"
#define PREF_IO_MIDI_EXPANDREPEATS                          "io/midi/expandRepeats"
#define PREF_IO_MIDI_EXPORTRPNS                             "io/midi/exportRPNs"
#define PREF_IO_MIDI_REALTIMEDELAY                          "io/midi/realtimeDelay"
#define PREF_IO_MIDI_REMOTE                                 "io/midi/remote"
#define PREF_IO_MIDI_SHORTESTNOTE                           "io/midi/shortestNote"
#define PREF_IO_MIDI_SHOWCONTROLSINMIXER                    "io/midi/showControlsInMixer"
#define PREF_IO_MIDI_USEREMOTECONTROL                       "io/midi/useRemoteControl"
#define PREF_IO_OSC_PORTNUMBER                              "io/osc/portNumber"
#define PREF_IO_OSC_USEREMOTECONTROL                        "io/osc/useRemoteControl"
#define PREF_IO_PORTAUDIO_DEVICE                            "io/portAudio/device"
#define PREF_IO_PORTAUDIO_USEPORTAUDIO                      "io/portAudio/usePortAudio"
#define PREF_IO_PORTMIDI_INPUTBUFFERCOUNT                   "io/portMidi/inputBufferCount"
#define PREF_IO_PORTMIDI_INPUTDEVICE                        "io/portMidi/inputDevice"
#define PREF_IO_PORTMIDI_OUTPUTBUFFERCOUNT                  "io/portMidi/outputBufferCount"
#define PREF_IO_PORTMIDI_OUTPUTDEVICE                       "io/portMidi/outputDevice"
#define PREF_IO_PORTMIDI_OUTPUTLATENCYMILLISECONDS          "io/portMidi/outputLatencyMilliseconds"
#define PREF_IO_PULSEAUDIO_USEPULSEAUDIO                    "io/pulseAudio/usePulseAudio"
#define PREF_SCORE_CHORD_PLAYONADDNOTE                      "score/chord/playOnAddNote"
#define PREF_SCORE_MAGNIFICATION                            "score/magnification"
#define PREF_SCORE_NOTE_PLAYONCLICK                         "score/note/playOnClick"
#define PREF_SCORE_NOTE_DEFAULTPLAYDURATION                 "score/note/defaultPlayDuration"
#define PREF_SCORE_NOTE_WARNPITCHRANGE                      "score/note/warnPitchRange"
#define PREF_SCORE_STYLE_DEFAULTSTYLEFILE                   "score/style/defaultStyleFile"
#define PREF_SCORE_STYLE_PARTSTYLEFILE                      "score/style/partStyleFile"
#define PREF_UI_CANVAS_BG_USECOLOR                          "ui/canvas/background/useColor"
#define PREF_UI_CANVAS_FG_USECOLOR                          "ui/canvas/foreground/useColor"
#define PREF_UI_CANVAS_BG_COLOR                             "ui/canvas/background/color"
#define PREF_UI_CANVAS_FG_COLOR                             "ui/canvas/foreground/color"
#define PREF_UI_CANVAS_BG_WALLPAPER                         "ui/canvas/background/wallpaper"
#define PREF_UI_CANVAS_FG_WALLPAPER                         "ui/canvas/foreground/wallpaper"
#define PREF_UI_CANVAS_MISC_ANTIALIASEDDRAWING              "ui/canvas/misc/antialiasedDrawing"
#define PREF_UI_CANVAS_MISC_SELECTIONPROXIMITY              "ui/canvas/misc/selectionProximity"
#define PREF_UI_CANVAS_SCROLL_VERTICALORIENTATION           "ui/canvas/scroll/verticalOrientation"
#define PREF_UI_CANVAS_SCROLL_LIMITSCROLLAREA               "ui/canvas/scroll/limitScrollArea"
#define PREF_UI_APP_STARTUP_CHECKUPDATE                     "ui/application/startup/checkUpdate"
#define PREF_UI_APP_STARTUP_SHOWNAVIGATOR                   "ui/application/startup/showNavigator"
#define PREF_UI_APP_STARTUP_SHOWPLAYPANEL                   "ui/application/startup/showPlayPanel"
#define PREF_UI_APP_STARTUP_SHOWSPLASHSCREEN                "ui/application/startup/showSplashScreen"
#define PREF_UI_APP_STARTUP_SHOWSTARTCENTER                 "ui/application/startup/showStartCenter"
#define PREF_UI_APP_GLOBALSTYLE                             "ui/application/globalStyle"
#define PREF_UI_APP_LANGUAGE                                "ui/application/language"
#define PREF_UI_APP_RASTER_HORIZONTAL                       "ui/application/raster/horizontal"
#define PREF_UI_APP_RASTER_VERTICAL                         "ui/application/raster/vertical"
#define PREF_UI_APP_SHOWSTATUSBAR                           "ui/application/showStatusBar"
#define PREF_UI_APP_USENATIVEDIALOGS                        "ui/application/useNativeDialogs"
#define PREF_UI_PIANO_HIGHLIGHTCOLOR                        "ui/piano/highlightColor"
#define PREF_UI_SCORE_NOTE_DROPCOLOR                        "ui/score/note/dropColor"
#define PREF_UI_SCORE_DEFAULTCOLOR                          "ui/score/defaultColor"
#define PREF_UI_SCORE_FRAMEMARGINCOLOR                      "ui/score/frameMarginColor"
#define PREF_UI_SCORE_LAYOUTBREAKCOLOR                      "ui/score/layoutBreakColor"
#define PREF_UI_SCORE_VOICE1_COLOR                          "ui/score/voice1/color"
#define PREF_UI_SCORE_VOICE2_COLOR                          "ui/score/voice2/color"
#define PREF_UI_SCORE_VOICE3_COLOR                          "ui/score/voice3/color"
#define PREF_UI_SCORE_VOICE4_COLOR                          "ui/score/voice4/color"
#define PREF_UI_THEME_ICONHEIGHT                            "ui/theme/iconHeight"
#define PREF_UI_THEME_ICONWIDTH                             "ui/theme/iconWidth"


class PreferenceVisitor;

//---------------------------------------------------------
//   Preference
//---------------------------------------------------------
class Preference {
   private:
      QVariant _defaultValue = 0;
      bool _showInAdvancedList = true;

   protected:
      QMetaType::Type _type = QMetaType::UnknownType;
      Preference(QVariant defaultValue) : _defaultValue(defaultValue) {}

   public:
      Preference(QVariant defaultValue, QMetaType::Type type, bool showInAdvancedList = true);
      virtual ~Preference() {}

      QVariant defaultValue() const {return _defaultValue;}
      bool showInAdvancedList() const {return _showInAdvancedList;}
      QMetaType::Type type() {return _type;}
      virtual void accept(QString key, PreferenceVisitor&) = 0;
      };

class IntPreference : public Preference {
   public:
      IntPreference(int defaultValue, bool showInAdvancedList = true);
      virtual void accept(QString key, PreferenceVisitor&);
      };

class DoublePreference : public Preference {
   public:
      DoublePreference(double defaultValue, bool showInAdvancedList = true);
      virtual void accept(QString key, PreferenceVisitor&);
      };

class BoolPreference : public Preference {
   public:
      BoolPreference(bool defaultValue, bool showInAdvancedList = true);
      virtual void accept(QString key, PreferenceVisitor&);
      };

class StringPreference: public Preference {
   public:
      StringPreference(QString defaultValue, bool showInAdvancedList = true);
      virtual void accept(QString key, PreferenceVisitor&);
      };

class ColorPreference: public Preference {
   public:
      ColorPreference(QColor defaultValue, bool showInAdvancedList = true);
      virtual void accept(QString key, PreferenceVisitor&);
      };

// Support for EnumPreference is currently not fully implemented
class EnumPreference: public Preference {
   public:
      EnumPreference(QVariant defaultValue, bool showInAdvancedList = true);
      virtual void accept(QString, PreferenceVisitor&);
      };

//---------------------------------------------------------
//   Preferences
//---------------------------------------------------------

class Preferences {
   public:
      typedef QHash<QString, Preference*> prefs_map_t;

   private:

      // Map of all preferences and their default values
      // A preference can not be read or set if it is not present in this map
      // This map is not used for storing a preference it is only for default values
      prefs_map_t _allPreferences;
      // used for storing preferences in memory when _storeInMemoryOnly is true
      // and for storing temporary preferences
      QHash<QString, QVariant> _inMemorySettings;
      bool _storeInMemoryOnly = false;
      bool _returnDefaultValues = false;
      bool _initialized = false;
      QSettings* _settings; // should not be used directly but through settings() accessor

      QSettings* settings() const;
      // the following functions must be used to access and change a preference
      // instead of using QSettings directly
      QVariant get(const QString key) const;
      bool has(const QString key) const;
      void set(const QString key, QVariant value, bool temporary = false);
      void remove(const QString key);

      QVariant preference(const QString key) const;
      QMetaType::Type type(const QString key) const;
      bool checkIfKeyExists(const QString key) const;
      bool checkType(const QString key, QMetaType::Type t) const;

   public:
      Preferences();
      ~Preferences();
      void init(bool storeInMemoryOnly = false);
      void save();
      // set to true to let getters return default values instead of values from QSettings
      void setReturnDefaultValues(bool returnDefaultValues) {_returnDefaultValues = returnDefaultValues;}

      const prefs_map_t& allPreferences() const {return _allPreferences;}

      // general getters
      QVariant defaultValue(const QString key) const;
      bool getBool(const QString key) const;
      QColor getColor(const QString key) const;
      QString getString(const QString key) const;
      int getInt(const QString key) const;
      double getDouble(const QString key) const;

      // general setters
      void revertToDefaultValue(const QString key);
      void setPreference(const QString key, QVariant value);

      // A temporary preference is stored "in memory" only and not written to file.
      // If there is both a "normal" preference and a temporary preference with the same
      // key the temporary preference is used
      void setTemporaryPreference(const QString key, QVariant value);

      /*
       * Some preferences like enums and structs/classes are not easily read using the general set/get methods
       * and therefore require specific getters and/or setters
       */
      SessionStart sessionStart() const;
      MusicxmlExportBreaks musicxmlExportBreaks() const;
      MuseScoreStyleType globalStyle() const;
      bool isThemeDark() const;

      template<typename T>
      void setCustomPreference(const QString key, T t)
            {
            set(key, QVariant::fromValue<T>(t));
            }

      // The midiRemote preference requires special handling due to its complexity
      MidiRemote midiRemote(int recordId) const;
      void updateMidiRemote(int recordId, MidiRemoteType type, int data);
      void clearMidiRemote(int recordId);
      };

// singleton
extern Preferences preferences;

// Stream operators for enum classes
// enum classes don't play well with QSettings without custom serialization
template<typename T, typename std::enable_if<std::is_enum<T>::value>::type* = nullptr>
inline QDataStream &operator<<(QDataStream &out, const T &val)
{
    return out << static_cast<int>(val);
}

template<typename T, typename std::enable_if<std::is_enum<T>::value>::type* = nullptr>
inline QDataStream &operator>>(QDataStream &in, T &val)
{
    int tmp;
    in >> tmp;
    val = static_cast<T>(tmp);
    return in;
}

class PreferenceVisitor {
   public:
      virtual void visit(QString key, IntPreference*) = 0;
      virtual void visit(QString key, DoublePreference*) = 0;
      virtual void visit(QString key, BoolPreference*) = 0;
      virtual void visit(QString key, StringPreference*) = 0;
      virtual void visit(QString key, ColorPreference*) = 0;
      };


} // namespace Ms

Q_DECLARE_METATYPE(Ms::SessionStart);
Q_DECLARE_METATYPE(Ms::MusicxmlExportBreaks);
Q_DECLARE_METATYPE(Ms::MuseScoreStyleType);

#endif
