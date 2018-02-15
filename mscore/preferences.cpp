//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: preferences.cpp 5660 2012-05-22 14:17:39Z wschweer $
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

#include "libmscore/style.h"
#include "libmscore/mscore.h"
#include "preferences.h"

namespace Ms {

Preferences preferences;

void Preferences::init(bool storeInMemoryOnly)
      {
      if (!storeInMemoryOnly) {
            if (_settings)
                  delete _settings;

            _settings = new QSettings();
            }

      _storeInMemoryOnly = storeInMemoryOnly;

#if defined(Q_OS_MAC) || (defined(Q_OS_WIN) && !defined(FOR_WINSTORE))
      bool checkUpdateStartup = true;
#else
      bool checkUpdateStartup = false;
#endif

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
      // use system native file dialogs
      // Qt file dialog is very slow on Windows and Mac
      bool nativeDialogs           = true;
#else
      bool nativeDialogs           = false;    // don't use system native file dialogs
#endif

      bool defaultUsePortAudio = false;
      bool defaultUsePulseAudio = false;
      bool defaultUseJackAudio = false;
      bool defaultUseAlsaAudio = false;

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
      defaultUsePortAudio  = true;
      // Linux
#elif defined(USE_PULSEAUDIO)
      defaultUsePulseAudio  = true;
#elif defined(USE_ALSA)
      defaultUseAlsaAudio = true;
#elif defined(USE_PORTAUDIO)
      defaultUsePortAudio = true;
#endif

      QString wd = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).arg(QCoreApplication::applicationName());

