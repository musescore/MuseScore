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
 * - Add a new define to the list of defines below (since the #defines define char[],
 *   don't go camelCase in there, it facilitates the translations)
 * - Add the preference to the _allPreferences map in the init() function in preferences.cpp
 * - Specify the default value for this preference.
 * - Specify if the preference will go in the advanced list.
 * That's it. The preference is stored and retrieved automatically and can be read
 * using getString(), getInt(), etc., and changed using setPreference()
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
#define PREF_APP_AUTOSAVE_AUTOSAVETIME                      "Application/Autosave/Autosave time"
#define PREF_APP_AUTOSAVE_USEAUTOSAVE                       "Application/Autosave/Use autosave"
#define PREF_APP_KEYBOARDLAYOUT                             "Application/Keyboard layout"
// file path of instrument templates
#define PREF_APP_PATHS_INSTRUMENTLIST1                      "Application/Paths/Instrument list 1"
#define PREF_APP_PATHS_INSTRUMENTLIST2                      "Application/Paths/Instrument list 2"
#define PREF_APP_PATHS_MYIMAGES                             "Application/Paths/My images"
#define PREF_APP_PATHS_MYPLUGINS                            "Application/Paths/My plugins"
#define PREF_APP_PATHS_MYSCORES                             "Application/Paths/My scores"
#define PREF_APP_PATHS_MYSHORTCUTS                          "Application/Paths/My shortcuts"
#define PREF_APP_PATHS_MYSOUNDFONTS                         "Application/Paths/My soundfonts"
#define PREF_APP_PATHS_MYSTYLES                             "Application/Paths/My styles"
#define PREF_APP_PATHS_MYTEMPLATES                          "Application/Paths/My templates"
#define PREF_APP_PATHS_MYEXTENSIONS                         "Application/Paths/My extensions"
#define PREF_APP_PLAYBACK_FOLLOWSONG                        "Application/Playback/Follow song"
#define PREF_APP_PLAYBACK_PANPLAYBACK                       "Application/Playback/Pan playback"
#define PREF_APP_PLAYBACK_PLAYREPEATS                       "Application/Playback/Play repeats"
#define PREF_APP_USESINGLEPALETTE                           "Application/Use single palette"
#define PREF_APP_STARTUP_FIRSTSTART                         "Application/Startup/First start"
#define PREF_APP_STARTUP_SESSIONSTART                       "Application/Startup/Session start"
#define PREF_APP_STARTUP_STARTSCORE                         "Application/Startup/Start score"
#define PREF_APP_WORKSPACE                                  "Application/Workspace"
#define PREF_EXPORT_AUDIO_SAMPLERATE                        "Export/Audio/Sample rate"
#define PREF_EXPORT_MP3_BITRATE                             "Export/Mp3/Bit rate"
#define PREF_EXPORT_MUSICXML_EXPORTLAYOUT                   "Export/MusicXML/Export layout"
#define PREF_EXPORT_MUSICXML_EXPORTBREAKS                   "Export/MusicXML/Export breaks"
#define PREF_EXPORT_PDF_DPI                                 "Export/Pdf/Dpi"
#define PREF_EXPORT_PNG_RESOLUTION                          "Export/Png/Resolution"
#define PREF_EXPORT_PNG_USETRANSPARENCY                     "Export/Png/Use transparency"
#define PREF_IMPORT_GUITARPRO_CHARSET                       "Import/Guitarpro/Charset"
#define PREF_IMPORT_MUSICXML_IMPORTBREAKS                   "Import/MusicXML/Import breaks"
#define PREF_IMPORT_MUSICXML_IMPORTLAYOUT                   "Import/MusicXML/Import layout"
#define PREF_IMPORT_OVERTURE_CHARSET                        "Import/Overture/Charset"
#define PREF_IMPORT_STYLE_STYLEFILE                         "Import/Style/Style file"
#define PREF_IO_ALSA_DEVICE                                 "IO/Alsa/Device"
#define PREF_IO_ALSA_FRAGMENTS                              "IO/Alsa/Fragments"
#define PREF_IO_ALSA_PERIODSIZE                             "IO/Alsa/Period size"
#define PREF_IO_ALSA_SAMPLERATE                             "IO/Alsa/Sample rate"
#define PREF_IO_ALSA_USEALSAAUDIO                           "IO/Alsa/Use AlsaAudio"
#define PREF_IO_JACK_REMEMBERLASTCONNECTIONS                "IO/Jack/Remember last connections"
#define PREF_IO_JACK_TIMEBASEMASTER                         "IO/Jack/Timebase master"
#define PREF_IO_JACK_USEJACKAUDIO                           "IO/Jack/Use JackAudio"
#define PREF_IO_JACK_USEJACKMIDI                            "IO/Jack/Use JackMIDI"
#define PREF_IO_JACK_USEJACKTRANSPORT                       "IO/Jack/Use JackTransport"
#define PREF_IO_MIDI_ADVANCEONRELEASE                       "IO/Midi/Advance on release"
#define PREF_IO_MIDI_ENABLEINPUT                            "IO/Midi/Enable input"
#define PREF_IO_MIDI_EXPANDREPEATS                          "IO/Midi/Expand repeats"
#define PREF_IO_MIDI_EXPORTRPNS                             "IO/Midi/Export RPN's"
#define PREF_IO_MIDI_REALTIMEDELAY                          "IO/Midi/Realtime delay"
#define PREF_IO_MIDI_REMOTE                                 "IO/Midi/Remote"
#define PREF_IO_MIDI_SHORTESTNOTE                           "IO/Midi/Shortest note"
#define PREF_IO_MIDI_SHOWCONTROLSINMIXER                    "IO/Midi/Show controls in mixer"
#define PREF_IO_MIDI_USEREMOTECONTROL                       "IO/Midi/Use remote control"
#define PREF_IO_OSC_PORTNUMBER                              "IO/Osc/Port number"
#define PREF_IO_OSC_USEREMOTECONTROL                        "IO/Osc/Use remote control"
#define PREF_IO_PORTAUDIO_DEVICE                            "IO/PortAudio/Device"
#define PREF_IO_PORTAUDIO_USEPORTAUDIO                      "IO/PortAudio/Use PortAudio"
#define PREF_IO_PORTMIDI_INPUTBUFFERCOUNT                   "IO/PortMidi/Input buffer count"
#define PREF_IO_PORTMIDI_INPUTDEVICE                        "IO/PortMidi/Input device"
#define PREF_IO_PORTMIDI_OUTPUTBUFFERCOUNT                  "IO/PortMidi/Output buffer count"
#define PREF_IO_PORTMIDI_OUTPUTDEVICE                       "IO/PortMidi/Output device"
#define PREF_IO_PORTMIDI_OUTPUTLATENCYMILLISECONDS          "IO/PortMidi/Output latency (milliseconds)"
#define PREF_IO_PULSEAUDIO_USEPULSEAUDIO                    "IO/PulseAudio/Use PulseAudio"
#define PREF_SCORE_CHORD_PLAYONADDNOTE                      "Score/Chord/Play on add note"
#define PREF_SCORE_MAGNIFICATION                            "Score/Magnification"
#define PREF_SCORE_NOTE_PLAYONCLICK                         "Score/Note/Play on click"
#define PREF_SCORE_NOTE_DEFAULTPLAYDURATION                 "Score/Note/Default play duration"
#define PREF_SCORE_NOTE_WARNPITCHRANGE                      "Score/Note/Warn pitch range"
#define PREF_SCORE_STYLE_DEFAULTSTYLEFILE                   "Score/Style/Default style file"
#define PREF_SCORE_STYLE_PARTSTYLEFILE                      "Score/Style/Part style file"
#define PREF_UI_CANVAS_BG_USECOLOR                          "UI/Canvas/Background/Use color"
#define PREF_UI_CANVAS_FG_USECOLOR                          "UI/Canvas/Foreground/Use color"
#define PREF_UI_CANVAS_BG_COLOR                             "UI/Canvas/Background/Color"
#define PREF_UI_CANVAS_FG_COLOR                             "UI/Canvas/Foreground/Color"
#define PREF_UI_CANVAS_BG_WALLPAPER                         "UI/Canvas/Background/Wallpaper"
#define PREF_UI_CANVAS_FG_WALLPAPER                         "UI/Canvas/Foreground/Wallpaper"
#define PREF_UI_CANVAS_MISC_ANTIALIASEDDRAWING              "UI/Canvas/Misc/Antialiased drawing"
#define PREF_UI_CANVAS_MISC_SELECTIONPROXIMITY              "UI/Canvas/Misc/Selection proximity"
#define PREF_UI_CANVAS_SCROLL_VERTICALORIENTATION           "UI/Canvas/Scroll/Vertical orientation"
#define PREF_UI_CANVAS_SCROLL_LIMITSCROLLAREA               "UI/Canvas/Scroll/Limit scroll area"
#define PREF_UI_APP_STARTUP_CHECKUPDATE                     "UI/Application/Startup/Check for update"
#define PREF_UI_APP_STARTUP_CHECK_EXTENSIONS_UPDATE         "UI/Application/Startup/Check for extensions update"
#define PREF_UI_APP_STARTUP_SHOWNAVIGATOR                   "UI/Application/Startup/Show navigator"
#define PREF_UI_APP_STARTUP_SHOWPLAYPANEL                   "UI/Application/Startup/Show play panel"
#define PREF_UI_APP_STARTUP_SHOWSPLASHSCREEN                "UI/Application/Startup/Show splash screen"
#define PREF_UI_APP_STARTUP_SHOWSTARTCENTER                 "UI/Application/Startup/Show start center"
#define PREF_UI_APP_GLOBALSTYLE                             "UI/Application/Global style"
#define PREF_UI_APP_LANGUAGE                                "UI/Application/Language"
#define PREF_UI_APP_RASTER_HORIZONTAL                       "UI/Application/Raster/Horizontal"
#define PREF_UI_APP_RASTER_VERTICAL                         "UI/Application/Raster/Vertical"
#define PREF_UI_APP_SHOWSTATUSBAR                           "UI/Application/Show status bar"
#define PREF_UI_APP_USENATIVEDIALOGS                        "UI/Application/Use native dialogs"
#define PREF_UI_PIANOHIGHLIGHTCOLOR                         "UI/Piano highlight color"
#define PREF_UI_SCORE_NOTEDROPCOLOR                         "UI/Score/Note drop color"
#define PREF_UI_SCORE_DEFAULTCOLOR                          "UI/Score/Default color"
#define PREF_UI_SCORE_FRAMEMARGINCOLOR                      "UI/Score/Frame margin color"
#define PREF_UI_SCORE_LAYOUTBREAKCOLOR                      "UI/Score/Layout break color"
#define PREF_UI_SCORE_VOICES_VOICE1COLOR                    "UI/Score/Voices/Voice 1 color"
#define PREF_UI_SCORE_VOICES_VOICE2COLOR                    "UI/Score/Voices/Voice 2 color"
#define PREF_UI_SCORE_VOICES_VOICE3COLOR                    "UI/Score/Voices/Voice 3 color"
#define PREF_UI_SCORE_VOICES_VOICE4COLOR                    "UI/Score/Voices/Voice 4 color"
#define PREF_UI_THEME_ICONHEIGHT                            "UI/Theme/Icon height"
#define PREF_UI_THEME_ICONWIDTH                             "UI/Theme/Icon width"


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
      virtual void accept(QString, QTreeWidgetItem*, PreferenceVisitor&) = 0;
      };

