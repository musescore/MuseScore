//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: preferences.cpp 5660 2012-05-22 14:17:39Z wschweer $
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

#include "config.h"
#include "libmscore/xml.h"
#include "libmscore/score.h"
#include "musescore.h"
#include "preferences.h"
#include "prefsdialog.h"
#include "seq.h"
#include "libmscore/note.h"
#include "icons.h"
#include "shortcutcapturedialog.h"
#include "scoreview.h"
#include "libmscore/sym.h"
#include "pa.h"

#ifdef USE_PORTMIDI
#include "pm.h"
#endif

#include "libmscore/page.h"
#include "file.h"
#include "libmscore/mscore.h"
#include "shortcut.h"
#include "zerberus/zerberus.h"
#include "fluid/fluid.h"
#include "pathlistdialog.h"
#include "mstyle/mconfig.h"
#include "resourceManager.h"
#include "synthesizer/msynthesizer.h"

namespace Ms {

bool useALSA = false, useJACK = false, usePortaudio = false, usePulseAudio = false;

extern bool externalStyle;

//---------------------------------------------------------
//   Preferences
//---------------------------------------------------------

Preferences preferences;

Preferences::Preferences()
      {
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Preferences::init()
      {
      // set fallback defaults:

      bgUseColor         = true;
      fgUseColor         = true;
      bgWallpaper        = QString();
      fgWallpaper        = ":/data/paper5.png";
      fgColor.setNamedColor("#f9f9f9");
      pianoHlColor.setNamedColor("#1259d0");
      iconHeight         = 24;
      iconWidth          = 28;

      enableMidiInput    = true;
      realtimeDelay      = 750; // ms
      playNotes          = true;
      playChordOnAddNote = true;

      showNavigator      = false;
      showPlayPanel      = false;
      showSplashScreen   = true;
      showStartcenter    = true;

      showStatusBar      = true;

//      playPanelPos       = QPoint(100, 300);

      useAlsaAudio       = false;
      useJackAudio       = false;
      useJackMidi        = false;
      useJackTransport   = false;
      jackTimebaseMaster = false;
      usePortaudioAudio  = false;
      usePulseAudio      = false;
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
      usePortaudioAudio  = true;
      // Linux
#elif defined(USE_PULSEAUDIO)
      usePulseAudio  = true;
#elif defined(USE_ALSA)
      useAlsaAudio = true;
#elif defined(USE_PORTAUDIO)
      usePortaudioAudio = true;
#endif

      rememberLastConnections = true;

      alsaDevice                = "default";
      alsaSampleRate            = 48000;
      alsaPeriodSize            = 1024;
      alsaFragments             = 3;
      portaudioDevice           = -1;
      portMidiInput             = "";
      portMidiInputBufferCount  = 100;
      portMidiOutput            = "";
      portMidiOutputBufferCount = 65536;
      portMidiOutputLatencyMilliseconds = 0;

      antialiasedDrawing       = true;
      limitScrollArea          = false;
      sessionStart             = SessionStart::SCORE;
      startScore               = ":/data/My_First_Score.mscz";
      defaultStyleFile         = "";

      useMidiRemote      = false;
      for (int i = 0; i < MIDI_REMOTES; ++i)
            midiRemote[i].type = MIDI_REMOTE_TYPE_INACTIVE;
      advanceOnRelease   = true;

      midiExpandRepeats        = true;
      midiExportRPNs           = false;
      MScore::playRepeats      = true;
      MScore::panPlayback      = true;
      instrumentList1          = ":/data/instruments.xml";
      instrumentList2          = "";

      musicxmlImportLayout     = true;
      musicxmlImportBreaks     = true;
      musicxmlExportLayout     = true;
      musicxmlExportBreaks     = MusicxmlExportBreaks::ALL;

      alternateNoteEntryMethod = false;
      proximity                = 6;
      autoSave                 = true;
      autoSaveTime             = 2;       // minutes
      pngResolution            = 300.0;
      pngTransparent           = true;
      language                 = "system";

      mag                     = 1.0;

#if defined(Q_OS_MAC) || (defined(Q_OS_WIN) && !defined(FOR_WINSTORE))
      checkUpdateStartup      = true;
      checkExtensionsUpdateStartup = true;
#else
      checkUpdateStartup      = false;
      checkExtensionsUpdateStartup = false;
#endif

      followSong              = true;
      importCharsetOve        = "GBK";
      importCharsetGP         = "UTF-8";
      importStyleFile         = "";
      shortestNote            = MScore::division/4;

      useOsc                  = false;
      oscPort                 = 5282;
      singlePalette           = false;

      styleName               = "light";   // ??
      globalStyle             = MuseScoreStyleType::LIGHT;
#ifdef Q_OS_MAC
      animations              = false;
#else
      animations              = true;
#endif

      QString wd      = QString("%1/%2").arg(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)).arg(QCoreApplication::applicationName());

      myScoresPath    = QFileInfo(QString("%1/%2").arg(wd).arg(QCoreApplication::translate("scores_directory",     "Scores"))).absoluteFilePath();
      myStylesPath    = QFileInfo(QString("%1/%2").arg(wd).arg(QCoreApplication::translate("styles_directory",     "Styles"))).absoluteFilePath();
      myImagesPath    = QFileInfo(QString("%1/%2").arg(wd).arg(QCoreApplication::translate("images_directory",     "Images"))).absoluteFilePath();
      myTemplatesPath = QFileInfo(QString("%1/%2").arg(wd).arg(QCoreApplication::translate("templates_directory",  "Templates"))).absoluteFilePath();
      myPluginsPath   = QFileInfo(QString("%1/%2").arg(wd).arg(QCoreApplication::translate("plugins_directory",    "Plugins"))).absoluteFilePath();
      mySoundfontsPath = QFileInfo(QString("%1/%2").arg(wd).arg(QCoreApplication::translate("soundfonts_directory", "Soundfonts"))).absoluteFilePath();
      myExtensionsPath = QFileInfo(QString("%1/%2").arg(wd).arg(QCoreApplication::translate("extensions_directory", "Extensions"))).absoluteFilePath();

      MScore::setNudgeStep(.1);         // cursor key (default 0.1)
      MScore::setNudgeStep10(1.0);      // Ctrl + cursor key (default 1.0)
      MScore::setNudgeStep50(0.01);     // Alt  + cursor key (default 0.01)

      MScore::setHRaster(2);        // _spatium / value
      MScore::setVRaster(2);
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
      // use system native file dialogs
      // Qt file dialog is very slow on Windows and Mac
      nativeDialogs           = true;
#else
      nativeDialogs           = false;    // don't use system native file dialogs
#endif
      exportAudioSampleRate   = 44100;
      exportMp3BitRate        = 128;

      workspace               = "Basic";
      exportPdfDpi            = 300;
      };

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Preferences::write()
      {
      dirty = false;
      QSettings s;

      s.setValue("bgUseColor",         bgUseColor);
      s.setValue("fgUseColor",         fgUseColor);
      s.setValue("bgWallpaper",        bgWallpaper);
      s.setValue("fgWallpaper",        fgWallpaper);
      s.setValue("fgColor",            fgColor.name(QColor::NameFormat::HexArgb));
      s.setValue("bgColor",            MScore::bgColor.name(QColor::NameFormat::HexArgb));
      s.setValue("iconHeight",         iconHeight);
      s.setValue("iconWidth",          iconWidth);

      s.setValue("selectColor1",       MScore::selectColor[0].name(QColor::NameFormat::HexArgb));
      s.setValue("selectColor2",       MScore::selectColor[1].name(QColor::NameFormat::HexArgb));
      s.setValue("selectColor3",       MScore::selectColor[2].name(QColor::NameFormat::HexArgb));
      s.setValue("selectColor4",       MScore::selectColor[3].name(QColor::NameFormat::HexArgb));
      s.setValue("dropColor",          MScore::dropColor.name(QColor::NameFormat::HexArgb));
      s.setValue("defaultColor",       MScore::defaultColor.name(QColor::NameFormat::HexArgb));
      s.setValue("pianoHlColor",       pianoHlColor.name(QColor::NameFormat::HexArgb));
      s.setValue("enableMidiInput",    enableMidiInput);
      s.setValue("realtimeDelay",      realtimeDelay);
      s.setValue("playNotes",          playNotes);
      s.setValue("playChordOnAddNote", playChordOnAddNote);

      s.setValue("showNavigator",      showNavigator);
      s.setValue("showPlayPanel",      showPlayPanel);
      s.setValue("showSplashScreen",   showSplashScreen);
      s.setValue("showStartcenter1",    showStartcenter);

      s.setValue("showStatusBar",      showStatusBar);

      s.setValue("useAlsaAudio",       useAlsaAudio);
      s.setValue("useJackAudio",       useJackAudio);
      s.setValue("useJackMidi",        useJackMidi);
      s.setValue("useJackTransport",   useJackTransport);
      s.setValue("jackTimebaseMaster", jackTimebaseMaster);
      s.setValue("usePortaudioAudio",  usePortaudioAudio);
      s.setValue("usePulseAudio",      usePulseAudio);
      s.setValue("rememberLastMidiConnections", rememberLastConnections);

      s.setValue("alsaDevice",         alsaDevice);
      s.setValue("alsaSampleRate",     alsaSampleRate);
      s.setValue("alsaPeriodSize",     alsaPeriodSize);
      s.setValue("alsaFragments",      alsaFragments);
      s.setValue("portaudioDevice",    portaudioDevice);
      s.setValue("portMidiInput",      portMidiInput);
      s.setValue("portMidiOutput",     portMidiOutput);
      s.setValue("portMidiOutputLatencyMilliseconds", portMidiOutputLatencyMilliseconds);

      s.setValue("layoutBreakColor",   MScore::layoutBreakColor.name(QColor::NameFormat::HexArgb));
      s.setValue("frameMarginColor",   MScore::frameMarginColor.name(QColor::NameFormat::HexArgb));
      s.setValue("antialiasedDrawing", antialiasedDrawing);
      s.setValue("limitScrollArea",    limitScrollArea);
      switch(sessionStart) {
            case SessionStart::EMPTY:  s.setValue("sessionStart", "empty"); break;
            case SessionStart::LAST:   s.setValue("sessionStart", "last"); break;
            case SessionStart::NEW:    s.setValue("sessionStart", "new"); break;
            case SessionStart::SCORE:  s.setValue("sessionStart", "score"); break;
            }
      s.setValue("startScore",         startScore);
      s.setValue("defaultStyle",       defaultStyleFile);

      s.setValue("midiExpandRepeats",  midiExpandRepeats);
      s.setValue("midiExportRPNs",     midiExportRPNs);
      s.setValue("playRepeats",        MScore::playRepeats);
      s.setValue("panPlayback",        MScore::panPlayback);
      s.setValue("instrumentList",     instrumentList1);
      s.setValue("instrumentList2",    instrumentList2);

      s.setValue("musicxmlImportLayout",  musicxmlImportLayout);
      s.setValue("musicxmlImportBreaks",  musicxmlImportBreaks);
      s.setValue("musicxmlExportLayout",  musicxmlExportLayout);
      switch(musicxmlExportBreaks) {
            case MusicxmlExportBreaks::ALL:     s.setValue("musicxmlExportBreaks", "all"); break;
            case MusicxmlExportBreaks::MANUAL:  s.setValue("musicxmlExportBreaks", "manual"); break;
            case MusicxmlExportBreaks::NO:      s.setValue("musicxmlExportBreaks", "no"); break;
            }

      s.setValue("alternateNoteEntry", alternateNoteEntryMethod);
      s.setValue("proximity",          proximity);
      s.setValue("autoSave",           autoSave);
      s.setValue("autoSaveTime",       autoSaveTime);
      s.setValue("pngResolution",      pngResolution);
      s.setValue("pngTransparent",     pngTransparent);
      s.setValue("language",           language);

      s.setValue("paperWidth",  MScore::defaultStyle()->pageFormat()->width());
      s.setValue("paperHeight", MScore::defaultStyle()->pageFormat()->height());

      s.setValue("twosided",    MScore::defaultStyle()->pageFormat()->twosided());
      s.setValue("spatium",     MScore::defaultStyle()->spatium() / DPI);

      s.setValue("mag", mag);

      s.setValue("defaultPlayDuration", MScore::defaultPlayDuration);
      s.setValue("importStyleFile", importStyleFile);
      s.setValue("shortestNote", shortestNote);
      s.setValue("importCharsetOve", importCharsetOve);
      s.setValue("importCharsetGP", importCharsetGP);
      s.setValue("warnPitchRange", MScore::warnPitchRange);
      s.setValue("followSong", followSong);

      s.setValue("useOsc", useOsc);
      s.setValue("oscPort", oscPort);
      s.setValue("style", styleName);
      s.setValue("animations", animations);
      s.setValue("singlePalette", singlePalette);

      s.setValue("myScoresPath", myScoresPath);
      s.setValue("myStylesPath", myStylesPath);
      s.setValue("myImagesPath", myImagesPath);
      s.setValue("myTemplatesPath", myTemplatesPath);
      s.setValue("myPluginsPath", myPluginsPath);
      s.setValue("mySoundfontsPath", mySoundfontsPath);
      s.setValue("myExtensionsPath", myExtensionsPath);
      s.remove("sfPath");

      s.setValue("hraster", MScore::hRaster());
      s.setValue("vraster", MScore::vRaster());
      s.setValue("nativeDialogs", nativeDialogs);
      s.setValue("exportAudioSampleRate", exportAudioSampleRate);
      s.setValue("exportMp3BitRate", exportMp3BitRate);

      s.setValue("workspace", workspace);
      s.setValue("exportPdfDpi", exportPdfDpi);
      s.setValue("verticalPageOrientation", MScore::verticalOrientation());

      //update
      s.setValue("checkUpdateStartup", checkUpdateStartup);
      s.setValue("checkExtensionsUpdateStartup", checkExtensionsUpdateStartup);

      s.setValue("useMidiRemote", useMidiRemote);
      for (int i = 0; i < MIDI_REMOTES; ++i) {
            if (midiRemote[i].type != MIDI_REMOTE_TYPE_INACTIVE) {
                  QChar t;
                  if (midiRemote[i].type == MIDI_REMOTE_TYPE_NOTEON)
                        t = QChar('P');
                  else
                        t = QChar('C');
                  s.setValue(QString("remote%1").arg(i),
                     QString("%1%2").arg(t).arg(midiRemote[i].data));
                  }
            }
      s.setValue("advanceOnRelease", advanceOnRelease);

      writePluginList();
      if (Shortcut::dirty)
            Shortcut::save();
      Shortcut::dirty = false;
      }

//---------------------------------------------------------
//   readColor
//---------------------------------------------------------

QColor Preferences::readColor(QString key, QColor def) {
     QSettings s;
     QVariant v = s.value(key, def);
     if (v.type() == QVariant::Color)
           return v.value<QColor>();
     else {
           QColor c(v.toString());
           return c.isValid() ? c : def;
      }
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Preferences::read()
      {
      QSettings s;

      bgUseColor              = s.value("bgUseColor", bgUseColor).toBool();
      fgUseColor              = s.value("fgUseColor", fgUseColor).toBool();
      bgWallpaper             = s.value("bgWallpaper", bgWallpaper).toString();
      fgWallpaper             = s.value("fgWallpaper", fgWallpaper).toString();
      fgColor                 = readColor("fgColor", fgColor);
      MScore::bgColor         = readColor("bgColor", MScore::bgColor);
      iconHeight              = s.value("iconHeight", iconHeight).toInt();
      iconWidth               = s.value("iconWidth", iconWidth).toInt();

      MScore::selectColor[0]  = readColor("selectColor1", MScore::selectColor[0]);
      MScore::selectColor[1]  = readColor("selectColor2", MScore::selectColor[1]);
      MScore::selectColor[2]  = readColor("selectColor3", MScore::selectColor[2]);
      MScore::selectColor[3]  = readColor("selectColor4", MScore::selectColor[3]);

      MScore::defaultColor    = readColor("defaultColor", MScore::defaultColor);
      MScore::dropColor       = readColor("dropColor",    MScore::dropColor);
      pianoHlColor            = readColor("pianoHlColor", pianoHlColor);

      enableMidiInput         = s.value("enableMidiInput", enableMidiInput).toBool();
      realtimeDelay           = s.value("realtimeDelay", realtimeDelay).toInt();
      playNotes               = s.value("playNotes", playNotes).toBool();
      playChordOnAddNote      = s.value("playChordOnAddNote", playChordOnAddNote).toBool();

      showNavigator           = s.value("showNavigator", showNavigator).toBool();
      showSplashScreen        = s.value("showSplashScreen", showSplashScreen).toBool();
      showStartcenter         = s.value("showStartcenter1", showStartcenter).toBool();
      showPlayPanel           = s.value("showPlayPanel", showPlayPanel).toBool();

      showStatusBar           = s.value("showStatusBar", showStatusBar).toBool();

      useAlsaAudio       = s.value("useAlsaAudio", useAlsaAudio).toBool();
      useJackAudio       = s.value("useJackAudio", useJackAudio).toBool();
      useJackMidi        = s.value("useJackMidi",  useJackMidi).toBool();
      jackTimebaseMaster = s.value("jackTimebaseMaster",  jackTimebaseMaster).toBool();
      useJackTransport   = s.value("useJackTransport",  useJackTransport).toBool();
      usePortaudioAudio  = s.value("usePortaudioAudio", usePortaudioAudio).toBool();
      usePulseAudio      = s.value("usePulseAudio", usePulseAudio).toBool();

      alsaDevice         = s.value("alsaDevice", alsaDevice).toString();
      alsaSampleRate     = s.value("alsaSampleRate", alsaSampleRate).toInt();
      alsaPeriodSize     = s.value("alsaPeriodSize", alsaPeriodSize).toInt();
      alsaFragments      = s.value("alsaFragments", alsaFragments).toInt();
      portaudioDevice    = s.value("portaudioDevice", portaudioDevice).toInt();
      portMidiInput      = s.value("portMidiInput", portMidiInput).toString();
      portMidiOutput     = s.value("portMidiOutput", portMidiOutput).toString();
      portMidiOutputLatencyMilliseconds = s.value("portMidiOutputLatencyMilliseconds", portMidiOutputLatencyMilliseconds).toInt();
      MScore::layoutBreakColor   = readColor("layoutBreakColor", MScore::layoutBreakColor);
      MScore::frameMarginColor   = readColor("frameMarginColor", MScore::frameMarginColor);
      antialiasedDrawing      = s.value("antialiasedDrawing", antialiasedDrawing).toBool();
      limitScrollArea         = s.value("limitScrollArea", limitScrollArea).toBool();

      defaultStyleFile         = s.value("defaultStyle", defaultStyleFile).toString();

      midiExpandRepeats        = s.value("midiExpandRepeats", midiExpandRepeats).toBool();
      midiExportRPNs           = s.value("midiExportRPNs", midiExportRPNs).toBool();
      MScore::playRepeats      = s.value("playRepeats", MScore::playRepeats).toBool();
      MScore::panPlayback      = s.value("panPlayback", MScore::panPlayback).toBool();
      alternateNoteEntryMethod = s.value("alternateNoteEntry", alternateNoteEntryMethod).toBool();
      rememberLastConnections  = s.value("rememberLastMidiConnections", rememberLastConnections).toBool();
      proximity                = s.value("proximity", proximity).toInt();
      autoSave                 = s.value("autoSave", autoSave).toBool();
      autoSaveTime             = s.value("autoSaveTime", autoSaveTime).toInt();
      pngResolution            = s.value("pngResolution", pngResolution).toDouble();
      pngTransparent           = s.value("pngTransparent", pngTransparent).toBool();
      language                 = s.value("language", language).toString();

      musicxmlImportLayout     = s.value("musicxmlImportLayout", musicxmlImportLayout).toBool();
      musicxmlImportBreaks     = s.value("musicxmlImportBreaks", musicxmlImportBreaks).toBool();
      musicxmlExportLayout     = s.value("musicxmlExportLayout", musicxmlExportLayout).toBool();
      QString br(s.value("musicxmlExportBreaks", "all").toString());
      if (br == "all")
            musicxmlExportBreaks = MusicxmlExportBreaks::ALL;
      else if (br == "manual")
            musicxmlExportBreaks = MusicxmlExportBreaks::MANUAL;
      else if (br == "no")
            musicxmlExportBreaks = MusicxmlExportBreaks::NO;

      mag                    = s.value("mag", mag).toDouble();

      MScore::defaultPlayDuration = s.value("defaultPlayDuration", MScore::defaultPlayDuration).toInt();
      importStyleFile        = s.value("importStyleFile", importStyleFile).toString();
      shortestNote           = s.value("shortestNote", shortestNote).toInt();
      importCharsetOve          = s.value("importCharsetOve", importCharsetOve).toString();
      importCharsetGP          = s.value("importCharsetGP", importCharsetGP).toString();
      MScore::warnPitchRange = s.value("warnPitchRange", MScore::warnPitchRange).toBool();
      followSong             = s.value("followSong", followSong).toBool();

      useOsc                 = s.value("useOsc", useOsc).toBool();
      oscPort                = s.value("oscPort", oscPort).toInt();
      styleName              = s.value("style", styleName).toString();
      if (styleName == "dark")
            globalStyle  = MuseScoreStyleType::DARK;
      else
            globalStyle  = MuseScoreStyleType::LIGHT;

      animations       = s.value("animations",       animations).toBool();
      singlePalette    = s.value("singlePalette",    singlePalette).toBool();
      myScoresPath     = s.value("myScoresPath",     myScoresPath).toString();
      myStylesPath     = s.value("myStylesPath",     myStylesPath).toString();
      myImagesPath     = s.value("myImagesPath",     myImagesPath).toString();
      myTemplatesPath  = s.value("myTemplatesPath",  myTemplatesPath).toString();
      myPluginsPath    = s.value("myPluginsPath",    myPluginsPath).toString();
      // compatibility: 2.0.X with X < 3 was storing sfPath and included global path
      mySoundfontsPath = s.value("sfPath",           mySoundfontsPath).toString();
      QStringList pl = mySoundfontsPath.split(";");
      pl.removeAll(QFileInfo(QString("%1%2").arg(mscoreGlobalShare).arg("sound")).absoluteFilePath());
      mySoundfontsPath = pl.join(";");
      mySoundfontsPath = s.value("mySoundfontsPath", mySoundfontsPath).toString();
      myExtensionsPath = s.value("myExtensionsPath",    myExtensionsPath).toString();

      //Create directories if they are missing
      QDir dir;
      dir.mkpath(myScoresPath);
      dir.mkpath(myStylesPath);
      dir.mkpath(myImagesPath);
      dir.mkpath(myTemplatesPath);
      dir.mkpath(myPluginsPath);
      foreach (QString path, mySoundfontsPath.split(";"))
            dir.mkpath(path);
      dir.mkpath(myExtensionsPath);

      MScore::setHRaster(s.value("hraster", MScore::hRaster()).toInt());
      MScore::setVRaster(s.value("vraster", MScore::vRaster()).toInt());

      nativeDialogs    = s.value("nativeDialogs", nativeDialogs).toBool();
      exportAudioSampleRate = s.value("exportAudioSampleRate", exportAudioSampleRate).toInt();
      exportMp3BitRate   = s.value("exportMp3Bitrate", exportMp3BitRate).toInt();

      workspace          = s.value("workspace", workspace).toString();
      exportPdfDpi       = s.value("exportPdfDpi", exportPdfDpi).toInt();
      MScore::setVerticalOrientation(s.value("verticalPageOrientation", MScore::verticalOrientation()).toBool());

      checkUpdateStartup = s.value("checkUpdateStartup", checkUpdateStartup).toBool();
      checkExtensionsUpdateStartup = s.value("checkExtensionsUpdateStartup", checkExtensionsUpdateStartup).toBool();

      QString ss(s.value("sessionStart", "score").toString());
      if (ss == "last")
            sessionStart = SessionStart::LAST;
      else if (ss == "new")
            sessionStart = SessionStart::NEW;
      else if (ss == "score")
            sessionStart = SessionStart::SCORE;
      else if (ss == "empty")
            sessionStart = SessionStart::EMPTY;

      startScore     = s.value("startScore", startScore).toString();
      instrumentList1 = s.value("instrumentList",  instrumentList1).toString();
      instrumentList2 = s.value("instrumentList2", instrumentList2).toString();

      useMidiRemote  = s.value("useMidiRemote", useMidiRemote).toBool();
      for (int i = 0; i < MIDI_REMOTES; ++i) {
            QString data = s.value(QString("remote%1").arg(i)).toString();
            if (data.isEmpty())
                  midiRemote[i].type = MIDI_REMOTE_TYPE_INACTIVE;
            else {
                  midiRemote[i].data = data.mid(1).toInt();
                  if (data[0] == QChar('P')) {
                        midiRemote[i].type = MIDI_REMOTE_TYPE_NOTEON;
                        }
                  else if (data[0] == QChar('C')) {
                        midiRemote[i].type = MIDI_REMOTE_TYPE_CTRL;
                        }
                  }
            }
      advanceOnRelease  = s.value("advanceOnRelease", advanceOnRelease).toBool();

//      s.beginGroup("PlayPanel");
//      playPanelPos = s.value("pos", playPanelPos).toPoint();
//      s.endGroup();

      readPluginList();

      // store preferences with locale-dependent default values
      // so that the values from first start will be used later
      s.setValue("myScoresPath", myScoresPath);
      s.setValue("myStylesPath", myStylesPath);
      s.setValue("myImagesPath", myImagesPath);
      s.setValue("myTemplatesPath", myTemplatesPath);
      s.setValue("myPluginsPath", myPluginsPath);
      s.setValue("mySoundfontsPath", mySoundfontsPath);
      s.setValue("myExtensionsPath", myExtensionsPath);
      s.remove("sfPath");
      }

//---------------------------------------------------------
//   preferences
//---------------------------------------------------------

void MuseScore::startPreferenceDialog()
      {
      if (!preferenceDialog) {
            preferenceDialog = new PreferenceDialog(this);
            connect(preferenceDialog, SIGNAL(preferencesChanged()),
               SLOT(preferencesChanged()));
            }
      preferenceDialog->setPreferences(preferences);
      preferenceDialog->show();
      }

//---------------------------------------------------------
//   PreferenceDialog
//---------------------------------------------------------

PreferenceDialog::PreferenceDialog(QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("PreferenceDialog");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setModal(true);
      startWithButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      instrumentList1Button->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      instrumentList2Button->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      defaultStyleButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      partStyleButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      myScoresButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      myStylesButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      myTemplatesButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      myPluginsButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      mySoundfontsButton->setIcon(*icons[int(Icons::edit_ICON)]);
      myImagesButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      myExtensionsButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);

      bgWallpaperSelect->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      fgWallpaperSelect->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      styleFileButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      shortcutsChanged        = false;

#ifndef USE_JACK
      jackDriver->setVisible(false);
#endif
#ifndef USE_ALSA
      alsaDriver->setVisible(false);
      alsaDriver->setChecked(false);
#else
      alsaSampleRate->clear();
      alsaSampleRate->addItem(tr("192000"), 192000);
      alsaSampleRate->addItem( tr("96000"),  96000);
      alsaSampleRate->addItem( tr("88200"),  88200);
      alsaSampleRate->addItem( tr("48000"),  48000); // default
      alsaSampleRate->addItem( tr("44100"),  44100);
      alsaSampleRate->addItem( tr("32000"),  32000);
      alsaSampleRate->addItem( tr("22050"),  22050);

      alsaPeriodSize->clear();
      alsaPeriodSize->addItem(tr("4096"), 4096);
      alsaPeriodSize->addItem(tr("2048"), 2048);
      alsaPeriodSize->addItem(tr("1024"), 1024); // default
      alsaPeriodSize->addItem( tr("512"),  512);
      alsaPeriodSize->addItem( tr("256"),  256);
      alsaPeriodSize->addItem( tr("128"),  128);
      alsaPeriodSize->addItem(  tr("64"),   64);
#endif

      exportAudioSampleRate->clear();
      exportAudioSampleRate->addItem(tr("32000"), 32000);
      exportAudioSampleRate->addItem(tr("44100"), 44100); // default
      exportAudioSampleRate->addItem(tr("48000"), 48000);

#ifndef USE_LAME
      exportMp3BitRateLabel->setVisible(false);
      exportMp3BitRate->setVisible(false);
      exportMp3BitRateUnit->setVisible(false);
#else
      exportMp3BitRate->clear();
      exportMp3BitRate->addItem( tr("32"),  32);
      exportMp3BitRate->addItem( tr("40"),  40);
      exportMp3BitRate->addItem( tr("48"),  48);
      exportMp3BitRate->addItem( tr("56"),  56);
      exportMp3BitRate->addItem( tr("64"),  64);
      exportMp3BitRate->addItem( tr("80"),  80);
      exportMp3BitRate->addItem( tr("96"),  96);
      exportMp3BitRate->addItem(tr("112"), 112);
      exportMp3BitRate->addItem(tr("128"), 128); // default
      exportMp3BitRate->addItem(tr("160"), 160);
      exportMp3BitRate->addItem(tr("192"), 192);
      exportMp3BitRate->addItem(tr("224"), 224);
      exportMp3BitRate->addItem(tr("256"), 256);
      exportMp3BitRate->addItem(tr("320"), 320);
#endif

#ifndef USE_PORTAUDIO
      portaudioDriver->setVisible(false);
#endif
#ifndef USE_PORTMIDI
      portMidiInput->setVisible(false);
      portMidiInputLabel->setVisible(false);
#endif
#ifndef USE_PULSEAUDIO
      pulseaudioDriver->setVisible(false);
#endif

      tabIO->setEnabled(!noSeq);

      QButtonGroup* fgButtons = new QButtonGroup(this);
      fgButtons->setExclusive(true);
      fgButtons->addButton(fgColorButton);
      fgButtons->addButton(fgWallpaperButton);
      connect(fgColorButton, SIGNAL(toggled(bool)), SLOT(fgClicked(bool)));

      QButtonGroup* bgButtons = new QButtonGroup(this);
      bgButtons->setExclusive(true);
      bgButtons->addButton(bgColorButton);
      bgButtons->addButton(bgWallpaperButton);
      connect(bgColorButton, SIGNAL(toggled(bool)), SLOT(bgClicked(bool)));

      connect(buttonBox,          SIGNAL(clicked(QAbstractButton*)), SLOT(buttonBoxClicked(QAbstractButton*)));
      connect(fgWallpaperSelect,  SIGNAL(clicked()), SLOT(selectFgWallpaper()));
      connect(bgWallpaperSelect,  SIGNAL(clicked()), SLOT(selectBgWallpaper()));

      connect(myScoresButton, SIGNAL(clicked()), SLOT(selectScoresDirectory()));
      connect(myStylesButton, SIGNAL(clicked()), SLOT(selectStylesDirectory()));
      connect(myTemplatesButton, SIGNAL(clicked()), SLOT(selectTemplatesDirectory()));
      connect(myPluginsButton, SIGNAL(clicked()), SLOT(selectPluginsDirectory()));
      connect(myImagesButton, SIGNAL(clicked()), SLOT(selectImagesDirectory()));
      connect(mySoundfontsButton, SIGNAL(clicked()), SLOT(changeSoundfontPaths()));
      connect(myExtensionsButton, SIGNAL(clicked()), SLOT(selectExtensionsDirectory()));

      connect(updateTranslation, SIGNAL(clicked()), SLOT(updateTranslationClicked()));

      connect(defaultStyleButton,     SIGNAL(clicked()), SLOT(selectDefaultStyle()));
      connect(partStyleButton,        SIGNAL(clicked()), SLOT(selectPartStyle()));
      connect(styleFileButton,        SIGNAL(clicked()), SLOT(styleFileButtonClicked()));
      connect(instrumentList1Button,  SIGNAL(clicked()), SLOT(selectInstrumentList1()));
      connect(instrumentList2Button,  SIGNAL(clicked()), SLOT(selectInstrumentList2()));
      connect(startWithButton,        SIGNAL(clicked()), SLOT(selectStartWith()));

      connect(shortcutList,   SIGNAL(itemActivated(QTreeWidgetItem*, int)), SLOT(defineShortcutClicked()));
      connect(resetShortcut,  SIGNAL(clicked()), SLOT(resetShortcutClicked()));
      connect(clearShortcut,  SIGNAL(clicked()), SLOT(clearShortcutClicked()));
      connect(defineShortcut, SIGNAL(clicked()), SLOT(defineShortcutClicked()));
      connect(resetToDefault, SIGNAL(clicked()), SLOT(resetAllValues()));
      connect(filterShortcuts, SIGNAL(textChanged(const QString&)), SLOT(filterShortcutsTextChanged(const QString &)));
      connect(printShortcuts, SIGNAL(clicked()), SLOT(printShortcutsClicked()));

      recordButtons = new QButtonGroup(this);
      recordButtons->setExclusive(false);
      recordButtons->addButton(recordRewind, RMIDI_REWIND);
      recordButtons->addButton(recordTogglePlay,   RMIDI_TOGGLE_PLAY);
      recordButtons->addButton(recordPlay,   RMIDI_PLAY);
      recordButtons->addButton(recordStop,   RMIDI_STOP);
      recordButtons->addButton(rcr2,         RMIDI_NOTE1);
      recordButtons->addButton(rcr3,         RMIDI_NOTE2);
      recordButtons->addButton(rcr4,         RMIDI_NOTE4);
      recordButtons->addButton(rcr5,         RMIDI_NOTE8);
      recordButtons->addButton(rcr6,         RMIDI_NOTE16);
      recordButtons->addButton(rcr7,         RMIDI_NOTE32);
      recordButtons->addButton(rcr8,         RMIDI_NOTE64);
      recordButtons->addButton(rcr9,         RMIDI_REST);
      recordButtons->addButton(rcr10,        RMIDI_DOT);
      recordButtons->addButton(rcr11,        RMIDI_DOTDOT);
      recordButtons->addButton(rcr12,        RMIDI_TIE);
      recordButtons->addButton(recordUndo,   RMIDI_UNDO);
      recordButtons->addButton(recordEditMode, RMIDI_NOTE_EDIT_MODE);
      recordButtons->addButton(recordRealtimeAdvance, RMIDI_REALTIME_ADVANCE);

      restartWarningLanguage->setText("");
      connect(language, SIGNAL(currentIndexChanged(int)), SLOT(languageChanged(int)));

      connect(recordButtons,          SIGNAL(buttonClicked(int)), SLOT(recordButtonClicked(int)));
      connect(midiRemoteControlClear, SIGNAL(clicked()), SLOT(midiRemoteControlClearClicked()));
      connect(portaudioDriver, SIGNAL(toggled(bool)), SLOT(exclusiveAudioDriver(bool)));
      connect(pulseaudioDriver, SIGNAL(toggled(bool)), SLOT(exclusiveAudioDriver(bool)));
      connect(alsaDriver, SIGNAL(toggled(bool)), SLOT(exclusiveAudioDriver(bool)));
      connect(jackDriver, SIGNAL(toggled(bool)), SLOT(exclusiveAudioDriver(bool)));
      connect(useJackAudio, SIGNAL(toggled(bool)), SLOT(nonExclusiveJackDriver(bool)));
      connect(useJackMidi,  SIGNAL(toggled(bool)), SLOT(nonExclusiveJackDriver(bool)));
      updateRemote();

      MuseScore::restoreGeometry(this);

#if !defined(Q_OS_MAC) && (!defined(Q_OS_WIN) || defined(FOR_WINSTORE))
      General->removeTab(General->indexOf(tabUpdate)); // updateTab not needed on Linux and not wanted in Windows Store
#endif
      }

//---------------------------------------------------------
//   setPreferences
//---------------------------------------------------------

void PreferenceDialog::setPreferences(const Preferences& p)
      {
      prefs = p;
      updateValues();
      }

//---------------------------------------------------------
//   ~PreferenceDialog
//---------------------------------------------------------

PreferenceDialog::~PreferenceDialog()
      {
      qDeleteAll(localShortcuts);
      }


//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void PreferenceDialog::hideEvent(QHideEvent* ev)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(ev);
      }

