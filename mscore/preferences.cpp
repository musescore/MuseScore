//=============================================================================
//  MuseScore
//  Linux Music Score Editor
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

#ifdef Q_OS_MAC
#include "macos/cocoabridge.h"
#endif

namespace Ms {
//! Note: see musescore.h getSharePath(). This copy is needed for build without MuseScore
QString sharePath()
      {
#ifdef Q_OS_WIN
      QDir dir(QCoreApplication::applicationDirPath() + QString("/../" INSTALL_NAME));
      return dir.absolutePath() + "/";
#else
#ifdef Q_OS_MAC
      QDir dir(QCoreApplication::applicationDirPath() + QString("/../Resources"));
      return dir.absolutePath() + "/";
#else
      // Try relative path (needed for portable AppImage and non-standard installations)
      QDir dir(QCoreApplication::applicationDirPath() + QString("/../share/" INSTALL_NAME));
      if (dir.exists())
            return dir.absolutePath() + "/";
      // Otherwise fall back to default location (e.g. if binary has moved relative to share)
      return QString( INSTPREFIX "/share/" INSTALL_NAME);
#endif
#endif
      }

Preferences preferences;


void Preferences::init(bool storeInMemoryOnly)
      {
      if (!storeInMemoryOnly) {
            if (_settings)
                  delete _settings;
            _settings = new QSettings();
            }

#if defined(Q_OS_WIN)
      // Use the correct default GUI font on Windows. Qt 5 and below use the deprecated function GetStockObject() with
      // DEFAULT_GUI_FONT, which returns MS Shell Dlg 2 in 8 pt. MS Shell Dlg 2 is a virtual font that maps to Tahoma, which has
      // not been the default Windows GUI font since 2006.
      //
      // The correct way to determine the default GUI font is to call SystemParametersInfoW() with SPI_GETNONCLIENTMETRICS and use
      // the returned lfMessageFont structure. On all versions of Windows from Windows Vista through Windows 10, this typically
      // returns Segoe UI in 9 pt.
      //
      // This problem is slated to be fixed in Qt 6. For details, see: https://bugreports.qt.io/browse/QTBUG-58610
      //
      // In the meantime, we can work around the problem by having Qt use the "QMessageBox" font instead, which is already being
      // initialized the correct way.
      QApplication::setFont(QApplication::font("QMessageBox"));

      // If the current settings file is using the deprecated default GUI font, reset it so that the correct default font can be
      // picked up automatically.
      if (_settings && _settings->value(PREF_UI_THEME_FONTFAMILY) == "MS Shell Dlg 2") {
            _settings->remove(PREF_UI_THEME_FONTFAMILY);
            _settings->remove(PREF_UI_THEME_FONTSIZE);
            }
#endif

      _storeInMemoryOnly = storeInMemoryOnly;

#if defined(Q_OS_MAC) || (defined(Q_OS_WIN) && !defined(FOR_WINSTORE))
      bool checkUpdateStartup = true;
      bool checkExtensionsUpdateStartup = true;
#else
      bool checkUpdateStartup = false;
      bool checkExtensionsUpdateStartup = false;
#endif
#if defined(WIN_PORTABLE)
      checkUpdateStartup = false;
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

#if defined(Q_OS_MAC) && !defined(TESTROOT)
      const MuseScorePreferredStyleType defaultAppGlobalStyle = CocoaBridge::isSystemDarkModeSupported() ? MuseScorePreferredStyleType::FOLLOW_SYSTEM : MuseScorePreferredStyleType::LIGHT_FUSION;
#else
      const MuseScorePreferredStyleType defaultAppGlobalStyle = MuseScorePreferredStyleType::LIGHT_FUSION;
#endif

#if defined(WIN_PORTABLE)
      QString wd = QString(QDir::cleanPath(QString("%1/../../../Data/%2").arg(QCoreApplication::applicationDirPath(), QCoreApplication::applicationName())));
#else
      QString wd = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), QCoreApplication::applicationName());