class IntPreference : public Preference {
   public:
      IntPreference(int defaultValue, bool showInAdvancedList = true);
      virtual void accept(QString, QTreeWidgetItem*, PreferenceVisitor&) override;
      };

class DoublePreference : public Preference {
   public:
      DoublePreference(double defaultValue, bool showInAdvancedList = true);
      virtual void accept(QString, QTreeWidgetItem*, PreferenceVisitor&) override;
      };

class BoolPreference : public Preference {
   public:
      BoolPreference(bool defaultValue, bool showInAdvancedList = true);
      virtual void accept(QString, QTreeWidgetItem*, PreferenceVisitor&) override;
      };

class StringPreference: public Preference {
   public:
      StringPreference(QString defaultValue, bool showInAdvancedList = true);
      virtual void accept(QString, QTreeWidgetItem*, PreferenceVisitor&) override;
      };

class FilePreference : public Preference {
      QString _filter;
   public:
      FilePreference(QString defaultValue, QString filter, bool showInAdvancedList = true);
      virtual void accept(QString, QTreeWidgetItem*, PreferenceVisitor&) override;

      QString filter() const;
};

class DirPreference : public Preference {
   public:
      DirPreference(QString defaultValue, bool showInAdvancedList = true);
      virtual void accept(QString, QTreeWidgetItem*, PreferenceVisitor&) override;
};