//---------------------------------------------------------
//   recordButtonClicked
//---------------------------------------------------------

void PreferenceDialog::languageChanged(int /*val*/)
      {
      restartWarningLanguage->setText(tr("The language will be changed once you restart MuseScore."));
      }

//---------------------------------------------------------
//   recordButtonClicked
//---------------------------------------------------------

void PreferenceDialog::recordButtonClicked(int val)
      {
      foreach(QAbstractButton* b, recordButtons->buttons()) {
            b->setChecked(recordButtons->id(b) == val);
            }
      mscore->setMidiRecordId(val);
      }

//---------------------------------------------------------
//   updateRemote
//---------------------------------------------------------

void PreferenceDialog::updateRemote()
      {
      rewindActive->setChecked(preferences.midiRemote[RMIDI_REWIND].type != -1);
      togglePlayActive->setChecked(preferences.midiRemote[RMIDI_TOGGLE_PLAY].type   != -1);
      playActive->setChecked(preferences.midiRemote[RMIDI_PLAY].type         != -1);
      stopActive->setChecked(preferences.midiRemote[RMIDI_STOP].type         != -1);
      rca2->setChecked(preferences.midiRemote[RMIDI_NOTE1].type        != -1);
      rca3->setChecked(preferences.midiRemote[RMIDI_NOTE2].type        != -1);
      rca4->setChecked(preferences.midiRemote[RMIDI_NOTE4].type        != -1);
      rca5->setChecked(preferences.midiRemote[RMIDI_NOTE8].type        != -1);
      rca6->setChecked(preferences.midiRemote[RMIDI_NOTE16].type       != -1);
      rca7->setChecked(preferences.midiRemote[RMIDI_NOTE32].type       != -1);
      rca8->setChecked(preferences.midiRemote[RMIDI_NOTE64].type      != -1);
      rca9->setChecked(preferences.midiRemote[RMIDI_REST].type        != -1);
      rca10->setChecked(preferences.midiRemote[RMIDI_DOT].type         != -1);
      rca11->setChecked(preferences.midiRemote[RMIDI_DOTDOT].type      != -1);
      rca12->setChecked(preferences.midiRemote[RMIDI_TIE].type        != -1);
      recordUndoActive->setChecked(preferences.midiRemote[RMIDI_UNDO].type != -1);
      editModeActive->setChecked(preferences.midiRemote[RMIDI_NOTE_EDIT_MODE].type != -1);
      realtimeAdvanceActive->setChecked(preferences.midiRemote[RMIDI_REALTIME_ADVANCE].type != -1);

      int id = mscore->midiRecordId();
      recordRewind->setChecked(id == RMIDI_REWIND);
      recordTogglePlay->setChecked(id == RMIDI_TOGGLE_PLAY);
      recordPlay->setChecked(id == RMIDI_PLAY);
      recordStop->setChecked(id == RMIDI_STOP);
      rcr2->setChecked(id       == RMIDI_NOTE1);
      rcr3->setChecked(id       == RMIDI_NOTE2);
      rcr4->setChecked(id       == RMIDI_NOTE4);
      rcr5->setChecked(id       == RMIDI_NOTE8);
      rcr6->setChecked(id       == RMIDI_NOTE16);
      rcr7->setChecked(id       == RMIDI_NOTE32);
      rcr8->setChecked(id       == RMIDI_NOTE64);
      rcr9->setChecked(id       == RMIDI_REST);
      rcr10->setChecked(id      == RMIDI_DOT);
      rcr11->setChecked(id      == RMIDI_DOTDOT);
      rcr12->setChecked(id      == RMIDI_TIE);
      recordUndo->setChecked(id == RMIDI_UNDO);
      recordEditMode->setChecked(id == RMIDI_NOTE_EDIT_MODE);
      recordRealtimeAdvance->setChecked(id == RMIDI_REALTIME_ADVANCE);
      }