#endif

      _allPreferences = prefs_map_t(
      {
            {PREF_APP_AUTOSAVE_AUTOSAVETIME,                       new IntPreference(2 /* minutes */, false)},
            {PREF_APP_AUTOSAVE_USEAUTOSAVE,                        new BoolPreference(true, false)},
            {PREF_APP_KEYBOARDLAYOUT,                              new StringPreference("US - International")},
            {PREF_APP_PATHS_INSTRUMENTLIST1,                       new StringPreference(":/data/instruments.xml", false)},
            {PREF_APP_PATHS_INSTRUMENTLIST2,                       new StringPreference("", false)},
            {PREF_APP_PATHS_SCOREORDERLIST1,                       new StringPreference(":/data/orders.xml", false)},
            {PREF_APP_PATHS_SCOREORDERLIST2,                       new StringPreference("", false)},
            {PREF_APP_PATHS_MYIMAGES,                              new StringPreference(QFileInfo(QString("%1/%2").arg(wd, QCoreApplication::translate("images_directory", "Images"))).absoluteFilePath(), false)},
            {PREF_APP_PATHS_MYPLUGINS,                             new StringPreference(QFileInfo(QString("%1/%2").arg(wd, QCoreApplication::translate("plugins_directory", "Plugins"))).absoluteFilePath(), false)},
            {PREF_APP_PATHS_MYSCOREFONTS,                          new StringPreference(QFileInfo(QString("%1/%2").arg(wd, QCoreApplication::translate("scorefonts_directory", "Score Fonts"))).absoluteFilePath(), false)},
            {PREF_APP_PATHS_MYSCORES,                              new StringPreference(QFileInfo(QString("%1/%2").arg(wd, QCoreApplication::translate("scores_directory", "Scores"))).absoluteFilePath(), false)},
            {PREF_APP_PATHS_MYSOUNDFONTS,                          new StringPreference(QFileInfo(QString("%1/%2").arg(wd, QCoreApplication::translate("soundfonts_directory", "SoundFonts"))).absoluteFilePath(), false)},
            {PREF_APP_PATHS_MYSHORTCUTS,                           new StringPreference(QFileInfo(QString("%1/%2").arg(wd, QCoreApplication::translate("shortcuts_directory", "Shortcuts"))).absoluteFilePath(), false)},
            {PREF_APP_PATHS_MYSTYLES,                              new StringPreference(QFileInfo(QString("%1/%2").arg(wd, QCoreApplication::translate("styles_directory", "Styles"))).absoluteFilePath(), false)},
            {PREF_APP_PATHS_MYTEMPLATES,                           new StringPreference(QFileInfo(QString("%1/%2").arg(wd, QCoreApplication::translate("templates_directory", "Templates"))).absoluteFilePath(), false)},
            {PREF_APP_PATHS_MYEXTENSIONS,                          new StringPreference(QFileInfo(QString("%1/%2").arg(wd, QCoreApplication::translate("extensions_directory", "Extensions"))).absoluteFilePath(), false)},
            {PREF_APP_PLAYBACK_FOLLOWSONG,                         new BoolPreference(true)},
            {PREF_APP_PLAYBACK_PANPLAYBACK,                        new BoolPreference(true, false)},
            {PREF_APP_PLAYBACK_PLAYREPEATS,                        new BoolPreference(true, false)},
            {PREF_APP_PLAYBACK_SPEEDINCREMENT,                     new IntPreference(5)},
            {PREF_APP_PLAYBACK_LOOPTOSELECTIONONPLAY,              new BoolPreference(true)},
            {PREF_APP_USESINGLEPALETTE,                            new BoolPreference(false, false)},
            {PREF_APP_PALETTESCALE,                                new DoublePreference(1.0)},
            {PREF_APP_STARTUP_FIRSTSTART,                          new BoolPreference(true)},
            {PREF_APP_STARTUP_SESSIONSTART,                        new EnumPreference(QVariant::fromValue(SessionStart::SCORE), false)},
            {PREF_APP_STARTUP_STARTSCORE,                          new StringPreference(":/data/My_First_Score.mscx", false)},
            {PREF_APP_WORKSPACE,                                   new StringPreference("Basic", false)},
            {PREF_APP_TELEMETRY_ALLOWED,                           new BoolPreference(false, false)},
            {PREF_APP_STARTUP_TELEMETRY_ACCESS_REQUESTED,          new StringPreference("", false)},
            {PREF_MIGRATION_DO_NOT_ASK_ME_AGAIN,                   new BoolPreference(false, false)},
            {PREF_MIGRATION_APPLY_LELAND_STYLE,                    new BoolPreference(false, false)},
            {PREF_MIGRATION_APPLY_EDWIN_STYLE,                     new BoolPreference(false, false)},
            {PREF_MIGRATION_RESET_ELEMENT_POSITIONS,               new BoolPreference(false, false)},
            {PREF_MIGRATION_APPLY_EDWIN_FOR_XML_FILES,             new BoolPreference(true, false)}, // TODO: this default does not appear to be honored
            {PREF_MIGRATION_DO_NOT_ASK_ME_AGAIN_XML,               new BoolPreference(false, false)},
            {PREF_APP_BACKUP_GENERATE_BACKUP,                      new BoolPreference(true)},
            {PREF_APP_BACKUP_SUBFOLDER,                            new StringPreference(".mscbackup")},
            {PREF_EXPORT_AUDIO_NORMALIZE,                          new BoolPreference(true, false)},
            {PREF_EXPORT_AUDIO_SAMPLERATE,                         new IntPreference(44100, false)},
            {PREF_EXPORT_AUDIO_PCMRATE,                            new IntPreference(16)},
            {PREF_EXPORT_MP3_BITRATE,                              new IntPreference(128, false)},
            {PREF_EXPORT_MUSICXML_EXPORTBREAKS,                    new EnumPreference(QVariant::fromValue(MusicxmlExportBreaks::ALL), false)},
            {PREF_EXPORT_MUSICXML_EXPORTLAYOUT,                    new BoolPreference(true, false)},
            {PREF_EXPORT_PDF_DPI,                                  new IntPreference(DPI, false)},
            {PREF_EXPORT_PNG_RESOLUTION,                           new DoublePreference(DPI, false)},
            {PREF_EXPORT_PNG_USETRANSPARENCY,                      new BoolPreference(true, false)},
            {PREF_EXPORT_BG_STYLE,                                 new IntPreference(0, false)},
            {PREF_EXPORT_BG_CUSTOM_COLOR,                          new ColorPreference(QColor(0xffffff), false)},
            {PREF_IMPORT_GUITARPRO_CHARSET,                        new StringPreference("UTF-8", false)},
            {PREF_IMPORT_MUSICXML_IMPORTBREAKS,                    new BoolPreference(true, false)},
            {PREF_IMPORT_MUSICXML_IMPORTLAYOUT,                    new BoolPreference(true, false)},
            {PREF_IMPORT_MUSICXML_REMOVEINSTRUMENTNAMES,           new BoolPreference(true, false)}, // TODO: add to preferences menu
            {PREF_IMPORT_OVERTURE_CHARSET,                         new StringPreference("GBK", false)},
            {PREF_IMPORT_STYLE_STYLEFILE,                          new StringPreference("", false)},
     #ifdef USE_ALSA
            {PREF_IO_ALSA_DEVICE,                                  new StringPreference("default", false)},
            {PREF_IO_ALSA_FRAGMENTS,                               new IntPreference(3, false)},
            {PREF_IO_ALSA_PERIODSIZE,                              new IntPreference(1024, false)},
            {PREF_IO_ALSA_SAMPLERATE,                              new IntPreference(48000, false)},
     #endif
            {PREF_IO_ALSA_USEALSAAUDIO,                            new BoolPreference(defaultUseAlsaAudio, false)},
            {PREF_IO_JACK_REMEMBERLASTCONNECTIONS,                 new BoolPreference(true, false)},
            {PREF_IO_JACK_TIMEBASEMASTER,                          new BoolPreference(false, false)},
            {PREF_IO_JACK_USEJACKAUDIO,                            new BoolPreference(defaultUseJackAudio, false)},
            {PREF_IO_JACK_USEJACKMIDI,                             new BoolPreference(false, false)},
            {PREF_IO_JACK_USEJACKTRANSPORT,                        new BoolPreference(false, false)},
            {PREF_IO_MIDI_ADVANCEONRELEASE,                        new BoolPreference(true, false)},
            {PREF_IO_MIDI_ENABLEINPUT,                             new BoolPreference(true, false)},
            {PREF_IO_MIDI_EXPANDREPEATS,                           new BoolPreference(true, false)},
            {PREF_IO_MIDI_EXPORTRPNS,                              new BoolPreference(true, false)},
            {PREF_IO_MIDI_PEDAL_EVENTS_MIN_TICKS,                  new IntPreference(1)},
            {PREF_IO_MIDI_REALTIMEDELAY,                           new IntPreference(750 /* ms */, false)},
            {PREF_IO_MIDI_SHORTESTNOTE,                            new IntPreference(MScore::division/4, false)},
            {PREF_IO_MIDI_SHOWCONTROLSINMIXER,                     new BoolPreference(true, false)},
            {PREF_IO_MIDI_USEREMOTECONTROL,                        new BoolPreference(false, false)},
            {PREF_IO_OSC_PORTNUMBER,                               new IntPreference(5282, false)},
            {PREF_IO_OSC_USEREMOTECONTROL,                         new BoolPreference(false, false)},
      #ifdef USE_PORTAUDIO
            {PREF_IO_PORTAUDIO_DEVICE,                             new IntPreference(-1, false)},
            {PREF_IO_PORTAUDIO_USEPORTAUDIO,                       new BoolPreference(defaultUsePortAudio, false)},
      #endif
      #ifdef USE_PORTMIDI
            {PREF_IO_PORTMIDI_INPUTBUFFERCOUNT,                    new IntPreference(100)},
            {PREF_IO_PORTMIDI_INPUTDEVICE,                         new StringPreference("")},
            {PREF_IO_PORTMIDI_OUTPUTBUFFERCOUNT,                   new IntPreference(65536)},
            {PREF_IO_PORTMIDI_OUTPUTDEVICE,                        new StringPreference("")},
            {PREF_IO_PORTMIDI_OUTPUTLATENCYMILLISECONDS,           new IntPreference(0)},
      #endif
            {PREF_IO_PULSEAUDIO_USEPULSEAUDIO,                     new BoolPreference(defaultUsePulseAudio, false)},
            {PREF_SCORE_CHORD_PLAYONADDNOTE,                       new BoolPreference(true, false)},
            {PREF_SCORE_HARMONY_PLAY_ONEDIT,                       new BoolPreference(true, false)},
            {PREF_SCORE_HARMONY_PLAY_DISABLE_COMPATIBILITY,        new BoolPreference(true, true)},
            {PREF_SCORE_HARMONY_PLAY_DISABLE_NEW,                  new BoolPreference(false, true)},
            {PREF_SCORE_NOTE_PLAYONCLICK,                          new BoolPreference(true, false)},
            {PREF_SCORE_NOTE_DEFAULTPLAYDURATION,                  new IntPreference(300 /* ms */, false)},
            {PREF_SCORE_NOTE_WARNPITCHRANGE,                       new BoolPreference(true, false)},
            {PREF_SCORE_NOTE_INPUT_DISABLE_MOUSE_INPUT,            new BoolPreference(false, true)},
            {PREF_SCORE_STYLE_DEFAULTSTYLEFILE,                    new StringPreference("", false)},
            {PREF_SCORE_STYLE_PARTSTYLEFILE,                       new StringPreference("", false)},
            {PREF_UI_CANVAS_BG_USECOLOR,                           new BoolPreference(true, false)},
            {PREF_UI_CANVAS_FG_USECOLOR,                           new BoolPreference(true, false)},
            {PREF_UI_CANVAS_FG_USECOLOR_IN_PALETTES,               new BoolPreference(false, false)},
            {PREF_UI_CANVAS_BG_COLOR,                              new ColorPreference(QColor(0x385F94), false)},
            {PREF_UI_CANVAS_FG_COLOR,                              new ColorPreference(QColor(0xf9f9f9), false)},
            {PREF_UI_CANVAS_BG_WALLPAPER,                          new StringPreference(QFileInfo(QString("%1%2").arg(sharePath(), "wallpaper/background1.png")).absoluteFilePath(), false)},
            {PREF_UI_CANVAS_FG_WALLPAPER,                          new StringPreference(QFileInfo(QString("%1%2").arg(sharePath(), "wallpaper/paper5.png")).absoluteFilePath(), false)},
            {PREF_UI_CANVAS_ZOOM_DEFAULT_TYPE,                     new IntPreference(0, false)},
            {PREF_UI_CANVAS_ZOOM_DEFAULT_LEVEL,                    new IntPreference(100, false)},
            {PREF_UI_CANVAS_ZOOM_PRECISION_KEYBOARD,               new IntPreference(2, false)},
            {PREF_UI_CANVAS_ZOOM_PRECISION_MOUSE,                  new IntPreference(6, false)},
            {PREF_UI_CANVAS_MISC_ANTIALIASEDDRAWING,               new BoolPreference(true, false)},
            {PREF_UI_CANVAS_MISC_SELECTIONPROXIMITY,               new IntPreference(6, false)},
            {PREF_UI_CANVAS_SCROLL_LIMITSCROLLAREA,                new BoolPreference(false, false)},
            {PREF_UI_CANVAS_SCROLL_VERTICALORIENTATION,            new BoolPreference(false, false)},
            {PREF_UI_APP_STARTUP_CHECKUPDATE,                      new BoolPreference(checkUpdateStartup, false)},
            {PREF_UI_APP_STARTUP_CHECK_EXTENSIONS_UPDATE,          new BoolPreference(checkExtensionsUpdateStartup, false)},
            {PREF_UI_APP_STARTUP_SHOWNAVIGATOR,                    new BoolPreference(false, false)},
            {PREF_UI_APP_STARTUP_SHOWPLAYPANEL,                    new BoolPreference(false, false)},
            {PREF_UI_APP_STARTUP_SHOWSPLASHSCREEN,                 new BoolPreference(true, false)},
            {PREF_UI_APP_STARTUP_SHOWSTARTCENTER,                  new BoolPreference(true, false)},
            {PREF_UI_APP_STARTUP_SHOWTOURS,                        new BoolPreference(true, false)},
            {PREF_UI_APP_GLOBALSTYLE,                              new EnumPreference(QVariant::fromValue(defaultAppGlobalStyle), false)},
            {PREF_UI_APP_LANGUAGE,                                 new StringPreference("system", false)},
            {PREF_UI_APP_RASTER_HORIZONTAL,                        new IntPreference(2)},
            {PREF_UI_APP_RASTER_VERTICAL,                          new IntPreference(2)},
            {PREF_UI_APP_SHOWSTATUSBAR,                            new BoolPreference(true)},
#if defined(Q_OS_MAC) || defined(Q_OS_WIN) // use system native file dialog, Qt file dialog is very slow on Windows and Mac
            {PREF_UI_APP_USENATIVEDIALOGS,                         new BoolPreference(true)},
#else // don't use system native file dialog, this is causing issues on some Linuxes
            {PREF_UI_APP_USENATIVEDIALOGS,                         new BoolPreference(false)},
#endif
            {PREF_UI_PIANO_HIGHLIGHTCOLOR,                         new ColorPreference(QColor(0x0065BF))},
            {PREF_UI_PIANO_SHOWPITCHHELP,                          new BoolPreference(true)},
            {PREF_UI_SCORE_NOTE_DROPCOLOR,                         new ColorPreference(QColor(0x0065BF))},
            {PREF_UI_SCORE_DEFAULTCOLOR,                           new ColorPreference(QColor(Qt::black))}, //"#000000"
            {PREF_UI_SCORE_FRAMEMARGINCOLOR,                       new ColorPreference(QColor(0xA0A0A4))},
            {PREF_UI_SCORE_LAYOUTBREAKCOLOR,                       new ColorPreference(QColor(0xA0A0A4))},
            {PREF_UI_SCORE_VOICE1_COLOR,                           new ColorPreference(QColor(0x0065BF))},
            {PREF_UI_SCORE_VOICE2_COLOR,                           new ColorPreference(QColor(0x007F00))},
            {PREF_UI_SCORE_VOICE3_COLOR,                           new ColorPreference(QColor(0xC53F00))},
            {PREF_UI_SCORE_VOICE4_COLOR,                           new ColorPreference(QColor(0xC31989))},
            {PREF_UI_SCORE_CURSOR_COLOR,                           new ColorPreference(QColor(0x0065BF))},
            {PREF_UI_THEME_ICONWIDTH,                              new IntPreference(28, false)},
            {PREF_UI_THEME_ICONHEIGHT,                             new IntPreference(24, false)},
            {PREF_UI_THEME_FONTFAMILY,                             new StringPreference(QApplication::font().family(), false) },
            {PREF_UI_THEME_FONTSIZE,                               new DoublePreference(QApplication::font().pointSizeF(), false) },
            {PREF_UI_PIANOROLL_DARK_SELECTION_BOX_COLOR,           new ColorPreference(QColor(0x0cebff))},
            {PREF_UI_PIANOROLL_DARK_NOTE_UNSEL_COLOR,              new ColorPreference(QColor(0x1dcca0))},
            {PREF_UI_PIANOROLL_DARK_NOTE_SEL_COLOR,                new ColorPreference(QColor(0xffff00))},
            {PREF_UI_PIANOROLL_DARK_NOTE_DRAG_COLOR,               new ColorPreference(QColor(0xffbb33))},
            {PREF_UI_PIANOROLL_DARK_BG_BASE_COLOR,                 new ColorPreference(QColor(0x3a3a3a))},
            {PREF_UI_PIANOROLL_DARK_BG_KEY_HIGHLIGHT_COLOR,        new ColorPreference(QColor(0x555577))},
            {PREF_UI_PIANOROLL_DARK_BG_KEY_WHITE_COLOR,            new ColorPreference(QColor(0x3a3a3a))},
            {PREF_UI_PIANOROLL_DARK_BG_KEY_BLACK_COLOR,            new ColorPreference(QColor(0x262626))},
            {PREF_UI_PIANOROLL_DARK_BG_GRIDLINE_COLOR,             new ColorPreference(QColor(0x111111))},
            {PREF_UI_PIANOROLL_DARK_BG_TEXT_COLOR,                 new ColorPreference(QColor(0x999999))},
            {PREF_UI_PIANOROLL_DARK_BG_TIE_COLOR,                  new ColorPreference(QColor(0xff0000))},
            {PREF_UI_PIANOROLL_LIGHT_SELECTION_BOX_COLOR,          new ColorPreference(QColor(0x2085c3))},
            {PREF_UI_PIANOROLL_LIGHT_NOTE_UNSEL_COLOR,             new ColorPreference(QColor(0x1dcca0))},
            {PREF_UI_PIANOROLL_LIGHT_NOTE_SEL_COLOR,               new ColorPreference(QColor(0xffff00))},
            {PREF_UI_PIANOROLL_LIGHT_NOTE_DRAG_COLOR,              new ColorPreference(QColor(0xffbb33))},
            {PREF_UI_PIANOROLL_LIGHT_BG_BASE_COLOR,                new ColorPreference(QColor(0xe0e0e7))},
            {PREF_UI_PIANOROLL_LIGHT_BG_KEY_HIGHLIGHT_COLOR,       new ColorPreference(QColor(0xaaaaff))},
            {PREF_UI_PIANOROLL_LIGHT_BG_KEY_WHITE_COLOR,           new ColorPreference(QColor(0xffffff))},
            {PREF_UI_PIANOROLL_LIGHT_BG_KEY_BLACK_COLOR,           new ColorPreference(QColor(0xe6e6e6))},
            {PREF_UI_PIANOROLL_LIGHT_BG_GRIDLINE_COLOR,            new ColorPreference(QColor(0xa2a2a6))},
            {PREF_UI_PIANOROLL_LIGHT_BG_TEXT_COLOR,                new ColorPreference(QColor(0x111111))},
            {PREF_UI_PIANOROLL_LIGHT_BG_TIE_COLOR,                 new ColorPreference(QColor(0xff0000))},
            {PREF_UI_BUTTON_HIGHLIGHT_COLOR_DISABLED_DARK_ON,      new ColorPreference(QColor(0x7F7F7F))},
            {PREF_UI_BUTTON_HIGHLIGHT_COLOR_DISABLED_DARK_OFF,     new ColorPreference(QColor(0xa0a0a0))},
            {PREF_UI_BUTTON_HIGHLIGHT_COLOR_DISABLED_LIGHT_ON,     new ColorPreference(QColor(0x7F7F7F))},
            {PREF_UI_BUTTON_HIGHLIGHT_COLOR_DISABLED_LIGHT_OFF,    new ColorPreference(QColor(0xa0a0a0))},
            {PREF_UI_BUTTON_HIGHLIGHT_COLOR_ENABLED_DARK_ON,       new ColorPreference(QColor(0x36B2FF))},
            {PREF_UI_BUTTON_HIGHLIGHT_COLOR_ENABLED_DARK_OFF,      new ColorPreference(QColor(0xeff0f1))},
            {PREF_UI_BUTTON_HIGHLIGHT_COLOR_ENABLED_LIGHT_ON,      new ColorPreference(QColor(0x0065BF))},
            {PREF_UI_BUTTON_HIGHLIGHT_COLOR_ENABLED_LIGHT_OFF,     new ColorPreference(QColor(0x3b3f45))},
            {PREF_UI_INSPECTOR_STYLED_TEXT_COLOR_LIGHT,            new ColorPreference(QColor(0x0066BF))},
            {PREF_UI_INSPECTOR_STYLED_TEXT_COLOR_DARK,             new ColorPreference(QColor(0x36B2FF))},
            {PREF_PAN_MODIFIER_BASE,                               new DoublePreference(1, true)},
            {PREF_PAN_MODIFIER_STEP,                               new DoublePreference(0.01, true)},
            {PREF_PAN_MODIFIER_MIN,                                new DoublePreference(0.2, true)},
            {PREF_PAN_MODIFIER_MAX,                                new DoublePreference(5, true)},
            {PREF_PAN_CURSOR_VISIBLE,                              new BoolPreference(false, true)},
            {PREF_PAN_CURSOR_POS,                                  new DoublePreference(0.3, true)},
            {PREF_PAN_SMOOTHLY_ENABLED,                            new BoolPreference(false, true)},
            {PREF_PAN_TELEPORT_LEFT,                               new BoolPreference(true, true) },
            {PREF_PAN_TELEPORT_RIGHT,                              new BoolPreference(false, true) },
//            {PREF_PAN_DISTANCE_LEFT,                               new DoublePreference(-250, false)},
//            {PREF_PAN_DISTANCE_LEFT1,                              new DoublePreference(-125, false)},
//            {PREF_PAN_DISTANCE_LEFT2,                              new DoublePreference(-50, false)},
//            {PREF_PAN_DISTANCE_LEFT3,                              new DoublePreference(-25, false)},
//            {PREF_PAN_DISTANCE_RIGHT,                              new DoublePreference(500, false)},
//            {PREF_PAN_DISTANCE_RIGHT1,                             new DoublePreference(250, false)},
//            {PREF_PAN_DISTANCE_RIGHT2,                             new DoublePreference(125, false)},
//            {PREF_PAN_DISTANCE_RIGHT3,                             new DoublePreference(50, false)},
//            {PREF_PAN_MODIFIER_LEFT1,                              new DoublePreference(0.8, false)},
//            {PREF_PAN_MODIFIER_LEFT2,                              new DoublePreference(0.9, false)},
//            {PREF_PAN_MODIFIER_LEFT3,                              new DoublePreference(0.95, false)},
//            {PREF_PAN_MODIFIER_RIGHT1,                             new DoublePreference(1.2, false)},
//            {PREF_PAN_MODIFIER_RIGHT2,                             new DoublePreference(1.1, false)},
//            {PREF_PAN_MODIFIER_RIGHT3,                             new DoublePreference(1.05, false)},
//            {PREF_PAN_WEIGHT_NORMAL,                               new DoublePreference(1, false)},
//            {PREF_PAN_WEIGHT_SMART,                                new DoublePreference(0, false)},
//            {PREF_PAN_WEIGHT_ADVANCED,                             new BoolPreference(false, false)},
//            {PREF_PAN_SMART_TIMER_DURATION,                        new IntPreference(1000, false)},
#ifdef AVSOMR
            {PREF_IMPORT_AVSOMR_USELOCAL,                          new BoolPreference(false, false)},
            {PREF_UI_AVSOMR_RECOGNITION_COLOR,                     new ColorPreference(QColor("#1DCCA0"))},
            {PREF_UI_AVSOMR_NOT_RECOGNITION_COLOR,                 new ColorPreference(QColor("#C31989"))},
#endif
      });

      _initialized = true;
      updateLocalPreferences();
      }