      _allPreferences.insert(
      {
            {PREF_APP_AUTOSAVE_AUTOSAVETIME,                       Preference(2 /* minutes */)},
            {PREF_APP_AUTOSAVE_USEAUTOSAVE,                        Preference(true)},
            {PREF_APP_PATHS_INSTRUMENTLIST1,                       Preference(":/data/instruments.xml")},
            {PREF_APP_PATHS_INSTRUMENTLIST2,                       Preference("")},
            {PREF_APP_PATHS_MYIMAGES,                              Preference(QFileInfo(QString("%1/%2").arg(wd).arg(QCoreApplication::translate("images_directory", "Images"))).absoluteFilePath())},
            {PREF_APP_PATHS_MYPLUGINS,                             Preference(QFileInfo(QString("%1/%2").arg(wd).arg(QCoreApplication::translate("plugins_directory", "Plugins"))).absoluteFilePath())},
            {PREF_APP_PATHS_MYSCORES,                              Preference(QFileInfo(QString("%1/%2").arg(wd).arg(QCoreApplication::translate("scores_directory", "Scores"))).absoluteFilePath())},
            {PREF_APP_PATHS_MYSOUNDFONTS,                          Preference(QFileInfo(QString("%1/%2").arg(wd).arg(QCoreApplication::translate("soundfonts_directory", "SoundFonts"))).absoluteFilePath())},
            {PREF_APP_PATHS_MYSHORTCUTS,                           Preference(QFileInfo(QString("%1/%2").arg(wd).arg(QCoreApplication::translate("shortcuts_directory", "Shortcuts"))).absoluteFilePath())},
            {PREF_APP_PATHS_MYSTYLES,                              Preference(QFileInfo(QString("%1/%2").arg(wd).arg(QCoreApplication::translate("styles_directory", "Styles"))).absoluteFilePath())},
            {PREF_APP_PATHS_MYTEMPLATES,                           Preference(QFileInfo(QString("%1/%2").arg(wd).arg(QCoreApplication::translate("templates_directory", "Templates"))).absoluteFilePath())},
            {PREF_APP_PLAYBACK_FOLLOWSONG,                         Preference(true)},
            {PREF_APP_PLAYBACK_PANPLAYBACK,                        Preference(true)},
            {PREF_APP_PLAYBACK_PLAYREPEATS,                        Preference(true)},
            {PREF_APP_USESINGLEPALETTE,                            Preference(false)},
            {PREF_APP_STARTUP_SESSIONSTART,                        Preference(QVariant::fromValue(SessionStart::SCORE))},
            {PREF_APP_STARTUP_STARTSCORE,                          Preference(":/data/My_First_Score.mscz")},
            {PREF_APP_WORKSPACE,                                   Preference("Basic")},
            {PREF_EXPORT_AUDIO_SAMPLERATE,                         Preference(44100)},
            {PREF_EXPORT_MP3_BITRATE,                              Preference(128)},
            {PREF_EXPORT_MUSICXML_EXPORTBREAKS,                    Preference(QVariant::fromValue(MusicxmlExportBreaks::ALL))},
            {PREF_EXPORT_MUSICXML_EXPORTLAYOUT,                    Preference(true)},
            {PREF_EXPORT_PDF_DPI,                                  Preference(300)},
            {PREF_EXPORT_PNG_RESOLUTION,                           Preference(300.0)},
            {PREF_EXPORT_PNG_USETRANSPARENCY,                      Preference(true)},
            {PREF_IMPORT_GUITARPRO_CHARSET,                        Preference("UTF-8")},
            {PREF_IMPORT_MUSICXML_IMPORTBREAKS,                    Preference(true)},
            {PREF_IMPORT_MUSICXML_IMPORTLAYOUT,                    Preference(true)},
            {PREF_IMPORT_OVERTURE_CHARSET,                         Preference("GBK")},
            {PREF_IMPORT_STYLE_STYLEFILE,                          Preference("")},
            {PREF_IO_ALSA_DEVICE,                                  Preference("default")},
            {PREF_IO_ALSA_FRAGMENTS,                               Preference(3)},
            {PREF_IO_ALSA_PERIODSIZE,                              Preference(1024)},
            {PREF_IO_ALSA_SAMPLERATE,                              Preference(48000)},
            {PREF_IO_ALSA_USEALSAAUDIO,                            Preference(defaultUseAlsaAudio)},
            {PREF_IO_JACK_REMEMBERLASTCONNECTIONS,                 Preference(true)},
            {PREF_IO_JACK_TIMEBASEMASTER,                          Preference(false)},
            {PREF_IO_JACK_USEJACKAUDIO,                            Preference(defaultUseJackAudio)},
            {PREF_IO_JACK_USEJACKMIDI,                             Preference(false)},
            {PREF_IO_JACK_USEJACKTRANSPORT,                        Preference(false)},
            {PREF_IO_MIDI_ADVANCEONRELEASE,                        Preference(true)},
            {PREF_IO_MIDI_ENABLEINPUT,                             Preference(true)},
            {PREF_IO_MIDI_EXPANDREPEATS,                           Preference(true)},
            {PREF_IO_MIDI_EXPORTRPNS,                              Preference(false)},
            {PREF_IO_MIDI_REALTIMEDELAY,                           Preference(750 /* ms */)},
            {PREF_IO_MIDI_SHORTESTNOTE,                            Preference(MScore::division/4)},
            {PREF_IO_MIDI_SHOWCONTROLSINMIXER,                     Preference(false)},
            {PREF_IO_MIDI_USEREMOTECONTROL,                        Preference(false)},
            {PREF_IO_OSC_PORTNUMBER,                               Preference(5282)},
            {PREF_IO_OSC_USEREMOTECONTROL,                         Preference(false)},
            {PREF_IO_PORTAUDIO_DEVICE,                             Preference(-1)},
            {PREF_IO_PORTAUDIO_USEPORTAUDIO,                       Preference(defaultUsePortAudio)},
            {PREF_IO_PORTMIDI_INPUTBUFFERCOUNT,                    Preference(100)},
            {PREF_IO_PORTMIDI_INPUTDEVICE,                         Preference("")},
            {PREF_IO_PORTMIDI_OUTPUTBUFFERCOUNT,                   Preference(65536)},
            {PREF_IO_PORTMIDI_OUTPUTDEVICE,                        Preference("")},
            {PREF_IO_PORTMIDI_OUTPUTLATENCYMILLISECONDS,           Preference(0)},
            {PREF_IO_PULSEAUDIO_USEPULSEAUDIO,                     Preference(defaultUsePulseAudio)},
            {PREF_SCORE_CHORD_PLAYONADDNOTE,                       Preference(true)},
            {PREF_SCORE_MAGNIFICATION,                             Preference(1.0)},
            {PREF_SCORE_NOTE_PLAYONCLICK,                          Preference(true)},
            {PREF_SCORE_NOTE_DEFAULTPLAYDURATION,                  Preference(300 /* ms */)},
            {PREF_SCORE_NOTE_WARNPITCHRANGE,                       Preference(true)},
            {PREF_SCORE_STYLE_DEFAULTSTYLEFILE,                    Preference("")},
            {PREF_SCORE_STYLE_PARTSTYLEFILE,                       Preference("")},
            {PREF_UI_CANVAS_BG_USECOLOR,                           Preference(true)},
            {PREF_UI_CANVAS_FG_USECOLOR,                           Preference(true)},
            {PREF_UI_CANVAS_BG_COLOR,                              Preference(QColor("#dddddd"))},
            {PREF_UI_CANVAS_FG_COLOR,                              Preference(QColor("#f9f9f9"))},
            {PREF_UI_CANVAS_BG_WALLPAPER,                          Preference(QFileInfo(QString("%1%2").arg(mscoreGlobalShare).arg("wallpaper/background1.png")).absoluteFilePath())},
            {PREF_UI_CANVAS_FG_WALLPAPER,                          Preference(QFileInfo(QString("%1%2").arg(mscoreGlobalShare).arg("wallpaper/paper5.png")).absoluteFilePath())},
            {PREF_UI_CANVAS_MISC_ANTIALIASEDDRAWING,               Preference(true)},
            {PREF_UI_CANVAS_MISC_SELECTIONPROXIMITY,               Preference(6)},
            {PREF_UI_CANVAS_SCROLL_VERTICALORIENTATION,            Preference(false)},
            {PREF_UI_APP_STARTUP_CHECKUPDATE,                      Preference(checkUpdateStartup)},
            {PREF_UI_APP_STARTUP_SHOWNAVIGATOR,                    Preference(false)},
            {PREF_UI_APP_STARTUP_SHOWPLAYPANEL,                    Preference(false)},
            {PREF_UI_APP_STARTUP_SHOWSPLASHSCREEN,                 Preference(true)},
            {PREF_UI_APP_STARTUP_SHOWSTARTCENTER,                  Preference(true)},
            {PREF_UI_APP_GLOBALSTYLE,                              Preference(QVariant::fromValue(MuseScoreStyleType::LIGHT_FUSION))},
            {PREF_UI_APP_LANGUAGE,                                 Preference("system")},
            {PREF_UI_APP_RASTER_HORIZONTAL,                        Preference(2)},
            {PREF_UI_APP_RASTER_VERTICAL,                          Preference(2)},
            {PREF_UI_APP_SHOWSTATUSBAR,                            Preference(true)},
            {PREF_UI_APP_USENATIVEDIALOGS,                         Preference(nativeDialogs)},
            {PREF_UI_PIANO_HIGHLIGHTCOLOR,                         Preference(QColor("#1259d0"))},
            {PREF_UI_SCORE_NOTE_DROPCOLOR,                         Preference(QColor("#1778db"))},
            {PREF_UI_SCORE_DEFAULTCOLOR,                           Preference(QColor("#000000"))},
            {PREF_UI_SCORE_FRAMEMARGINCOLOR,                       Preference(QColor("#5999db"))},
            {PREF_UI_SCORE_LAYOUTBREAKCOLOR,                       Preference(QColor("#5999db"))},
            {PREF_UI_SCORE_VOICE1_COLOR,                           Preference(QColor("#1259d0"))},    // blue
            {PREF_UI_SCORE_VOICE2_COLOR,                           Preference(QColor("#009234"))},    // green
            {PREF_UI_SCORE_VOICE3_COLOR,                           Preference(QColor("#c04400"))},    // orange
            {PREF_UI_SCORE_VOICE4_COLOR,                           Preference(QColor("#70167a"))},    // purple
            {PREF_UI_THEME_ICONWIDTH,                              Preference(28)},
            {PREF_UI_THEME_ICONHEIGHT,                             Preference(24)}
            });