//---------------------------------------------------------
//   updateValues
//---------------------------------------------------------

void PreferenceDialog::updateValues()
      {
      rcGroup->setChecked(prefs.useMidiRemote);
      advanceOnRelease->setChecked(prefs.advanceOnRelease);

      fgWallpaper->setText(prefs.fgWallpaper);
      bgWallpaper->setText(prefs.bgWallpaper);

      bgColorButton->setChecked(prefs.bgUseColor);
      bgWallpaperButton->setChecked(!prefs.bgUseColor);
      fgColorButton->setChecked(prefs.fgUseColor);
      fgWallpaperButton->setChecked(!prefs.fgUseColor);

      if (prefs.bgUseColor) {
            bgColorLabel->setColor(MScore::bgColor);
            bgColorLabel->setPixmap(0);
            }
      else {
            bgColorLabel->setPixmap(new QPixmap(bgWallpaper->text()));
            }

      if (prefs.fgUseColor) {
            fgColorLabel->setColor(prefs.fgColor);
            fgColorLabel->setPixmap(0);
            }
      else {
            fgColorLabel->setPixmap(new QPixmap(fgWallpaper->text()));
            }

      iconWidth->setValue(prefs.iconWidth);
      iconHeight->setValue(prefs.iconHeight);

      enableMidiInput->setChecked(prefs.enableMidiInput);
      realtimeDelay->setValue(prefs.realtimeDelay);
      playNotes->setChecked(prefs.playNotes);
      playChordOnAddNote->setChecked(prefs.playChordOnAddNote);

      //Update
      checkUpdateStartup->setChecked(prefs.checkUpdateStartup);
      checkExtensionsUpdateStartup->setChecked(prefs.checkExtensionsUpdateStartup);

      navigatorShow->setChecked(prefs.showNavigator);
      playPanelShow->setChecked(prefs.showPlayPanel);
      showSplashScreen->setChecked(prefs.showSplashScreen);
      showStartcenter->setChecked(prefs.showStartcenter);

      alsaDriver->setChecked(prefs.useAlsaAudio);
      jackDriver->setChecked(prefs.useJackAudio || prefs.useJackMidi);
      useJackAudio->setChecked(prefs.useJackAudio);
      portaudioDriver->setChecked(prefs.usePortaudioAudio);
      pulseaudioDriver->setChecked(prefs.usePulseAudio);
      useJackMidi->setChecked(prefs.useJackMidi);
      useJackTransport->setChecked(prefs.useJackTransport);
      becomeTimebaseMaster->setChecked(prefs.jackTimebaseMaster);

      alsaDevice->setText(prefs.alsaDevice);

      int index = alsaSampleRate->findData(prefs.alsaSampleRate);
      alsaSampleRate->setCurrentIndex(index);
      index = alsaPeriodSize->findData(prefs.alsaPeriodSize);
      alsaPeriodSize->setCurrentIndex(index);

      alsaFragments->setValue(prefs.alsaFragments);
      drawAntialiased->setChecked(prefs.antialiasedDrawing);
      limitScrollArea->setChecked(prefs.limitScrollArea);
      switch(prefs.sessionStart) {
            case SessionStart::EMPTY:  emptySession->setChecked(true); break;
            case SessionStart::LAST:   lastSession->setChecked(true); break;
            case SessionStart::NEW:    newSession->setChecked(true); break;
            case SessionStart::SCORE:  scoreSession->setChecked(true); break;
            }
      sessionScore->setText(prefs.startScore);
      expandRepeats->setChecked(prefs.midiExpandRepeats);
      exportRPNs->setChecked(prefs.midiExportRPNs);
      instrumentList1->setText(prefs.instrumentList1);
      instrumentList2->setText(prefs.instrumentList2);

      importLayout->setChecked(prefs.musicxmlImportLayout);
      importBreaks->setChecked(prefs.musicxmlImportBreaks);
      exportLayout->setChecked(prefs.musicxmlExportLayout);
      switch(prefs.musicxmlExportBreaks) {
            case MusicxmlExportBreaks::ALL:     exportAllBreaks->setChecked(true); break;
            case MusicxmlExportBreaks::MANUAL:  exportManualBreaks->setChecked(true); break;
            case MusicxmlExportBreaks::NO:      exportNoBreaks->setChecked(true); break;
            }

      rememberLastMidiConnections->setChecked(prefs.rememberLastConnections);
      proximity->setValue(prefs.proximity);
      autoSave->setChecked(prefs.autoSave);
      autoSaveTime->setValue(prefs.autoSaveTime);
      pngResolution->setValue(prefs.pngResolution);
      pngTransparent->setChecked(prefs.pngTransparent);
      language->blockSignals(true);
      for (int i = 0; i < language->count(); ++i) {
            if (language->itemText(i).startsWith(prefs.language)) {
                  language->setCurrentIndex(i);
                  break;
                  }
            }
      language->blockSignals(false);

      //
      // initialize local shortcut table
      //    we need a deep copy to be able to rewind all
      //    changes on "Abort"
      //
      qDeleteAll(localShortcuts);
      localShortcuts.clear();
      foreach(const Shortcut* s, Shortcut::shortcuts())
            localShortcuts[s->key()] = new Shortcut(*s);
      updateSCListView();

      //Generate the filtered Shortcut List
      filterShortcutsTextChanged(filterShortcuts->text());

      //
      // initialize portaudio
      //
#ifdef USE_PORTAUDIO
      if (usePortaudio) {
            Portaudio* audio = static_cast<Portaudio*>(seq->driver());
            if (audio) {
                  QStringList apis = audio->apiList();
                  portaudioApi->clear();
                  portaudioApi->addItems(apis);
                  portaudioApi->setCurrentIndex(audio->currentApi());

                  QStringList devices = audio->deviceList(audio->currentApi());
                  portaudioDevice->clear();
                  portaudioDevice->addItems(devices);
                  portaudioDevice->setCurrentIndex(audio->currentDevice());

                  connect(portaudioApi, SIGNAL(activated(int)), SLOT(portaudioApiActivated(int)));
#ifdef USE_PORTMIDI
                  PortMidiDriver* midiDriver = static_cast<PortMidiDriver*>(audio->mididriver());
                  if (midiDriver) {
                        QStringList midiInputs = midiDriver->deviceInList();
                        int curMidiInIdx = 0;
                        portMidiInput->clear();
                        for(int i = 0; i < midiInputs.size(); ++i) {
                              portMidiInput->addItem(midiInputs.at(i), i);
                              if (midiInputs.at(i) == prefs.portMidiInput)
                                    curMidiInIdx = i;
                              }
                        portMidiInput->setCurrentIndex(curMidiInIdx);

                        QStringList midiOutputs = midiDriver->deviceOutList();
                        int curMidiOutIdx = -1; // do not set a midi out device if user never selected one
                        portMidiOutput->clear();
                        portMidiOutput->addItem("", -1);
                        for(int i = 0; i < midiOutputs.size(); ++i) {
                              portMidiOutput->addItem(midiOutputs.at(i), i);
                              if (midiOutputs.at(i) == prefs.portMidiOutput)
                                    curMidiOutIdx = i + 1;
                              }
                        portMidiOutput->setCurrentIndex(curMidiOutIdx);

                        portMidiOutputLatencyMilliseconds->setValue(prefs.portMidiOutputLatencyMilliseconds);
                        }
#endif
                  }
            }
#endif

#ifndef HAS_MIDI
      enableMidiInput->setEnabled(false);
      rcGroup->setEnabled(false);
#endif
      //
      // score settings
      //
      scale->setValue(prefs.mag * 100.0);

      defaultPlayDuration->setValue(MScore::defaultPlayDuration);
      importStyleFile->setText(prefs.importStyleFile);
      int shortestNoteIndex = 2;
      int nn = (prefs.shortestNote * 16)/MScore::division;
      switch(nn) {
            case 16: shortestNoteIndex = 0; break;
            case 8:  shortestNoteIndex = 1; break;
            case 4:  shortestNoteIndex = 2; break;
            case 2:  shortestNoteIndex = 3; break;
            case 1:  shortestNoteIndex = 4; break;
            }
      shortestNote->setCurrentIndex(shortestNoteIndex);
      useImportBuiltinStyle->setChecked(prefs.importStyleFile.isEmpty());
      useImportStyleFile->setChecked(!prefs.importStyleFile.isEmpty());

      QList<QByteArray> charsets = QTextCodec::availableCodecs();
      qSort(charsets.begin(), charsets.end());
      int idx = 0;
      importCharsetListOve->clear();
      importCharsetListGP->clear();
      foreach (QByteArray charset, charsets) {
            importCharsetListOve->addItem(charset);
            importCharsetListGP->addItem(charset);
            if (charset == prefs.importCharsetOve)
                  importCharsetListOve->setCurrentIndex(idx);
            if (charset == prefs.importCharsetGP)
                  importCharsetListGP->setCurrentIndex(idx);
            idx++;
            }

      warnPitchRange->setChecked(MScore::warnPitchRange);

      language->blockSignals(true);
      language->clear();
      int curIdx = 0;
      for(int i = 0; i < mscore->languages().size(); ++i) {
            language->addItem(mscore->languages().at(i).name, i);
            if (mscore->languages().at(i).key == prefs.language)
                  curIdx = i;
            }
      language->setCurrentIndex(curIdx);
      language->blockSignals(false);

      oscServer->setChecked(prefs.useOsc);
      oscPort->setValue(prefs.oscPort);

      styleName->setCurrentIndex(int(prefs.globalStyle));
      animations->setChecked(prefs.animations);

      defaultStyle->setText(prefs.defaultStyleFile);
      QSettings s;
      partStyle->setText(s.value("partStyle").toString());

      myScores->setText(prefs.myScoresPath);
      myStyles->setText(prefs.myStylesPath);
      myImages->setText(prefs.myImagesPath);
      myTemplates->setText(prefs.myTemplatesPath);
      myPlugins->setText(prefs.myPluginsPath);
      mySoundfonts->setText(prefs.mySoundfontsPath);
      myExtensions->setText(prefs.myExtensionsPath);

      index = exportAudioSampleRate->findData(prefs.exportAudioSampleRate);
      exportAudioSampleRate->setCurrentIndex(index);

      index = exportMp3BitRate->findData(prefs.exportMp3BitRate);
      exportMp3BitRate->setCurrentIndex(index);

      exportPdfDpi->setValue(prefs.exportPdfDpi);
      pageVertical->setChecked(MScore::verticalOrientation());
      }