void Preferences::save()
      {
      settings()->sync();
      }

QVariant Preferences::defaultValue(const QString key) const
      {
      checkIfKeyExists(key);
      Preference* pref = _allPreferences.value(key);
      return pref ? pref->defaultValue() : QVariant();
      }

QSettings* Preferences::settings() const
      {
      if (!_initialized) {
            qDebug("Preferences are not initialized. Call init() to initialize.");
            Q_ASSERT(_initialized);
            }

      return _settings;
      }

QVariant Preferences::get(const QString key) const
      {
      QVariant pref = _inMemorySettings.value(key);

      if (_storeInMemoryOnly)
            return (_inMemorySettings.contains(key)) ? pref : QVariant(); // invalid QVariant returned when not found
      else if (_inMemorySettings.contains(key)) // if there exists a temporary value stored "in memory" return this value
            return pref;
      else if (useLocalPrefs && localPreferences.contains(key))
            return localPreferences.value(key);
      else
            return settings()->value(key);
      }

void Preferences::set(const QString key, QVariant value, bool temporary)
      {
      if (_storeInMemoryOnly || temporary)
            _inMemorySettings[key] = value;
      else if (useLocalPrefs && localPreferences.contains(key))
            localPreferences[key] = value;
      else
            settings()->setValue(key, value);
      }