      _initialized = true;
      }

void Preferences::save()
      {
      settings()->sync();
      }

QVariant Preferences::defaultValue(const QString key) const
      {
      checkIfKeyExists(key);
      auto pref = _allPreferences.find(key.toStdString());
      return pref->second.defaultValue();
      }

QSettings* Preferences::settings() const
      {
      if (!_initialized) {
            qWarning("Preferences is not initialized. Call init() to initialize.");
            Q_ASSERT(_initialized);
            }

      return _settings;
      }

QVariant Preferences::get(const QString key) const
      {
      auto pref = _inMemorySettings.find(key.toStdString());

      if (_storeInMemoryOnly)
            return (pref != _inMemorySettings.end()) ? pref->second : QVariant(); // invalid QVariant returned when not found
      else if (pref != _inMemorySettings.end()) // if there exists a temporary value stored "in memory" return this value
            return pref->second;
      else
            return settings()->value(key);
      }

void Preferences::set(const QString key, QVariant value, bool temporary)
      {
      if (_storeInMemoryOnly || temporary)
            _inMemorySettings[key.toStdString()] = value;
      else
            settings()->setValue(key, value);
      }

void Preferences::remove(const QString key)
      {
      // remove both preference stored "in memory" and in QSettings
      _inMemorySettings.erase(key.toStdString());
      settings()->remove(key);
      }