//---------------------------------------------------------
//   portaudioApiActivated
//---------------------------------------------------------

#ifdef USE_PORTAUDIO
void PreferenceDialog::portaudioApiActivated(int idx)
      {
      Portaudio* audio = static_cast<Portaudio*>(seq->driver());
      QStringList devices = audio->deviceList(idx);
      portaudioDevice->clear();
      portaudioDevice->addItems(devices);
      }
#else
void PreferenceDialog::portaudioApiActivated(int)  {}
#endif

//---------------------------------------------------------
//   ShortcutItem
//---------------------------------------------------------

bool ShortcutItem::operator<(const QTreeWidgetItem& item) const
      {

      const QTreeWidget * pTree =treeWidget ();
      int column   = pTree ? pTree->sortColumn() : 0;
      return QString::localeAwareCompare(text(column).toLower(), item.text(column).toLower()) > 0;
      }

//---------------------------------------------------------
//   updateSCListView
//---------------------------------------------------------

void PreferenceDialog::updateSCListView()
      {
      shortcutList->clear();
      foreach (Shortcut* s, localShortcuts) {
            if (!s)
                  continue;
            ShortcutItem* newItem = new ShortcutItem;
            newItem->setText(0, s->descr());
            if (s->icon() != Icons::Invalid_ICON)
                  newItem->setIcon(0, *icons[int(s->icon())]);
            newItem->setText(1, s->keysToString());
            newItem->setData(0, Qt::UserRole, s->key());
            QString accessibleInfo = tr("Action: %1; Shortcut: %2")
               .arg(newItem->text(0)).arg(newItem->text(1).isEmpty()
                  ? tr("No shortcut defined") : newItem->text(1));
            newItem->setData(0, Qt::AccessibleTextRole, accessibleInfo);
            newItem->setData(1, Qt::AccessibleTextRole, accessibleInfo);
            if (enableExperimental
                        || (!s->key().startsWith("media")
                            && !s->key().startsWith("layer")
                            && !s->key().startsWith("insert-fretframe"))) {
                  shortcutList->addTopLevelItem(newItem);
                  }
            }
      shortcutList->resizeColumnToContents(0);
      }