void Preferences::remove(const QString key)
      {
      // remove both preference stored "in memory" and in QSettings
      _inMemorySettings.remove(key);
      settings()->remove(key);
      }

bool Preferences::has(const QString key) const
      {
      return _inMemorySettings.contains(key) || settings()->contains(key);
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

bool Preferences::checkIfKeyExists(const QString key) const
      {
      bool exists = _allPreferences.contains(key);
      if (!exists) {
            qDebug("Preference not found: %s", key.toUtf8().constData());
            Q_ASSERT(exists);
            }
      return exists;
      }

QMetaType::Type Preferences::type(const QString key) const
      {
      if (_allPreferences.contains(key))
            return _allPreferences.value(key)->type();
      else {
            return QMetaType::UnknownType;
            }
      }

bool Preferences::checkType(const QString key, QMetaType::Type t) const
      {
      if (type(key) != t) {
            qDebug("Preference is not of correct type: %s", key.toUtf8().constData());
            Q_ASSERT(type(key) == QMetaType::Bool);
            }
      return type(key) == t;
      }

Preferences::Preferences()
      : _settings(0)
      {}

Preferences::~Preferences()
      {
      // clean up _allPreferences
      for (Preference* pref : _allPreferences.values())
            delete pref;

      if (_settings) {
            delete _settings;
            }
      }

bool Preferences::getBool(const QString key) const
      {
      checkType(key, QMetaType::Bool);
      return preference(key).toBool();
      }

QColor Preferences::getColor(const QString key) const
      {
      checkType(key, QMetaType::QColor);
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
      checkType(key, QMetaType::QString);
      return preference(key).toString();
      }

int Preferences::getInt(const QString key) const
      {
      checkType(key, QMetaType::Int);
      QVariant v = preference(key);
      bool ok;
      int pref = v.toInt(&ok);
      if (!ok) {
            qDebug("Can not convert preference %s to int. Returning default value.", key.toUtf8().constData());
            return defaultValue(key).toInt();
            }
      return pref;
}

double Preferences::getDouble(const QString key) const
      {
      checkType(key, QMetaType::Double);
      QVariant v = preference(key);
      bool ok;
      double pref = v.toDouble(&ok);
      if (!ok) {
            qDebug("Can not convert preference %s to double. Returning default value.", key.toUtf8().constData());
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

// The "theme" the user chooses in Preferences
MuseScorePreferredStyleType Preferences::preferredGlobalStyle() const
      {
      return preference(PREF_UI_APP_GLOBALSTYLE).value<MuseScorePreferredStyleType>();
      }

// The actual "theme", resulting from the user's choice
MuseScoreEffectiveStyleType Preferences::effectiveGlobalStyle() const
      {
      MuseScorePreferredStyleType s = preferredGlobalStyle();
#ifdef Q_OS_MAC // On Mac, follow system theme if desired
      if (s == MuseScorePreferredStyleType::FOLLOW_SYSTEM)
            return CocoaBridge::isSystemDarkTheme() ? MuseScoreEffectiveStyleType::DARK_FUSION : MuseScoreEffectiveStyleType::LIGHT_FUSION;
#endif
      return MuseScoreEffectiveStyleType(s);
      }

bool Preferences::isThemeDark() const
      {
      return effectiveGlobalStyle() == MuseScoreEffectiveStyleType::DARK_FUSION;
      }

void Preferences::setToDefaultValue(const QString key)
      {
      set(key, defaultValue(key));
      }


void Preferences::setPreference(const QString key, QVariant value)
      {
      checkIfKeyExists(key);
      set(key, value);
      for (const OnSetListener& l : qAsConst(_onSetListeners))
          l(key, value);
      }

Preferences::ListenerID Preferences::addOnSetListener(const OnSetListener& l)
      {
      static ListenerID lastId{0};
      ++lastId;
      _onSetListeners[lastId] = l;
      return lastId;
      }

void Preferences::removeOnSetListener(const ListenerID& id)
      {
      _onSetListeners.remove(id);
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

QMap<QString, QVariant> Preferences::getDefaultLocalPreferences() {
      bool tmp = useLocalPrefs;
      useLocalPrefs = false;
      QMap<QString, QVariant> defaultLocalPreferences;
      for (const char* const& s : {PREF_UI_CANVAS_BG_USECOLOR,
                        PREF_UI_CANVAS_FG_USECOLOR,
                        PREF_UI_CANVAS_FG_USECOLOR_IN_PALETTES,
                        PREF_UI_CANVAS_BG_COLOR,
                        PREF_UI_CANVAS_FG_COLOR,
                        PREF_UI_CANVAS_BG_WALLPAPER,
                        PREF_UI_CANVAS_FG_WALLPAPER,
                        PREF_UI_CANVAS_ZOOM_DEFAULT_TYPE,
                        PREF_UI_CANVAS_ZOOM_DEFAULT_LEVEL,
                        PREF_UI_CANVAS_ZOOM_PRECISION_KEYBOARD,
                        PREF_UI_CANVAS_ZOOM_PRECISION_MOUSE,
                        PREF_UI_CANVAS_MISC_ANTIALIASEDDRAWING,
                        PREF_UI_CANVAS_MISC_SELECTIONPROXIMITY,
                        PREF_UI_CANVAS_SCROLL_LIMITSCROLLAREA,
                        PREF_UI_CANVAS_SCROLL_VERTICALORIENTATION,
                        PREF_UI_APP_SHOWSTATUSBAR,
                        PREF_UI_APP_USENATIVEDIALOGS,
                        PREF_UI_PIANO_HIGHLIGHTCOLOR,
                        PREF_UI_PIANO_SHOWPITCHHELP,
                        PREF_UI_SCORE_NOTE_DROPCOLOR,
                        PREF_UI_SCORE_DEFAULTCOLOR,
                        PREF_UI_SCORE_FRAMEMARGINCOLOR,
                        PREF_UI_SCORE_LAYOUTBREAKCOLOR,
                        PREF_UI_SCORE_VOICE1_COLOR,
                        PREF_UI_SCORE_VOICE2_COLOR,
                        PREF_UI_SCORE_VOICE3_COLOR,
                        PREF_UI_SCORE_VOICE4_COLOR,
                        PREF_UI_SCORE_CURSOR_COLOR,
                        PREF_UI_THEME_ICONWIDTH,
                        PREF_UI_THEME_ICONHEIGHT,
                        PREF_UI_THEME_FONTFAMILY,
                        PREF_UI_THEME_FONTSIZE}) {
            QVariant value = get(s);
            if (!value.isValid())
                  value = _allPreferences.value(s)->defaultValue();
            defaultLocalPreferences.insert(s, value);
            }
      useLocalPrefs = tmp;
      return defaultLocalPreferences;
      }

void Preferences::setLocalPreference(QString key, QVariant value)
      {
      if (localPreferences.contains(key))
            localPreferences[key] = value;
      }

Preference::Preference(QVariant defaultValue, QMetaType::Type type, bool showInAdvancedList)
      : _defaultValue(defaultValue),
        _showInAdvancedList(showInAdvancedList),
        _type(type)
      {}

IntPreference::IntPreference(int defaultValue, bool showInAdvancedList)
      : Preference(defaultValue, QMetaType::Int, showInAdvancedList)
      {}

void IntPreference::accept(QString key, PreferenceVisitor& v)
      {
      v.visit(key, this);
      }

DoublePreference::DoublePreference(double defaultValue, bool showInAdvancedList)
      : Preference(defaultValue, QMetaType::Double, showInAdvancedList)
      {}

void DoublePreference::accept(QString key, PreferenceVisitor& v)
      {
      v.visit(key, this);
      }

BoolPreference::BoolPreference(bool defaultValue, bool showInAdvancedList)
      : Preference(defaultValue, QMetaType::Bool, showInAdvancedList)
      {}

void BoolPreference::accept(QString key, PreferenceVisitor& v)
      {
      v.visit(key, this);
      }

StringPreference::StringPreference(QString defaultValue, bool showInAdvancedList)
      : Preference(defaultValue, QMetaType::QString, showInAdvancedList)
      {}

void StringPreference::accept(QString key, PreferenceVisitor& v)
      {
      v.visit(key, this);
      }

ColorPreference::ColorPreference(QColor defaultValue, bool showInAdvancedList)
      : Preference(defaultValue, QMetaType::QColor, showInAdvancedList)
      {}

void ColorPreference::accept(QString key, PreferenceVisitor& v)
      {
      v.visit(key, this);
      }

EnumPreference::EnumPreference(QVariant defaultValue, bool showInAdvancedList)
      : Preference(defaultValue, QMetaType::User, showInAdvancedList)
      {}

void EnumPreference::accept(QString, PreferenceVisitor&)
      {}

} // namespace Ms