bool Preferences::has(const QString key) const
      {
      return _inMemorySettings.count(key.toStdString()) > 0 || settings()->contains(key);
      }

QVariant Preferences::preference(const QString key) const
      {
      checkIfKeyExists(key);
      QVariant pref = get(key);

      // pref is invalid both if setting is not found or pref is an invalid QVariant object
      if (!pref.isValid() || pref.isNull() || _returnDefaultValues)
            return defaultValue(key);
      else
            return pref;
      }

void Preferences::checkIfKeyExists(const QString key) const
      {
      auto pref = _allPreferences.find(key.toStdString());
      if (pref == _allPreferences.end()) {
            qWarning("Preference not found: %s", key.toStdString().c_str());
            Q_ASSERT(pref != _allPreferences.end());
            }
      }

Preferences::Preferences()
      : _settings(0)
      {}

Preferences::~Preferences()
      {
      if (_settings) {
            delete _settings;
            }
      }

void Preferences::setReturnDefaultValues(bool returnDefaultValues)
      {
      _returnDefaultValues = returnDefaultValues;
      }

bool Preferences::getBool(const QString key) const
      {
      return preference(key).toBool();
      }

QColor Preferences::getColor(const QString key) const
      {
      QVariant v = preference(key);
      if (v.type() == QVariant::Color)
            return v.value<QColor>();
      else {
            // in case the color is expressed in settings file as a textual color representation
            QColor c(v.toString());
            return c.isValid() ? c : defaultValue(key).value<QColor>();
            }
      }

QString Preferences::getString(const QString key) const
      {
      return preference(key).toString();
      }

int Preferences::getInt(const QString key) const
      {
      QVariant v = preference(key);
      bool ok;
      int pref = v.toInt(&ok);
      if (!ok) {
            qWarning("Can not convert preference %s to int. Returning default value.", key.toStdString().c_str());
            return defaultValue(key).toInt();
            }
      return pref;
}

double Preferences::getDouble(const QString key) const
      {
      QVariant v = preference(key);
      bool ok;
      double pref = v.toDouble(&ok);
      if (!ok) {
            qWarning("Can not convert preference %s to double. Returning default value.", key.toStdString().c_str());
            return defaultValue(key).toDouble();
            }
      return pref;
      }

SessionStart Preferences::sessionStart() const
      {
      return preference(PREF_APP_STARTUP_SESSIONSTART).value<SessionStart>();
      }

MusicxmlExportBreaks Preferences::musicxmlExportBreaks() const
      {
      return preference(PREF_EXPORT_MUSICXML_EXPORTBREAKS).value<MusicxmlExportBreaks>();
      }

MuseScoreStyleType Preferences::globalStyle() const
      {
      return preference(PREF_UI_APP_GLOBALSTYLE).value<MuseScoreStyleType>();
      }

bool Preferences::isThemeDark() const
      {
      return globalStyle() == MuseScoreStyleType::DARK_FUSION;
      }

void Preferences::revertToDefaultValue(const QString key)
      {
      set(key, defaultValue(key));
      }


void Preferences::setPreference(const QString key, QVariant value)
      {
      checkIfKeyExists(key);
      set(key, value, false);
      }

void Preferences::setTemporaryPreference(const QString key, QVariant value)
      {
      // note: this function should not call checkIfKeyExists() because it may be
      // called before init() which is ok since the preference is only stored "in memory"
      set(key, value, true);
      }

MidiRemote Preferences::midiRemote(int recordId) const
      {
      MidiRemote remote;
      QString baseKey = QString(PREF_IO_MIDI_REMOTE) + QString("%1%2%3").arg("/").arg(recordId).arg("/");

      if (has(baseKey + "type")) {
            remote.type = MidiRemoteType(get(baseKey + "type").toInt());
            remote.data = get(baseKey + "data").toInt();
            }
      else {
            remote.type = MIDI_REMOTE_TYPE_INACTIVE;
            }

      return remote;
      }

void Preferences::updateMidiRemote(int recordId, MidiRemoteType type, int data)
      {
      QString baseKey = QString(PREF_IO_MIDI_REMOTE) + QString("%1%2%3").arg("/").arg(recordId).arg("/");
      set(baseKey + "type", static_cast<int>(type));
      set(baseKey + "data", data);
      }

void Preferences::clearMidiRemote(int recordId)
      {
      QString baseKey = QString(PREF_IO_MIDI_REMOTE) + QString("%1%2").arg("/").arg(recordId);
      remove(baseKey);
      }


} // namespace Ms