//---------------------------------------------------------
//   resetShortcutClicked
//    reset selected shortcut to buildin default
//---------------------------------------------------------

void PreferenceDialog::resetShortcutClicked()
      {
      QTreeWidgetItem* active = shortcutList->currentItem();
      if (!active)
            return;
      QString str = active->data(0, Qt::UserRole).toString();
      if (str.isEmpty())
            return;
      Shortcut* shortcut = localShortcuts[str.toLatin1().data()];

      shortcut->reset();

      active->setText(1, shortcut->keysToString());
      shortcutsChanged = true;
      }

//---------------------------------------------------------
//   clearShortcutClicked
//---------------------------------------------------------

void PreferenceDialog::clearShortcutClicked()
      {
      QTreeWidgetItem* active = shortcutList->currentItem();
      if (!active)
            return;
      QString str = active->data(0, Qt::UserRole).toString();
      if (str.isEmpty())
            return;
      Shortcut* s = localShortcuts[str.toLatin1().data()];
      s->clear();
      active->setText(1, "");
      shortcutsChanged = true;
      }

//--------------------------------------------------------
//   filterShortcutsTextChanged
//--------------------------------------------------------

void  PreferenceDialog::filterShortcutsTextChanged(const QString &query )
      {
      QTreeWidgetItem *item;
      for(int i = 0; i < shortcutList->topLevelItemCount(); i++) {
          item = shortcutList->topLevelItem(i);

          if(item->text(0).toLower().contains(query.toLower()))
              item->setHidden(false);
          else
              item->setHidden(true);
          }
      }