class ColorPreference: public Preference {
   public:
      ColorPreference(QColor defaultValue, bool showInAdvancedList = true);
      virtual void accept(QString, QTreeWidgetItem*, PreferenceVisitor&) override;
      };

// Support for EnumPreference is currently not fully implemented
#define PREFS_NO_SUPPORT_FOR_ENUMS
class EnumPreference: public Preference {
   public:
      EnumPreference(QVariant defaultValue, bool showInAdvancedList = true);
      virtual void accept(QString, QTreeWidgetItem*, PreferenceVisitor&) override;
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
      virtual void visit(const QString& key, QTreeWidgetItem*, IntPreference*) = 0;
      virtual void visit(const QString& key, QTreeWidgetItem*, DoublePreference*) = 0;
      virtual void visit(const QString& key, QTreeWidgetItem*, BoolPreference*) = 0;
      virtual void visit(const QString& key, QTreeWidgetItem*, StringPreference*) = 0;
      virtual void visit(const QString& key, QTreeWidgetItem*, FilePreference*) = 0;
      virtual void visit(const QString& key, QTreeWidgetItem*, DirPreference*) = 0;
      virtual void visit(const QString& key, QTreeWidgetItem*, ColorPreference*) = 0;
      };


} // namespace Ms

Q_DECLARE_METATYPE(Ms::SessionStart);
Q_DECLARE_METATYPE(Ms::MusicxmlExportBreaks);
Q_DECLARE_METATYPE(Ms::MuseScoreStyleType);


#endif