//---------------------------------------------------------
//   selectFgWallpaper
//---------------------------------------------------------

void PreferenceDialog::selectFgWallpaper()
      {
      QString s = mscore->getWallpaper(tr("Choose Notepaper"));
      if (!s.isNull())
            fgWallpaper->setText(s);
      }

//---------------------------------------------------------
//   selectBgWallpaper
//---------------------------------------------------------

void PreferenceDialog::selectBgWallpaper()
      {
      QString s = mscore->getWallpaper(tr("Choose Background Wallpaper"));
      if (!s.isNull())
            bgWallpaper->setText(s);
      }

//---------------------------------------------------------
//   selectDefaultStyle
//---------------------------------------------------------

void PreferenceDialog::selectDefaultStyle()
      {
      QString s = mscore->getStyleFilename(true, tr("Choose Default Style"));
      if (!s.isNull())
            defaultStyle->setText(s);
      }

//---------------------------------------------------------
//   selectPartStyle
//---------------------------------------------------------

void PreferenceDialog::selectPartStyle()
      {
      QString s = mscore->getStyleFilename(true, tr("Choose Default Style for Parts"));
      if (!s.isNull())
            partStyle->setText(s);
      }

//---------------------------------------------------------
//   selectInstrumentList1
//---------------------------------------------------------

void PreferenceDialog::selectInstrumentList1()
      {
      QString s = QFileDialog::getOpenFileName(
         this,
         tr("Choose Instrument List"),
         instrumentList1->text(),
         tr("Instrument List") + " (*.xml)",
         0,
         preferences.nativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog
         );
      if (!s.isNull())
            instrumentList1->setText(s);
      }

//---------------------------------------------------------
//   selectInstrumentList2
//---------------------------------------------------------

void PreferenceDialog::selectInstrumentList2()
      {
      QString s = QFileDialog::getOpenFileName(
         this,
         tr("Choose Instrument List"),
         instrumentList2->text(),
         tr("Instrument List") + " (*.xml)",
         0,
         preferences.nativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog
         );
      if (!s.isNull())
            instrumentList2->setText(s);
      }

//---------------------------------------------------------
//   selectStartWith
//---------------------------------------------------------

void PreferenceDialog::selectStartWith()
      {
      QString s = QFileDialog::getOpenFileName(
         this,
         tr("Choose Starting Score"),
         sessionScore->text(),
         tr("MuseScore Files") + " (*.mscz *.mscx);;" + tr("All") + " (*)",
         0,
         preferences.nativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog
         );
      if (!s.isNull())
            sessionScore->setText(s);
      }

//---------------------------------------------------------
//   fgClicked
//---------------------------------------------------------

void PreferenceDialog::fgClicked(bool id)
      {
      fgWallpaper->setEnabled(!id);
      fgWallpaperSelect->setEnabled(!id);

      if (id) {
            // fgColorLabel->setColor(p->fgColor);
            fgColorLabel->setPixmap(0);
            }
      else {
            fgColorLabel->setPixmap(new QPixmap(fgWallpaper->text()));
            }
      }

//---------------------------------------------------------
//   bgClicked
//---------------------------------------------------------

void PreferenceDialog::bgClicked(bool id)
      {
      bgWallpaper->setEnabled(!id);
      bgWallpaperSelect->setEnabled(!id);

      if (id) {
            // bgColorLabel->setColor(p->bgColor);
            bgColorLabel->setPixmap(0);
            }
      else {
            bgColorLabel->setPixmap(new QPixmap(bgWallpaper->text()));
            }
      }

//---------------------------------------------------------
//   buttonBoxClicked
//---------------------------------------------------------

void PreferenceDialog::buttonBoxClicked(QAbstractButton* button)
      {
      switch(buttonBox->standardButton(button)) {
            case QDialogButtonBox::Apply:
                  apply();
                  break;
            case QDialogButtonBox::Ok:
                  apply();
            case QDialogButtonBox::Cancel:
            default:
                  hide();
                  break;
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void PreferenceDialog::apply()
      {
      QSettings s;
      if (partStyle->text() != s.value("partStyle").toString()) {
            s.setValue("partStyle", partStyle->text());
            MScore::defaultStyleForPartsHasChanged();
            }

      prefs.useMidiRemote  = rcGroup->isChecked();
      for (int i = 0; i < MIDI_REMOTES; ++i)
            prefs.midiRemote[i] = preferences.midiRemote[i];
      prefs.advanceOnRelease = advanceOnRelease->isChecked();
      prefs.fgWallpaper    = fgWallpaper->text();
      prefs.bgWallpaper    = bgWallpaper->text();
      prefs.fgColor        = fgColorLabel->color();
      MScore::bgColor      = bgColorLabel->color();

      prefs.iconWidth      = iconWidth->value();
      prefs.iconHeight     = iconHeight->value();

      prefs.bgUseColor     = bgColorButton->isChecked();
      prefs.fgUseColor     = fgColorButton->isChecked();
      prefs.enableMidiInput = enableMidiInput->isChecked();
      prefs.realtimeDelay   = realtimeDelay->value();
      prefs.playNotes      = playNotes->isChecked();
      prefs.playChordOnAddNote = playChordOnAddNote->isChecked();

      prefs.showNavigator      = navigatorShow->isChecked();
      prefs.showPlayPanel      = playPanelShow->isChecked();
      prefs.showSplashScreen   = showSplashScreen->isChecked();
      prefs.showStartcenter    = showStartcenter->isChecked();

      prefs.antialiasedDrawing = drawAntialiased->isChecked();
      prefs.limitScrollArea    = limitScrollArea->isChecked();

      prefs.useJackTransport   = jackDriver->isChecked() && useJackTransport->isChecked();
      prefs.jackTimebaseMaster = becomeTimebaseMaster->isChecked();
      prefs.rememberLastConnections = rememberLastMidiConnections->isChecked();

      bool wasJack = (prefs.useJackMidi || prefs.useJackAudio);
      prefs.useJackAudio       = jackDriver->isChecked() && useJackAudio->isChecked();
      prefs.useJackMidi        = jackDriver->isChecked() && useJackMidi->isChecked();
      bool nowJack = (prefs.useJackMidi || prefs.useJackAudio);
      bool jackParametersChanged = (prefs.useJackAudio != preferences.useJackAudio
                  || prefs.useJackMidi != preferences.useJackMidi
                  || prefs.rememberLastConnections != preferences.rememberLastConnections
                  || prefs.jackTimebaseMaster != preferences.jackTimebaseMaster)
                  && (wasJack && nowJack);

      if (jackParametersChanged) {
            // Change parameters of JACK driver without unload
            preferences = prefs;
            if (seq) {
                  seq->driver()->init(true);
                  if (!seq->init(true))
                        qDebug("sequencer init failed");
                  }
            }
      else if (
         (wasJack != nowJack)
         || (prefs.usePortaudioAudio != portaudioDriver->isChecked())
         || (prefs.usePulseAudio != pulseaudioDriver->isChecked())
#ifdef USE_ALSA
         || (prefs.alsaDevice != alsaDevice->text())
         || (prefs.alsaSampleRate != alsaSampleRate->currentData().toInt())
         || (prefs.alsaPeriodSize != alsaPeriodSize->currentData().toInt())
         || (prefs.alsaFragments != alsaFragments->value())
#endif
            ) {
            if (seq)
                  seq->exit();
            prefs.useAlsaAudio       = alsaDriver->isChecked();
            prefs.usePortaudioAudio  = portaudioDriver->isChecked();
            prefs.usePulseAudio      = pulseaudioDriver->isChecked();
            prefs.alsaDevice         = alsaDevice->text();
            prefs.alsaSampleRate     = alsaSampleRate->currentData().toInt();
            prefs.alsaPeriodSize     = alsaPeriodSize->currentData().toInt();
            prefs.alsaFragments      = alsaFragments->value();
            preferences = prefs;
            if (seq) {
                  Driver* driver = driverFactory(seq, "");
                  if (driver) {
                        // Updating synthesizer's sample rate
                        if (seq->synti()) {
                              seq->synti()->setSampleRate(driver->sampleRate());
                              seq->synti()->init();
                              }
                        seq->setDriver(driver);
                        }
                  if (!seq->init())
                        qDebug("sequencer init failed");
                  }
            }

#ifdef USE_PORTAUDIO
      if (usePortaudio && !noSeq) {
            Portaudio* audio = static_cast<Portaudio*>(seq->driver());
            prefs.portaudioDevice = audio->deviceIndex(portaudioApi->currentIndex(),
               portaudioDevice->currentIndex());
            }
#endif

#ifdef USE_PORTMIDI
      prefs.portMidiInput = portMidiInput->currentText();
      prefs.portMidiOutput = portMidiOutput->currentText();
      if (seq->driver() && static_cast<PortMidiDriver*>(static_cast<Portaudio*>(seq->driver())->mididriver())->isSameCoreMidiIacBus(prefs.portMidiInput, prefs.portMidiOutput)) {
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Possible MIDI Loopback"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Warning: You used the same CoreMIDI IAC bus for input and output.  This will cause problematic loopback, whereby MuseScore's outputted MIDI messages will be sent back to MuseScore as input, causing confusion.  To avoid this problem, access Audio MIDI Setup via Spotlight to create a dedicated virtual port for MuseScore's MIDI output, restart MuseScore, return to Preferences, and select your new virtual port for MuseScore's MIDI output.  Other programs may then use that dedicated virtual port to receive MuseScore's MIDI output."));
            msgBox.exec();
            }
      prefs.portMidiOutputLatencyMilliseconds = portMidiOutputLatencyMilliseconds->value();
#endif

      if (lastSession->isChecked())
            prefs.sessionStart = SessionStart::LAST;
      else if (newSession->isChecked())
            prefs.sessionStart = SessionStart::NEW;
      else if (scoreSession->isChecked())
            prefs.sessionStart = SessionStart::SCORE;
      else if (emptySession->isChecked())
            prefs.sessionStart = SessionStart::EMPTY;
      prefs.startScore         = sessionScore->text();
      prefs.myScoresPath       = myScores->text();
      prefs.myStylesPath       = myStyles->text();
      prefs.myImagesPath       = myImages->text();
      prefs.myTemplatesPath    = myTemplates->text();
      prefs.myPluginsPath      = myPlugins->text();
      prefs.mySoundfontsPath = mySoundfonts->text();
      prefs.myExtensionsPath = myExtensions->text();

      prefs.exportAudioSampleRate = exportAudioSampleRate->currentData().toInt();
      prefs.exportMp3BitRate   = exportMp3BitRate->currentData().toInt();

      prefs.midiExpandRepeats  = expandRepeats->isChecked();
      prefs.midiExportRPNs     = exportRPNs->isChecked();
      prefs.instrumentList1    = instrumentList1->text();
      prefs.instrumentList2    = instrumentList2->text();

      prefs.musicxmlImportLayout  = importLayout->isChecked();
      prefs.musicxmlImportBreaks  = importBreaks->isChecked();
      prefs.musicxmlExportLayout  = exportLayout->isChecked();
      if (exportAllBreaks->isChecked())
            prefs.musicxmlExportBreaks = MusicxmlExportBreaks::ALL;
      else if (exportManualBreaks->isChecked())
            prefs.musicxmlExportBreaks = MusicxmlExportBreaks::MANUAL;
      else if (exportNoBreaks->isChecked())
            prefs.musicxmlExportBreaks = MusicxmlExportBreaks::NO;

      prefs.proximity          = proximity->value();
      prefs.autoSave           = autoSave->isChecked();
      prefs.autoSaveTime       = autoSaveTime->value();
      prefs.pngResolution      = pngResolution->value();
      prefs.pngTransparent     = pngTransparent->isChecked();
      converterDpi             = prefs.pngResolution;
      prefs.exportPdfDpi       = exportPdfDpi->value();
      if (MScore::verticalOrientation() != pageVertical->isChecked()) {
            MScore::setVerticalOrientation(pageVertical->isChecked());
            for (Score* s : mscore->scores()) {
                  s->doLayout();
                  for (Score* ss : s->scoreList())
                        ss->doLayout();
                  }
            if (mscore->currentScoreView())
                  mscore->currentScoreView()->setOffset(0.0, 0.0);
            mscore->scorePageLayoutChanged();
            mscore->update();
            }

      if (shortcutsChanged) {
            shortcutsChanged = false;
            foreach(const Shortcut* s, localShortcuts) {
                  Shortcut* os = Shortcut::getShortcut(s->key());
                  if (os) {
                        if (!os->compareKeys(*s))
                              os->setKeys(s->keys());
                        }
                  }
            Shortcut::dirty = true;
            }

      int lang = language->itemData(language->currentIndex()).toInt();
      QString l = lang == 0 ? "system" : mscore->languages().at(lang).key;
      bool languageChanged = l != prefs.language;
      prefs.language = l;

      //update
      prefs.checkUpdateStartup = checkUpdateStartup->isChecked();
      prefs.checkExtensionsUpdateStartup = checkExtensionsUpdateStartup->isChecked();

      prefs.mag         = scale->value()/100.0;

      MScore::defaultPlayDuration = defaultPlayDuration->value();

      if (useImportStyleFile->isChecked())
            prefs.importStyleFile = importStyleFile->text();
      else
            prefs.importStyleFile.clear();

      int ticks = MScore::division/4;
      switch(shortestNote->currentIndex()) {
            case 0: ticks = MScore::division;    break;
            case 1: ticks = MScore::division/2;  break;
            case 2: ticks = MScore::division/4;  break;
            case 3: ticks = MScore::division/8;  break;
            case 4: ticks = MScore::division/16; break;
            }
      prefs.shortestNote = ticks;

      prefs.importCharsetOve = importCharsetListOve->currentText();
      prefs.importCharsetGP = importCharsetListGP->currentText();
      MScore::warnPitchRange = warnPitchRange->isChecked();

      prefs.useOsc  = oscServer->isChecked();
      prefs.oscPort = oscPort->value();
      if (styleName->currentIndex() == int(MuseScoreStyleType::DARK)) {
            prefs.styleName = "dark";
            prefs.globalStyle = MuseScoreStyleType::DARK;
            }
      else {
            prefs.styleName = "light";
            prefs.globalStyle = MuseScoreStyleType::LIGHT;
            }

      prefs.animations = animations->isChecked();
      MgStyleConfigData::animationsEnabled = prefs.animations;

      if (languageChanged) {
            setMscoreLocale(prefs.language);
            mscore->update();
            }
      if (defaultStyle->text() != prefs.defaultStyleFile) {
            prefs.defaultStyleFile = defaultStyle->text();
            prefs.readDefaultStyle();
            }

      genIcons();

      mscore->setIconSize(QSize(prefs.iconWidth * guiScaling, prefs.iconHeight * guiScaling));

      preferences = prefs;
      emit preferencesChanged();
      preferences.write();
      mscore->startAutoSave();
      }

//---------------------------------------------------------
//   readDefaultStyle
//---------------------------------------------------------

bool Preferences::readDefaultStyle()
      {
      if (defaultStyleFile.isEmpty())
            return false;
      MStyle* style = new MStyle(*MScore::defaultStyle());
      QFile f(defaultStyleFile);
      if (!f.open(QIODevice::ReadOnly))
            return false;
      bool rv = style->load(&f);
      if (rv)
            MScore::setDefaultStyle(style);     // transfer ownership
      f.close();
      return rv;
      }

//---------------------------------------------------------
//   resetAllValues
//---------------------------------------------------------

void PreferenceDialog::resetAllValues()
      {
      prefs.init();
      updateValues();

      shortcutsChanged = true;
      qDeleteAll(localShortcuts);
      localShortcuts.clear();
      Shortcut::resetToDefault();
      foreach(const Shortcut* s, Shortcut::shortcuts())
            localShortcuts[s->key()] = new Shortcut(*s);
      updateSCListView();
      }

//---------------------------------------------------------
//   styleFileButtonClicked
//---------------------------------------------------------

void PreferenceDialog::styleFileButtonClicked()
      {
      QString fn = mscore->getStyleFilename(true, tr("Choose Default Style for Imports"));
      if (fn.isEmpty())
            return;
      importStyleFile->setText(fn);
      }

//---------------------------------------------------------
//   midiRemoteControlClearClicked
//---------------------------------------------------------

void PreferenceDialog::midiRemoteControlClearClicked()
      {
      for (int i = 0; i < MIDI_REMOTES; ++i)
            preferences.midiRemote[i].type = MIDI_REMOTE_TYPE_INACTIVE;
      updateRemote();
      }

//---------------------------------------------------------
//   exclusiveAudioDriver
//   Allow user to select only one audio driver, restrict
//   to uncheck all drivers.
//---------------------------------------------------------

void PreferenceDialog::exclusiveAudioDriver(bool on)
      {
      if (on) {
            if (portaudioDriver != QObject::sender())
                  portaudioDriver->setChecked(false);
            if (pulseaudioDriver != QObject::sender())
                  pulseaudioDriver->setChecked(false);
            if (alsaDriver != QObject::sender())
                  alsaDriver->setChecked(false);
            if (jackDriver != QObject::sender())
                  jackDriver->setChecked(false);
            if (jackDriver == QObject::sender() && !useJackMidi->isChecked() && !useJackAudio->isChecked()) {
                  useJackMidi->setChecked(true);
                  useJackAudio->setChecked(true);
                  }
            }
      else {
            // True if QGroupBox is checked now or was checked before clicking on it
            bool portAudioChecked =  portaudioDriver->isVisible()  && ((QObject::sender() != portaudioDriver  && portaudioDriver->isChecked())  || QObject::sender() == portaudioDriver);
            bool pulseaudioChecked = pulseaudioDriver->isVisible() && ((QObject::sender() != pulseaudioDriver && pulseaudioDriver->isChecked()) || QObject::sender() == pulseaudioDriver);
            bool alsaChecked =       alsaDriver->isVisible()       && ((QObject::sender() != alsaDriver       && alsaDriver->isChecked())       || QObject::sender() == alsaDriver);
            bool jackChecked =       jackDriver->isVisible()       && ((QObject::sender() != jackDriver       && jackDriver->isChecked())       || QObject::sender() == jackDriver);
            // If nothing is checked, prevent looping (runned with -s, sequencer disabled)
            if (!(portAudioChecked || pulseaudioChecked || alsaChecked || jackChecked))
                  return;
            // Don't allow to uncheck all drivers
            if (portaudioDriver == QObject::sender())
                  portaudioDriver->setChecked(!(pulseaudioChecked || alsaChecked || jackChecked));
            if (pulseaudioDriver == QObject::sender())
                  pulseaudioDriver->setChecked(!(portAudioChecked || alsaChecked || jackChecked));
            if (alsaDriver == QObject::sender())
                  alsaDriver->setChecked(!(portAudioChecked || pulseaudioChecked || jackChecked));
            if (jackDriver == QObject::sender())
                  jackDriver->setChecked(!(portAudioChecked || pulseaudioChecked || alsaChecked));
            }
      }

//---------------------------------------------------------
//   exclusiveJackDriver
//   Allow user to select "use Jack Audio", "use Jack MIDI"
//   or both, but forbid unchecking both.
//   There is no need to select "JACK audio server" without
//   checking any of them.
//---------------------------------------------------------

void PreferenceDialog::nonExclusiveJackDriver(bool on)
      {
      if (!on) {
            if (QObject::sender() == useJackAudio)
                  useJackAudio->setChecked(!useJackMidi->isChecked());
            else if (QObject::sender() == useJackMidi)
                  useJackMidi->setChecked(!useJackAudio->isChecked());
            }
      }

//---------------------------------------------------------
//   selectScoresDirectory
//---------------------------------------------------------

void PreferenceDialog::selectScoresDirectory()
      {
      QString s = QFileDialog::getExistingDirectory(
         this,
         tr("Choose Score Folder"),
         myScores->text(),
         QFileDialog::ShowDirsOnly | (preferences.nativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog)
         );
      if (!s.isNull())
            myScores->setText(s);
      }

//---------------------------------------------------------
//   selectStylesDirectory
//---------------------------------------------------------

void PreferenceDialog::selectStylesDirectory()
      {
      QString s = QFileDialog::getExistingDirectory(
         this,
         tr("Choose Style Folder"),
         myStyles->text(),
         QFileDialog::ShowDirsOnly | (preferences.nativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog)
         );
      if (!s.isNull())
            myStyles->setText(s);
      }

//---------------------------------------------------------
//   selectTemplatesDirectory
//---------------------------------------------------------

void PreferenceDialog::selectTemplatesDirectory()
      {
      QString s = QFileDialog::getExistingDirectory(
         this,
         tr("Choose Template Folder"),
         myTemplates->text(),
         QFileDialog::ShowDirsOnly | (preferences.nativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog)
         );
      if (!s.isNull())
            myTemplates->setText(s);
      }

//---------------------------------------------------------
//   selectPluginsDirectory
//---------------------------------------------------------

void PreferenceDialog::selectPluginsDirectory()
      {
      QString s = QFileDialog::getExistingDirectory(
         this,
         tr("Choose Plugin Folder"),
         myPlugins->text(),
         QFileDialog::ShowDirsOnly | (preferences.nativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog)
         );
      if (!s.isNull())
            myPlugins->setText(s);
      }

//---------------------------------------------------------
//   selectImagesDirectory
//---------------------------------------------------------

void PreferenceDialog::selectImagesDirectory()
      {
      QString s = QFileDialog::getExistingDirectory(
         this,
         tr("Choose Image Folder"),
         myImages->text(),
         QFileDialog::ShowDirsOnly | (preferences.nativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog)
         );
      if (!s.isNull())
            myImages->setText(s);
      }

//---------------------------------------------------------
//   selectExtensionsDirectory
//---------------------------------------------------------

void PreferenceDialog::selectExtensionsDirectory()
      {
      QString s = QFileDialog::getExistingDirectory(
         this,
         tr("Choose Extensions Folder"),
         myExtensions->text(),
         QFileDialog::ShowDirsOnly | (preferences.nativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog)
         );
      if (!s.isNull())
            myExtensions->setText(s);
      }

//---------------------------------------------------------
//   changeSoundfontPaths
//---------------------------------------------------------

void PreferenceDialog::changeSoundfontPaths()
      {
      PathListDialog pld(this);
      pld.setWindowTitle(tr("SoundFont Folders"));
      pld.setPath(mySoundfonts->text());
      if(pld.exec())
            mySoundfonts->setText(pld.path());
      }

//---------------------------------------------------------
//   updateLanguagesClicked
//---------------------------------------------------------

void PreferenceDialog::updateTranslationClicked()
      {
      ResourceManager r(0);
      r.selectLanguagesTab();
      r.exec();
      }

//---------------------------------------------------------
//   defineShortcutClicked
//---------------------------------------------------------

void PreferenceDialog::defineShortcutClicked()
      {
      QTreeWidgetItem* active = shortcutList->currentItem();
      if (!active)
            return;
      QString str = active->data(0, Qt::UserRole).toString();
      if (str.isEmpty())
            return;
      Shortcut* s = localShortcuts[str.toLatin1().data()];
      ShortcutCaptureDialog sc(s, localShortcuts, this);
      int rv = sc.exec();
      if (rv == 0)
            return;
      if (rv == 2)
            s->clear();
      s->addShortcut(sc.getKey());
      active->setText(1, s->keysToString());
      shortcutsChanged = true;
      }

//---------------------------------------------------------
//   readPluginList
//---------------------------------------------------------

bool Preferences::readPluginList()
      {
      QFile f(dataPath + "/plugins.xml");
      if (!f.exists())
            return false;
      if (!f.open(QIODevice::ReadOnly)) {
            qDebug("Cannot open plugins file <%s>", qPrintable(f.fileName()));
            return false;
            }
      XmlReader e(&f);
      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "Plugin") {
                              PluginDescription d;
                              while (e.readNextStartElement()) {
                                    const QStringRef& tag(e.name());
                                    if (tag == "path")
                                          d.path = e.readElementText();
                                    else if (tag == "load")
                                          d.load = e.readInt();
                                    else if (tag == "SC")
                                          d.shortcut.read(e);
                                    else if (tag == "version")
                                          d.version = e.readElementText();
                                    else if (tag == "description")
                                          d.description = e.readElementText();
                                    else
                                          e.unknown();
                                    }
                              d.shortcut.setState(STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT |
                                          STATE_ALLTEXTUAL_EDIT | STATE_PLAY | STATE_FOTO | STATE_LOCK );
                              if (d.path.endsWith(".qml"))
                                    pluginList.append(d);
                              }
                        else
                              e.unknown();
                        }
                  }
            else
                  e.unknown();
            }
      return true;
      }


//---------------------------------------------------------
//   writePluginList
//---------------------------------------------------------

void Preferences::writePluginList()
      {
      QDir dir;
      dir.mkpath(dataPath);
      QFile f(dataPath + "/plugins.xml");
      if (!f.open(QIODevice::WriteOnly)) {
            qDebug("cannot create plugin file <%s>", qPrintable(f.fileName()));
            return;
            }
      Xml xml(&f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      foreach(const PluginDescription& d, pluginList) {
            xml.stag("Plugin");
            xml.tag("path", d.path);
            xml.tag("load", d.load);
            xml.tag("version", d.version);
            xml.tag("description", d.description);
            if (!d.shortcut.keys().isEmpty())
                  d.shortcut.write(xml);
            xml.etag();
            }
      xml.etag();
      f.close();
      }

//---------------------------------------------------------
//   updatePluginList
//    scan plugin folders for new plugins and update
//    pluginList
//---------------------------------------------------------

#ifdef SCRIPT_INTERFACE
static void updatePluginList(QList<QString>& pluginPathList, const QString& pluginPath,
   QList<PluginDescription>& pluginList)
      {
      QDirIterator it(pluginPath, QDir::NoDot|QDir::NoDotDot|QDir::Dirs|QDir::Files,
         QDirIterator::Subdirectories);
      while (it.hasNext()) {
            it.next();
            QFileInfo fi = it.fileInfo();
            QString path(fi.absoluteFilePath());
            if (fi.isFile()) {
                  if (path.endsWith(".qml")) {
                        bool alreadyInList = false;
                        foreach (const PluginDescription& p, pluginList) {
                              if (p.path == path) {
                                    alreadyInList = true;
                                    break;
                                    }
                              }
                        if (!alreadyInList) {
                              PluginDescription p;
                              p.path = path;
                              p.load = false;
                              collectPluginMetaInformation(&p);
                              pluginList.append(p);
                              }
                        }
                  }
            else
                  updatePluginList(pluginPathList, path, pluginList);
            }
      }
#endif

void Preferences::updatePluginList(bool forceRefresh)
      {
#ifdef SCRIPT_INTERFACE
      QList<QString> pluginPathList;
      pluginPathList.append(dataPath + "/plugins");
      pluginPathList.append(mscoreGlobalShare + "plugins");
      pluginPathList.append(myPluginsPath);
      if (forceRefresh) {
            pluginList.clear();
            QQmlEngine* engine=Ms::MScore::qml();
            engine->clearComponentCache(); //TODO: Check this doesn't have unwanted side effects.
            }

      foreach(QString pluginPath, pluginPathList) {
            Ms::updatePluginList(pluginPathList, pluginPath, pluginList);
            }
      //remove non existing files
      auto i = pluginList.begin();
      while (i != pluginList.end()) {
            PluginDescription d = *i;
            QFileInfo fi(d.path);
            if (!fi.exists())
                  i = pluginList.erase(i);
            else
                  ++i;
            }
#endif
      }

//---------------------------------------------------------
//   printShortcutsClicked
//---------------------------------------------------------

void PreferenceDialog::printShortcutsClicked()
      {
      QPrinter printer(QPrinter::HighResolution);
      MStyle* s = MScore::defaultStyle();
      const PageFormat* pf = s->pageFormat();
      printer.setPaperSize(pf->size(), QPrinter::Inch);

      printer.setCreator("MuseScore Version: " VERSION);
      printer.setFullPage(true);
      printer.setColorMode(QPrinter::Color);
      printer.setDocName(tr("MuseScore Shortcuts"));
      printer.setOutputFormat(QPrinter::NativeFormat);

      QPrintDialog pd(&printer, 0);
      pd.setWindowTitle(tr("Print Shortcuts"));
      if (!pd.exec())
            return;
      qreal dpmm = printer.logicalDpiY() / 25.4;
      qreal ph   = printer.height();
      qreal tm   = dpmm * 15;
      qreal bm   = dpmm * 15;
      qreal lm   = dpmm * 20;

      QPainter p;
      p.begin(&printer);
      qreal y;
      qreal lh = QFontMetricsF(p.font()).lineSpacing();

      // get max width for description
      QMapIterator<QString, Shortcut*> isc(localShortcuts);
      qreal col1Width = 0.0;
      while (isc.hasNext()) {
            isc.next();
            Shortcut* s = isc.value();
            col1Width = qMax(col1Width, QFontMetricsF(p.font()).width(s->descr()));
            }

      int idx = 0;
      QTreeWidgetItem* item = shortcutList->topLevelItem(0);
      while (item) {
            if (idx == 0 || y >= (ph - bm)) {
                  y = tm;
                  if (idx)
                        printer.newPage();
                  else {
                        p.save();
                        QFont font(p.font());
                        font.setPointSizeF(14.0);
                        font.setBold(true);
                        p.setFont(font);
                        p.drawText(lm, y, tr("MuseScore Shortcuts"));
                        p.restore();
                        y += QFontMetricsF(font).lineSpacing();
                        y += 5 * dpmm;
                        }
                  }
            p.drawText(lm, y, item->text(0));
            p.drawText(col1Width + lm + 5 * dpmm, y, item->text(1));
            y += lh;
            ++idx;
            item = shortcutList->itemBelow(item);
            }
      p.end();
      }
}
