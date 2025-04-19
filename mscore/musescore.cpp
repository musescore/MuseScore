//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "musescore.h"

#include <QDir>
#include <QStandardPaths>
#include <QStyleFactory>

#include "accessibletoolbutton.h"
#include "config.h"
#include "drumroll.h"
#include "drumtools.h"
#include "editraster.h"
#include "editstafftype.h"
#include "editstyle.h"
#include "extension.h"
#include "harmonyedit.h"
#include "icons.h"
#include "instrdialog.h"
#include "keyedit.h"
#include "mediadialog.h"
#include "metaedit.h"
#include "mssplashscreen.h"
#include "musescoredialogs.h"
#include "navigator.h"
#include "newwizard.h"
#ifdef OMR
#include "omrpanel.h"
#endif
#include "pagesettings.h"
#include "palette.h"
#include "pianotools.h"
#include "playpanel.h"
#include "preferences.h"
#include "prefsdialog.h"
#include "qjsondocument.h"
#include "realizeharmonydialog.h"
#include "resourceManager.h"
#include "scoreaccessibility.h"
#include "scoretab.h"
#include "scoreview.h"
#include "searchComboBox.h"
#include "selectdialog.h"
#include "selectionwindow.h"
#include "selectnotedialog.h"
#include "seq.h"
#include "shortcut.h"
#include "startcenter.h"
#include "startupWizard.h"
#include "synthcontrol.h"
#include "textpalette.h"
#include "texttools.h"
#include "timedialog.h"
#include "timeline.h"
#include "toolbuttonmenu.h"
#include "tourhandler.h"
#include "transposedialog.h"
#include "workspace.h"
#include "workspacecombobox.h"
#include "zoombox.h"

#include "audio/drivers/driver.h"

#ifdef USE_LAME
#include "audio/exports/exportmp3.h"
#endif

#include "audio/midi/event.h"
#include "audio/midi/fluid/fluid.h"
#include "audio/midi/msynthesizer.h"
#include "audio/midi/synthesizer.h"
#include "audio/midi/synthesizergui.h"

#include "awl/aslider.h"

#include "cloud/loginmanager.h"
#include "cloud/uploadscoredialog.h"

#include "debugger/debugger.h"

#include "effects/compressor/compressor.h"
#include "effects/noeffect/noeffect.h"
#include "effects/zita1/zita.h"

#include "importexport/midiimport/importmidi_instrument.h"
#include "importexport/midiimport/importmidi_operations.h"

#include "importmidi_ui/importmidi_panel.h"

#include "inspector/inspector.h"

#include "libmscore/chord.h"
#include "libmscore/chordlist.h"
#include "libmscore/drumset.h"
#include "libmscore/excerpt.h"
#include "libmscore/harmony.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/measure.h"
#include "libmscore/mscore.h"
#include "libmscore/note.h"
#include "libmscore/page.h"
#include "libmscore/part.h"
#include "libmscore/score.h"
#include "libmscore/scorediff.h"
#include "libmscore/scoreOrder.h"
#include "libmscore/segment.h"
#include "libmscore/sig.h"
#include "libmscore/staff.h"
#include "libmscore/style.h"
#include "libmscore/sym.h"
#include "libmscore/synthesizerstate.h"
#include "libmscore/system.h"
#include "libmscore/tempo.h"
#include "libmscore/undo.h"
#include "libmscore/utils.h"
#include "libmscore/volta.h"
#include "libmscore/xml.h"

#ifdef Q_OS_MAC
#include "macos/cocoabridge.h"
#endif

#include "migration/scoremigrationdialog.h"

#include "mixer/mixer.h"

#include "palette/palettemodel.h"
#include "palette/palettewidget.h"
#include "palette/paletteworkspace.h"

#include "plugin/qmlplugin.h"
#ifdef SCRIPT_INTERFACE
#include "plugin/pluginCreator.h"
#include "plugin/pluginManager.h"
#include "plugin/qmlpluginengine.h"
#endif

#include "pianoroll/pianoroll.h"

#include "scorecmp/scorecmp.h"

#include "script/recorderwidget.h"

#include "sparkle/autoUpdater.h"
#if defined(WIN_SPARKLE_ENABLED)
#include "sparkle/winSparkleAutoUpdater.h"
#elif defined(MAC_SPARKLE_ENABLED)
#include "sparkle/sparkleAutoUpdater.h"
#endif

#include "thirdparty/qzip/qzipreader_p.h"

#ifdef AEOLUS
extern Ms::Synthesizer* createAeolus();
#endif

#ifdef ZERBERUS
extern Ms::Synthesizer* createZerberus();
#endif

#ifdef QT_NO_DEBUG
      Q_LOGGING_CATEGORY(undoRedo, "undoRedo", QtCriticalMsg);
#else
      Q_LOGGING_CATEGORY(undoRedo, "undoRedo", QtCriticalMsg);
//      Q_LOGGING_CATEGORY(undoRedo, "undoRedo");
#endif

#ifdef BUILD_CRASH_REPORTER
#include "thirdparty/libcrashreporter-qt/src/libcrashreporter-handler/Handler.h"
#endif

#ifndef TELEMETRY_DISABLED
#include "actioneventobserver.h"
#include "widgets/telemetrypermissiondialog.h"
#endif
#include "telemetrymanager.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#define endl Qt::endl
#endif

namespace Ms {

MuseScore* mscore;
MasterSynthesizer* synti;

bool enableExperimental = false;

QString dataPath;
QString iconPath;

bool converterMode = false;
static bool rawDiffMode = false;
static bool diffMode = false;
static bool scriptTestMode = false;
bool processJob = false;
bool externalIcons = false;
bool pluginMode = false;
static bool startWithNewScore = false;
double guiScaling = 0.0;
static double userDPI = 0.0;
int trimMargin = -1;
bool exportScoreParts = false;
bool saveScoreParts = false;
bool ignoreWarnings = false;
bool cliSaveOnline = false;
bool exportScoreMedia = false;
bool exportScoreMeta = false;
bool exportScoreMp3 = false;
bool exportScorePartsPdf = false;
bool unrollRepeats = false;
bool needUpdateSource = false;
static bool exportTransposedScore = false;
Q_GLOBAL_STATIC(QString, transposeExportOptions);
Q_GLOBAL_STATIC(QString, highlightConfigPath);

QString mscoreGlobalShare;

Q_GLOBAL_STATIC(QString, outFileName);
Q_GLOBAL_STATIC( QString, jsonFileName);
Q_GLOBAL_STATIC( QString, audioDriver);
Q_GLOBAL_STATIC(QString, pluginName);
Q_GLOBAL_STATIC(QString, styleFile);
Q_GLOBAL_STATIC(QString, extensionName);
static bool scoresOnCommandline { false };

Q_GLOBAL_STATIC(QList<QTranslator*>, translatorList);

bool useFactorySettings = false;
bool deletePreferences = false;
QString styleName;
QString revision;
QErrorMessage* errorMessage;
const char* voiceActions[] = { "voice-1", "voice-2", "voice-3", "voice-4" };

bool mscoreFirstStart = false;

const std::list<const char*> MuseScore::_allNoteInputMenuEntries {
            "note-input",
            "pad-note-1024",
            "pad-note-512",
            "pad-note-256",
            "pad-note-128",
            "pad-note-64",
            "pad-note-32",
            "pad-note-16",
            "pad-note-8",
            "pad-note-4",
            "pad-note-2",
            "pad-note-1",
            "note-breve",
            "note-longa",
            "pad-dot",
            "pad-dotdot",
            "pad-dot3",
            "pad-dot4",
            "tie",
            "",
            "pad-rest",
            "",
            "sharp2",
            "sharp",
            "nat",
            "flat",
            "flat2",
            "flip",
            "toggle-mouse-entry",
            "toggle-edit-playback",
            "",
            "voice-1",
            "voice-2",
            "voice-3",
            "voice-4"
            };

const std::list<const char*> MuseScore::_allFileOperationEntries {
            "file-new",
            "file-open",
            "file-save",
            "file-save-online",
            "print",
            "undo",
            "redo"
            };

const std::list<const char*> MuseScore::_allPlaybackControlEntries {
#ifdef HAS_MIDI
            "midi-on",
            "",
#endif
            "rewind",
            "play",
            "loop",
            "",
            "repeat",
            "pan",
            "metronome",
            "countin"
            };

extern TextPalette* textPalette;

static const char* saveOnlineMenuItem = "file-save-online";

#ifdef BUILD_TELEMETRY_MODULE
std::unique_ptr<TelemetryManager> TelemetryManager::mgr;
#endif

//---------------------------------------------------------
// cmdInsertMeasure
//---------------------------------------------------------

void MuseScore::cmdInsertMeasures()
      {
      if (cs) {
            if (cs->selection().isNone() && !cs->selection().findMeasure()) {
                  QMessageBox::warning(0, "MuseScore",
                        tr("No measure selected:\n" "Please select a measure and try again"));
                  }
            else {
                  if (!insertMeasuresDialog)
                        insertMeasuresDialog = new InsertMeasuresDialog;
                  else {
                        insertMeasuresDialog->insmeasures->setFocus();
                        insertMeasuresDialog->insmeasures->selectAll();
                        }
                  insertMeasuresDialog->show();
                  }
            }
      }

//---------------------------------------------------------
//   getSharePath
//---------------------------------------------------------

QString getSharePath()
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

//---------------------------------------------------------
//   localeName
//---------------------------------------------------------

QString localeName()
      {
      if (useFactorySettings)
            return "system";
      return preferences.getString(PREF_UI_APP_LANGUAGE);
      }

//---------------------------------------------------------
//   printVersion
//---------------------------------------------------------

static void printVersion(const char* prog)
      {
      if (MuseScore::unstable())
            fprintf(stderr, "%s: Music Score Editor\nUnstable Prerelease for Version %s; Build %s\n",
         prog, VERSION, qPrintable(revision));
      else
            fprintf(stderr, "%s: Music Score Editor; Version %s; Build %s\n", prog, VERSION, qPrintable(revision));
      }

static const int RECENT_LIST_SIZE = 20;

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void MuseScore::closeEvent(QCloseEvent* ev)
      {
      unloadPlugins();
      QList<MasterScore*> removeList;
      for (MasterScore* score : qAsConst(scoreList)) {
            // Prompt the user to save the score if it's "dirty" (has unsaved changes) or if it's newly created but non-empty.
            if (checkDirty(score)) {
                  // The user has canceled out entirely, so ignore the close event.
                  ev->ignore();
                  return;
                  }
            // If the score is still flagged as newly created at this point, it means that either it's empty or the user has just
            // chosen to discard it, so we need to remove it from the list of scores to be saved to the session file.
            if (score->created())
                  removeList.append(score);
            }

      // remove all new created/not save score so they are
      // note saved as session data
      for (MasterScore* score : removeList) {
            scoreList.removeAll(score);
            scoreWasShown.remove(score);
            }

      writeSessionFile(true);
      for (MasterScore* score : qAsConst(scoreList)) {
            if (!score->tmpName().isEmpty()) {
                  QFile f(score->tmpName());
                  f.remove();
                  }
            }

      writeSettings();

      ev->accept();

      if (Shortcut::dirty)
            Shortcut::save();

      this->deleteLater();     //this is necessary on windows http://musescore.org/node/16713
      qApp->quit();
      }

void updateExternalValuesFromPreferences() {
      // set values in libmscore
      MScore::bgColor = preferences.getColor(PREF_UI_CANVAS_BG_COLOR);
      MScore::dropColor = preferences.getColor(PREF_UI_SCORE_NOTE_DROPCOLOR);
      MScore::defaultColor = preferences.getColor(PREF_UI_SCORE_DEFAULTCOLOR);
      MScore::defaultPlayDuration = preferences.getInt(PREF_SCORE_NOTE_DEFAULTPLAYDURATION);
      MScore::panPlayback = preferences.getBool(PREF_APP_PLAYBACK_PANPLAYBACK);
      MScore::harmonyPlayDisableCompatibility = preferences.getBool(PREF_SCORE_HARMONY_PLAY_DISABLE_COMPATIBILITY);
      MScore::harmonyPlayDisableNew = preferences.getBool(PREF_SCORE_HARMONY_PLAY_DISABLE_NEW);
      MScore::playRepeats = preferences.getBool(PREF_APP_PLAYBACK_PLAYREPEATS);
      MScore::playbackSpeedIncrement = preferences.getInt(PREF_APP_PLAYBACK_SPEEDINCREMENT);
      MScore::warnPitchRange = preferences.getBool(PREF_SCORE_NOTE_WARNPITCHRANGE);
      MScore::disableMouseEntry = preferences.getBool(PREF_SCORE_NOTE_INPUT_DISABLE_MOUSE_INPUT);
      MScore::pedalEventsMinTicks = preferences.getInt(PREF_IO_MIDI_PEDAL_EVENTS_MIN_TICKS);
      MScore::layoutBreakColor = preferences.getColor(PREF_UI_SCORE_LAYOUTBREAKCOLOR);
      MScore::frameMarginColor = preferences.getColor(PREF_UI_SCORE_FRAMEMARGINCOLOR);
      MScore::setVerticalOrientation(preferences.getBool(PREF_UI_CANVAS_SCROLL_VERTICALORIENTATION));

      MScore::selectColor[0] = preferences.getColor(PREF_UI_SCORE_VOICE1_COLOR);
      MScore::selectColor[1] = preferences.getColor(PREF_UI_SCORE_VOICE2_COLOR);
      MScore::selectColor[2] = preferences.getColor(PREF_UI_SCORE_VOICE3_COLOR);
      MScore::selectColor[3] = preferences.getColor(PREF_UI_SCORE_VOICE4_COLOR);
      MScore::cursorColor    = preferences.getColor(PREF_UI_SCORE_CURSOR_COLOR);

      MScore::setHRaster(preferences.getInt(PREF_UI_APP_RASTER_HORIZONTAL));
      MScore::setVRaster(preferences.getInt(PREF_UI_APP_RASTER_VERTICAL));

      MScore::setNudgeStep(.1);         // cursor key (default 0.1)
      MScore::setNudgeStep10(1.0);      // Ctrl + cursor key (default 1.0)
      MScore::setNudgeStep50(0.01);     // Alt  + cursor key (default 0.01)

      if (!preferences.getBool(PREF_APP_STARTUP_FIRSTSTART)) {
            //Create directories if they are missing
            QDir dir;
            dir.mkpath(preferences.getString(PREF_APP_PATHS_MYSCORES));
            dir.mkpath(preferences.getString(PREF_APP_PATHS_MYSTYLES));
            dir.mkpath(preferences.getString(PREF_APP_PATHS_MYIMAGES));
            dir.mkpath(preferences.getString(PREF_APP_PATHS_MYTEMPLATES));
            dir.mkpath(preferences.getString(PREF_APP_PATHS_MYEXTENSIONS));
            dir.mkpath(preferences.getString(PREF_APP_PATHS_MYPLUGINS));
            dir.mkpath(preferences.getString(PREF_APP_PATHS_MYSCOREFONTS));
            for (QString& path : preferences.getString(PREF_APP_PATHS_MYSOUNDFONTS).split(";"))
                  dir.mkpath(path);
                  }
            }

//---------------------------------------------------------
//   preferencesChanged
//---------------------------------------------------------

void MuseScore::preferencesChanged(bool fromWorkspace, bool changeUI)
      {
      updateExternalValuesFromPreferences();

      getAction("repeat")->setChecked(MScore::playRepeats);
      getAction("pan")->setChecked(MScore::panPlayback);
      getAction("follow")->setChecked(preferences.getBool(PREF_APP_PLAYBACK_FOLLOWSONG));
      getAction("midi-on")->setChecked(preferences.getBool(PREF_IO_MIDI_ENABLEINPUT));
      getAction("toggle-statusbar")->setChecked(preferences.getBool(PREF_UI_APP_SHOWSTATUSBAR));
      getAction("show-tours")->setChecked(preferences.getBool(PREF_UI_APP_STARTUP_SHOWTOURS));
      getAction("toggle-mouse-entry")->setChecked(!preferences.getBool(PREF_SCORE_NOTE_INPUT_DISABLE_MOUSE_INPUT));
      getAction("toggle-edit-playback")->setChecked(preferences.getBool(PREF_SCORE_NOTE_PLAYONCLICK));

      _statusBar->setVisible(preferences.getBool(PREF_UI_APP_SHOWSTATUSBAR));

      if (!cs)
            zoomBox->resetToDefaultLogicalZoom();

      if (playPanel)
            playPanel->setSpeedIncrement(preferences.getInt(PREF_APP_PLAYBACK_SPEEDINCREMENT));

      if (changeUI)
            MuseScore::updateUiStyleAndTheme(); // this is a slow operation
      updateIcons();

      QString fgWallpaper = preferences.getString(PREF_UI_CANVAS_FG_WALLPAPER);
      for (int i = 0; i < tab1->count(); ++i) {
            ScoreView* canvas = tab1->view(i);
            if (canvas == 0)
                  continue;
            if (preferences.getBool(PREF_UI_CANVAS_BG_USECOLOR))
                  canvas->setBackground(preferences.getColor(PREF_UI_CANVAS_BG_COLOR));
            else {
                  QPixmap* pm = new QPixmap(preferences.getString(PREF_UI_CANVAS_BG_WALLPAPER));
                  canvas->setBackground(pm);
                  }
            if (preferences.getBool(PREF_UI_CANVAS_FG_USECOLOR))
                  canvas->setForeground(preferences.getColor(PREF_UI_CANVAS_FG_COLOR));
            else {
                  QPixmap* pm = new QPixmap(fgWallpaper);
                  if (pm == 0 || pm->isNull())
                        qDebug("no valid pixmap %s", fgWallpaper.toLatin1().data());
                  canvas->setForeground(pm);
                  }
            }
      if (tab2) {
            for (int i = 0; i < tab2->count(); ++i) {
                  ScoreView* canvas = tab2->view(i);
                  if (canvas == 0)
                        continue;
                  if (preferences.getBool(PREF_UI_CANVAS_BG_USECOLOR))
                        canvas->setBackground(preferences.getColor(PREF_UI_CANVAS_BG_COLOR));
                  else {
                        QPixmap* pm = new QPixmap(preferences.getString(PREF_UI_CANVAS_BG_WALLPAPER));
                        canvas->setBackground(pm);
                        }
                  if (preferences.getBool(PREF_UI_CANVAS_FG_USECOLOR))
                        canvas->setForeground(preferences.getColor(PREF_UI_CANVAS_FG_COLOR));
                  else {
                        QPixmap* pm = new QPixmap(fgWallpaper);
                        if (pm == 0 || pm->isNull())
                              qDebug("no valid pixmap %s", fgWallpaper.toLatin1().data());
                        canvas->setForeground(pm);
                        }
                  }
            }

      transportTools->setEnabled(!noSeq && seq && seq->isRunning());
      playId->setEnabled(!noSeq && seq && seq->isRunning());

      // change workspace
      if (!fromWorkspace && preferences.getString(PREF_APP_WORKSPACE) != WorkspacesManager::currentWorkspace()->name()) {
            Workspace* workspace = WorkspacesManager::findByName(preferences.getString(PREF_APP_WORKSPACE));

            if (workspace)
                  mscore->changeWorkspace(workspace);
            }

      delete newWizard;
      newWizard = 0;
      reloadInstrumentTemplates();
      updateInstrumentDialog();

      if (seq)
            seq->preferencesChanged();
      }

//---------------------------------------------------------
//   populateNoteInputMenu
//---------------------------------------------------------

void MuseScore::populateNoteInputMenu()
      {
      entryTools->clear();

      for (const auto s : _noteInputMenuEntries) {
            if (!*s)
                  entryTools->addSeparator();
            else {
                  QAction* a = getAction(s);
                  QWidget* w;
                  if (strcmp(s, "note-input") == 0) {
                        //-----------------------------------------------------------------
                        // Note Entry Modes menu
                        // ToolButtonMenu to swap between Note Entry Methods
                        //-----------------------------------------------------------------
                        QActionGroup* noteEntryMethods = new QActionGroup(entryTools);

                        noteEntryMethods->addAction(getAction("note-input-steptime"));
                        noteEntryMethods->addAction(getAction("note-input-repitch"));
                        noteEntryMethods->addAction(getAction("note-input-rhythm"));
                        noteEntryMethods->addAction(getAction("note-input-realtime-auto"));
                        noteEntryMethods->addAction(getAction("note-input-realtime-manual"));
                        noteEntryMethods->addAction(getAction("note-input-timewise"));

                        connect(noteEntryMethods, SIGNAL(triggered(QAction*)), this, SLOT(cmd(QAction*)));

                        w = new ToolButtonMenu(tr("Note Entry Methods"),
                           getAction("note-input"),
                           noteEntryMethods,
                           this,
                           false);
                        w->setObjectName("note-entry-methods");
                        }
                  else if (strncmp(s, "voice-", 6) == 0) {
                        AccessibleToolButton* tb = new AccessibleToolButton(this, a);
                        tb->setObjectName("voice");
                        tb->setFocusPolicy(Qt::ClickFocus);
                        tb->setToolButtonStyle(Qt::ToolButtonTextOnly);
                        tb->setProperty((QString("voice%1").arg(atoi(s+6))).toUtf8().data(), true);
                        QPalette p(tb->palette());
                        int i = atoi(s+6) - 1;
                        p.setColor(QPalette::Base, MScore::selectColor[i]);
                        tb->setPalette(p);
                        a->setCheckable(true);
                        // tb->setDefaultAction(a);
                        w = tb;
                        }
                  else {
                        w = new AccessibleToolButton(entryTools, a);
                        w->setObjectName(s);
                        }
                  entryTools->addWidget(w);
                  }
            }
      }

//---------------------------------------------------------
//   notifyElementDraggedToScoreView
//---------------------------------------------------------

void MuseScore::notifyElementDraggedToScoreView()
      {
      if (paletteWidget)
            paletteWidget->notifyElementDraggedToScoreView();
      }

//---------------------------------------------------------
//   onLongOperationFinished
//---------------------------------------------------------

void MuseScore::onLongOperationFinished()
      {
      infoMsgBox->accept();
      }

//---------------------------------------------------------
//   moveControlCursor
//---------------------------------------------------------

void MuseScore::moveControlCursor()
      {
      if (!cv)
            return;
      cv->moveControlCursorNearCursor();
      }

//---------------------------------------------------------
//   importExtension
//---------------------------------------------------------

bool MuseScore::importExtension(QString path)
      {
      MQZipReader zipFile(path);
      // compute total unzipped size
      qint64 totalZipSize = 0;
      for (auto& fi : zipFile.fileInfoList())
            totalZipSize += fi.size;

      // check if extension path is writable and has enough space
      const QString extensionsPath = preferences.getString(PREF_APP_PATHS_MYEXTENSIONS);
      const QFileInfo extensionsDirInfo(extensionsPath);
      if (!extensionsDirInfo.isWritable()) {
            if (!MScore::noGui)
                  QMessageBox::critical(mscore, QWidget::tr("Import Extension File"), QWidget::tr("Cannot import extension on read-only storage: %1").arg(extensionsPath)); // TODO: on read-only *directory*
            return false;
            }
      QStorageInfo storage = QStorageInfo(extensionsPath);
      if (totalZipSize >= storage.bytesAvailable()) {
            if (!MScore::noGui)
                  QMessageBox::critical(mscore, QWidget::tr("Import Extension File"), QWidget::tr("Cannot import extension: storage %1 is full").arg(storage.displayName()));
            return false;
            }
      // Check structure of the extension
      bool hasMetadata = false;
      bool hasAlienDirectory = false;
      bool hasAlienFiles = false;
      QSet<QString> acceptableFolders = { Extension::sfzsDir, Extension::soundfontsDir, Extension::templatesDir, Extension::instrumentsDir, Extension::workspacesDir };
      for (auto& fi : zipFile.fileInfoList()) {
            if (fi.filePath == "metadata.json")
                  hasMetadata = true;
            else {
                  // get folders
                  auto fpath = QDir::cleanPath(fi.filePath);
                  QStringList folders(fpath);
                  while ((fpath = QFileInfo(fpath).path()).length() < folders.last().length())
                        folders << fpath;
                  if (folders.size() < 2) {
                        hasAlienFiles = true; // in root dir
                        break;
                        }
                  QString rootDir = folders.at(folders.size() - 2);
                  if (!acceptableFolders.contains(rootDir)) {
                        hasAlienDirectory = true; // in root dir
                        break;
                        }
                  }
            }
      if (!hasMetadata) {
            if (!MScore::noGui)
                  QMessageBox::critical(mscore, QWidget::tr("Import Extension File"), QWidget::tr("Corrupted extension: no metadata.json"));
            return false;
            }
      if (hasAlienDirectory) {
            if (!MScore::noGui)
                  QMessageBox::critical(mscore, QWidget::tr("Import Extension File"), QWidget::tr("Corrupted extension: unsupported directories in root directory"));
            return false;
            }
      if (hasAlienFiles) {
            if (!MScore::noGui)
                  QMessageBox::critical(mscore, QWidget::tr("Import Extension File"), QWidget::tr("Corrupted extension: unsupported files in root directory"));
            return false;
            }
      zipFile.close();

      MQZipReader zipFile2(path);
      // get extension id from metadata.json
      QByteArray mdba = zipFile2.fileData("metadata.json");
      zipFile2.close();
      QJsonDocument loadDoc = QJsonDocument::fromJson(mdba);
      QJsonObject mdObject = loadDoc.object();
      QString extensionId = mdObject["id"].toString();
      QString version = mdObject["version"].toString();
      if (extensionId.isEmpty() || version.isEmpty()) {
            if (!MScore::noGui)
                  QMessageBox::critical(mscore, QWidget::tr("Import Extension File"), QWidget::tr("Corrupted extension: corrupted metadata.json"));
            return false;
            }

      // Check if extension is already installed, ask for uninstall
      QDir dir(preferences.getString(PREF_APP_PATHS_MYEXTENSIONS));
      auto dirList = dir.entryList(QStringList(extensionId), QDir::Dirs | QDir::NoDotAndDotDot);
      if (dirList.contains(extensionId)) {
            QString extDirName = QString("%1/%2").arg(preferences.getString(PREF_APP_PATHS_MYEXTENSIONS), extensionId);
            QDir extDir(extDirName);
            auto versionDirList = extDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            if (versionDirList.size() > 0) {
                  // potentially other versions
                  // is there a more recent version?
                  for (auto& versionDir : versionDirList) {
                        if (compareVersion(version, versionDir)) {
                              qDebug() << "There is a newer version. We don't install";
                              if (!MScore::noGui)
                                    QMessageBox::critical(mscore, QWidget::tr("Import Extension File"), QWidget::tr("A newer version is already installed"));
                              return false;
                              }
                        }
                  }
            qDebug() << "found already install extension without newer version: deleting it";
            QDir d(QString("%1/%2").arg(preferences.getString(PREF_APP_PATHS_MYEXTENSIONS), extensionId));
            if (!d.removeRecursively()) {
                  if (!MScore::noGui)
                        QMessageBox::critical(mscore, QWidget::tr("Import Extension File"), QWidget::tr("Error while deleting previous version of the extension: %1").arg(extensionId));
                  return false;
                  }
            }

      //setup the message box
      infoMsgBox = new QMessageBox();
      infoMsgBox->setWindowModality(Qt::ApplicationModal);
      infoMsgBox->setWindowFlags(Qt::WindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowTitleHint));
      infoMsgBox->setTextFormat(Qt::RichText);
      infoMsgBox->setMinimumSize(300, 100);
      infoMsgBox->setMaximumSize(300, 100);
      infoMsgBox->setStandardButtons({});
      infoMsgBox->setText(QString("<p align='center'>") + tr("Please wait; unpacking extension…") + QString("</p>"));

      //setup async run of long operations
      QFutureWatcher<bool> futureWatcherUnzip;
      connect(&futureWatcherUnzip, SIGNAL(finished()), this, SLOT(onLongOperationFinished()));

      MQZipReader* zipFile3 = new MQZipReader(path);
      // Unzip the extension asynchronously
      QFuture<bool> futureUnzip = QtConcurrent::run(zipFile3, &MQZipReader::extractAll, QString("%1/%2/%3").arg(preferences.getString(PREF_APP_PATHS_MYEXTENSIONS), extensionId, version));
      futureWatcherUnzip.setFuture(futureUnzip);
      if (!MScore::noGui)
            infoMsgBox->exec();
      bool unzipResult = futureUnzip.result();
      zipFile3->close();
      delete zipFile3;
      zipFile3 = nullptr;

      if (!unzipResult) {
            if (!MScore::noGui)
                  QMessageBox::critical(mscore, QWidget::tr("Import Extension File"), QWidget::tr("Unable to extract files from the extension"));
            return false;
            }

      delete newWizard;
      newWizard = 0;
      mscore->reloadInstrumentTemplates();
      mscore->updateInstrumentDialog();

      auto loadSoundFontAsync = [&]() {
            // After install: add sfz to zerberus
            QDir sfzDir(QString("%1/%2/%3/%4").arg(preferences.getString(PREF_APP_PATHS_MYEXTENSIONS), extensionId, version, QString(Extension::sfzsDir)));
            if (sfzDir.exists()) {
                  // get all sfz files
                  QDirIterator it(sfzDir.absolutePath(), QStringList("*.sfz"), QDir::Files, QDirIterator::Subdirectories);
                  Synthesizer* s = synti->synthesizer("Zerberus");
                  QStringList sfzs;
                  while (it.hasNext()) {
                        it.next();
                        sfzs.append(it.fileName());
                        }
                  sfzs.sort();
                  for (int sfzNum = 0; sfzNum < sfzs.size(); ++sfzNum)
                        s->addSoundFont(sfzs[sfzNum]);

                  if (!sfzs.isEmpty())
                        synti->storeState();

                  s->gui()->synthesizerChanged();
                  }

            // After install: add soundfont to fluid
            QDir sfDir(QString("%1/%2/%3/%4").arg(preferences.getString(PREF_APP_PATHS_MYEXTENSIONS), extensionId, version, QString(Extension::soundfontsDir)));
            if (sfDir.exists()) {
                  // get all soundfont files
                  QStringList filters("*.sf2");
                  filters.append("*.sf3");
                  QDirIterator it(sfDir.absolutePath(), filters, QDir::Files, QDirIterator::Subdirectories);
                  QStringList sfs;
                  while (it.hasNext()) {
                        it.next();
                        sfs.append(it.fileInfo().absoluteFilePath());
                        }
                  sfs.sort();
                  Synthesizer* s = synti->synthesizer("Fluid");
                  for (auto& sf : sfs) {
                        s->addSoundFont(sf);
                        }
                  if (!sfs.isEmpty())
                        synti->storeState();

                  s->gui()->synthesizerChanged();
                  }
            };
      if (!enableExperimental) {
            //load soundfonts async
            QFuture<void> futureLoadSFs = QtConcurrent::run(loadSoundFontAsync);
            QFutureWatcher<void> futureWatcherLoadSFs;
            futureWatcherLoadSFs.setFuture(futureLoadSFs);
            connect(&futureWatcherLoadSFs, SIGNAL(finished()), this, SLOT(onLongOperationFinished()));
            infoMsgBox->setText(QString("<p align='center'>") + tr("Please wait; loading SoundFonts…") + QString("</p>"));
            if (!MScore::noGui)
                  infoMsgBox->exec();
            else
                  futureLoadSFs.waitForFinished();
            }

      // after install: refresh workspaces if needed
      QDir workspacesDir(QString("%1/%2/%3/%4").arg(preferences.getString(PREF_APP_PATHS_MYEXTENSIONS), extensionId, version, QString(Extension::workspacesDir)));
      if (workspacesDir.exists() && !MScore::noGui) {
            auto wsList = workspacesDir.entryInfoList(QStringList("*.workspace"), QDir::Files);
            if (!wsList.isEmpty()) {
                  WorkspacesManager::refreshWorkspaces();
                  emit workspacesChanged();
                  changeWorkspace(WorkspacesManager::workspaces().last());
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   uninstallExtension
//---------------------------------------------------------

bool MuseScore::uninstallExtension(QString extensionId)
      {
      QString version = Extension::getLatestVersion(extensionId);
      // Before install: remove sfz from zerberus
      QDir sfzDir(QString("%1/%2/%3/%4").arg(preferences.getString(PREF_APP_PATHS_MYEXTENSIONS), extensionId, version, QString(Extension::sfzsDir)));
      if (sfzDir.exists()) {
            // get all sfz files
            QDirIterator it(sfzDir.absolutePath(), QStringList("*.sfz"), QDir::Files, QDirIterator::Subdirectories);
            Synthesizer* s = synti->synthesizer("Zerberus");
            bool found = it.hasNext();
            while (it.hasNext()) {
                  it.next();
                  s->removeSoundFont(it.fileInfo().absoluteFilePath());
                  }
            if (found)
                  synti->storeState();
            s->gui()->synthesizerChanged();
            }
      // Before install: remove soundfont from fluid
      QDir sfDir(QString("%1/%2/%3/%4").arg(preferences.getString(PREF_APP_PATHS_MYEXTENSIONS), extensionId, version, QString(Extension::soundfontsDir)));
      if (sfDir.exists()) {
            // get all soundfont files
            QStringList filters("*.sf2");
            filters.append("*.sf3");
            QDirIterator it(sfDir.absolutePath(), filters, QDir::Files, QDirIterator::Subdirectories);
            Synthesizer* s = synti->synthesizer("Fluid");
            bool found = it.hasNext();
            while (it.hasNext()) {
                  it.next();
                  s->removeSoundFont(it.fileName());
                  }
            if (found)
                  synti->storeState();

            s->gui()->synthesizerChanged();
            }
      bool refreshWorkspaces = false;
      QDir workspacesDir(QString("%1/%2/%3/%4").arg(preferences.getString(PREF_APP_PATHS_MYEXTENSIONS), extensionId, version, QString(Extension::workspacesDir)));
      if (workspacesDir.exists()) {
            auto wsList = workspacesDir.entryInfoList(QStringList("*.workspace"), QDir::Files);
            if (!wsList.isEmpty())
                  refreshWorkspaces = true;
            }

      // delete directories
      QDir extensionDir(QString("%1/%2").arg(preferences.getString(PREF_APP_PATHS_MYEXTENSIONS), extensionId));
      extensionDir.removeRecursively();

      // update UI
      delete newWizard;
      newWizard = 0;
      mscore->reloadInstrumentTemplates();
      mscore->updateInstrumentDialog();
      if (refreshWorkspaces) {
            const auto curWorkspaceName = WorkspacesManager::currentWorkspace()->name();
            WorkspacesManager::refreshWorkspaces();
            emit workspacesChanged();
            //If current worksapce is alive, do nothing
            //Select first available workspace in the list otherwise
            bool curWorkspaceDisappeared = false;
            if (WorkspacesManager::findByName(curWorkspaceName))
                  curWorkspaceDisappeared = true;

            if (curWorkspaceDisappeared)
                  changeWorkspace(WorkspacesManager::workspaces().last());
            }
      return true;
      }

//---------------------------------------------------------
//   isInstalled
///  used from javascript in start center webview
//---------------------------------------------------------

bool MuseScore::isInstalledExtension(QString extensionId)
      {
      return Extension::isInstalled(extensionId);
      }

//---------------------------------------------------------
//   populateFileOperations
//---------------------------------------------------------

void MuseScore::populateFileOperations()
      {
      // Save the current zoom and view-mode combobox states. if any.
      const auto zoomBoxState = zoomBox
         ? std::make_pair(zoomBox->currentIndex(), zoomBox->itemText(static_cast<int>(ZoomIndex::ZOOM_FREE)))
         : std::make_pair(-1, QString());
      const auto viewModeComboIndex = viewModeCombo ? viewModeCombo->currentIndex() : -1;

      fileTools->clear();

      if (qApp->layoutDirection() == Qt::LayoutDirection::LeftToRight) {
            for (auto s : _fileOperationEntries) {
                  if (!*s)
                        fileTools->addSeparator();
                  else
                        fileTools->addWidget(new AccessibleToolButton(fileTools, getAction(s)));
                  }
            }
      else {
            _fileOperationEntries.reverse();
            for (auto s : _fileOperationEntries) {
                  if (!*s)
                        fileTools->addSeparator();
                  else
                        fileTools->addWidget(new AccessibleToolButton(fileTools, getAction(s)));
                  }
            _fileOperationEntries.reverse();
            }

      // Currently not customizable in ToolbarEditor
      fileTools->addSeparator();
      zoomBox = new ZoomBox;

      // Restore the saved zoom combobox index and text, if any.
      if (zoomBoxState.first != -1) {
            zoomBox->setCurrentIndex(zoomBoxState.first);
            zoomBox->setItemText(static_cast<int>(ZoomIndex::ZOOM_FREE), zoomBoxState.second);
            }

      connect(zoomBox, SIGNAL(zoomChanged(ZoomIndex,qreal)), SLOT(zoomBoxChanged(ZoomIndex,qreal)));
      fileTools->addWidget(zoomBox);

      viewModeCombo = new QComboBox(this);
#if defined(Q_OS_MAC)
      viewModeCombo->setFocusPolicy(Qt::StrongFocus);
#else
      viewModeCombo->setFocusPolicy(Qt::TabFocus);
#endif
      viewModeCombo->setFixedHeight(preferences.getInt(PREF_UI_THEME_ICONHEIGHT) + 8);  // hack

      viewModeCombo->setAccessibleName(tr("View Mode"));
      viewModeCombo->addItem(tr("Page View"), int(LayoutMode::PAGE));
      viewModeCombo->addItem(tr("Continuous View"), int(LayoutMode::LINE));
      viewModeCombo->addItem(tr("Single Page"), int(LayoutMode::SYSTEM));
#ifdef NDEBUG
      if (enableExperimental)
#endif
            viewModeCombo->addItem(tr("Floating"), int(LayoutMode::FLOAT));

      // Restore the saved view-mode combobox index, if any.
      if (viewModeComboIndex != -1)
            viewModeCombo->setCurrentIndex(viewModeComboIndex);

      connect(viewModeCombo, SIGNAL(activated(int)), SLOT(switchLayoutMode(int)));
      fileTools->addWidget(viewModeCombo);
      }

//---------------------------------------------------------
//   populatePlaybackControls
//---------------------------------------------------------

void MuseScore::populatePlaybackControls()
      {
      transportTools->clear();

      for (const auto s : _playbackControlEntries) {
            if (!*s)
                  transportTools->addSeparator();
            else {
                  if (QString(s) == "repeat") {
                              QAction* repeatAction = getAction("repeat");
                              repeatAction->setChecked(preferences.getBool(PREF_APP_PLAYBACK_PLAYREPEATS));
                              QWidget* w = new AccessibleToolButton(transportTools, repeatAction);
                              transportTools->addWidget(w);
                        }
                  else if (QString(s) == "play") {
                        _playButton = new AccessibleToolButton(transportTools, getAction("play"));
                        transportTools->addWidget(_playButton);
                        }
                  else {
                        QWidget* w = new AccessibleToolButton(transportTools, getAction(s));
                        transportTools->addWidget(w);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   MuseScore
//---------------------------------------------------------

MuseScore::MuseScore()
   : QMainWindow()
      {
      _tourHandler = new TourHandler(this);
      qApp->installEventFilter(_tourHandler);
      _tourHandler->loadTours();

      setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

      QScreen* screen = QGuiApplication::primaryScreen();
      if (qFuzzyIsNull(userDPI)) {
#if defined(Q_OS_WIN)
      if (QOperatingSystemVersion::current() <= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 7))
            _physicalDotsPerInch = screen->logicalDotsPerInch() * screen->devicePixelRatio();
      else
            _physicalDotsPerInch = screen->physicalDotsPerInch();  // physical resolution
#else
      _physicalDotsPerInch = screen->physicalDotsPerInch();        // physical resolution
#endif
            }
      else {
            _physicalDotsPerInch = userDPI;
            }
      if (qFuzzyIsNull(guiScaling)) {
            // set scale for icons, palette elements, window sizes, etc
            // the default values are hard coded in pixel sizes and assume ~96 DPI
            if (qAbs(_physicalDotsPerInch - DPI_DISPLAY) > 6.0)
                  guiScaling = _physicalDotsPerInch / DPI_DISPLAY;
            else
                  guiScaling = 1.0;
            }

      MScore::pixelRatio = DPI / screen->logicalDotsPerInch();

      setObjectName("MuseScore");
      _sstate = STATE_INIT;
      setWindowTitle(QString(MUSESCORE_NAME_VERSION));
      setIconSize(QSize(preferences.getInt(PREF_UI_THEME_ICONWIDTH) * guiScaling, preferences.getInt(PREF_UI_THEME_ICONHEIGHT) * guiScaling));

      ucheck = new UpdateChecker(this);
      packUChecker = new ExtensionsUpdateChecker(this);

      setAcceptDrops(true);
      setFocusPolicy(Qt::NoFocus);

#ifdef SCRIPT_INTERFACE
      pluginManager = new PluginManager(0);
#endif

      _positionLabel = new QLabel;
      _positionLabel->setObjectName("decoration widget");  // this prevents animations

      _modeText = new QLabel;
      _modeText->setAutoFillBackground(false);
      _modeText->setObjectName("modeLabel");

      hRasterAction   = getAction("hraster");
      vRasterAction   = getAction("vraster");
      loopAction      = getAction("loop");
      loopInAction    = getAction("loop-in");
      loopOutAction   = getAction("loop-out");
      metronomeAction = getAction("metronome");
      countInAction   = getAction("countin");
      panAction       = getAction("pan");

      _statusBar = new QStatusBar;
      _statusBar->addPermanentWidget(new QWidget(this), 2);
      _statusBar->addPermanentWidget(new QWidget(this), 100);
      _statusBar->addPermanentWidget(_modeText, 0);

      if (enableExperimental) {
            layerSwitch = new QComboBox(this);
            layerSwitch->setToolTip(tr("Switch layer"));
            connect(layerSwitch, SIGNAL(activated(QString)), SLOT(switchLayer(QString)));
            playMode = new QComboBox(this);
            playMode->addItem(tr("Synthesizer"));
            playMode->addItem(tr("Audio track"));
            playMode->setToolTip(tr("Switch play mode"));
            connect(playMode, SIGNAL(activated(int)), SLOT(switchPlayMode(int)));

            _statusBar->addPermanentWidget(playMode);
            _statusBar->addPermanentWidget(layerSwitch);
            }

      _statusBar->addPermanentWidget(_positionLabel, 0);

      setStatusBar(_statusBar);
      ScoreAccessibility::createInstance(this);

      // otherwise unused actions:
      //   must be added somewhere to work
      QActionGroup* ag = Shortcut::getActionGroupForWidget(MsWidget::MAIN_WINDOW);
      ag->setParent(this);
      addActions(ag->actions());
      connect(ag, SIGNAL(triggered(QAction*)), SLOT(cmd(QAction*)));

      mainWindow = new QSplitter;
      mainWindow->setObjectName("mainwindow");
      mainWindow->setAccessibleName("");
      mainWindow->setChildrenCollapsible(false);

      QWidget* mainScore = new QWidget;
      mainScore->setObjectName("mainscore");
      mainScore->setAccessibleName("");
      mainScore->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      mainWindow->addWidget(mainScore);

      layout = new QVBoxLayout;
      layout->setObjectName("layout");
      layout->setMargin(0);
      layout->setSpacing(0);
      mainScore->setLayout(layout);

      _navigator = new NScrollArea;
      _navigator->setFocusPolicy(Qt::NoFocus);
      mainWindow->addWidget(_navigator);
      scorePageLayoutChanged();
      showNavigator(preferences.getBool(PREF_UI_APP_STARTUP_SHOWNAVIGATOR));

      _timeline = new TDockWidget;
      _timeline->setFocusPolicy(Qt::NoFocus);
      addDockWidget(Qt::BottomDockWidgetArea, _timeline);
      scorePageLayoutChanged();
      showTimeline(false);

      scoreCmpTool = new ScoreComparisonTool;
      scoreCmpTool->setVisible(false);
      {
      QAction* a = getAction("toggle-scorecmp-tool");
      connect(
         scoreCmpTool, &ScoreComparisonTool::visibilityChanged,
         a,            &QAction::setChecked
         );
      }
      addDockWidget(Qt::BottomDockWidgetArea, scoreCmpTool);

      if (MuseScore::unstable()) {
            scriptRecorder = new ScriptRecorderWidget(this, this);
            scriptRecorder->setVisible(false);
            QAction* a = getAction("toggle-script-recorder");
            connect(
               scriptRecorder, &ScriptRecorderWidget::visibilityChanged,
               a,              &QAction::setChecked
               );
            addDockWidget(Qt::RightDockWidgetArea, scriptRecorder);
            }

      mainWindow->setStretchFactor(0, 1);
      mainWindow->setStretchFactor(1, 0);
      mainWindow->setSizes(QList<int>({500, 50}));

      QSplitter* envelope = new QSplitter;
      envelope->setObjectName("pane");
      envelope->setAccessibleName("");
      envelope->setChildrenCollapsible(false);
      envelope->setOrientation(Qt::Vertical);
      envelope->addWidget(mainWindow);

      importmidiPanel = new ImportMidiPanel(this);
      importmidiPanel->setVisible(false);
      envelope->addWidget(importmidiPanel);

      {
      importmidiShowPanel = new QFrame;
      QHBoxLayout *hl = new QHBoxLayout;
      hl->setMargin(0);
      hl->setSpacing(0);
      importmidiShowPanel->setLayout(hl);
      showMidiImportButton = new QPushButton();
      showMidiImportButton->setFocusPolicy(Qt::ClickFocus);
      importmidiShowPanel->setVisible(false);
      connect(showMidiImportButton, SIGNAL(clicked()), SLOT(showMidiImportPanel()));
      connect(importmidiPanel, SIGNAL(closeClicked()), importmidiShowPanel, SLOT(show()));
      hl->addWidget(showMidiImportButton);
      QSpacerItem *item = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);
      hl->addSpacerItem(item);
      envelope->addWidget(importmidiShowPanel);
      }

      envelope->setSizes(QList<int>({550, 180}));

      splitter = new QSplitter;
      splitter->setObjectName("splitter");
      splitter->setAccessibleName("");
      tab1 = createScoreTab();
      splitter->addWidget(tab1);
      ctab = tab1; // make tab1 active by default.

      if (splitScreen()) {
            tab2 = createScoreTab();
            splitter->addWidget(tab2);
            tab2->setVisible(false);
            }
      else
            tab2 = 0;
      layout->addWidget(splitter);


      //---------------------------------------------------
      //    Transport Action
      //---------------------------------------------------

      QAction* a;
#ifdef HAS_MIDI
      a  = getAction("midi-on");
      a->setChecked(preferences.getBool(PREF_IO_MIDI_ENABLEINPUT));
#endif

      getAction("undo")->setEnabled(false);
      getAction("redo")->setEnabled(false);
      getAction("paste")->setEnabled(false);
      getAction("swap")->setEnabled(false);
      selectionChanged(SelState::NONE);

      //---------------------------------------------------
      //    File Tool Bar
      //---------------------------------------------------

      fileTools = addToolBar("");
      fileTools->setObjectName("file-operations");
      populateFileOperations();

      //---------------------
      //    Transport Tool Bar
      //---------------------

      transportTools = addToolBar("");
      transportTools->setObjectName("transport-tools");
      populatePlaybackControls();

      //-------------------------------
      //    Concert Pitch Tool Bar
      //-------------------------------

      cpitchTools = addToolBar("");
      cpitchTools->setObjectName("pitch-tools");
      a = getAction("concert-pitch");
      a->setCheckable(true);
      AccessibleToolButton* concertPitchButton = new AccessibleToolButton(cpitchTools, a);
      concertPitchButton->setProperty("iconic-text", true);
      cpitchTools->addWidget(concertPitchButton);

      //-------------------------------
      //    Image Capture Tool Bar
      //-------------------------------

      fotoTools = addToolBar("");
      fotoTools->setObjectName("foto-tools");
      fotoTools->addWidget(new AccessibleToolButton(fotoTools, getAction("fotomode")));

#if 0
      //-------------------------------
      //    Feedback Tool Bar
      //-------------------------------

      {
      feedbackTools = addToolBar("");
      feedbackTools->setObjectName("feedback-tools");
      // Forbid to move or undock the toolbar...
      feedbackTools->setMovable(false);
      feedbackTools->setFloatable(false);
      // Add a spacer to align the buttons to the right side.
      QWidget* spacer = new QWidget(feedbackTools);
      spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
      feedbackTools->addWidget(spacer);
      // And, finally, add the buttons themselves.
      AccessibleToolButton* feedbackButton = new AccessibleToolButton(feedbackTools, getAction("leave-feedback"));
      feedbackButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
      feedbackTools->addWidget(feedbackButton);
      }
#endif

      addToolBarBreak();

      //-------------------------------
      //    Note Input Tool Bar
      //-------------------------------

      entryTools = addToolBar("");
      entryTools->setObjectName("entry-tools");

      populateNoteInputMenu();

      getAction("toggle-mouse-entry")->setChecked(!preferences.getBool(PREF_SCORE_NOTE_INPUT_DISABLE_MOUSE_INPUT));
      getAction("toggle-edit-playback")->setChecked(preferences.getBool(PREF_SCORE_NOTE_PLAYONCLICK));

      //-------------------------------
      //    Workspaces Tool Bar
      //-------------------------------

      {
      workspacesTools = addToolBar("");
      workspacesTools->setObjectName("workspaces-tools");

      // Add a spacer to align the buttons to the right side.
      QWidget* spacer = new QWidget(workspacesTools);
      spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
      workspacesTools->addWidget(spacer);

      WorkspaceComboBox* box = new WorkspaceComboBox(this);
      workspacesTools->addWidget(box);

      AccessibleToolButton* addWorkspaceButton = new AccessibleToolButton(workspacesTools, getAction("create-new-workspace"));
      addWorkspaceButton->setMinimumHeight(24);
      workspacesTools->addWidget(addWorkspaceButton);
      }

      addToolBarBreak();

      //---------------------
      //    Menus
      //---------------------

      QMenuBar* mb = menuBar();

      //---------------------
      //    Menu File
      //---------------------

      menuFile = mb->addMenu("");
      menuFile->setObjectName("File");

      a = getAction("startcenter");
      a->setCheckable(true);
      menuFile->addAction(a);
      menuFile->addAction(getAction("file-new"));
      menuFile->addAction(getAction("file-open"));

      openRecent = menuFile->addMenu("");

      connect(openRecent, SIGNAL(aboutToShow()), SLOT(openRecentMenu()));
      connect(openRecent, SIGNAL(triggered(QAction*)), SLOT(selectScore(QAction*)));

      menuFile->addSeparator();
      menuFile->addAction(getAction("file-close"));
      menuFile->addAction(getAction("file-save"));
      menuFile->addAction(getAction("file-save-as"));
      menuFile->addAction(getAction("file-save-a-copy"));
      menuFile->addAction(getAction("file-save-selection"));
      menuFile->addAction(getAction(saveOnlineMenuItem));

      menuFile->addSeparator();
      menuFile->addAction(getAction("file-import-pdf"));
      menuFile->addAction(getAction("file-export"));

      menuFile->addSeparator();
      menuFile->addAction(getAction("edit-info"));
      menuFile->addAction(getAction("parts"));

      // Show these items only in experimental mode
      if (enableExperimental) {
            menuFile->addAction(getAction("album"));
            menuFile->addAction(getAction("layer"));
            menuFile->addAction(getAction("media"));
      }
      menuFile->addSeparator();
      menuFile->addAction(getAction("print"));
#ifndef Q_OS_MAC // On Mac, the Quit option is already in the "MuseScore" menu
      menuFile->addSeparator();
      menuFile->addAction(getAction("quit"));
#endif

      //---------------------
      //    Menu Edit
      //---------------------

      menuEdit = mb->addMenu("");
      menuEdit->setObjectName("Edit");
      menuEdit->addAction(getAction("undo"));
      menuEdit->addAction(getAction("redo"));

      menuEdit->addSeparator();
      menuEdit->addAction(getAction("cut"));
      menuEdit->addAction(getAction("copy"));
      menuEdit->addAction(getAction("paste"));
      menuEdit->addAction(getAction("paste-half"));
      menuEdit->addAction(getAction("paste-double"));
      menuEdit->addAction(getAction("swap"));
      menuEdit->addAction(getAction("delete"));

      menuEdit->addSeparator();
      menuEdit->addAction(getAction("select-all"));
      menuEdit->addAction(getAction("select-section"));
      menuEdit->addAction(getAction("find"));

      menuEdit->addSeparator();

      menuEdit->addAction(getAction("instruments"));

#ifdef NDEBUG
      if (enableExperimental) {
#endif
            menuEdit->addSeparator();
            menuEdit->addAction(getAction("debugger"));
#ifdef NDEBUG
            }
#endif

      menuEdit->addSeparator();
      pref = new QAction("", 0);
      connect(pref, SIGNAL(triggered()), this, SLOT(startPreferenceDialog()));
      menuEdit->addAction(pref);
      pref->setMenuRole(QAction::PreferencesRole);
      Workspace::addActionAndString(pref, "preference-dialog");

      //---------------------
      //    Menu View
      //---------------------

      menuView = mb->addMenu("");
      menuView->setObjectName("View");

      a = getAction("toggle-palette");
      a->setCheckable(true);
      menuView->addAction(a);

      a = getAction("masterpalette");
      a->setCheckable(true);
      menuView->addAction(a);

      a = getAction("inspector");
      a->setCheckable(true);
      menuView->addAction(a);
#ifdef OMR
      a = getAction("omr");
      a->setCheckable(true);
      menuView->addAction(a);
#endif
      playId = getAction("toggle-playpanel");
      playId->setCheckable(true);
      menuView->addAction(playId);

      a = getAction("toggle-navigator");
      a->setCheckable(true);
      a->setChecked(preferences.getBool(PREF_UI_APP_STARTUP_SHOWNAVIGATOR));
      menuView->addAction(a);

      a = getAction("toggle-timeline");
      a->setCheckable(true);
      menuView->addAction(a);

      a = getAction("toggle-mixer");
      a->setCheckable(true);
      menuView->addAction(a);

      a = getAction("synth-control");
      a->setCheckable(true);
      menuView->addAction(a);

      a = getAction("toggle-selection-window");
      a->setCheckable(true);
      menuView->addAction(a);

      a = getAction("toggle-piano");
      a->setCheckable(true);
      menuView->addAction(a);

      a = getAction("toggle-scorecmp-tool");
      a->setCheckable(true);
      menuView->addAction(a);

      menuView->addSeparator();
      menuView->addAction(getAction("zoomin"));
      menuView->addAction(getAction("zoomout"));
      menuView->addSeparator();

      menuToolbars = new QMenu();

      a = getAction("toggle-fileoperations");
      a->setCheckable(true);
      a->setChecked(fileTools->isVisible());
      connect(fileTools, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));
      menuToolbars->addAction(a);

      a = getAction("toggle-transport");
      a->setCheckable(true);
      a->setChecked(transportTools->isVisible());
      connect(transportTools, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));
      menuToolbars->addAction(a);

      a = getAction("toggle-concertpitch");
      a->setCheckable(true);
      a->setChecked(cpitchTools->isVisible());
      connect(cpitchTools, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));
      menuToolbars->addAction(a);

      a = getAction("toggle-imagecapture");
      a->setCheckable(true);
      a->setChecked(fotoTools->isVisible());
      connect(fotoTools, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));
      menuToolbars->addAction(a);

      a = getAction("toggle-noteinput");
      a->setCheckable(true);
      a->setChecked(entryTools->isVisible());
      connect(entryTools, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));
      menuToolbars->addAction(a);

#if 0
      a = getAction("toggle-feedback");
      a->setCheckable(true);
      a->setChecked(feedbackTools->isVisible());
      connect(feedbackTools, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));
      menuToolbars->addAction(a);
#endif

      a = getAction("toggle-workspaces-toolbar");
      a->setCheckable(true);
      a->setChecked(workspacesTools->isVisible());
      connect(workspacesTools, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));
      menuToolbars->addAction(a);

      menuToolbars->addSeparator();

      menuToolbars->addAction(getAction("edit-toolbars"));

      menuView->addMenu(menuToolbars);

      menuWorkspaces = new QMenu();
      connect(menuWorkspaces, SIGNAL(aboutToShow()), SLOT(showWorkspaceMenu()));
      menuView->addMenu(menuWorkspaces);

      a = getAction("toggle-statusbar");
      a->setCheckable(true);
      a->setChecked(preferences.getBool(PREF_UI_APP_SHOWSTATUSBAR));
      menuView->addAction(a);

      menuView->addSeparator();
      a = getAction("split-h");
      a->setCheckable(true);
      menuView->addAction(a);
      a = getAction("split-v");
      a->setCheckable(true);
      menuView->addAction(a);

      menuView->addSeparator();
      menuView->addAction(getAction("show-invisible"));
      menuView->addAction(getAction("show-unprintable"));
      menuView->addAction(getAction("show-frames"));
      menuView->addAction(getAction("show-pageborders"));
      menuView->addAction(getAction("mark-irregular"));
      menuView->addSeparator();

      a = getAction("fullscreen");
      a->setCheckable(true);
      a->setChecked(false);
#ifndef Q_OS_MAC
      menuView->addAction(a);
#endif

      //---------------------
      //    Menu Add
      //---------------------

      menuAdd = mb->addMenu("");
      menuAdd->setObjectName("Add");

      menuAddPitch = new QMenu();
      menuAddPitch->addAction(getAction("note-input"));
      menuAddPitch->addSeparator();

      for (int i = 0; i < 7; ++i) {
            char buffer[8];
            snprintf(buffer, sizeof buffer, "note-%c", "cdefgab"[i]);
            a = getAction(buffer);
            menuAddPitch->addAction(a);
            }
      menuAddPitch->addSeparator();
      for (int i = 0; i < 7; ++i) {
            char buffer[8];
            snprintf(buffer, sizeof buffer, "chord-%c", "cdefgab"[i]);
            a = getAction(buffer);
            menuAddPitch->addAction(a);
            }
      menuAdd->addMenu(menuAddPitch);

      menuAddInterval = new QMenu();
      for (int i = 1; i < 10; ++i) {
            char buffer[16];
            snprintf(buffer, sizeof buffer, "interval%d", i);
            a = getAction(buffer);
            menuAddInterval->addAction(a);
            }
      menuAddInterval->addSeparator();
      for (int i = 2; i < 10; ++i) {
            char buffer[16];
            snprintf(buffer, sizeof buffer, "interval-%d", i);
            a = getAction(buffer);
            menuAddInterval->addAction(a);
            }
      menuAdd->addMenu(menuAddInterval);

      menuTuplet = new QMenu();
      for (auto i : { "duplet", "triplet", "quadruplet", "quintuplet", "sextuplet",
            "septuplet", "octuplet", "nonuplet", "tuplet-dialog" })
            menuTuplet->addAction(getAction(i));
      menuAdd->addMenu(menuTuplet);

      menuAdd->addSeparator();

      menuAddMeasures = new QMenu("");
      menuAddMeasures->addAction(getAction("insert-measure"));
      menuAddMeasures->addAction(getAction("insert-measures"));
      menuAddMeasures->addSeparator();
      menuAddMeasures->addAction(getAction("append-measure"));
      menuAddMeasures->addAction(getAction("append-measures"));
      menuAdd->addMenu(menuAddMeasures);

      menuAddFrames = new QMenu();
      menuAddFrames->addAction(getAction("insert-hbox"));
      menuAddFrames->addAction(getAction("insert-vbox"));
      menuAddFrames->addAction(getAction("insert-textframe"));
      if (enableExperimental)
            menuAddFrames->addAction(getAction("insert-fretframe"));
      menuAddFrames->addSeparator();
      menuAddFrames->addAction(getAction("append-hbox"));
      menuAddFrames->addAction(getAction("append-vbox"));
      menuAddFrames->addAction(getAction("append-textframe"));
      menuAdd->addMenu(menuAddFrames);

      menuAddText = new QMenu();
      menuAddText->addAction(getAction("title-text"));
      menuAddText->addAction(getAction("subtitle-text"));
      menuAddText->addAction(getAction("composer-text"));
      menuAddText->addAction(getAction("poet-text"));
      menuAddText->addAction(getAction("part-text"));
      menuAddText->addSeparator();
      menuAddText->addAction(getAction("system-text"));
      menuAddText->addAction(getAction("staff-text"));
      menuAddText->addAction(getAction("expression-text"));
      menuAddText->addAction(getAction("chord-text"));
      menuAddText->addAction(getAction("rehearsalmark-text"));
      menuAddText->addAction(getAction("instrument-change-text"));
      menuAddText->addAction(getAction("fingering-text"));
      menuAddText->addSeparator();
      menuAddText->addAction(getAction("lyrics"));
      menuAddText->addAction(getAction("sticking-text"));
      menuAddText->addAction(getAction("roman-numeral-text"));
      menuAddText->addAction(getAction("nashville-number-text"));
      menuAddText->addAction(getAction("figured-bass"));
      menuAddText->addAction(getAction("tempo"));
      menuAdd->addMenu(menuAddText);

      menuAddLines = new QMenu();
      menuAddLines->addAction(getAction("add-slur"));
      menuAddLines->addAction(getAction("add-hairpin"));
      menuAddLines->addAction(getAction("add-hairpin-reverse"));
      menuAddLines->addAction(getAction("add-8va"));
      menuAddLines->addAction(getAction("add-8vb"));
      menuAddLines->addAction(getAction("add-noteline"));
      menuAdd->addMenu(menuAddLines);

      //---------------------
      //    Menu Format
      //---------------------

      menuFormat = mb->addMenu("");
      menuFormat->setObjectName("Format");

      menuFormat->addAction(getAction("edit-style"));
      QAction* pageSettingsAction = getAction("page-settings");
      // in some locale (fr), page settings ends up in Application menu on mac
      // this line prevents it.
      pageSettingsAction->setMenuRole(QAction::NoRole);
      menuFormat->addAction(pageSettingsAction);
      menuFormat->addSeparator();

      menuFormat->addAction(getAction("add-remove-breaks"));

      menuStretch = new QMenu(tr("&Stretch"));
      for (auto i : { "stretch+", "stretch-", "reset-stretch" })
            menuStretch->addAction(getAction(i));
      Workspace::addMenuAndString(menuStretch, "menu-stretch");
      menuFormat->addMenu(menuStretch);
      menuFormat->addSeparator();

      menuFormat->addAction(getAction("reset-text-style-overrides"));
      menuFormat->addAction(getAction("reset-beammode"));
      menuFormat->addAction(getAction("reset"));
      menuFormat->addSeparator();

      if (enableExperimental)
            menuFormat->addAction(getAction("edit-harmony"));

      menuFormat->addSeparator();
      menuFormat->addAction(getAction("load-style"));
      menuFormat->addAction(getAction("save-style"));

      //---------------------
      //    Menu Tools
      //---------------------

      menuTools = mb->addMenu("");
      menuTools->setObjectName("Tools");

      menuTools->addAction(getAction("transpose"));
      menuTools->addSeparator();
      menuTools->addAction(getAction("explode"));
      menuTools->addAction(getAction("implode"));

      menuTools->addAction(getAction("realize-chord-symbols"));

      menuVoices = new QMenu("");
      for (auto i : { "voice-x12", "voice-x13", "voice-x14", "voice-x23", "voice-x24", "voice-x34" })
            menuVoices->addAction(getAction(i));
      menuTools->addMenu(menuVoices);

      menuMeasure = new QMenu("");
      for (auto i : { "split-measure", "join-measures" })
            menuMeasure->addAction(getAction(i));
      menuTools->addMenu(menuMeasure);
      menuTools->addAction(getAction("time-delete"));

      menuTools->addSeparator();

      menuTools->addAction(getAction("slash-fill"));
      menuTools->addAction(getAction("slash-rhythm"));
      menuTools->addSeparator();

      menuTools->addAction(getAction("pitch-spell"));
      menuTools->addAction(getAction("reset-groupings"));
      menuTools->addAction(getAction("resequence-rehearsal-marks"));
      menuTools->addAction(getAction("unroll-repeats"));
      menuTools->addSeparator();

      menuTools->addAction(getAction("copy-lyrics-to-clipboard"));
      menuTools->addAction(getAction("fotomode"));

      menuTools->addAction(getAction("del-empty-measures"));

      if (MuseScore::unstable()) {
            menuTools->addSeparator();
            a = getAction("toggle-script-recorder");
            a->setCheckable(true);
            menuTools->addAction(a);
            }

      //---------------------
      //    Menu Plugins
      //---------------------

#ifdef SCRIPT_INTERFACE
      menuPlugins = mb->addMenu("");
      menuPlugins->setObjectName("Plugins");

      menuPlugins->addAction(getAction("plugin-manager"));

      a = getAction("plugin-creator");
      a->setCheckable(true);
      menuPlugins->addAction(a);

      menuPlugins->addSeparator();
#endif

      //---------------------
      //    Menu Debug
      //---------------------

#ifndef NDEBUG
      menuDebug = mb->addMenu("Debug");
      menuDebug->setObjectName("Debug");
      a = getAction("no-horizontal-stretch");
      a->setCheckable(true);
      menuDebug->addAction(a);
      a = getAction("no-vertical-stretch");
      a->setCheckable(true);
      menuDebug->addAction(a);
      menuDebug->addSeparator();
      a = getAction("show-segment-shapes");
      a->setCheckable(true);
      menuDebug->addAction(a);
      a = getAction("show-skylines");
      a->setCheckable(true);
      a->setChecked(MScore::showSkylines);
      menuDebug->addAction(a);
      a = getAction("show-bounding-rect");
      a->setCheckable(true);
      menuDebug->addAction(a);
      a = getAction("show-system-bounding-rect");
      a->setCheckable(true);
      menuDebug->addAction(a);
      a = getAction("show-corrupted-measures");
      a->setCheckable(true);
      a->setChecked(true);
      menuDebug->addAction(a);
      a = getAction("relayout");
      menuDebug->addAction(a);
      a = getAction("qml-reload-source");
      menuDebug->addAction(a);
      Workspace::addMenuAndString(menuDebug, "menu-debug");
#endif

      //---------------------
      //    Menu Help
      //---------------------

      mb->addSeparator();
      menuHelp = mb->addMenu("");
      menuHelp->setObjectName("Help");

#if 0
      if (_helpEngine) {
            HelpQuery* hw = new HelpQuery(menuHelp);
            menuHelp->addAction(hw);
            connect(menuHelp, SIGNAL(aboutToShow()), hw, SLOT(setFocus()));
            }
#endif
      //menuHelp->addAction(getAction("help"));
      onlineHandbookAction = new QAction("", 0);
      connect(onlineHandbookAction, SIGNAL(triggered()), this, SLOT(helpBrowser1()));
      onlineHandbookAction->setMenuRole(QAction::NoRole);
      menuHelp->addAction(onlineHandbookAction);
      Workspace::addActionAndString(onlineHandbookAction, "online-handbook");

      menuTours = new QMenu();
      a = getAction("show-tours");
      a->setChecked(preferences.getBool(PREF_UI_APP_STARTUP_SHOWTOURS));
      menuTours->addAction(a);
      menuTours->addAction(getAction("reset-tours"));
      menuHelp->addMenu(menuTours);

      menuHelp->addSeparator();

      aboutAction = new QAction("", 0);

      aboutAction->setMenuRole(QAction::AboutRole);
      connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
      menuHelp->addAction(aboutAction);
      Workspace::addActionAndString(aboutAction, "about");

      aboutQtAction = new QAction("", 0);
      aboutQtAction->setMenuRole(QAction::AboutQtRole);
      connect(aboutQtAction, SIGNAL(triggered()), this, SLOT(aboutQt()));
      menuHelp->addAction(aboutQtAction);
      Workspace::addActionAndString(aboutQtAction, "about-qt");

      aboutMusicXMLAction = new QAction("", 0);
      //aboutMusicXMLAction->setMenuRole(QAction::ApplicationSpecificRole);
      aboutMusicXMLAction->setMenuRole(QAction::NoRole);
      connect(aboutMusicXMLAction, SIGNAL(triggered()), this, SLOT(aboutMusicXML()));
      menuHelp->addAction(aboutMusicXMLAction);
      Workspace::addActionAndString(aboutMusicXMLAction, "about-musicxml");

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
#if (!defined(FOR_WINSTORE)) && (!defined(WIN_PORTABLE) && 0)
      checkForUpdateAction = new QAction("", 0);
      connect(checkForUpdateAction, SIGNAL(triggered()), this, SLOT(checkForUpdatesUI()));
      checkForUpdateAction->setMenuRole(QAction::NoRole);
      menuHelp->addAction(checkForUpdateAction);
      Workspace::addActionAndString(checkForUpdateAction, "check-update");
#endif
#endif
      menuHelp->addSeparator();

      askForHelpAction = new QAction("", 0);
      connect(askForHelpAction, SIGNAL(triggered()), this, SLOT(askForHelp()));
      askForHelpAction->setMenuRole(QAction::NoRole);
      menuHelp->addAction(askForHelpAction);
      Workspace::addActionAndString(askForHelpAction, "ask-help");

      reportBugAction = new QAction("", 0);
      connect(reportBugAction, &QAction::triggered, this, [this]{ reportBug("menu"); });
      reportBugAction->setMenuRole(QAction::NoRole);
      menuHelp->addAction(reportBugAction);
      Workspace::addActionAndString(reportBugAction, "report-bug");

#if 0
      leaveFeedbackAction = new QAction("", 0);
      connect(leaveFeedbackAction, &QAction::triggered, this, [this]{ leaveFeedback("menu"); });
      leaveFeedbackAction->setMenuRole(QAction::NoRole);
      menuHelp->addAction(leaveFeedbackAction);
      Workspace::addActionAndString(leaveFeedbackAction, "leave-feedback");
#endif

      menuHelp->addSeparator();
      menuHelp->addAction(getAction("resource-manager"));
      menuHelp->addSeparator();

      revertToFactoryAction = new QAction("", 0);
      connect(revertToFactoryAction, SIGNAL(triggered()), this, SLOT(resetAndRestart()));
      revertToFactoryAction->setMenuRole(QAction::NoRole);
      menuHelp->addAction(revertToFactoryAction);
      Workspace::addActionAndString(revertToFactoryAction, "revert-factory");

      Workspace::addRemainingFromMenuBar(mb);

      // Add all menus to workspace for loading
      Workspace::addMenuAndString(menuFile,        "menu-file");
      Workspace::addMenuAndString(openRecent,      "menu-open-recent");
      Workspace::addMenuAndString(menuEdit,        "menu-edit");
      Workspace::addMenuAndString(menuView,        "menu-view");
      Workspace::addMenuAndString(menuToolbars,    "menu-toolbars");
      Workspace::addMenuAndString(menuWorkspaces,  "menu-workspaces");
      Workspace::addMenuAndString(menuAdd,         "menu-add");
      Workspace::addMenuAndString(menuAddMeasures, "menu-add-measures");
      Workspace::addMenuAndString(menuAddFrames,   "menu-add-frames");
      Workspace::addMenuAndString(menuAddText,     "menu-add-text");
      Workspace::addMenuAndString(menuAddLines,    "menu-add-lines");
      Workspace::addMenuAndString(menuAddPitch,    "menu-add-pitch");
      Workspace::addMenuAndString(menuAddInterval, "menu-add-interval");
      Workspace::addMenuAndString(menuTuplet,      "menu-tuplet");
      Workspace::addMenuAndString(menuFormat,      "menu-format");
      Workspace::addMenuAndString(menuTools,       "menu-tools");
      Workspace::addMenuAndString(menuVoices,      "menu-voices");
      Workspace::addMenuAndString(menuMeasure,     "menu-measure");
#ifdef SCRIPT_INTERFACE
      Workspace::addMenuAndString(menuPlugins,     "menu-plugins");
#endif
      Workspace::addMenuAndString(menuHelp,        "menu-help");
      Workspace::addMenuAndString(menuTours,       "menu-tours");

      Workspace::writeGlobalMenuBar(mb);

      if (!MScore::noGui) {
            retranslate();
            //accessibility for menus
            for (QMenu*& menu : mb->findChildren<QMenu*>()) {
                  menu->setAccessibleName(menu->objectName());
                  menu->setAccessibleDescription(Shortcut::getMenuShortcutString(menu));
                  }
            }

      setCentralWidget(envelope);

      if (!MScore::noGui)
            preferencesChanged();

      if (seq) {
            connect(seq, SIGNAL(started()), SLOT(seqStarted()));
            connect(seq, SIGNAL(stopped()), SLOT(seqStopped()));
            }
      loadScoreList();

      QClipboard* cb = QApplication::clipboard();
      connect(cb, SIGNAL(dataChanged()), SLOT(clipboardChanged()));
      connect(cb, SIGNAL(selectionChanged()), SLOT(clipboardChanged()));

      autoSaveTimer = new QTimer(this);
      autoSaveTimer->setSingleShot(true);
      connect(autoSaveTimer, SIGNAL(timeout()), this, SLOT(autoSaveTimerTimeout()));
      initOsc();
      startAutoSave();

#ifndef NDEBUG
      // for debugging, but possible may need
      QInputMethod* im = QGuiApplication::inputMethod();
      connect(im, SIGNAL(anchorRectangleChanged()), SLOT(inputMethodAnchorRectangleChanged()));
      connect(im, SIGNAL(animatingChanged()), SLOT(inputMethodAnimatingChanged()));
      connect(im, SIGNAL(cursorRectangleChanged()), SLOT(inputMethodCursorRectangleChanged()));
//signal does not exist:
//      connect(im, SIGNAL(inputDirectionChanged(Qt::LayoutDirection newDirection)), SLOT(inputMethodInputDirectionChanged(Qt::LayoutDirection newDirection)));
      connect(im, SIGNAL(inputItemClipRectangleChanged()), SLOT(inputMethodInputItemClipRectangleChanged()));
      connect(im, SIGNAL(keyboardRectangleChanged()), SLOT(inputMethodKeyboardRectangleChanged()));
      connect(im, SIGNAL(localeChanged()), SLOT(inputMethodLocaleChanged()));
      connect(im, SIGNAL(visibleChanged()), SLOT(inputMethodVisibleChanged()));
#endif

      if (enableExperimental) {
            cornerLabel = new QLabel(this);
            cornerLabel->setScaledContents(true);
            cornerLabel->setPixmap(QPixmap(":/data/mscore.png"));
            cornerLabel->setGeometry(width() - 48, 0, 48, 48);
            }
#if defined(WIN_SPARKLE_ENABLED)
      autoUpdater.reset(new WinSparkleAutoUpdater);
#elif defined(MAC_SPARKLE_ENABLED)
      autoUpdater.reset(new SparkleAutoUpdater);
#endif
      connect(this, SIGNAL(musescoreWindowWasShown()), this, SLOT(checkForUpdates()),
            Qt::ConnectionType(Qt::QueuedConnection | Qt::UniqueConnection));

      if (!converterMode && !pluginMode) {
            _loginManager = new LoginManager(getAction(saveOnlineMenuItem), this);
            const bool loadSuccess = _loginManager->load();

            if (cliSaveOnline && !loadSuccess) {
                  fprintf(stderr, "%s\n", qPrintable(tr("No login credentials stored. Please sign in via the GUI.")));
                  ::exit(EXIT_FAILURE);
                  }
            }

      connect(qApp, &QGuiApplication::focusWindowChanged, this, &MuseScore::onFocusWindowChanged);
      }

MuseScore::~MuseScore()
      {
#ifdef Q_OS_MAC // On Mac, remove the Cocoa Notification Observers
      // (Currently, there is only one for Dark Mode)
      CocoaBridge::removeObservers();
#endif

      if (autoUpdater)
            autoUpdater->cleanup();

      delete synti;
      synti = nullptr;

      // A crash is possible if paletteWorkspace gets
      // deleted before paletteWidget, so force the widget
      // be deleted before paletteWorkspace.
      delete paletteWidget;
      paletteWidget = nullptr;
      }

//---------------------------------------------------------
//   playPanelInterface
//---------------------------------------------------------

IPlayPanel* MuseScore::playPanelInterface() const
      {
      return playPanel;
      }

//---------------------------------------------------------
//   onFocusWindowChanged
//---------------------------------------------------------

void MuseScore::onFocusWindowChanged(QWindow* w)
      {
      const QWindow* mscoreWindow = windowHandle();

      if (!QApplication::focusWidget() && _lastFocusWindow
         && ((!w && _lastFocusWindow != mscoreWindow) || (w == mscoreWindow && _lastFocusWindowIsQQuickView))) {
            // Switch to temporary window to work around inability to return focus to QML-based windows
            QWidget* tmpContainer = createWindowContainer(new QWindow, this);
            tmpContainer->show();
            tmpContainer->setFocus();

            QTimer::singleShot(0, this, [this, tmpContainer]() {
                  focusScoreView();
                  tmpContainer->deleteLater();
                  });
            }

      _lastFocusWindow = w;
      _lastFocusWindowIsQQuickView = qobject_cast<QQuickView*>(_lastFocusWindow);
      }

//---------------------------------------------------------
//   showError
//---------------------------------------------------------

void MuseScore::showError()
      {
      static QErrorMessage* msg = 0;
      if (msg == 0)
            msg = new QErrorMessage(this);
      msg->showMessage(qApp->translate("error", MScore::errorMessage()), MScore::errorGroup());
      MScore::setError(MS_NO_ERROR);
      }

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void MuseScore::retranslate()
      {
      setMenuTitles();
      _positionLabel->setToolTip(tr("Measure:Beat:Tick"));
      pref->setText(tr("&Preferences…"));
      aboutAction->setText(tr("&About…"));
      aboutQtAction->setText(tr("About &Qt…"));
      aboutMusicXMLAction->setText(tr("About &MusicXML…"));
      onlineHandbookAction->setText(tr("&Online Handbook"));
      if (checkForUpdateAction)
            checkForUpdateAction->setText(tr("Check for &Update"));
      askForHelpAction->setText(tr("Ask for Help"));
      reportBugAction->setText(tr("Report a Bug"));
#if 0
      leaveFeedbackAction->setText(tr("Feedback"));
#endif
      revertToFactoryAction->setText(tr("Revert to Factory Settings"));

      fileTools->setWindowTitle(tr("File Operations"));
      transportTools->setWindowTitle(tr("Playback Controls"));
      cpitchTools->setWindowTitle(tr("Concert Pitch"));
      fotoTools->setWindowTitle(tr("Image Capture"));
      entryTools->setWindowTitle(tr("Note Input"));
#if 0
      feedbackTools->setWindowTitle(tr("Feedback"));
#endif
      workspacesTools->setWindowTitle(tr("Workspaces"));

      // keep translatable (con)texts in sync with those from zoombox.cpp
      zoomBox->setAccessibleName(qApp->translate("Ms::ZoomBox", "Zoom"));
      zoomBox->setWhatsThis(qApp->translate("Ms::ZoomBox", "Zoom"));
      zoomBox->setAccessibleName(qApp->translate("Ms::ZoomBox", "Zoom"));
      zoomBox->setItemText(zoomBox->findData(int(ZoomIndex::ZOOM_PAGE_WIDTH)), qApp->translate("magTable", "Page Width"));
      zoomBox->setItemText(zoomBox->findData(int(ZoomIndex::ZOOM_WHOLE_PAGE)), qApp->translate("magTable", "Whole Page"));
      zoomBox->setItemText(zoomBox->findData(int(ZoomIndex::ZOOM_TWO_PAGES)), qApp->translate("magTable", "Two Pages"));

      viewModeCombo->setAccessibleName(tr("View Mode"));
      viewModeCombo->setItemText(viewModeCombo->findData(int(LayoutMode::PAGE)), tr("Page View"));
      viewModeCombo->setItemText(viewModeCombo->findData(int(LayoutMode::LINE)), tr("Continuous View"));
      viewModeCombo->setItemText(viewModeCombo->findData(int(LayoutMode::SYSTEM)), tr("Single Page"));
#ifdef NDEBUG
      if (enableExperimental)
#endif
            viewModeCombo->setItemText(viewModeCombo->findData(int(LayoutMode::FLOAT)), tr("Floating"));

      showMidiImportButton->setText(tr("Show MIDI import panel"));

      Shortcut::retranslate();
      WorkspacesManager::retranslateAll();

      if (paletteWorkspace)
          paletteWorkspace->retranslate();
      }

//---------------------------------------------------------
//   setMenuTitles
//---------------------------------------------------------

void MuseScore::setMenuTitles()
      {
      // The list below is not static, both strings
      // and menu pointers need refreshing.
      const std::initializer_list<std::pair<QMenu*, QString>> titles {
            { menuFile,             tr("&File")             },
            { openRecent,           tr("Open &Recent")      },
            { menuEdit,             tr("&Edit")             },
            { menuView,             tr("&View")             },
            { menuToolbars,         tr("&Toolbars")         },
            { menuWorkspaces,       tr("W&orkspaces")       },
            { menuAdd,              tr("&Add")              },
            { menuAddMeasures,      tr("&Measures")         },
            { menuAddFrames,        tr("&Frames")           },
            { menuAddText,          tr("&Text")             },
            { menuAddLines,         tr("&Lines")            },
            { menuAddPitch,         tr("N&otes")            },
            { menuAddInterval,      tr("&Intervals")        },
            { menuTuplet,           tr("T&uplets")          },
            { menuFormat,           tr("F&ormat")           },
            { menuStretch,          tr("&Stretch")          },
            { menuTools,            tr("&Tools")            },
            { menuVoices,           tr("&Voices")           },
            { menuMeasure,          tr("&Measure")          },
#ifdef SCRIPT_INTERFACE
            { menuPlugins,          tr("&Plugins")          },
#endif
#ifndef NDEBUG
            { menuDebug,            "Debug"                 }, // not translated
#endif
            { menuHelp,             tr("&Help")             },
            { menuTours,            tr("&Tours")            }
            };

      for (const auto& t : titles) {
            QMenu* m = t.first;
            if (m)
                  m->setTitle(t.second);
            }
      }

//---------------------------------------------------------
//   updateMenu
//---------------------------------------------------------

void MuseScore::updateMenu(QMenu*& menu, QString menu_id, QString name)
      {
      menu = Workspace::findMenuFromString(menu_id);
      if (menu) {
            if (name != "")
                  menu->setObjectName(name);
            }
      }

//---------------------------------------------------------
//   updateMenus
//---------------------------------------------------------

void MuseScore::updateMenus()
      {
      updateMenu(menuFile,        "menu-file",         "File");
      updateMenu(openRecent,      "menu-open-recent",  "");
      updateMenu(menuEdit,        "menu-edit",         "Edit");
      updateMenu(menuView,        "menu-view",         "View");
      updateMenu(menuToolbars,    "menu-toolbars",     "");
      updateMenu(menuWorkspaces,  "menu-workspaces",   "");
      updateMenu(menuAdd,         "menu-add",          "Add");
      updateMenu(menuAddMeasures, "menu-add-measures", "");
      updateMenu(menuAddFrames,   "menu-add-frames",   "");
      updateMenu(menuAddText,     "menu-add-text",     "");
      updateMenu(menuAddLines,    "menu-add-lines",    "");
      updateMenu(menuAddPitch,    "menu-add-pitch",    "");
      updateMenu(menuAddInterval, "menu-add-interval", "");
      updateMenu(menuTuplet,      "menu-tuplet",       "");
      updateMenu(menuFormat,      "menu-format",       "Format");
      updateMenu(menuStretch,     "menu-stretch",      "");
      updateMenu(menuTools,       "menu-tools",        "Tools");
      updateMenu(menuVoices,      "menu-voices",       "");
      updateMenu(menuMeasure,     "menu-measure",      "");
#ifdef SCRIPT_INTERFACE
      updateMenu(menuPlugins,     "menu-plugins",      "Plugins");
#endif
      updateMenu(menuHelp,        "menu-help",         "Help");
      updateMenu(menuTours,       "menu-tours",        "");
#ifndef NDEBUG
      updateMenu(menuDebug,       "menu-debug",        "Debug");
#endif
      connect(openRecent,     SIGNAL(aboutToShow()),       SLOT(openRecentMenu()));
      connect(openRecent,     SIGNAL(triggered(QAction*)), SLOT(selectScore(QAction*)));
      connect(menuWorkspaces, SIGNAL(aboutToShow()),       SLOT(showWorkspaceMenu()));
      setMenuTitles();
#ifdef SCRIPT_INTERFACE
      addPluginMenuEntries();
#endif
      }

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void MuseScore::resizeEvent(QResizeEvent*)
      {
      if (enableExperimental) {
            cornerLabel->setGeometry(width() - 48, 0, 48, 48);
            }
      }

//---------------------------------------------------------
//   startAutoSave
//---------------------------------------------------------

void MuseScore::startAutoSave()
      {
      if (preferences.getBool(PREF_APP_AUTOSAVE_USEAUTOSAVE)) {
            int t = preferences.getInt(PREF_APP_AUTOSAVE_AUTOSAVETIME) * 60 * 1000;
            autoSaveTimer->start(t);
            }
      else
            autoSaveTimer->stop();
      }

//---------------------------------------------------------
//   getLocaleISOCode
//---------------------------------------------------------

QString MuseScore::getLocaleISOCode() const
      {
      QString l = localeName();
      if (l.toLower() == "system")
            return QLocale::system().name();
      return l;
      }

//---------------------------------------------------------
//   helpBrowser1
//    show online help
//---------------------------------------------------------

void MuseScore::helpBrowser1() const
      {
      QString lang = getLocaleISOCode();

      if (MScore::debugMode)
            qDebug("open online handbook for language <%s>", qPrintable(lang));
      QString help = QString("https://musescore.org/redirect/help?tag=handbook&locale=%1").arg(getLocaleISOCode());
      //try to find an exact match
      bool found = false;
      const QList<LanguageItem> langItems = _languages;
      for (const LanguageItem& item : langItems) {
            if (item.key == lang) {
                  QString handbook = item.handbook;
                  if (!handbook.isNull()) {
                      help = item.handbook;
                      found = true;
                      }
                  break;
                  }
            }
      //try a to find a match on first two letters
      if (!found && lang.size() > 2) {
            lang = lang.left(2);
            for (const LanguageItem& item : langItems) {
                  if (item.key == lang){
                      QString handbook = item.handbook;
                      if (!handbook.isNull())
                          help = item.handbook;
                      break;
                      }
                  }
            }

      //track visits. see: http://www.google.com/support/googleanalytics/bin/answer.py?answer=55578
      help += '&';
      help += getUtmParameters("menu");
      QDesktopServices::openUrl(QUrl(help));
      }

//---------------------------------------------------------
//   resetAndRestart
//---------------------------------------------------------

void MuseScore::resetAndRestart()
      {
      int ret = QMessageBox::question(0, tr("Are you sure?"),
                  tr("This will reset all your preferences.\n"
                   "Custom palettes, custom shortcuts, and the list of recent scores will be deleted. "
                   "MuseScore will restart with its default settings.\n"
                   "Reverting will not remove any scores from your computer.\n"
                   "Are you sure you want to proceed?"),
                   QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
      if (ret == QMessageBox::Yes ) {
             close();
             QStringList args("-F");
             QProcess::startDetached(qApp->arguments()[0], args);
             }
      }

//---------------------------------------------------------
//   aboutQt
//---------------------------------------------------------

void MuseScore::aboutQt()
      {
      QMessageBox::aboutQt(this, QString("About Qt"));
      }

//---------------------------------------------------------
//   aboutMusicXML
//---------------------------------------------------------

void MuseScore::aboutMusicXML()
      {
      AboutMusicXMLBoxDialog ab;
      ab.show();
      ab.exec();
      }

//---------------------------------------------------------
//   selectScore
//    "open recent"
//---------------------------------------------------------

void MuseScore::selectScore(QAction* action)
      {
      QVariant actionData = action->data();

      if (!actionData.isValid())
            return;

      switch (actionData.type()) {
            case QVariant::String: {
                  if (actionData.toString() == "clear-recent") {
                        _recentScores.clear();
#ifdef Q_OS_MAC
                        CocoaBridge::clearRecentFiles();
#endif

                        if (startcenter)
                              startcenter->updateRecentScores();
                        }
                  break;
                  }
            case QVariant::Map: {
                  QVariantMap pathMap = actionData.toMap();

                  openScore(pathMap.value("filePath").toString());
                  break;
                  }
            default:
                  return;
            }
      }

//---------------------------------------------------------
//   selectionChanged
//---------------------------------------------------------

void MuseScore::selectionChanged(SelState selectionState)
      {
      bool enable = selectionState != SelState::NONE;
      getAction("cut")->setEnabled(enable);
      getAction("copy")->setEnabled(enable);
      getAction("select-similar-range")->setEnabled(selectionState == SelState::RANGE);
      if (pianorollEditor)
            pianorollEditor->changeSelection(selectionState);
      if (drumrollEditor)
            drumrollEditor->changeSelection(selectionState);
      if (timeline())
            timeline()->changeSelection(selectionState);
      if (_pianoTools && _pianoTools->isVisible()) {
            if (cs) {
                  if (seq->isStopped())
                        _pianoTools->changeSelection(cs->selection());
                  }
            else
                  _pianoTools->clearSelection();
            }
      if (_inspector)
            updateInspector();
      }

//---------------------------------------------------------
//   updatePaletteBeamMode
//
//   Updates the selected index of the Beam Properties
//   palette to reflect the beam mode of the selected
//   chord rest
//---------------------------------------------------------

void MuseScore::updatePaletteBeamMode()
      {
      if (paletteWorkspace && cs)
            paletteWorkspace->updateCellsState(cs->selection());
      }

//---------------------------------------------------------
//   updateInspector
//---------------------------------------------------------

void MuseScore::updateInspector()
      {
      // skip update if no inspector, or if inspector is hidden and there is a GUI
      // (important not to skip when running test scripts)
      if (_inspector && (_inspector->isVisible() || MScore::testMode || scriptTestMode))
            _inspector->update(cs);
      }

//---------------------------------------------------------
//   updateShadowNote
//   update the shadow note in the current ScoreView
//---------------------------------------------------------

void MuseScore::updateShadowNote()
      {
      currentScoreView()->updateShadowNotes();
      }

//---------------------------------------------------------
//   appendScore
//    append score to project list
//---------------------------------------------------------

int MuseScore::appendScore(MasterScore* score)
      {
      int index = scoreList.size();
      for (int i = 0; i < scoreList.size(); ++i) {
            if ((!score->importedFilePath().isEmpty()
                 && scoreList[i]->importedFilePath() == score->importedFilePath())
                    || (scoreList[i]->fileInfo()->canonicalFilePath() == score->fileInfo()->canonicalFilePath() && score->fileInfo()->exists())) {
                  removeTab(i);
                  index = i;
                  break;
                  }
            }
      scoreList.insert(index, score);
      scoreWasShown[score] = false;
      tab1->insertTab(score);
      if (tab2)
            tab2->insertTab(score);
      return index;
      }

//---------------------------------------------------------
//   addRecentScore
//---------------------------------------------------------

void MuseScore::addRecentScore(Score* score)
      {
      QString path = score->importedFilePath(); // defined for scores imported from e.g. MIDI files
      addRecentScore(path);
      path = score->masterScore()->fileInfo()->absoluteFilePath();
      addRecentScore(path);
      if (startcenter)
            startcenter->updateRecentScores();
      }

void MuseScore::addRecentScore(const QString& scorePath)
      {
      if (scorePath.isEmpty())
            return;

      QFileInfo fi(scorePath);
      QString absoluteFilePath = fi.absoluteFilePath();
      _recentScores.removeAll(absoluteFilePath);
      _recentScores.prepend(absoluteFilePath);
#ifdef Q_OS_MAC
      CocoaBridge::addRecentFile(absoluteFilePath);
#endif

      while (_recentScores.size() > RECENT_LIST_SIZE)
            _recentScores.removeLast();
      }

#if 0
//---------------------------------------------------------
//   updateTabNames
//---------------------------------------------------------

void MuseScore::updateTabNames()
      {
      for (int i = 0; i < tab1->count(); ++i) {
            ScoreView* view = tab1->view(i);
            if (view)
                  tab1->setTabText(i, view->score()->masterScore()->fileInfo()->completeBaseName());
            }
      if (tab2) {
            for (int i = 0; i < tab2->count(); ++i) {
                  ScoreView* view = tab2->view(i);
                  if (view)
                        tab2->setTabText(i, view->score()->masterScore()->fileInfo()->completeBaseName());
                  }
            }
      }
#endif

//---------------------------------------------------------
//   loadScoreList
//    read list of "Recent Scores"
//---------------------------------------------------------

void MuseScore::loadScoreList()
      {
      if (useFactorySettings)
            return;
      QSettings s;
      for (int i = RECENT_LIST_SIZE-1; i >= 0; --i) {
            QString path = s.value(QString("recent-%1").arg(i),"").toString();
            if (!path.isEmpty() && QFileInfo::exists(path)) {
                  _recentScores.removeAll(path);
                  _recentScores.prepend(path);
                  }
            }
      }

//---------------------------------------------------------
//   openRecentMenu
//---------------------------------------------------------

void MuseScore::openRecentMenu()
      {
      openRecent->clear();
      bool hasAnyRecentFiles = false;
      for (QFileInfo& fi : recentScores()) {
            QAction* action = openRecent->addAction(fi.fileName().replace("&", "&&"));  // show filename only

            QString filePath = fi.canonicalFilePath();

            QVariantMap actionData;
            actionData.insert("actionName", "open-recent");
            actionData.insert("filePath", filePath);

            action->setData(actionData);
            action->setToolTip(filePath);
            hasAnyRecentFiles = true;
            }
      if (hasAnyRecentFiles) {
            openRecent->addSeparator();
            QAction* action = openRecent->addAction(tr("Clear Recent Files"));
            action->setData("clear-recent");
            }
      else {
            // Don't leave the menu empty, but add a hint
            QAction* hint = openRecent->addAction(tr("No recent files"));
            hint->setEnabled(false);
            }
      }

//---------------------------------------------------------
//   reloadInstrumentTemplates
//---------------------------------------------------------

void MuseScore::reloadInstrumentTemplates()
      {
      clearInstrumentTemplates();
      // load cascading instrument templates
      loadInstrumentTemplates(preferences.getString(PREF_APP_PATHS_INSTRUMENTLIST1));
      QString list2 = preferences.getString(PREF_APP_PATHS_INSTRUMENTLIST2);
      if (!list2.isEmpty())
            loadInstrumentTemplates(list2);

      // load instrument templates from extension
      QStringList extensionDir = Extension::getDirectoriesByType(Extension::instrumentsDir);
      QStringList filter("*.xml");
      for (const QString& s : qAsConst(extensionDir)) {
            QDir extDir(s);
            extDir.setNameFilters(filter);
            auto instFiles = extDir.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::Readable);
            for (auto& instFile : instFiles)
                  loadInstrumentTemplates(instFile.absoluteFilePath());
            }

      // load cascading score orders
      loadScoreOrders(preferences.getString(PREF_APP_PATHS_SCOREORDERLIST1));
      list2 = preferences.getString(PREF_APP_PATHS_SCOREORDERLIST2);
      if (!list2.isEmpty())
            loadScoreOrders(list2);


      MidiInstr::instrumentTemplatesChanged();
      if (importmidiPanel)
            importmidiPanel->instrumentTemplatesChanged();
      }

//---------------------------------------------------------
//   askMigrateScore
//---------------------------------------------------------

void MuseScore::askMigrateScore(Score* score)
      {
      if (!preferences.getBool(PREF_MIGRATION_DO_NOT_ASK_ME_AGAIN) && score->mscVersion() < MSCVERSION) {
            ScoreMigrationDialog* migrationDialog = new ScoreMigrationDialog(mscore->getQmlUiEngine(), score);
            migrationDialog->show();
            }
      }

//---------------------------------------------------------
//   setCurrentView
//---------------------------------------------------------

void MuseScore::setCurrentScoreView(int idx)
      {
      tab1->blockSignals(ctab != tab1);
      setCurrentView(0, idx);
      tab1->blockSignals(false);

      if (tab2) {
            tab2->blockSignals(ctab != tab2);
            setCurrentView(1, idx);
            tab2->blockSignals(false);
            }
      }

void MuseScore::setCurrentView(int tabIdx, int idx)
      {
      if (idx == -1)
            setCurrentScoreView(nullptr);
      else {
            ScoreTab* tab = tabIdx ? tab2 : tab1;
            if (tab)
                  tab->setCurrentIndex(idx);
            }
      }

//---------------------------------------------------------
//   setCurrentScoreView
//---------------------------------------------------------

void MuseScore::setCurrentScoreView(ScoreView* view)
      {
      cv = view;
      if (cv) {
            ctab = (tab2 && tab2->view() == view) ? tab2 : tab1;
            if (timeline())
                  timeline()->setScoreView(cv);
            if (cv->score() && (cs != cv->score())) {
                  // exit note entry mode
                  if (cv->noteEntryMode()) {
                        cv->cmd(getAction("escape"));
                        qApp->processEvents();
                        }
                  updateInputState(cv->score());
                  }
            cs = cv->score();
            cv->setFocusRect();
            }
      else
            cs = 0;

      updateWindowTitle(cs);
      setWindowModified(cs ? cs->dirty() : false);

      if (ScriptRecorder* rec = getScriptRecorder())
            rec->recordCurrentScoreChange();

      if (cs)
            cs->masterScore()->setPlaybackScore(_playPartOnly ? cs : cs->masterScore());

      // set midi import panel
      QString fileName = cs ? cs->importedFilePath() : "";
      midiPanelOnSwitchToFile(fileName);

      if (enableExperimental) {
            updateLayer();
            updatePlayMode();
            }

      if (seq)
            seq->setScoreView(cv);
      if (playPanel)
            playPanel->setScore(cs);
      if (synthControl)
            synthControl->setScore(cs);
      if (selectionWindow)
            selectionWindow->setScore(cs);
      if (mixer)
            mixer->setScore(cs);
#ifdef OMR
      if (omrPanel) {
            if (cv && cv->omrView())
                  omrPanel->setOmrView(cv->omrView());
            else
                  omrPanel->setOmrView(0);
            }
#endif
      if (!cs) {
            if (_navigator && _navigator->widget()) {
                  navigator()->setScoreView(cv);
                  }
            if (timeline()) {
                  timeline()->setScoreView(cv);
                  timeline()->setScore(0);
                  }
            if (_inspector)
                  _inspector->update(0);
            zoomBox->resetToDefaultLogicalZoom();
            viewModeCombo->setEnabled(false);
            if (_textTools) {
                  _textTools->hide();
                  if (textPalette)
                        textPalette->hide();
                  }
            if (_pianoTools)
                  _pianoTools->hide();
            if (_drumTools)
                  _drumTools->hide();
            changeState(STATE_DISABLED);
            ScoreAccessibility::instance()->clearAccessibilityInfo();
            return;
            }

      viewModeCombo->setEnabled(true);
      updateViewModeCombo();

      selectionChanged(cs->selection().state());

      _sstate = STATE_DISABLED; // defeat optimization
      changeState(cv->mscoreState());

      cv->setFocus(Qt::OtherFocusReason);
      setFocusProxy(cv);

      getAction("file-save")->setEnabled(cs->masterScore()->isSavable());
      getAction("show-invisible")->setChecked(cs->showInvisible());
      getAction("show-unprintable")->setChecked(cs->showUnprintable());
      getAction("show-frames")->setChecked(cs->showFrames());
      getAction("show-pageborders")->setChecked(cs->showPageborders());
      getAction("mark-irregular")->setChecked(cs->markIrregularMeasures());
      getAction("fotomode")->setChecked(cv->fotoMode());
      getAction("join-measures")->setEnabled(cs->masterScore()->excerpts().size() == 0);
      getAction("split-measure")->setEnabled(cs->masterScore()->excerpts().size() == 0);
      getAction("concert-pitch")->setChecked(cs->styleB(Sid::concertPitch));
      updateUndoRedo();

      setZoom(cv->zoomIndex(), cv->logicalZoomLevel());
      setPos(cs->inputPos());
      //showMessage(cs->filePath(), 2000);
      if (_navigator && _navigator->widget()) {
            navigator()->setScoreView(view);
            }
      if (timeline()) {
            timeline()->setScore(cs);
            timeline()->setScoreView(view);
            }
      ScoreAccessibility::instance()->updateAccessibilityInfo();

      MasterScore* master = cs->masterScore();
      if (!scoreWasShown[master]) {
            scoreWasShown[master] = true;
            askMigrateScore(master);
            }
      }

//---------------------------------------------------------
//   setCurrentScores
//---------------------------------------------------------

void MuseScore::setCurrentScores(Score* s1, Score* s2)
      {
      if (s1)
            tab1->setCurrentScore(s1);
      if (s2) {
            setSplitScreen(true);
            tab2->setCurrentScore(s2);
            }
      }

//---------------------------------------------------------
//   setSplitScreen
//---------------------------------------------------------

void MuseScore::setSplitScreen(bool val)
      {
      if (splitScreen() != val)
            splitWindow(_horizontalSplit);
      }

//---------------------------------------------------------
//   updateViewModeCombo
//---------------------------------------------------------

void MuseScore::updateViewModeCombo()
      {
      int idx;
      switch (cs->layoutMode()) {
            case LayoutMode::PAGE:
                  idx = 0;
                  break;
            case LayoutMode::LINE:
                  idx = 1;
                  break;
            case LayoutMode::SYSTEM:
                  idx = 2;
                  break;
            case LayoutMode::FLOAT:
                  idx = 3;
                  break;
            default:
                  idx = 0;
                  break;
            }
      viewModeCombo->setCurrentIndex(idx);
      }

//---------------------------------------------------------
//   showMessage
//---------------------------------------------------------

void MuseScore::showMessage(const QString& s, int timeout)
      {
      _statusBar->showMessage(s, timeout);
      }

//---------------------------------------------------------
//   midiPanel
//---------------------------------------------------------

void MuseScore::midiPanelOnSwitchToFile(const QString &file)
      {
      bool isMidiFile = ImportMidiPanel::isMidiFile(file);
      if (isMidiFile) {
            importmidiPanel->setMidiFile(file);
            if (importmidiPanel->isPreferredVisible())
                  importmidiPanel->setVisible(true);
            }
      else
            importmidiPanel->setVisible(false);
      importmidiShowPanel->setVisible(!importmidiPanel->isPreferredVisible() && isMidiFile);
      }

//---------------------------------------------------------
//   midiPanelOnCloseFile
//---------------------------------------------------------

void MuseScore::midiPanelOnCloseFile(const QString &file)
      {
      if (ImportMidiPanel::isMidiFile(file))
            importmidiPanel->excludeMidiFile(file);
      }

//---------------------------------------------------------
//   allowShowMidiPanel
//---------------------------------------------------------

void MuseScore::allowShowMidiPanel(const QString &file)
      {
      if (ImportMidiPanel::isMidiFile(file))
            importmidiPanel->setPreferredVisible(true);
      }

//---------------------------------------------------------
//   setMidiReopenInProgress
//---------------------------------------------------------

void MuseScore::setMidiReopenInProgress(const QString &file)
      {
      if (ImportMidiPanel::isMidiFile(file))
            importmidiPanel->setReopenInProgress();
      }

//---------------------------------------------------------
//   showMidiImportPanel
//---------------------------------------------------------

void MuseScore::showMidiImportPanel()
      {
      importmidiPanel->setPreferredVisible(true);
      QString fileName = cs ? cs->importedFilePath() : "";
      if (ImportMidiPanel::isMidiFile(fileName))
            importmidiPanel->setVisible(true);
      importmidiShowPanel->hide();
      }

//---------------------------------------------------------
//   dragEnterEvent
//---------------------------------------------------------

void MuseScore::dragEnterEvent(QDragEnterEvent* event)
      {
      const QMimeData* dta = event->mimeData();
      if (dta->hasUrls()) {
            QList<QUrl>ul = event->mimeData()->urls();
            for (const QUrl& u : qAsConst(ul)) {
                  if (MScore::debugMode)
                        qDebug("drag Url: %s scheme <%s>", qPrintable(u.toString()), qPrintable(u.scheme()));
                  if (u.scheme() == "file") {
                        // QFileInfo fi(u.toLocalFile());
                        event->acceptProposedAction();
                        break;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   dropEvent
//---------------------------------------------------------

void MuseScore::dropEvent(QDropEvent* event)
      {
      const QMimeData* dta = event->mimeData();
      if (dta->hasUrls()) {
            for (QUrl& u : event->mimeData()->urls()) {
                  if (u.scheme() == "file") {
                        QString file = u.toLocalFile();
                        openScore(file);
                        }
                  }
            event->acceptProposedAction();
            }
      }

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void MuseScore::changeEvent(QEvent *e)
      {
      QMainWindow::changeEvent(e);
      switch (e->type()) {
            case QEvent::LanguageChange:
                  retranslate();
                  break;
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   showEvent
//---------------------------------------------------------

void MuseScore::showEvent(QShowEvent* showEvent)
{
      QMainWindow::showEvent(showEvent);
      emit musescoreWindowWasShown();
}


//---------------------------------------------------------
//   showPageSettings
//---------------------------------------------------------

void MuseScore::showPageSettings()
      {
      if (pageSettings == 0)
            pageSettings = new PageSettings();
      pageSettings->setScore(cs);
      pageSettings->show();
      pageSettings->raise();
      }

//---------------------------------------------------------
//   startDebugger
//---------------------------------------------------------

void MuseScore::startDebugger()
      {
      if (!cs)
            return;
      if (debugger == 0)
            debugger = new Debugger(this);
      debugger->updateList(cs);
      debugger->show();
      }

//---------------------------------------------------------
//   showElementContext
//---------------------------------------------------------

void MuseScore::showElementContext(Element* el)
      {
      if (el == 0)
            return;
      startDebugger();
      debugger->setElement(el);
      }

//---------------------------------------------------------
//   reDisplayDockWidget
//
//   Helper function to ensure when un-docked widgets are
//   re-displayed, they are also re-dockable
//---------------------------------------------------------

void MuseScore::reDisplayDockWidget(QDockWidget* widget, bool visible)
      {
      if (widget->isFloating() && !widget->isVisible()) {
            // Ensure the widget is re-dockable if it has been closed and re-opened when un-docked. This is a workaround for a known QT bug. See QTBUG-69922.
            widget->setFloating(false);
            widget->setFloating(true);
            }
      widget->setVisible(visible);

      //Bring the widget to the top of the dock
      widget->raise();
      }

//---------------------------------------------------------
//   createPlayPanel
//---------------------------------------------------------

void MuseScore::createPlayPanel()
      {
      if (!playPanel) {
            playPanel = new PlayPanel(this);
            connect(playPanel, SIGNAL(metronomeGainChanged(float)), seq, SLOT(setMetronomeGain(float)));
            connect(playPanel, SIGNAL(speedChanged(double)), seq, SLOT(setRelTempo(double)));
            connect(playPanel, SIGNAL(posChange(int)), seq, SLOT(seek(int)));
            connect(playPanel, SIGNAL(closed(bool)), playId, SLOT(setChecked(bool)));
            connect(synti, SIGNAL(gainChanged(float)), playPanel, SLOT(setGain(float)));
            playPanel->setSpeedIncrement(preferences.getInt(PREF_APP_PLAYBACK_SPEEDINCREMENT));
            playPanel->setGain(synti->gain());
            playPanel->setScore(cs);
            addDockWidget(Qt::RightDockWidgetArea, playPanel);
            playPanel->setVisible(false);
            playPanel->setFloating(false);
            }
      }

//---------------------------------------------------------
//   showPlayPanel
//---------------------------------------------------------

void MuseScore::showPlayPanel(bool visible)
      {
      if (noSeq || !(seq && seq->isRunning()))
            return;
      if (playPanel == 0) {
            if (!visible)
                  return;

            createPlayPanel();

            // The play panel must be set visible before being set floating for positioning
            // and window geometry reasons.
            playPanel->setVisible(visible);
            playPanel->setFloating(false);
            }
      else
            reDisplayDockWidget(playPanel, visible);
      playId->setChecked(visible);
      }

//---------------------------------------------------------
//   cmdAppendMeasures
//---------------------------------------------------------

void MuseScore::cmdAppendMeasures()
      {
      if (cs) {
            if (!measuresDialog)
                  measuresDialog = new MeasuresDialog;
            else {
                  measuresDialog->measures->setFocus();
                  measuresDialog->measures->selectAll();
                  }
            measuresDialog->show();
            }
      }

//---------------------------------------------------------
//   rebuildAudioDrivers
//---------------------------------------------------------

void MuseScore::restartAudioEngine()
      {
      if (seq)
            seq->exit();

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

//---------------------------------------------------------
//   enableMidiIn
//---------------------------------------------------------

void MuseScore::enableMidiIn(const bool enable)
      {
      // This function must be called only when handling the "midi-on" action.
      Q_ASSERT(getAction("midi-on")->isChecked() == enable);

      const auto wasEnabled = isMidiInEnabled();

      preferences.setPreference(PREF_IO_MIDI_ENABLEINPUT, enable);

      if (enable && !wasEnabled)
            restartAudioEngine();
      }

//---------------------------------------------------------
//   isMidiInEnabled
//---------------------------------------------------------

bool MuseScore::isMidiInEnabled() const
      {
      return preferences.getBool(PREF_IO_MIDI_ENABLEINPUT);
      }

//---------------------------------------------------------
//   processMidiRemote
//    return if midi remote command detected
//---------------------------------------------------------

bool MuseScore::processMidiRemote(MidiRemoteType type, int dta, int value)
      {
      if (!preferences.getBool(PREF_IO_MIDI_USEREMOTECONTROL))
            return false;
      if (!value) {
            // This was a "NoteOff" or "CtrlOff" event. Most MidiRemote actions should only
            // be triggered by an "On" event, so we need to check if this is one of those.
            if (!preferences.getBool(PREF_IO_MIDI_ADVANCEONRELEASE)
                    || type != preferences.midiRemote(RMIDI_REALTIME_ADVANCE).type
                    || dta != preferences.midiRemote(RMIDI_REALTIME_ADVANCE).data)
                  return false;
            }
      for (int i = 0; i < MIDI_REMOTES; ++i) {
            if (preferences.midiRemote(i).type == type && preferences.midiRemote(i).data == dta) {
                  if (cv == 0)
                        return false;
                  QAction* a = 0;
                  switch (i) {
                        case RMIDI_REWIND:      a = getAction("rewind"); break;
                        case RMIDI_TOGGLE_PLAY: a = getAction("play");  break;
                        case RMIDI_PLAY:
                              a = getAction("play");
                              if (a->isChecked())
                                    return true;
                              break;
                        case RMIDI_STOP:
                              a = getAction("play");
                              if (!a->isChecked())
                                    return true;
                              break;
                        case RMIDI_NOTE1:   a = getAction("pad-note-1");  break;
                        case RMIDI_NOTE2:   a = getAction("pad-note-2");  break;
                        case RMIDI_NOTE4:   a = getAction("pad-note-4");  break;
                        case RMIDI_NOTE8:   a = getAction("pad-note-8");  break;
                        case RMIDI_NOTE16:  a = getAction("pad-note-16");  break;
                        case RMIDI_NOTE32:  a = getAction("pad-note-32");  break;
                        case RMIDI_NOTE64:  a = getAction("pad-note-64");  break;
                        case RMIDI_REST:    a = getAction("rest");  break;
                        case RMIDI_DOT:     a = getAction("pad-dot");  break;
                        case RMIDI_DOTDOT:  a = getAction("pad-dotdot");  break;
                        case RMIDI_TIE:     a = getAction("tie");  break;
                        case RMIDI_UNDO:    a = getAction("undo"); break;
                        case RMIDI_NOTE_EDIT_MODE: a = getAction("note-input");  break;
                        case RMIDI_REALTIME_ADVANCE: a = getAction("realtime-advance");  break;
                        }
                  if (a)
                        a->trigger();
                  return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   midiNoteReceived
//---------------------------------------------------------

void MuseScore::midiNoteReceived(int channel, int pitch, int velo)
      {
      static const int THRESHOLD_DRUMS = 5; // iterations required before consecutive drum notes
                                     // are not considered part of a chord
      static int active = 0;
      static int iterDrums = 0;
      static int activeDrums = 0;

      if (!isMidiInEnabled())
            return;

// qDebug("midiNoteReceived %d %d %d", channel, pitch, velo);

      if (_midiRecordId != -1) {
            preferences.updateMidiRemote(_midiRecordId, MIDI_REMOTE_TYPE_NOTEON, pitch);
            _midiRecordId = -1;
            if (preferenceDialog)
                  preferenceDialog->updateRemote();
            return;
            }

      if (processMidiRemote(MIDI_REMOTE_TYPE_NOTEON, pitch, velo)) {
            active = 0;
            return;
            }

      QWidget* w = QApplication::activeModalWidget();
      if (!cv || w) {
            active = 0;
            return;
            }
      if (velo) {
             /*
             * Since some drum modules do not overlap note on / off messages
             * we need to take a bit of a different tactic to allow chords
             * to be entered.
             *
             * Rather than decrement active for drums (midi on ch10),
             * we'll just assume that if read() has been called a couple
             * times in a row without a drum note, that this note is not
             * part of a cord.
             */
            if (channel == 0x09) {
                  if (iterDrums >= THRESHOLD_DRUMS)
                        activeDrums = 0;
                  iterDrums = 0;
                  cv->midiNoteReceived(pitch, activeDrums > 0, velo);
                  }
            else {
                  //qDebug("    midiNoteReceived %d active %d", pitch, active);
                  cv->midiNoteReceived(pitch, active > 0, velo);
                  ++active;
                  }
            }
      else {
            /*
            * Since a note may be assigned to a midi_remote,
            * don't decrease active below zero on noteoff.
            */
            if ((channel != 0x09) && (active > 0))
                  --active;
            if ((channel == 0x09) && (activeDrums > 0))
                  --activeDrums;
            cv->midiNoteReceived(pitch, false, velo);
            }

      if (_pianoTools && _pianoTools->isVisible()) {
            if (velo)
                  _pianoTools->pressPitch(pitch);
            else
                  _pianoTools->releasePitch(pitch);
            }
      }

//---------------------------------------------------------
//   midiCtrlReceived
//---------------------------------------------------------

void MuseScore::midiCtrlReceived(int controller, int value)
      {
      if (!isMidiInEnabled())
            return;
      if (_midiRecordId != -1) {
            preferences.updateMidiRemote(_midiRecordId, MIDI_REMOTE_TYPE_CTRL, controller);
            _midiRecordId = -1;
            if (preferenceDialog)
                  preferenceDialog->updateRemote();
            return;
            }
      // when value is 0 (usually when a key is released ) nothing happens
      if (processMidiRemote(MIDI_REMOTE_TYPE_CTRL, controller, value))
            return;
      }

//---------------------------------------------------------
//   removeTab
//---------------------------------------------------------

void MuseScore::removeTab()
      {
      int n = scoreList.indexOf(cs->masterScore());
      if (n == -1) {
            qDebug("removeTab: %p not found", cs);
            return;
            }
      removeTab(n);
      }

void MuseScore::removeTab(int i)
      {
      MasterScore* score = scoreList.value(i);

      if (score == 0)
            return;

      QString tmpName = score->tmpName();

      if (!scriptTestMode && !converterMode && checkDirty(score))
            return;
      if (seq && seq->score() == score) {
            seq->stopWait();
            seq->setScoreView(0);
            }

      int idx1      = tab1->currentIndex();
      bool firstTab = tab1->view(idx1) == cv;

      midiPanelOnCloseFile(score->importedFilePath());
      scoreList.removeAt(i);
      scoreWasShown.remove(score);

      tab1->removeTab(i, /* noCurrentChangedSignals */ true);
      if (tab2)
            tab2->removeTab(i, /* noCurrentChangedSignals */ true);

      cs = 0;
      cv = 0;
      int n = scoreList.size();
      if (n == 0)
            setCurrentScoreView(nullptr);
      else
            setCurrentScoreView((firstTab ? tab1 : tab2)->view());
      writeSessionFile(false);
      if (!tmpName.isEmpty()) {
            QFile f(tmpName);
            f.remove();
            }
      delete score;
      // Shouldn't be necessary... but fix #21841
      update();
      }

//---------------------------------------------------------
//   runTestScripts
//---------------------------------------------------------

bool MuseScore::runTestScripts(const QStringList& scriptFiles)
      {
      setDefaultPalette();
      showInspector(true);

      ScriptContext ctx(this);
      bool allPassed = true;
      int passed = 0;
      int total = 0;
      for (const QString& scriptFile : scriptFiles) {
            // ensure no scores are open not to interfere with
            // the next script initialization process.
            while (!scores().empty()) {
                  closeScore(scores().back());
                  }
            QTextStream(stdout) << "Start test: " << scriptFile << endl;
            std::unique_ptr<Script> script = Script::fromFile(scriptFile);
            const bool pass = script->execute(ctx);
            QTextStream(stdout) << "Test " << scriptFile << (pass ? " PASS" : " FAIL") << endl;
            ++total;
            if (pass)
                  ++passed;
            else
                  allPassed = false;
            }
      QTextStream(stdout) << "Test scripts: total: " << total << ", passed: " << passed << endl;
      return allPassed;
      }

//---------------------------------------------------------
//   loadTranslation
//---------------------------------------------------------

void loadTranslation(QString filename, QString _localeName)
      {
      QString userPrefix    = dataPath + "/locale/"+ filename +"_";
      QString defaultPrefix = mscoreGlobalShare + "locale/"+ filename +"_";
      QString userlp        = userPrefix + _localeName;
      QString defaultlp     = defaultPrefix + _localeName;
      QString lp            = defaultlp;

      QFileInfo userFi(userlp + ".qm");
      QFileInfo defaultFi(defaultlp + ".qm");

      if (!defaultFi.exists()) {     // try with a shorter locale name
            QString shortLocaleName = _localeName.left(_localeName.lastIndexOf("_"));
            QString shortDefaultlp = defaultPrefix + shortLocaleName;
            QFileInfo shortDefaultFi(shortDefaultlp + ".qm");
            if (shortDefaultFi.exists()) {
                  userlp    = userPrefix + shortLocaleName;
                  userFi    = QFileInfo(userlp + ".qm");
                  defaultFi = shortDefaultFi;
                  defaultlp = shortDefaultlp;
                  }
            }

      if (userFi.exists()) {
            if (userFi.lastModified() > defaultFi.lastModified())
                  lp = userlp;
//            else
//REVIEW                  QFile::remove(userlp + ".qm");
            }

      if (MScore::debugMode)
            qDebug("load translator <%s>", qPrintable(lp));

      QTranslator* translator = new QTranslator;
      bool success = translator->load(lp);
      if (success) {
            qApp->installTranslator(translator);
            translatorList->append(translator);
            }
      else {
            if (MScore::debugMode)
                  qDebug("load translator <%s> failed", qPrintable(lp));
            delete translator;
            }
      }

//---------------------------------------------------------
//   setLocale
//---------------------------------------------------------

void setMscoreLocale(QString _localeName)
      {
      for (QTranslator*& t : *translatorList) {
            qApp->removeTranslator(t);
            delete t;
            }
      translatorList->clear();

      if (MScore::debugMode)
            qDebug("configured localeName <%s>", qPrintable(_localeName));
      if (_localeName.toLower() == "system") {
            _localeName = QLocale::system().name();
            if (MScore::debugMode)
                  qDebug("real localeName <%s>", qPrintable(_localeName));
            if (_localeName == "en_AU") {
                  _localeName = "en_GB"; // otherwise Australia would fall back to US English
                  if (MScore::debugMode)
                        qDebug("modified localeName <%s>", qPrintable(_localeName));
                  }
            }

      // find the most recent translation file
      // try to replicate QTranslator.load algorithm in our particular case
      loadTranslation("mscore", _localeName);
      loadTranslation("instruments", _localeName);
      loadTranslation("tours", _localeName);

      QString resourceDir;
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
      resourceDir = mscoreGlobalShare + "locale/";
#else
      resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif
      QTranslator* qtTranslator = new QTranslator;
      if (MScore::debugMode)
            qDebug("load translator <qt_%s> from <%s>",
               qPrintable(_localeName), qPrintable(resourceDir));

      if (!qtTranslator->load(QLatin1String("qt_") + _localeName, resourceDir) && MScore::debugMode)
            qDebug("load translator <qt_%s> failed", qPrintable(_localeName));
      else {
            qApp->installTranslator(qtTranslator);
            translatorList->append(qtTranslator);
            }
      QLocale locale(_localeName);
      QLocale::setDefault(locale);
      qApp->setLayoutDirection(locale.textDirection());
      // initShortcuts();
      }

//---------------------------------------------------------
//   loadScores
//    load scores for a new session
//---------------------------------------------------------

static void loadScores(const QStringList& argv)
      {
      int currentScoreView = 0;
      if (argv.isEmpty()) {
            if (startWithNewScore)
                  mscore->newFile();
            else {
                  switch (preferences.sessionStart()) {
                        case SessionStart::LAST:
                              return; // Already managed by `restoreSession`
                        case SessionStart::EMPTY:
                              break;
                        case SessionStart::NEW:
                              mscore->newFile();
                              break;
                        case SessionStart::SCORE:
                              {
                              QString startScore = preferences.getString(PREF_APP_STARTUP_STARTSCORE);
                              if (startScore == ":/data/My_First_Score.mscz") {
                                    startScore = ":/data/My_First_Score.mscx";
                                    preferences.setPreference(PREF_APP_STARTUP_STARTSCORE, startScore);
                                    }

                              MasterScore* score = mscore->readScore(startScore);
                              if (startScore.startsWith(":/") && score) {
                                    score->setStyle(MScore::defaultStyle());
                                    if (preferences.getBool(PREF_SCORE_HARMONY_PLAY_DISABLE_NEW)) {
                                          score->style().set(Sid::harmonyPlay, false);
                                          }
                                    score->style().checkChordList();
                                    score->styleChanged();
                                    score->setName(mscore->createDefaultName());
                                    // TODO score->setPageFormat(*MScore::defaultStyle().pageFormat());
                                    score->doLayout();
                                    score->setStartedEmpty(true);
                                    }
                              if (score == 0) {
                                    score = mscore->readScore(":/data/My_First_Score.mscx");
                                    if (score) {
                                          score->setStyle(MScore::defaultStyle());
                                          if (preferences.getBool(PREF_SCORE_HARMONY_PLAY_DISABLE_NEW)) {
                                                score->style().set(Sid::harmonyPlay, false);
                                                }
                                          score->style().checkChordList();
                                          score->styleChanged();
                                          score->setName(mscore->createDefaultName());
                                          // TODO score->setPageFormat(*MScore::defaultStyle().pageFormat());
                                          score->doLayout();
                                          score->setStartedEmpty(true);
                                          }
                                    }
                              if (score)
                                    currentScoreView = mscore->appendScore(score);
                              }
                              break;
                        }
                  }
            }
      else {
            for(const QString& name : argv) {
                  if (name.isEmpty())
                        continue;

                  Score* score = mscore->openScore(name);

                  if (score)
                        scoresOnCommandline = true;
                  }
            }

      if (mscore->noScore())
            currentScoreView = -1;
      mscore->setCurrentView(0, currentScoreView);
      mscore->setCurrentView(1, currentScoreView);
      }

//---------------------------------------------------------
//   doConvert
//---------------------------------------------------------

static bool doConvert(Score *cs, const QString& fn);

static bool doConvert(Score* cs, const QJsonArray& outFiles, QString plugin)
      {
      LayoutMode layoutMode = cs->layoutMode();
      cs->setLayoutMode(LayoutMode::PAGE);
      if (cs->layoutMode() != layoutMode)
            cs->doLayout();

      if (!styleFile->isEmpty()) {
            QFile f(*styleFile);
            if (f.open(QIODevice::ReadOnly)) {
                  fprintf(stderr, "\tusing style <%s>\n", qPrintable(*styleFile));
                  cs->style().load(&f);
                  }
            }
      if (!plugin.isEmpty()) {
            if (mscore->loadPlugin(plugin)) {
                  fprintf(stderr, "\tusing plugin <%s>\n", qPrintable(plugin));
                  mscore->pluginTriggered(0);
                  }
            mscore->unloadPlugins();
            }
      for (const QJsonValue& outFile : outFiles) {
            if (outFile.isArray()) {
                  QJsonArray fns = outFile.toArray();
                  if (fns.size() != 2 || !fns[0].isString() || !fns[1].isString()) {
                        fprintf(stderr, "out element array length or type mismatch: %s\n",
                                QJsonDocument(QJsonArray{outFile}).toJson().constData());
                        return false;
                        }
                  if (cs->excerpts().size() == 0)
                        // no parts, silently ignore
                        continue;
                  // convert parts
                  QString fnbeg = fns[0].toString();
                  QString fnend = fns[1].toString();
                  for (Excerpt* e : qAsConst(cs->excerpts())) {
                        Score* pScore = e->partScore();
                        QString partfn = fnbeg + mscore->saveFilename(pScore->title()) + fnend;
                        fprintf(stderr, "\tpart <%s>\n", qPrintable(partfn));
                        QFileInfo fi(partfn);
                        if (!mscore->saveAs(pScore, true, partfn, fi.suffix()))
                              return false;
                        }
                  }
            else if (!outFile.isString()) {
                  fprintf(stderr, "out element type mismatch: %s\n",
                          QJsonDocument(QJsonArray{outFile}).toJson().constData());
                  return false;
                  }
            else {
                  QString fn = outFile.toString();
                  fprintf(stderr, "\tto <%s>\n", qPrintable(fn));
                  if (!doConvert(cs, fn))
                        return false;
                  }
            }
      // don’t bother cleaning up layoutMode:
      // cs is destroyed immediately after return anyway
      return true;
      }

static bool doConvert(Score *cs, const QString& fn)
      {
      if (fn.endsWith(".mscx")) {
            QFileInfo fi(fn);
            return cs->saveFile(fi);
            }
      else if (fn.endsWith(".mscz")) {
            QFileInfo fi(fn);
            return cs->saveCompressedFile(fi, false);
            }
      else if (fn.endsWith(".xml") || fn.endsWith(".musicxml"))
            return saveXml(cs, fn);
      else if (fn.endsWith(".mxl"))
            return saveMxl(cs, fn);
      else if (fn.endsWith(".mid") || fn.endsWith(".midi"))
            return mscore->saveMidi(cs, fn);
      else if (fn.endsWith(".pdf")) {
            if (!exportScoreParts)
                  return mscore->savePdf(cs, fn);
            if (cs->excerpts().size() == 0) {
                  auto excerpts = Excerpt::createAllExcerpt(cs->masterScore());

                  for (Excerpt* e : qAsConst(excerpts)) {
                        Score* nscore = new Score(e->oscore());
                        e->setPartScore(nscore);
                        nscore->style().set(Sid::createMultiMeasureRests, true);
                        cs->startCmd();
                        cs->undo(new AddExcerpt(e));
                        Excerpt::createExcerpt(e);
                        cs->endCmd();
                        }
                  }
            QList<Score*> scores;
            scores.append(cs);
            for (Excerpt* e : qAsConst(cs->excerpts()))
                  scores.append(e->partScore());
            return mscore->savePdf(scores, fn);
            }
      else if (fn.endsWith(".png")) {
            if (!exportScoreParts)
                  return mscore->savePng(cs, fn);
            if (cs->excerpts().size() == 0) {
                  auto excerpts = Excerpt::createAllExcerpt(cs->masterScore());

                  for (Excerpt* e: qAsConst(excerpts)) {
                        Score* nscore = new Score(e->oscore());
                        e->setPartScore(nscore);
                        nscore->setExcerpt(e);
                        // nscore->setName(e->title()); // needed before AddExcerpt
                        nscore->style().set(Sid::createMultiMeasureRests, true);
                        cs->startCmd();
                        cs->undo(new AddExcerpt(e));
                        Excerpt::createExcerpt(e);
                        cs->endCmd();
                        }
                  }
            if (!mscore->savePng(cs, fn))
                  return false;
            int idx = 0;
            int padding = QString("%1").arg(cs->excerpts().size()).size();
            for (Excerpt* e: qAsConst(cs->excerpts())) {
                  QString suffix = QString("__excerpt__%1.png").arg(idx, padding, 10, QLatin1Char('0'));
                  QString excerptFn = fn.left(fn.size() - 4) + suffix;
                  if (!mscore->savePng(e->partScore(), excerptFn))
                        return false;
                  idx++;
                  }
            return true;
            }
      else if (fn.endsWith(".svg"))
            return mscore->saveSvg(cs, fn);
#ifdef HAS_AUDIOFILE
      else if (fn.endsWith(".wav") || fn.endsWith(".ogg") || fn.endsWith(".flac"))
            return mscore->saveAudio(cs, fn);
#endif
#ifdef USE_LAME
      else if (fn.endsWith(".mp3"))
            return mscore->saveMp3(cs, fn);
#endif
      else if (fn.endsWith(".spos"))
            return mscore->savePositions(cs, fn, true);
      else if (fn.endsWith(".mpos"))
            return mscore->savePositions(cs, fn, false);
      else if (fn.endsWith(".mlog"))
            return cs->sanityCheck(fn);
      else if (fn.endsWith(".metajson"))
            return mscore->saveMetadataJSON(cs, fn);
      // unknown file type
      fprintf(stderr, "Unknown file type!\n");
      return false;
      }

//---------------------------------------------------------
//   convert
//---------------------------------------------------------

static bool convert(const QString& inFile, const QJsonArray& outFiles, const QString& plugin = "")
      {
      if (inFile.isEmpty() || (outFiles.isEmpty() && plugin.isEmpty())) {
            fprintf(stderr, "cannot convert <%s>: neither out nor plugin given\n", qPrintable(inFile));
            return false;
            }
      fprintf(stderr, "convert <%s>...\n", qPrintable(inFile));
      Score* score = mscore->openScore(inFile, false, false);
      if (!score)
            return false;
      mscore->setCurrentScore(score);
      bool success = doConvert(score, outFiles, plugin);
      fprintf(stderr, success ? "... success!\n" : "... failed!\n");
      if (plugin.isEmpty())
            delete score;
      else
            mscore->closeScore(score);
      mscore->setCurrentScore(nullptr);
      return success;
      }

static bool convert(const QString& inFile, const QString& outFile)
      {
      if (pluginMode)
            return convert(inFile, QJsonArray { outFile }, *pluginName);
      else
            return convert(inFile, QJsonArray{ outFile });
      }

//---------------------------------------------------------
//   doProcessJob
//---------------------------------------------------------

static bool doProcessJob(QString jsonFile)
      {
      QFile f(jsonFile);
      if (!f.open(QIODevice::ReadOnly)) {
            fprintf(stderr, "cannon open json file <%s>\n", qPrintable(jsonFile));
            return false;
            }
      QJsonParseError pe;
      QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &pe);
      if (pe.error != QJsonParseError::NoError) {
            fprintf(stderr, "error reading json file <%s> at %d: %s\n",
               qPrintable(jsonFile), pe.offset, qPrintable(pe.errorString()));
            return false;
            }
      if (!doc.isArray()) {
            fprintf(stderr, "json file <%s> is not an array\n", qPrintable(jsonFile));
            return false;
            }
      QJsonArray a = doc.array();
      for (const auto& i : qAsConst(a)) {
            QString inFile;
            QJsonArray outFiles;
            QString plugin;
            if (!i.isObject()) {
                  fprintf(stderr, "array value is not an object\n");
                  return false;
                  }
            QJsonObject obj = i.toObject();
            for (auto& key : obj.keys()) {
                  if (key == "in")
                        inFile = obj.value(key).toString();
                  else if (key == "out") {
                        if (obj.value(key).isArray())
                              outFiles = obj.value(key).toArray();
                        else
                              outFiles.push_back(obj.value(key));
                        }
                  else if (key == "plugin")
                        plugin = obj.value(key).toString();
                  else {
                        fprintf(stderr, "unknown key <%s>\n", qPrintable(key));
                        return false;
                        }
                  }
            if (!convert(inFile, outFiles, plugin))
                  return false;
            }
      return true;
      }

//---------------------------------------------------------
//   processNonGui
//---------------------------------------------------------

static bool processNonGui(const QStringList& argv)
      {
      if (cliSaveOnline)
            return mscore->saveOnline(argv);
      if (exportScoreMedia)
            return mscore->exportAllMediaFiles(argv[0], *highlightConfigPath);
      if (exportScoreMeta)
            return mscore->exportScoreMetadata(argv[0]);
      else if (exportScoreMp3)
            return mscore->exportMp3AsJSON(argv[0]);
      else if (saveScoreParts)
            return mscore->saveScoreParts(argv[0]);
      else if (exportScorePartsPdf)
            return mscore->exportPartsPdfsToJSON(argv[0]);
      else if (unrollRepeats)
            return mscore->exportUnrolled(*outFileName);
      else if (exportTransposedScore)
            return mscore->exportTransposedScoreToJSON(argv[0], *transposeExportOptions);
      else if (needUpdateSource)
            return mscore->updateSource(argv[0] /* scorePath */, argv[1] /* newSource */);

      if (pluginMode && !converterMode) {
            loadScores(argv);
            QString pn(*pluginName);
            bool res = false;
            if (mscore->loadPlugin(pn)){
                  Score* cs = mscore->currentScore();
                  if (!styleFile->isEmpty()) {
                        QFile f(*styleFile);
                        if (f.open(QIODevice::ReadOnly))
                              cs->style().load(&f);
                        }
                  LayoutMode layoutMode = cs->layoutMode();
                  if (layoutMode != LayoutMode::PAGE) {
                        cs->setLayoutMode(LayoutMode::PAGE);
                        cs->doLayout();
                        }
                  mscore->pluginTriggered(0);
                  if (layoutMode != cs->layoutMode()) {
                        cs->setLayoutMode(layoutMode);
                        cs->doLayout();
                        }
                  res = true;
                  }
            return res;
            }

      if (converterMode) {
            if (processJob)
                  return doProcessJob(*jsonFileName);
            else
                  return convert(argv[0], *outFileName);
            }

      if (!extensionName->isEmpty()) {
            QFileInfo fi(*extensionName);
            QString suffix = fi.suffix().toLower();
            if (suffix == "muxt")
                  return mscore->importExtension(*extensionName);
            else {
                  fprintf(stderr, "cannot install extension: <%s>\n", qPrintable(*extensionName));
                  return false;
                  }
            }

      if (rawDiffMode || diffMode) {
            if (argv.size() != 2)
                  return false;
            MasterScore* s1 = mscore->readScore(argv[0]);
            MasterScore* s2 = mscore->readScore(argv[1]);
            if (!s1 || !s2)
                  return false;
            ScoreDiff diff(s1, s2, /* textDiffOnly */ !diffMode);

            if (rawDiffMode)
                  QTextStream(stdout) << diff.rawDiff() << endl;
            if (diffMode)
                  QTextStream(stdout) << diff.userDiff() << endl;

            delete s1;
            delete s2;
            }

      if (scriptTestMode)
            return mscore->runTestScripts(argv);

      return true;
      }

//---------------------------------------------------------
//   Message handler
//---------------------------------------------------------

#if defined(QT_DEBUG) && defined(Q_OS_WIN)
static void mscoreMessageHandler(QtMsgType type, const QMessageLogContext &context,const QString &msg)
     {
     QTextStream err(stderr);
     QByteArray localMsg = msg.toLocal8Bit();

     switch (type) {
     case QtInfoMsg:
         err << "Info: "     << localMsg.constData() << " ("  << context.file << ":" << context.line << ", " << context.function << ")" << endl;
         break;
     case QtDebugMsg:
         err << "Debug: "    << localMsg.constData() << " ("  << context.file << ":" << context.line << ", " << context.function << ")" << endl;
         break;
     case QtWarningMsg:
         err << "Warning: "  << localMsg.constData() << " ("  << context.file << ":" << context.line << ", " << context.function << ")" << endl;
         break;
     case QtCriticalMsg: // same as QtSystemMsg
         err << "Critical: " << localMsg.constData() << " ("  << context.file << ":" << context.line << ", " << context.function << ")" << endl;
         break;
     case QtFatalMsg: // set your breakpoint here, if you want to catch the abort
         err << "Fatal: "    << localMsg.constData() << " ("  << context.file << ":" << context.line << ", " << context.function << ")" << endl;
         abort();
         }
     }
#endif

//---------------------------------------------------------
//   synthesizerFactory
//    create and initialize the master synthesizer
//---------------------------------------------------------

MasterSynthesizer* synthesizerFactory()
      {
      MasterSynthesizer* ms = new MasterSynthesizer();

      FluidS::Fluid* fluid = new FluidS::Fluid();
      ms->registerSynthesizer(fluid);

#ifdef AEOLUS
      ms->registerSynthesizer(::createAeolus());
#endif
#ifdef ZERBERUS
      ms->registerSynthesizer(createZerberus());
#endif
      ms->registerEffect(0, new NoEffect);

#ifdef ZITA_REVERB
      ms->registerEffect(0, new ZitaReverb);
#endif

      ms->registerEffect(0, new Compressor);
      // ms->registerEffect(0, new Freeverb);
      ms->registerEffect(1, new NoEffect);

#ifdef ZITA_REVERB
      ms->registerEffect(1, new ZitaReverb);
#endif

      ms->registerEffect(1, new Compressor);
      // ms->registerEffect(1, new Freeverb);
      ms->setEffect(0, 1);
      ms->setEffect(1, 0);
      return ms;
      }

//---------------------------------------------------------
//   unstable
//---------------------------------------------------------

bool MuseScore::unstable()
      {
//#ifdef MSCORE_UNSTABLE
//      return true;
//#else
      return false;
//#endif
      }

//---------------------------------------------------------
//   MuseScoreApplication::event (mac only)
//---------------------------------------------------------

bool MuseScoreApplication::event(QEvent* event)
      {
      switch(event->type()) {
            case QEvent::FileOpen:
                  // store names of files requested to be loaded by OS X to be handled later
                  // this event is generated when a file is dragged onto the MuseScore icon
                  // in the dock and MuseScore is not running yet
                  paths.append(static_cast<QFileOpenEvent *>(event)->file());
                  return true;
            default:
                  return QtSingleApplication::event(event);
            }
      }

//---------------------------------------------------------
//   focusScoreView
//---------------------------------------------------------

void MuseScore::focusScoreView()
      {
      if (currentScoreView()) {
            currentScoreView()->setFocus();
            }
      else
            mscore->setFocus();
      }

//---------------------------------------------------------
//   eventFilter
//---------------------------------------------------------

bool MuseScore::eventFilter(QObject *obj, QEvent *event)
      {
      switch(event->type()) {
#ifdef Q_OS_MAC
            case QEvent::FileOpen:
                  // open files requested to be loaded by OS X
                  // this event is generated when a file is dragged onto the MuseScore icon
                  // in the dock when MuseScore is already running
                  scoresOnCommandline = true;
                  handleMessage(static_cast<QFileOpenEvent *>(event)->file());
                  return true;
#endif
            case QEvent::MouseMove: {
                  QMouseEvent* me = static_cast<QMouseEvent*>(event);
                  globalX = me->globalX();
                  globalY = me->globalY();
                  return QMainWindow::eventFilter(obj, event);
                  }
            case QEvent::StatusTip:
                  return true; // prevent updates to the status bar
            case QEvent::KeyPress:
                  {
                  QKeyEvent* e = static_cast<QKeyEvent*>(event);
                  if(obj->isWidgetType() && e->key() == Qt::Key_Escape && e->modifiers() == Qt::NoModifier) {
                        // Close the search dialog when Escape is pressed:
                        if(_searchDialog != 0)
                              endSearch();
                        if (isActiveWindow()) {
                              obj->event(e);
                              focusScoreView();
                              return true;
                              }

                        QWidget* w = static_cast<QWidget*>(obj);
                        if (inspector()->isAncestorOf(w) ||
                           (selectionWindow && selectionWindow->isAncestorOf(w))) {
                              activateWindow();
                              focusScoreView();
                              return true;
                              }
                        }
                  break;
                  }
            case QEvent::ShortcutOverride:
                  if (qobject_cast<QMenu*>(obj)) {
                        // Disable one-letter shortcuts while in menu
                        // to prevent blocking menu mnemonics
                        QKeyEvent* ke = static_cast<QKeyEvent*>(event);
                        const QString evtText = ke->text();
                        const bool letterOrNumber = !ke->modifiers() && evtText.size() == 1 && evtText.at(0).isLetterOrNumber();

                        if (letterOrNumber) {
                              ke->accept();
                              return true;
                              }
                        }
                  break;
            default:
                  return QMainWindow::eventFilter(obj, event);
            }
      return QMainWindow::eventFilter(obj, event);
      }

//---------------------------------------------------------
//   hasToCheckForUpdate
//---------------------------------------------------------

bool MuseScore::hasToCheckForUpdate()
      {
      if (ucheck)
            return ucheck->hasToCheck();
      else
            return false;
      }

//---------------------------------------------------------
//   hasToCheckForExtensionsUpdate
//---------------------------------------------------------

bool MuseScore::hasToCheckForExtensionsUpdate()
      {
      return packUChecker ? packUChecker->hasToCheck() : false;
      }

//---------------------------------------------------------
//   checkForUpdatesNoUI
//         Doesn't show any messages if software is up to date
//---------------------------------------------------------

void MuseScore::checkForUpdatesNoUI()
      {
      if (autoUpdater)
            autoUpdater->checkUpdates();
      else if (ucheck)
            ucheck->check(version(), false);
      }

//---------------------------------------------------------
//   checkForUpdatesUI
//          Show message like "Software is up to date" if software is up to date
//---------------------------------------------------------
void MuseScore::checkForUpdatesUI()
      {
      if (autoUpdater)
            autoUpdater->checkForUpdatesNow();
      else if (ucheck)
            ucheck->check(version(), true);
      }

//---------------------------------------------------------
//   checkForExtensionsUpdate
//---------------------------------------------------------

void MuseScore::checkForExtensionsUpdate()
      {
      if (packUChecker)
            packUChecker->check();
      }

//---------------------------------------------------------
//   readLanguages
//---------------------------------------------------------

bool MuseScore::readLanguages(const QString& path)
      {
      //: The default language of the operating system. NOT a music system.
      _languages.append(LanguageItem("system", tr("System")));
      QFile qf(path);
      if (qf.exists()) {
            QDomDocument doc;
            int line, column;
            QString err;
            if (!doc.setContent(&qf, false, &err, &line, &column)) {
                  QString error;
                  error = QString::asprintf(qPrintable(tr("Error reading language file %s at line %d column %d: %s\n")),
                                            qPrintable(qf.fileName()), line, column, qPrintable(err));
                  QMessageBox::warning(0,
                     QWidget::tr("Load Languages Failed:"),
                     error,
                     QString(), QWidget::tr("Quit"), QString(), 0, 1);
                  return false;
                  }

            for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
                  if(e.tagName() == "languages") {
                        for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
                              if (e.tagName() == "language") {
                                    QString code = e.attribute(QString("code"));
                                    QString name = e.attribute(QString("name"));
                                    QString handbook = e.attribute(QString("handbook"));
                                    _languages.append(LanguageItem(code, name, handbook));
                                    }
                              }
                        }
                  }
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   clipboardChanged
//---------------------------------------------------------

void MuseScore::clipboardChanged()
      {
#if 0
      const QMimeData* ms = QApplication::clipboard()->mimeData();
      QStringList formats = ms->formats();

      bool flag = ms->hasFormat(mimeSymbolFormat)
            ||    ms->hasFormat(mimeStaffListFormat)
            ||    ms->hasFormat(mimeSymbolListFormat)
            ||    ms->hasText();
      // TODO: depends on selection state
#endif

      bool flag = true;
      getAction("paste")->setEnabled(flag);
      getAction("paste-clone")->setEnabled(flag);
      getAction("swap")->setEnabled(flag);
      }

//---------------------------------------------------------
//   inputMethodAnchorRectangleChanged
//---------------------------------------------------------

void MuseScore::inputMethodAnchorRectangleChanged()
      {
//      QRectF r =  QGuiApplication::inputMethod()->anchorRectangle();
//      qDebug("inputMethod AnchorRectangle Changed: (%f, %f) + (%f, %f)", r.x(), r.y(), r.width(), r.height());
      }

//---------------------------------------------------------
//   inputMethodAnimatingChanged
//---------------------------------------------------------

void MuseScore::inputMethodAnimatingChanged()
      {
//      qDebug("inputMethod Animating Changed: %d", QGuiApplication::inputMethod()->isAnimating());
      }

//---------------------------------------------------------
//   inputMethodCursorRectangleChanged
//---------------------------------------------------------

void MuseScore::inputMethodCursorRectangleChanged()
      {
//      QRectF r =  QGuiApplication::inputMethod()->cursorRectangle();
//      qDebug("inputMethod CursorRectangle Changed: (%f, %f) + (%f, %f)", r.x(), r.y(), r.width(), r.height());
      }

//---------------------------------------------------------
//   inputMethodInputDirectionChanged
//---------------------------------------------------------

void MuseScore::inputMethodInputDirectionChanged(Qt::LayoutDirection /*newDirection*/)
      {
//      qDebug("inputMethod InputDirection Changed: (QLocale::LayoutDirection enum) #%d", newDirection);
      }

//---------------------------------------------------------
//   inputMethodInputItemClipRectangleChanged
//---------------------------------------------------------

void MuseScore::inputMethodInputItemClipRectangleChanged()
      {
//      QRectF r =  QGuiApplication::inputMethod()->inputItemClipRectangle();
//      qDebug("inputMethod InputItemClipRectangle Changed: (%f, %f) + (%f, %f)", r.x(), r.y(), r.width(), r.height());
      }

//---------------------------------------------------------
//   inputMethodKeyboardRectangleChanged
//---------------------------------------------------------

void MuseScore::inputMethodKeyboardRectangleChanged()
      {
//      QRectF r =  QGuiApplication::inputMethod()->keyboardRectangle();
//      qDebug("inputMethod KeyboardRectangle Changed: (%f, %f) + (%f, %f)", r.x(), r.y(), r.width(), r.height());
      }

//---------------------------------------------------------
//   inputMethodLocaleChanged
//---------------------------------------------------------

void MuseScore::inputMethodLocaleChanged()
      {
//      qDebug("inputMethod Locale Changed: (QLocale::Script enum) #%d.", QGuiApplication::inputMethod()->locale().script());
      }

//---------------------------------------------------------
//   inputMethodVisibleChanged
//---------------------------------------------------------

void MuseScore::inputMethodVisibleChanged()
      {
//      qDebug("inputMethodVisibleChanged: %d", QGuiApplication::inputMethod()->isVisible());
      }

//---------------------------------------------------------
//   showModeText
//---------------------------------------------------------

void MuseScore::showModeText(const QString& s, bool informScreenReader)
      {
      if (s == _modeText->text())
            return;
      if (informScreenReader && cs)
            cs->setAccessibleMessage(s);
      _modeText->setText(s);
      }

//---------------------------------------------------------
//   changeState
//---------------------------------------------------------

void MuseScore::changeState(ScoreState val)
      {
      if (MScore::debugMode)
            qDebug("MuseScore::changeState: %s", stateName(val));

      // disallow change to edit modes if currently in play mode
      if (_sstate == STATE_PLAY && (val == STATE_EDIT || val & STATE_ALLTEXTUAL_EDIT || val & STATE_NOTE_ENTRY))
            return;

      static const char* stdNames[] = {
            "note-longa", "note-breve", "pad-note-1", "pad-note-2", "pad-note-4",
            "pad-note-8", "pad-note-16", "pad-note-32", "pad-note-64", "pad-note-128",
            "pad-note-256","pad-note-512","pad-note-1024","pad-rest", "rest"};
      static const char* tabNames[] = {
            "note-longa-TAB", "note-breve-TAB", "pad-note-1-TAB", "pad-note-2-TAB", "pad-note-4-TAB",
            "pad-note-8-TAB", "pad-note-16-TAB", "pad-note-32-TAB", "pad-note-64-TAB", "pad-note-128-TAB",
            "pad-note-256-TAB", "pad-note-512-TAB", "pad-note-1024-TAB", "pad-rest-TAB", "rest-TAB"};
      bool intoTAB = (_sstate != STATE_NOTE_ENTRY_STAFF_TAB) && (val == STATE_NOTE_ENTRY_STAFF_TAB);
      bool fromTAB = (_sstate == STATE_NOTE_ENTRY_STAFF_TAB) && (val != STATE_NOTE_ENTRY_STAFF_TAB);
      // if activating TAB note entry, swap "pad-note-...-TAB" shortcuts into "pad-note-..." actions
      if (intoTAB) {
            for (unsigned i = 0; i < sizeof(stdNames)/sizeof(char*); ++i) {
                  QAction* act = getAction(stdNames[i]);
                  const Shortcut* srt = Shortcut::getShortcut(tabNames[i]);
                  act->setShortcuts(srt->keys());
                  }
            }
      // if de-ativating TAB note entry, restore shortcuts for "pad-note-..." actions
      else if (fromTAB) {
            for (unsigned i = 0; i < sizeof(stdNames)/sizeof(char*); ++i) {
                  QAction* act = getAction(stdNames[i]);
                  const Shortcut* srt = Shortcut::getShortcut(stdNames[i]);
                  act->setShortcuts(srt->keys());
                  }
            }

      bool enable = (val != STATE_DISABLED) && (val != STATE_LOCK);

      for (const Shortcut* s : Shortcut::shortcuts()) {
            QAction* a = s->action();
            if (!a)
                  continue;
            if (enable && (s->key() == "undo"))
                  a->setEnabled((s->state() & val) && (cs ? cs->undoStack()->canUndo() : false));
            else if (enable && (s->key() == "redo"))
                  a->setEnabled((s->state() & val) && (cs ? cs->undoStack()->canRedo() : false));
            else if (enable && (s->key() == "cut"))
                  a->setEnabled(cs && cs->selection().state() != SelState::NONE);
            else if (enable && (s->key() == "copy"))
                  a->setEnabled(cs && (cs->selection().state() != SelState::NONE || val == STATE_FOTO));
            else if (enable && (s->key() == "select-similar-range"))
                  a->setEnabled(cs && cs->selection().state() == SelState::RANGE);
            else if (enable && (s->key() == "synth-control" || s->key() == "toggle-mixer")) {
                  Driver* driver = seq ? seq->driver() : 0;
                  // a->setEnabled(driver && driver->getSynth());
                  if (MScore::debugMode)
                        qDebug("disable synth control");
                  a->setEnabled(driver);
                  }
            else {
                  a->setEnabled(s->state() & val);
                  }
            }

      if (getAction("join-measures")->isEnabled())
            getAction("join-measures")->setEnabled(cs && cs->masterScore()->excerpts().size() == 0);
      if (getAction("split-measure")->isEnabled())
            getAction("split-measure")->setEnabled(cs && cs->masterScore()->excerpts().size() == 0);

      // disabling top level menu entries does not
      // work for MAC

      QList<QObject*> ol = menuBar()->children();
      for (QObject* o : ol) {
            QMenu* menu = qobject_cast<QMenu*>(o);
            if (!menu)
                  continue;
            QString s(menu->objectName());
            if (s == "File" || s == "Help" || s == "Edit" || s == "Plugins" || s == "View")
                  continue;
            menu->setEnabled(enable);
            }

      menuWorkspaces->setEnabled(enable);

      transportTools->setEnabled(enable && !noSeq && seq && seq->isRunning());
      cpitchTools->setEnabled(enable);
      zoomBox->setEnabled(enable);
      entryTools->setEnabled(enable);

      if (_sstate == STATE_FOTO)
            updateInspector();

      //always hide drumtools to support hiding on switching docs, switching node entry mode off, etc.
      //it will be shown later if (_sstate == STATE_NOTE_ENTRY_STAFF_DRUM)
      showDrumTools(0, 0);

      switch (val) {
            case STATE_DISABLED:
                  showModeText(tr("No score"));
                  if (debugger)
                        debugger->hide();
                  showPianoKeyboard(false);
                  break;
            case STATE_NORMAL:
                  showModeText(tr("Normal mode"));;
                  break;
            case STATE_NOTE_ENTRY:
                  if (cv && !cv->noteEntryMode())
                        cv->cmd("note-input");
                  // fall through
            case STATE_NOTE_ENTRY_STAFF_PITCHED:
                  if (getAction("note-input-repitch")->isChecked()) {
                        showModeText(tr("Repitch input mode"));
                        cs->setNoteEntryMethod(NoteEntryMethod::REPITCH);
                        val = STATE_NOTE_ENTRY_METHOD_REPITCH;
                        }
                  else if (getAction("note-input-rhythm")->isChecked()) {
                        showModeText(tr("Rhythm input mode"));
                        cs->setNoteEntryMethod(NoteEntryMethod::RHYTHM);
                        val = STATE_NOTE_ENTRY_METHOD_RHYTHM;
                        }
                  else if (getAction("note-input-realtime-auto")->isChecked()) {
                        showModeText(tr("Realtime (automatic) note input mode"));
                        cs->setNoteEntryMethod(NoteEntryMethod::REALTIME_AUTO);
                        val = STATE_NOTE_ENTRY_METHOD_REALTIME_AUTO;
                        }
                  else if (getAction("note-input-realtime-manual")->isChecked()) {
                        showModeText(tr("Realtime (manual) note input mode"));
                        cs->setNoteEntryMethod(NoteEntryMethod::REALTIME_MANUAL);
                        val = STATE_NOTE_ENTRY_METHOD_REALTIME_MANUAL;
                        }
                  else if (getAction("note-input-timewise")->isChecked()) {
                        showModeText(tr("Insert mode"));
                        cs->setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
                        val = STATE_NOTE_ENTRY_METHOD_TIMEWISE;
                        }
                  else {
                        showModeText(tr("Steptime note input mode"));
                        cs->setNoteEntryMethod(NoteEntryMethod::STEPTIME);
                        val = STATE_NOTE_ENTRY_METHOD_STEPTIME;
                        }
                  break;
            case STATE_NOTE_ENTRY_STAFF_DRUM:
                  {
                  if (getAction("note-input-repitch")->isChecked())
                        cs->setNoteEntryMethod(NoteEntryMethod::REPITCH);
                  else if (getAction("note-input-rhythm")->isChecked())
                        cs->setNoteEntryMethod(NoteEntryMethod::RHYTHM);
                  else if (getAction("note-input-realtime-auto")->isChecked())
                        cs->setNoteEntryMethod(NoteEntryMethod::REALTIME_AUTO);
                  else if (getAction("note-input-realtime-manual")->isChecked())
                        cs->setNoteEntryMethod(NoteEntryMethod::REALTIME_MANUAL);
                  else if (getAction("note-input-timewise")->isChecked())
                        cs->setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
                  else
                        cs->setNoteEntryMethod(NoteEntryMethod::STEPTIME);
                  showModeText(tr("Drumset input mode"));
                  InputState& is = cs->inputState();
                  showDrumTools(is.drumset(), cs->staff(is.track() / VOICES));
                  if (_drumTools)
                        is.setDrumNote(_drumTools->selectedDrumNote());
                  }
                  break;
            case STATE_NOTE_ENTRY_STAFF_TAB:
                  if (!cs)
                        break;
                  if (getAction("note-input-repitch")->isChecked())
                        cs->setNoteEntryMethod(NoteEntryMethod::REPITCH);
                  else if (getAction("note-input-rhythm")->isChecked())
                        cs->setNoteEntryMethod(NoteEntryMethod::RHYTHM);
                  else if (getAction("note-input-realtime-auto")->isChecked())
                        cs->setNoteEntryMethod(NoteEntryMethod::REALTIME_AUTO);
                  else if (getAction("note-input-realtime-manual")->isChecked())
                        cs->setNoteEntryMethod(NoteEntryMethod::REALTIME_MANUAL);
                  else if (getAction("note-input-timewise")->isChecked())
                        cs->setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
                  else
                        cs->setNoteEntryMethod(NoteEntryMethod::STEPTIME);
                  showModeText(tr("TAB input mode"));
                  break;
            case STATE_EDIT:
                  showModeText(tr("Edit mode"));
                  break;
            case STATE_TEXT_EDIT:
                  showModeText(tr("Text edit mode"));
                  break;
            case STATE_LYRICS_EDIT:
                  showModeText(tr("Lyrics edit mode"));
                  break;
            case STATE_HARMONY_FIGBASS_EDIT:
                  showModeText(tr("Chord symbol/figured bass edit mode"));
                  break;
            case STATE_PLAY:
                  showModeText(tr("Play"), false); // don't talk over playback
                  break;
            case STATE_FOTO:
                  showModeText(tr("Image capture mode"));
                  updateInspector();
                  break;
            case STATE_LOCK:
                  showModeText(tr("Score locked"));
                  break;
            default:
                  qFatal("MuseScore::changeState: illegal state %d", val);
                  break;
            }
      if (paletteWidget)
            paletteWidget->setDisabled(val == STATE_PLAY || val == STATE_DISABLED);
      if (selectionWindow)
            selectionWindow->setDisabled(val == STATE_PLAY || val == STATE_DISABLED);
      QAction* a = getAction("note-input");
      bool noteEntry = val & STATE_NOTE_ENTRY;
      a->setChecked(noteEntry);
      _sstate = val;

      emit scoreStateChanged(_sstate);

      Element* e = cv && (_sstate & STATE_ALLTEXTUAL_EDIT || _sstate == STATE_EDIT) ? cv->getEditElement() : 0;
      if (!e) {
            textTools()->hide();
            if (textTools()->kbAction()->isChecked())
                  textTools()->kbAction()->setChecked(false);
            }
      else {
            if (e->isTextBase()) {
                  textTools()->updateTools(cv->getEditData());
                  if (!(e->isFiguredBass() || e->isHarmony())) {  // do not show text tools for f.b.
                        if (timelineScrollArea() && timelineScrollArea()->isVisible()) {
                              if (dockWidgetArea(timelineScrollArea()) != dockWidgetArea(textTools()) || timelineScrollArea()->isFloating()) {
                                    QSizePolicy policy(QSizePolicy::Maximum, QSizePolicy::Maximum);
                                    textTools()->widget()->setSizePolicy(policy);
                                    }
                              else {
                                    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                                    textTools()->widget()->setSizePolicy(policy);
                                    }
                              }
                        else {
                              QSizePolicy policy(QSizePolicy::Maximum, QSizePolicy::Maximum);
                              textTools()->widget()->setSizePolicy(policy);
                              }
                        if (timelineScrollArea())
                              splitDockWidget(textTools(), timelineScrollArea(), Qt::Vertical);
                        textTools()->show();
                        }
                  }
            if (_inspector)
                  _inspector->update(e->score());
            }
      }

//---------------------------------------------------------
//   saveDialogState
//---------------------------------------------------------

void MuseScore::saveDialogState(const char* name, QFileDialog* d)
      {
      if (d) {
            settings.beginGroup(name);
            settings.setValue("geometry", d->saveGeometry());
            settings.setValue("state", d->saveState());
            settings.endGroup();
            }
      }

//---------------------------------------------------------
//   restoreDialogState
//---------------------------------------------------------

void MuseScore::restoreDialogState(const char* name, QFileDialog* d)
      {
      settings.beginGroup(name);
      d->restoreGeometry(settings.value("geometry").toByteArray());
      d->restoreState(settings.value("state").toByteArray());
      settings.endGroup();
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void MuseScore::writeSettings()
      {
      // save score list
      for (int i = 0; i < RECENT_LIST_SIZE; ++i)
            settings.setValue(QString("recent-%1").arg(i), _recentScores.value(i));

      settings.setValue("scores", scoreList.size());
      if (cs) {
            int curScore = scoreList.indexOf(cs->masterScore());
            if (curScore == -1)  // cs removed if new created and not modified
                  curScore = 0;
            settings.setValue("currentScore", curScore);

            for (int idx = 0; idx < scoreList.size(); ++idx)
                  settings.setValue(QString("score-%1").arg(idx), scoreList[idx]->masterScore()->fileInfo()->absoluteFilePath());
            }

      settings.setValue("lastSaveCopyDirectory", lastSaveCopyDirectory);
      settings.setValue("lastSaveCopyFormat", lastSaveCopyFormat);
      settings.setValue("lastSaveDirectory", lastSaveDirectory);

      MuseScore::saveGeometry(this);

      settings.beginGroup("MainWindow");
      settings.setValue("showPanel", paletteWidget && paletteWidget->isVisible());
      settings.setValue("showInspector", _inspector && _inspector->isVisible());
      settings.setValue("showPianoKeyboard", _pianoTools && _pianoTools->isVisible());
      settings.setValue("showSelectionWindow", selectionWindow && selectionWindow->isVisible());
      settings.setValue("state", saveState());
      settings.setValue("splitScreen", _splitScreen);
      settings.setValue("debuggerSplitter", mainWindow->saveState());
      settings.setValue("split", _horizontalSplit);
      settings.setValue("splitter", splitter->saveState());
      settings.endGroup();

      WorkspacesManager::currentWorkspace()->save();
      if (keyEditor && keyEditor->dirty())
            keyEditor->save();
      if (chordStyleEditor)
            chordStyleEditor->save();

      saveDialogState("loadScoreDialog",      loadScoreDialog);
      saveDialogState("saveScoreDialog",      saveScoreDialog);
      saveDialogState("loadStyleDialog",      loadStyleDialog);
      saveDialogState("saveStyleDialog",      saveStyleDialog);
      saveDialogState("saveImageDialog",      saveImageDialog);
      saveDialogState("loadChordStyleDialog", loadChordStyleDialog);
      saveDialogState("saveChordStyleDialog", saveChordStyleDialog);
      saveDialogState("loadScanDialog",       loadScanDialog);
      saveDialogState("loadAudioDialog",      loadAudioDialog);
      saveDialogState("loadDrumsetDialog",    loadDrumsetDialog);
      saveDialogState("saveDrumsetDialog",    saveDrumsetDialog);
      saveDialogState("loadPaletteDialog",    loadPaletteDialog);
      saveDialogState("savePaletteDialog",    savePaletteDialog);

      if (debugger)
            debugger->writeSettings();

#ifdef SCRIPT_INTERFACE
      if (_pluginCreator)
            _pluginCreator->writeSettings();
#endif
      if (synthControl)
            synthControl->writeSettings();
      settings.setValue("synthControlVisible", synthControl && synthControl->isVisible());
      settings.setValue("mixerVisible", mixer && mixer->isVisible());
      if (seq) {
            seq->exit();
            }
      if (instrList)
            instrList->writeSettings();
      if (pianorollEditor)
            pianorollEditor->writeSettings();
      if (drumrollEditor)
            drumrollEditor->writeSettings();
      if (startcenter)
            startcenter->writeSettings();

      _tourHandler->writeCompletedTours();
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void MuseScore::readSettings()
      {
      int margin = 100;
      int offset = margin / 2;
      int w = 1024;
      int h = 768;
      QScreen* screen      = QGuiApplication::primaryScreen();
      const QSize screenSize = screen->availableSize();
      if (screenSize.width() - margin > w)
            w = screenSize.width() - margin;
      else
            offset = 0;
      if (screenSize.height() - margin > h)
            h = screenSize.height() - margin;

      resize(QSize(w, h)); //ensure default size if no geometry in settings
      move(offset, 0);
      if (useFactorySettings) {
            QList<int> sizes;
            sizes << 500 << 100;
            mainWindow->setSizes(sizes);
            mscore->showPalette(true);
            mscore->showInspector(true);
            return;
            }

      MuseScore::restoreGeometry(this);

      // Grab the mixer visible state before the beginGroup.
      // Previously the showMixer() call was made at the end of
      // main(). Now it's made inside this beginGroup / endGroup.
      // This code ensures user previous setting is respected when
      // updating their version of MuseScore.
      bool mixerVisible = settings.value("mixerVisible", "0").toBool();
      settings.beginGroup("MainWindow");
      mainWindow->restoreState(settings.value("debuggerSplitter").toByteArray());
      mainWindow->setOpaqueResize(false);
      scorePageLayoutChanged();

      //for some reason when MuseScore starts maximized the screen-reader
      //doesn't respond to QAccessibleEvents --> so force normal mode
      if (isMaximized() && QAccessible::isActive()) {
            showNormal();
            }
      mscore->showPalette(settings.value("showPanel", "1").toBool());
      mscore->showInspector(settings.value("showInspector", "1").toBool());
      mscore->showPianoKeyboard(settings.value("showPianoKeyboard", "0").toBool());
      mscore->showSelectionWindow(settings.value("showSelectionWindow", "0").toBool());
      mscore->showMixer(mixerVisible);

      restoreState(settings.value("state").toByteArray());
      //if we were in full screen mode, go to maximized mode
      if (isFullScreen()) {
            showMaximized();
            }

      _horizontalSplit = settings.value("split", true).toBool();
      bool splitScreen = settings.value("splitScreen", false).toBool();
      if (splitScreen) {
            splitWindow(_horizontalSplit);
            QAction* a = getAction(_horizontalSplit ? "split-h" : "split-v");
            a->setChecked(true);
            }
      splitter->restoreState(settings.value("splitter").toByteArray());
      settings.endGroup();

      QAction* a = getAction("toggle-fileoperations");
      a->setChecked(!fileTools->isHidden());

      a = getAction("toggle-transport");
      a->setChecked(!transportTools->isHidden());

      a = getAction("toggle-concertpitch");
      a->setChecked(!cpitchTools->isHidden());

      a = getAction("toggle-imagecapture");
      a->setChecked(!fotoTools->isHidden());

      a = getAction("toggle-noteinput");
      a->setChecked(!entryTools->isHidden());
      }

//---------------------------------------------------------
//   play
//    play note for preferences.defaultPlayDuration
//---------------------------------------------------------

void MuseScore::play(Element* e) const
      {
      if (noSeq || !(seq && seq->isRunning()) || !preferences.getBool(PREF_SCORE_NOTE_PLAYONCLICK))
            return;

      if (e->isNote()) {
            Note* note = toNote(e);
            play(e, note->ppitch());
            }
      else if (e->isChord()) {
            seq->stopNotes();
            Chord* c   = toChord(e);
            Part* part = c->staff()->part();
            Fraction tick   = c->segment() ? c->segment()->tick() : Fraction(0,1);
            Instrument* instr = part->instrument(tick);
            for (Note* n : c->notes()) {
                  const int channel = instr->channel(n->subchannel())->channel();
                  seq->startNote(channel, n->ppitch(), 80, n->tuning());
                  }
            seq->startNoteTimer(MScore::defaultPlayDuration);
            }
      else if (e->isHarmony()
               && preferences.getBool(PREF_SCORE_HARMONY_PLAY_ONEDIT)) {
            seq->stopNotes();
            Harmony* h = toHarmony(e);
            if (!h->isRealizable())
                  return;
            RealizedHarmony r = h->getRealizedHarmony();
            QList<int> pitches = r.pitches();

            const Channel* hChannel = e->part()->harmonyChannel();
            if (!hChannel)
                  return;

            int channel = hChannel->channel();

            // reset the cc that is used for single note dynamics, if any
            int cc = synthesizerState().ccToUse();
            if (cc != -1)
                  seq->sendEvent(NPlayEvent(ME_CONTROLLER, channel, cc, 80));

            for (int& pitch : pitches)
                  seq->startNote(channel, pitch, 80, 0);
            seq->startNoteTimer(MScore::defaultPlayDuration);
            }
      }

void MuseScore::play(Element* e, int pitch) const
      {
      if (noSeq || !(seq && seq->isRunning()))
            return;
      if (preferences.getBool(PREF_SCORE_NOTE_PLAYONCLICK) && e->isNote()) {
            Note* note = static_cast<Note*>(e);

            Note* masterNote = note;
            if (note->linkList().size() > 1) {
                  for (ScoreElement*& se_ : note->linkList()) {
                        if (se_->score() == note->masterScore() && se_->isNote()) {
                              masterNote = toNote(se_);
                              break;
                              }
                        }
                  }

            Fraction tick = masterNote->chord()->tick();
            if (tick < Fraction(0,1))
                  tick = Fraction(0,1);
            Instrument* instr = masterNote->part()->instrument(tick);
            const int channel = instr->channel(masterNote->subchannel())->channel();

            // reset the cc that is used for single note dynamics, if any
            int cc = synthesizerState().ccToUse();
            if (cc != -1)
                  seq->sendEvent(NPlayEvent(ME_CONTROLLER, channel, cc, 80));

            seq->startNote(channel, pitch, 80, MScore::defaultPlayDuration, masterNote->tuning());
            }
      }

//---------------------------------------------------------
//   reportBug
//---------------------------------------------------------

void MuseScore::reportBug(QString medium)
      {
      QString url = QString("https://github.com/Jojo-Schmitz/MuseScore/issues")
         .arg(revision(), getLocaleISOCode(), getUtmParameters(medium));
      QDesktopServices::openUrl(QUrl(url.trimmed()));
      }

//---------------------------------------------------------
//   askForHelp
//---------------------------------------------------------

void MuseScore::askForHelp()
      {
      QString url = QString("https://musescore.org/redirect/post/question?locale=%1").arg(getLocaleISOCode());
      QDesktopServices::openUrl(QUrl(url.trimmed()));
      }

//---------------------------------------------------------
//   leaveFeedback
//---------------------------------------------------------

void MuseScore::leaveFeedback(QString medium)
      {
      QString url = QString("https://musescore.com/content/editor-feedback?%1")
         .arg(getUtmParameters(medium));
      QDesktopServices::openUrl(QUrl(url.trimmed()));
      }

//---------------------------------------------------------
//   getUtmParameters
//    Get UTM labels to track visits.
//    See: https://www.google.com/support/googleanalytics/bin/answer.py?answer=55578
//---------------------------------------------------------

QString MuseScore::getUtmParameters(QString medium) const
      {
      return QString("utm_source=desktop&utm_medium=%1&utm_content=%2&utm_campaign=MuseScore%3")
         .arg(medium, rev.trimmed(), QString(VERSION));
      }

//---------------------------------------------------------
//   about
//---------------------------------------------------------

void MuseScore::about()
      {
      AboutBoxDialog ab;
      ab.show();
      ab.exec();
      }

//---------------------------------------------------------
//   dirtyChanged
//---------------------------------------------------------

void MuseScore::dirtyChanged(Score* s)
      {
      MasterScore* score = s->masterScore();

      int idx = scoreList.indexOf(score);
      if (idx == -1) {
            qDebug("score not in list");
            return;
            }
      QString label(score->fileInfo()->completeBaseName());
      if (score->dirty())
            label += "*";
      tab1->setTabText(idx, label);
      if (tab2)
            tab2->setTabText(idx, label);
      setWindowModified(score->dirty());
      }

//---------------------------------------------------------
//   zoomBoxChanged
//    Called when the zoom-box value has changed; do not call directly.
//---------------------------------------------------------

void MuseScore::zoomBoxChanged(const ZoomIndex index, const qreal logicalLevel)
      {
      setZoom(index, logicalLevel);
      }

//---------------------------------------------------------
//   setZoom
//    Sets the zoom type and the logical free-zoom level.
//    logicalFreeZoomLevel is optional and may be omitted unless index is ZoomIndex::ZOOM_FREE.
//---------------------------------------------------------

void MuseScore::setZoom(const ZoomIndex index, const qreal logicalFreeZoomLevel/* = 0.0*/)
      {
      zoomAndSavePrevious([=]() { cv->setLogicalZoom(index, cv->calculateLogicalZoomLevel(index, logicalFreeZoomLevel)); });
      }

//---------------------------------------------------------
//   setZoomWithToggle
//    Sets the specified zoom type, or toggles back to the previous zoom state if the zoom type is already the specified one.
//---------------------------------------------------------

void MuseScore::setZoomWithToggle(const ZoomIndex index)
      {
      if (cv) {
            const ZoomIndex currentZoomIndex = cv->zoomIndex();
            if (currentZoomIndex != index) {
                  // The current zoom type isn't the specified one, so just set it to that.
                  setZoom(index);
                  }
            else {
                  // The current zoom type is already the specified one, so toggle back to the previous zoom state.
                  setZoom(cv->previousZoomIndex(), cv->previousLogicalZoomLevel());
                  }
            }
      }

//---------------------------------------------------------
//   zoomBySteps
//    Zooms in or out by the specified number of keyboard-based zoom steps (positive to zoom in, negative to zoom out).
//---------------------------------------------------------

void MuseScore::zoomBySteps(const qreal numSteps)
      {
      zoomAndSavePrevious([=]() { cv->zoomBySteps(numSteps); });
      }

//---------------------------------------------------------
//   zoomAndSavePrevious
//    Calls the specified zoom function and also saves the previous zoom state if it has changed by the zoom function.
//---------------------------------------------------------

void MuseScore::zoomAndSavePrevious(const std::function<void(void)>& zoomFunction)
      {
      if (cv) {
            // Make a copy of the current zoom state before it changes.
            const auto previousLogicalZoom = cv->logicalZoom();

            // Zoom!
            zoomFunction();

            // Save the previous zoom state, but only if the zoom state has actually changed.
            if (cv->logicalZoom() != previousLogicalZoom)
                  cv->setPreviousLogicalZoom(previousLogicalZoom);
            }
      }

//---------------------------------------------------------
//   updateZoomBox
//    Public function called by the score view to update the zoom box after the actual zoom type and/or level have changed.
//---------------------------------------------------------

void MuseScore::updateZoomBox(const ZoomIndex index, const qreal logicalLevel)
      {
      const QSignalBlocker blocker(zoomBox);
      zoomBox->setLogicalZoom(index, logicalLevel);
      }

//---------------------------------------------------------
//   setPos
//    set position label
//---------------------------------------------------------

void MuseScore::setPos(const Fraction& t)
      {
      if (cs == 0 || t < Fraction(0,1))
            return;

      TimeSigMap* s = cs->sigmap();
      int bar, beat, tick;
      s->tickValues(t.ticks(), &bar, &beat, &tick);
      _positionLabel->setText(QString("%1:%2:%3")
         .arg(bar + 1,  3, 10, QLatin1Char(' '))
         .arg(beat + 1, 2, 10, QLatin1Char('0'))
         .arg(tick,     3, 10, QLatin1Char('0'))
         );
      }

//---------------------------------------------------------
//   undoRedo
//---------------------------------------------------------

void MuseScore::undoRedo(bool undo)
      {
      Q_ASSERT(cv);
      Q_ASSERT(cs);
      if (_sstate & (STATE_EDIT | STATE_HARMONY_FIGBASS_EDIT | STATE_LYRICS_EDIT))
            cv->changeState(ViewState::NORMAL);
      cv->startUndoRedo(undo);
      updateInputState(cs);
      endCmd(/* undoRedo */ true);
      if (pianorollEditor)
            pianorollEditor->update();
      }

//---------------------------------------------------------
//   showProgressBar
//---------------------------------------------------------

QProgressBar* MuseScore::showProgressBar()
      {
      if (_progressBar == 0)
            _progressBar = new QProgressBar(this);
      _statusBar->addWidget(_progressBar);
      _progressBar->show();
      return _progressBar;
      }

//---------------------------------------------------------
//   hideProgressBar
//---------------------------------------------------------

void MuseScore::hideProgressBar()
      {
      if (_progressBar)
            _statusBar->removeWidget(_progressBar);
      }

//---------------------------------------------------------
//   handleMessage
//---------------------------------------------------------

void MuseScore::handleMessage(const QString& message)
      {
      if (message.isEmpty())
            return;
      if (startcenter)
            showStartcenter(false);
      ((QtSingleApplication*)(qApp))->activateWindow();
      openScore(message);
      }

//---------------------------------------------------------
//   editInPianoroll
//---------------------------------------------------------

void MuseScore::editInPianoroll(Staff* staff, Position* p)
      {
      if (pianorollEditor == 0)
            pianorollEditor = new PianorollEditor(this);
      pianorollEditor->setScore(staff->score());
      pianorollEditor->setStaff(staff);
      pianorollEditor->show();
      pianorollEditor->focusOnPosition(p);
      }

//---------------------------------------------------------
//   editInDrumroll
//---------------------------------------------------------

void MuseScore::editInDrumroll(Staff* staff)
      {
      if (drumrollEditor == 0)
            drumrollEditor = new DrumrollEditor;
      drumrollEditor->setStaff(staff);
      drumrollEditor->show();
      }

//---------------------------------------------------------
//   writeSessionFile
//---------------------------------------------------------

void MuseScore::writeSessionFile(bool cleanExit)
      {
// qDebug("write session file");

      QDir dir;
      dir.mkpath(dataPath);
      QFile f(dataPath + "/session");
      if (!f.open(QIODevice::WriteOnly)) {
            qDebug("cannot create session file <%s>", qPrintable(f.fileName()));
            return;
            }
      XmlWriter xml(0, &f);
      xml.header();
      xml.stag(QStringLiteral("museScore version=\"" MSC_VERSION "\" full-version=\"%1\"").arg(fullVersion()));
      xml.tagE(cleanExit ? "clean" : "dirty");

      for (MasterScore*& score : scoreList) {
            xml.stag("Score");
            xml.tag("created", score->created());
            xml.tag("dirty", score->dirty());

            QString path;
            if (score->importedFilePath().isEmpty()) {
                              // score was not imported from another format, e.g. MIDI file
                  path = score->masterScore()->fileInfo()->absoluteFilePath();
                  }
            else if (score->masterScore()->fileInfo()->exists()) {   // score was saved
                  path = score->masterScore()->fileInfo()->absoluteFilePath();
                  }
            else {      // score was imported but not saved - store original file (e.g. MIDI) path
                  path = score->importedFilePath();
                  }

            if (cleanExit || score->tmpName().isEmpty()) {
                  xml.tag("path", path);
                  }
            else {
                  xml.tag("name", path);
                  xml.tag("path", score->tmpName());
                  }
            xml.etag();
            }
      int tab = 0;
      int idx = 0;
      for (int i = 0; i < tab1->count(); ++i) {
            ScoreView* v = tab1->view(i);
            if (v) {
                  if (v == cv) {
                        tab = 0;
                        idx = i;
                        }
                  xml.stag("ScoreView");
                  xml.tag("tab", tab);    // 0 instead of "tab" does not work
                  xml.tag("idx", i);
                  if (v->zoomIndex() == ZoomIndex::ZOOM_FREE)
                        xml.tag("mag", v->logicalZoomLevel());
                  else
                        xml.tag("magIdx", int(v->zoomIndex()));
                  xml.tag("x",   v->xoffset() / DPMM);
                  xml.tag("y",   v->yoffset() / DPMM);
                  xml.etag();
                  }
            }
      if (splitScreen()) {
            for (int i = 0; i < tab2->count(); ++i) {
                  ScoreView* v = tab2->view(i);
                  if (v) {
                        if (v == cv) {
                              tab = 1;
                              idx = i;
                              }
                        xml.stag("ScoreView");
                        xml.tag("tab", 1);
                        xml.tag("idx", i);
                        if (v->zoomIndex() == ZoomIndex::ZOOM_FREE)
                              xml.tag("mag", v->logicalZoomLevel());
                        else
                              xml.tag("magIdx", int(v->zoomIndex()));
                        xml.tag("x",   v->xoffset() / DPMM);
                        xml.tag("y",   v->yoffset() / DPMM);
                        xml.etag();
                        }
                  }
            }
      xml.tag("tab", tab);
      xml.tag("idx", idx);
      xml.etag();
      f.close();
      if (cleanExit) {
            // TODO: remove all temporary session backup files
            }
      }

//---------------------------------------------------------
//   removeSessionFile
//    remove temp files and session file
//---------------------------------------------------------

void MuseScore::removeSessionFile()
      {
      QFile f(dataPath + "/session");
      if (!f.exists())
            return;
      if (!f.remove()) {
            qDebug("cannot remove session file <%s>", qPrintable(f.fileName()));
            }
      }

//---------------------------------------------------------
//   autoSaveTimerTimeout
//---------------------------------------------------------

void MuseScore::autoSaveTimerTimeout()
      {
      bool sessionChanged = false;

      ScoreLoad sl;           //disable debug message "no active command"

      for (MasterScore* s : qAsConst(scoreList)) {
            if (s->autosaveDirty()) {
                  qDebug("<%s>", qPrintable(s->fileInfo()->completeBaseName()));
                  QString tmp = s->tmpName();
                  if (!tmp.isEmpty()) {
                        QFileInfo fi(tmp);
                        // TODO: cannot catch exception here:
                        s->saveCompressedFile(fi, false, false); // no thumbnail
                        }
                  else {
                        QDir dir;
                        dir.mkpath(dataPath);
                        QTemporaryFile tf(dataPath + "/scXXXXXX.mscz");
                        tf.setAutoRemove(false); // do not remove when tf goes out of scope!
                        if (!tf.open()) {
                              qDebug("autoSaveTimerTimeout(): create temporary file failed");
                              return;
                              }
                        s->setTmpName(tf.fileName());

                        QString fileName = QFileInfo(tf.fileName()).completeBaseName() + ".mscx";
                        s->saveCompressedFile(&tf, fileName, false, false);  // no thumbnail

                        tf.close();
                        sessionChanged = true;
                        }
                  s->setAutosaveDirty(false);
                  }
            }
      if (sessionChanged)
            writeSessionFile(false);
      if (preferences.getBool(PREF_APP_AUTOSAVE_USEAUTOSAVE)) {
            int t = preferences.getInt(PREF_APP_AUTOSAVE_AUTOSAVETIME) * 60 * 1000;
            autoSaveTimer->start(t);
            }
      }

class CallOnReturn {
      std::function<void()> f;
   public:
      CallOnReturn(std::function<void()> func) : f(std::move(func)) {}
      CallOnReturn(const CallOnReturn&) = delete;
      CallOnReturn& operator=(const CallOnReturn&) = delete;
      ~CallOnReturn() { f(); }
      };

//---------------------------------------------------------
//   restoreSession
//    Restore last session. If "always" is true, then restore
//    last session even on a clean exit else only if last
//    session ended abnormal.
//    Return true if a session was found and restored.
//---------------------------------------------------------

bool MuseScore::restoreSession(bool always)
      {
      bool sessionFileFound = false;
      bool cleanExit = false;
      QString sessionFullVersion;

      CallOnReturn onReturn([this, &sessionFileFound, &sessionFullVersion, &cleanExit]() {
            sessionStatusObserver.prevSessionStatus(sessionFileFound, sessionFullVersion, cleanExit);
            });

      QFile f(dataPath + "/session");
      if (!f.exists())
            return false;
      if (!f.open(QIODevice::ReadOnly)) {
            qDebug("Cannot open session file <%s>", qPrintable(f.fileName()));
            return false;
            }
      sessionFileFound = true;

      XmlReader e(&f);
      int tab = 0;
      int idx = -1;
      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  /* QString version = e.attribute(QString("version"));
                  QStringList sl  = version.split('.');
                  int v           = sl[0].toInt() * 100 + sl[1].toInt();
                  */
                  sessionFullVersion = e.attribute("full-version");
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "clean") {
                              cleanExit = true;
                              if (!always)
                                    return false;
                              e.readNext();
                              }
                        else if (tag == "dirty") {
                              QMessageBox::StandardButton b = QMessageBox::question(0,
                                 tr("MuseScore"),
                                 tr("The previous session quit unexpectedly.\n\nRestore session?"),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::Yes
                                 );
                              if (b != QMessageBox::Yes)
                                    return false;
                              e.readNext();
                              }
                        else if (tag == "Score") {
                              QString name;
                              bool created = false;
                              while (e.readNextStartElement()) {
                                    const QStringRef& t(e.name());
                                    if (t == "name")
                                          name = e.readElementText();
                                    else if (t == "created")
                                          created = e.readInt();
                                    else if (t == "dirty")
                                          /*int dirty =*/ e.readInt();
                                    else if (t == "path") {
                                          Score* score = openScore(e.readElementText(), false, true, name);
                                          if (score) {
                                                if (cleanExit) {
                                                      // override if last session did a clean exit
                                                      created = false;
                                                      }
                                                score->setCreated(created);
                                                }
                                          else {
                                                //! NOTE Return true so that there is no attempt to open this file again
                                                //! See: static void loadScores(const QStringList& argv)
                                                return true;
                                                }
                                          }
                                    else {
                                          e.unknown();
                                          return false;
                                          }
                                    }
                              }
                        else if (tag == "ScoreView") {
                              qreal x                = 0.0;
                              qreal y                = 0.0;
                              qreal logicalZoomLevel = 1.0;
                              ZoomIndex zoomIndex    = ZoomIndex::ZOOM_FREE;
                              int tab3               = 0;
                              int idx1               = 0;
                              while (e.readNextStartElement()) {
                                    const QStringRef& t(e.name());
                                    if (t == "tab")
                                          tab3 = e.readInt();
                                    else if (t == "idx")
                                          idx1 = e.readInt();
                                    else if (t == "mag")
                                          logicalZoomLevel = e.readDouble();
                                    else if (t == "magIdx") {
                                          zoomIndex = ZoomIndex(e.readInt());

                                          // The zoom level isn't saved along with the index, so reconstruct it if possible.
                                          const auto i = std::find(zoomEntries.cbegin(), zoomEntries.cend(), zoomIndex);
                                          if ((i != zoomEntries.cend()) && i->isNumericPreset())
                                                logicalZoomLevel = i->level / 100.0;
                                          }
                                    else if (t == "x")
                                          x = e.readDouble() * DPMM;
                                    else if (t == "y")
                                          y = e.readDouble() * DPMM;
                                    else {
                                          e.unknown();
                                          return false;
                                          }
                                    }
                              (tab3 == 0 ? tab1 : tab2)->queueInitScoreView(idx1, logicalZoomLevel, zoomIndex, x, y);
                              }
                        else if (tag == "tab")
                              tab = e.readInt();
                        else if (tag == "idx")
                              idx = e.readInt();
                        else {
                              e.unknown();
                              return false;
                              }
                        }
                  }
            else {
                  e.unknown();
                  return false;
                  }
            }
      setCurrentView(tab, idx);
      return true;
      }

//---------------------------------------------------------
//   splitWindow
//---------------------------------------------------------

void MuseScore::splitWindow(bool horizontal)
      {
      if (!_splitScreen) {
            if (tab2 == 0) {
                  tab2 = createScoreTab();
                  splitter->addWidget(tab2);
                  }
            tab2->setVisible(true);
            _splitScreen = true;
            _horizontalSplit = horizontal;
            splitter->setOrientation(_horizontalSplit ? Qt::Horizontal : Qt::Vertical);
            if (!scoreList.isEmpty()) {
                  tab2->setCurrentIndex(0);
                  MasterScore* s = scoreList[0];
                  s->setLayoutAll();
                  s->update();
                  setCurrentView(1, 0);
                  }
            emit windowSplit(true);
            }
      else {
            if (_horizontalSplit == horizontal) {
                  _splitScreen = false;
                  tab2->setVisible(false);
                  setCurrentView(0, tab1->currentIndex());
                  emit windowSplit(false);
                  }
            else {
                  _horizontalSplit = horizontal;
                  splitter->setOrientation(_horizontalSplit ? Qt::Horizontal : Qt::Vertical);
                  }
            }
      getAction("split-h")->setChecked(_splitScreen && _horizontalSplit);
      getAction("split-v")->setChecked(_splitScreen && !_horizontalSplit);
      }

//---------------------------------------------------------
//   stateName
//---------------------------------------------------------

const char* stateName(ScoreState s)
      {
      switch(s) {
            case STATE_DISABLED:           return "STATE_DISABLED";
            case STATE_NORMAL:             return "STATE_NORMAL";
            case STATE_NOTE_ENTRY_STAFF_PITCHED: return "STATE_NOTE_ENTRY_STAFF_PITCHED";
            case STATE_NOTE_ENTRY_STAFF_DRUM:    return "STATE_NOTE_ENTRY_STAFF_DRUM";
            case STATE_NOTE_ENTRY_STAFF_TAB:     return "STATE_NOTE_ENTRY_STAFF_TAB";
            case STATE_NOTE_ENTRY:         return "STATE_NOTE_ENTRY";
            case STATE_NOTE_ENTRY_METHOD_STEPTIME:          return "STATE_NOTE_ENTRY_METHOD_STEPTIME";
            case STATE_NOTE_ENTRY_METHOD_REPITCH:           return "STATE_NOTE_ENTRY_METHOD_REPITCH";
            case STATE_NOTE_ENTRY_METHOD_RHYTHM:            return "STATE_NOTE_ENTRY_METHOD_RHYTHM";
            case STATE_NOTE_ENTRY_METHOD_REALTIME_AUTO:     return "STATE_NOTE_ENTRY_METHOD_REALTIME_AUTO";
            case STATE_NOTE_ENTRY_METHOD_REALTIME_MANUAL:   return "STATE_NOTE_ENTRY_METHOD_REALTIME_MANUAL";
            case STATE_EDIT:               return "STATE_EDIT";
            case STATE_TEXT_EDIT:          return "STATE_TEXT_EDIT";
            case STATE_LYRICS_EDIT:        return "STATE_LYRICS_EDIT";
            case STATE_HARMONY_FIGBASS_EDIT: return "STATE_HARMONY_FIGBASS_EDIT";
            case STATE_PLAY:               return "STATE_PLAY";
            case STATE_FOTO:               return "STATE_FOTO";
            default:                       return "??";
            }
      }

//---------------------------------------------------------
//   scorePageLayoutChanged
//---------------------------------------------------------

void MuseScore::scorePageLayoutChanged()
      {
      if (mainWindow) {
            mainWindow->setOrientation(MScore::verticalOrientation() ? Qt::Horizontal : Qt::Vertical);
            if (navigatorScrollArea())
                  navigatorScrollArea()->orientationChanged();
            }
      }

//---------------------------------------------------------
//   editRaster
//---------------------------------------------------------

void MuseScore::editRaster()
      {
      if (editRasterDialog == 0) {
            editRasterDialog = new EditRaster(this);
            }
      if (editRasterDialog->exec()) {
            qDebug("=====accept config raster");
            }
      }

//---------------------------------------------------------
//   showPianoKeyboard
//---------------------------------------------------------

void MuseScore::showPianoKeyboard(bool visible)
      {
      if (_pianoTools == 0) {
            QAction* a = getAction("toggle-piano");
            _pianoTools = new PianoTools(this);
            addDockWidget(Qt::BottomDockWidgetArea, _pianoTools);
            connect(_pianoTools, SIGNAL(keyPressed(int,bool,int)), SLOT(midiNoteReceived(int,bool,int)));
            connect(_pianoTools, SIGNAL(keyReleased(int,bool,int)), SLOT(midiNoteReceived(int,bool,int)));
            connect(_pianoTools, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));
            }
      if (visible) {
            reDisplayDockWidget(_pianoTools, visible);
            if (currentScore())
                  _pianoTools->changeSelection(currentScore()->selection());
            else
                  _pianoTools->clearSelection();
            }
      else {
            if (_pianoTools)
                  _pianoTools->hide();
            }
      }

//---------------------------------------------------------
//   showPluginCreator
//---------------------------------------------------------

void MuseScore::showPluginCreator(QAction* a)
      {
#ifdef SCRIPT_INTERFACE
      bool on = a->isChecked();
      if (on) {
            if (_pluginCreator == 0) {
                  _pluginCreator = new PluginCreator(0);
                  connect(_pluginCreator, SIGNAL(closed(bool)), a, SLOT(setChecked(bool)));
                  }
            _pluginCreator->show();
            }
      else {
            if (_pluginCreator)
                  _pluginCreator->hide();
            }
#endif
      }

//---------------------------------------------------------
//   showPluginManager
//---------------------------------------------------------

void MuseScore::showPluginManager()
      {
#ifdef SCRIPT_INTERFACE
      pluginManager->init();
      pluginManager->show();
#endif
      }

//---------------------------------------------------------
//   showMediaDialog
//---------------------------------------------------------

void MuseScore::showMediaDialog()
      {
      if (_mediaDialog == 0)
            _mediaDialog = new MediaDialog(this);
      _mediaDialog->setScore(cs);
      _mediaDialog->exec();
      }

//---------------------------------------------------------
//   getPaletteWorkspace
//---------------------------------------------------------

PaletteWorkspace* MuseScore::getPaletteWorkspace()
      {
      if (!paletteWorkspace) {
            PaletteTreeModel* emptyModel = new PaletteTreeModel(new PaletteTree);
            PaletteTreeModel* masterPaletteModel = new PaletteTreeModel(MuseScore::newMasterPaletteTree());

            paletteWorkspace = new PaletteWorkspace(emptyModel, masterPaletteModel, /* parent */ this);
            emptyModel->setParent(paletteWorkspace);
            masterPaletteModel->setParent(paletteWorkspace);

            if (WorkspacesManager::currentWorkspace())
                  connect(paletteWorkspace, &PaletteWorkspace::userPaletteChanged, WorkspacesManager::currentWorkspace(), QOverload<>::of(&Workspace::setDirty), Qt::UniqueConnection);
            }

      return paletteWorkspace;
      }

//---------------------------------------------------------
//   qmlDockWidgets
//---------------------------------------------------------

std::vector<QmlDockWidget*> MuseScore::qmlDockWidgets()
      {
      return { paletteWidget };
      }

//---------------------------------------------------------
//   midiNoteReceived
//---------------------------------------------------------

void MuseScore::midiNoteReceived(int pitch, bool ctrl, int vel)
      {
      if (cv)
            cv->midiNoteReceived(pitch, ctrl, vel);
      }

//---------------------------------------------------------
//   switchLayer
//---------------------------------------------------------

void MuseScore::switchLayer(const QString& s)
      {
      if (cs->switchLayer(s)) {
            cs->setLayoutAll();
            cs->update();
            }
      }

//---------------------------------------------------------
//   switchPlayMode
//---------------------------------------------------------

void MuseScore::switchPlayMode(int mode)
      {
      if (cs)
            cs->setPlayMode(PlayMode(mode));
      }

//---------------------------------------------------------
//   networkFinished
//---------------------------------------------------------

void MuseScore::networkFinished()
      {
      QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
      if (reply->error() != QNetworkReply::NoError) {
            qDebug("Error while checking update [%s]", qPrintable(reply->errorString()));
            return;
            }
      QByteArray ha = reply->rawHeader("Content-Disposition");
      QString s(ha);
      QString name;
      QRegExp re(".*filename=\"(.*)\"");

      if (!s.isEmpty() && re.indexIn(s) != -1) {
            name = re.cap(1);
             }
      else {
            QUrl url = reply->url();
            QString path = url.path();
            qDebug("Path <%s>", qPrintable(path));
            QFileInfo fi(path);
            name = fi.fileName();
            }

      qDebug("header <%s>", qPrintable(s));
      qDebug("name <%s>", qPrintable(name));

      QByteArray dta = reply->readAll();
      QString tmpName = QDir::tempPath () + "/"+ name;
      QFile f(tmpName);
      f.open(QIODevice::WriteOnly);
      f.write(dta);
      f.close();

      reply->deleteLater();

      Score* score = openScore(tmpName);
      // delete the temp file created above
      QFile::remove(tmpName);
      if (!score) {
            qDebug("readScore failed");
            return;
            }
      score->setCreated(true);
      }

//---------------------------------------------------------
//   loadFile
//    load file from url
//---------------------------------------------------------

void MuseScore::loadFile(const QString& s)
      {
      loadFile(QUrl(s));
      }

void MuseScore::loadFile(const QUrl& url)
      {
      QEventLoop loop;
      QNetworkReply* nr = mscore->networkManager()->get(QNetworkRequest(url));
      connect(nr, SIGNAL(finished()), this,
               SLOT(networkFinished()));
      connect(nr, SIGNAL(finished()), &loop, SLOT(quit()));
      loop.exec();
      }

//---------------------------------------------------------
//   networkManager
//---------------------------------------------------------

QNetworkAccessManager* MuseScore::networkManager()
      {
      if (!_networkManager)
            _networkManager = new QNetworkAccessManager(this);
      return _networkManager;
      }

//---------------------------------------------------------
//   selectSimilar
//---------------------------------------------------------

void MuseScore::selectSimilar(Element* e, bool sameStaff)
      {
      Score* score = e->score();
      score->selectSimilar(e, sameStaff);

      if (score->selectionChanged()) {
            score->setSelectionChanged(false);
            SelState ss = score->selection().state();
            selectionChanged(ss);
            }
      }

void MuseScore::selectSimilarInRange(Element* e)
      {
      Score* score = e->score();
      score->selectSimilarInRange(e);

      if (score->selectionChanged()) {
            score->setSelectionChanged(false);
            SelState ss = score->selection().state();
            selectionChanged(ss);
            }
      }

//---------------------------------------------------------
//   selectElementDialog
//---------------------------------------------------------

void MuseScore::selectElementDialog(Element* e)
      {
      Score* score = e->score();
      if (e->isNote()) {
            Note* n = toNote(e);
            SelectNoteDialog sd(n, 0);
            // hide same string option if it doesn't make sense for the instrument
            if (n->staff()->part()->instrument()->stringData()->strings() == 0)
                  sd.setSameStringVisible(false);
            if (sd.exec()) {
                  NotePattern pattern;
                  sd.setPattern(&pattern);

                  if (sd.isInSelection())
                        score->scanElementsInRange(&pattern, Score::collectNoteMatch);
                  else
                        score->scanElements(&pattern, Score::collectNoteMatch);

                  if (sd.doReplace()) {
                        score->select(0, SelectType::SINGLE, 0);
                        for (Note* ee : qAsConst(pattern.el))
                              score->select(ee, SelectType::ADD, 0);
                        }
                  else if (sd.doSubtract()) {
                        QList<Element*> sl(score->selection().elements());
                        for (Note* ee : qAsConst(pattern.el))
                              sl.removeOne(ee);
                        score->select(0, SelectType::SINGLE, 0);
                        for (Element* ee : sl)
                              score->select(ee, SelectType::ADD, 0);
                        }
                  else if (sd.doAdd()) {
                        QList<Element*> sl(score->selection().elements());
                        for (Note* ee : qAsConst(pattern.el)) {
                              if(!sl.contains(ee))
                                    score->select(ee, SelectType::ADD, 0);
                              }
                        }
                  }
            }
      else {
            SelectDialog sd(e, 0);
            if (sd.exec()) {
                  ElementPattern pattern;
                  sd.setPattern(&pattern);

                  if (sd.isInSelection())
                        score->scanElementsInRange(&pattern, Score::collectMatch);
                  else
                        score->scanElements(&pattern, Score::collectMatch);

                  if (sd.doReplace()) {
                        score->select(0, SelectType::SINGLE, 0);
                        for (Element* ee : qAsConst(pattern.el))
                              score->select(ee, SelectType::ADD, 0);
                        }
                  else if (sd.doSubtract()) {
                        QList<Element*> sl(score->selection().elements());
                        for (Element* ee : qAsConst(pattern.el))
                              sl.removeOne(ee);
                        score->select(0, SelectType::SINGLE, 0);
                        for (Element* ee : sl)
                              score->select(ee, SelectType::ADD, 0);
                        }
                  else if (sd.doAdd()) {
                        QList<Element*> sl(score->selection().elements());
                        for (Element* ee : qAsConst(pattern.el)) {
                              if(!sl.contains(ee))
                                    score->select(ee, SelectType::ADD, 0);
                              }
                        }
                  }
            }
      if (score->selectionChanged()) {
            score->setSelectionChanged(false);
            SelState ss = score->selection().state();
            selectionChanged(ss);
            }
      }

//---------------------------------------------------------
//   RecordButton
//---------------------------------------------------------

RecordButton::RecordButton(QWidget* parent)
   : SimpleButton(":/data/recordOn.svg", ":/data/recordOff.svg", parent)
      {
      setCheckable(true);
      defaultAction()->setCheckable(true);
      setToolTip(qApp->translate("RecordButton", "Record"));
      }

//---------------------------------------------------------
//   GreendotButton
//---------------------------------------------------------

GreendotButton::GreendotButton(QWidget* parent)
   : SimpleButton(":/data/greendot.svg", ":/data/darkgreendot.svg", parent)
      {
      setCheckable(true);
      setToolTip(qApp->translate("GreendotButton", "Record"));
      }

//---------------------------------------------------------
//   drawHandle
//---------------------------------------------------------

QRectF drawHandle(QPainter& p, const QPointF& pos, bool active)
      {
      p.save();
      p.setPen(QPen(QColor(MScore::selectColor[0]), 2.0/p.worldTransform().m11()));
      if (active)
            p.setBrush(MScore::selectColor[0]);
      else
            p.setBrush(Qt::NoBrush);
      qreal w = 8.0 / p.worldTransform().m11();
      qreal h = 8.0 / p.worldTransform().m22();

      QRectF r(-w/2, -h/2, w, h);
      r.translate(pos);
      p.drawRect(r);
      p.restore();
      return r;
      }

//---------------------------------------------------------
//   transpose
//---------------------------------------------------------

void MuseScore::transpose()
      {
      if (cs->last() == 0)     // empty score?
            return;

      bool noSelection = cs->selection().isNone();
      if (noSelection)
            cs->cmdSelectAll();
      bool rangeSelection = cs->selection().isRange();
      TransposeDialog td;

      // TRANSPOSE_TO_KEY and "transpose keys" is only possible if selection state is SelState::RANGE
      td.enableTransposeKeys(rangeSelection);
      td.enableTransposeToKey(rangeSelection);
      td.enableTransposeChordNames(rangeSelection);

      int startStaffIdx = 0;
      int endStaffIdx   = 0;
      Fraction startTick = Fraction(0,1);
      if (rangeSelection) {
            startStaffIdx = cs->selection().staffStart();
            endStaffIdx   = cs->selection().staffEnd();
            startTick     = cs->selection().tickStart();
            }

      // find the key of the first pitched staff
      Key key = Key::C;
      for (int i = startStaffIdx; i < endStaffIdx; ++i) {
            Staff* staff = cs->staff(i);
            if (staff->isPitchedStaff(startTick)) {
                  key = staff->key(startTick);
                  if (!cs->styleB(Sid::concertPitch)) {
                        int diff = staff->part()->instrument(startTick)->transpose().chromatic;
                        if (diff)
                              key = transposeKey(key, diff, staff->part()->preferSharpFlat());
                        }
                  break;
                  }
            }

      td.setKey(key);
      if (!td.exec())
            return;

      cs->transpose(td.mode(), td.direction(), td.transposeKey(), td.transposeInterval(),
         td.getTransposeKeys(), td.getTransposeChordNames(), td.useDoubleSharpsFlats());

      if (noSelection)
            cs->deselectAll();
      }

//---------------------------------------------------------
//   realizeChordSymbols
///   Realize selected chord symbols into notes on the staff.
///   Display dialog to offer overrides to default behavior
//---------------------------------------------------------

void MuseScore::realizeChordSymbols()
      {
      if (!cs)
            return;
      if (!cs->selection().isList()) {
            QErrorMessage err;
            err.showMessage(tr("Invalid selection. Cannot realize chord symbol"));
            err.exec();
            return;
            }
      QList<Harmony*> hlist;
      for (Element* e : cs->selection().elements()) {
            if (e->isHarmony())
                  hlist << toHarmony(e);
            }

      RealizeHarmonyDialog dialog;
      if (!hlist.empty()) {
            dialog.setChordList(hlist);
            }
      else {
            QErrorMessage err;
            err.showMessage(tr("No chord symbol selected. Cannot realize chord symbol"));
            err.exec();
            return;
            }

      if (dialog.exec()) {    //realize the chord symbols onto the track
            cs->startCmd();
            cs->cmdRealizeChordSymbols(dialog.getLiteral(),
                                       dialog.optionsOverride() ? Voicing(dialog.getVoicing()) : Voicing::INVALID,
                                       dialog.optionsOverride() ? HDuration(dialog.getDuration()) : HDuration::INVALID);
            cs->endCmd();
            }
      }


//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void MuseScore::cmd(QAction* a)
      {
      if (inChordEditor)      // HACK
            return;

      MScore::setError(MS_NO_ERROR);

      QString cmdn(a->data().toString());

      if (MScore::debugMode)
            qDebug("MuseScore::cmd <%s>", qPrintable(cmdn));

      const Shortcut* sc = Shortcut::getShortcut(cmdn.toLatin1().data());
      if (!sc ) {
            qDebug("MuseScore::cmd(): unknown action <%s>", qPrintable(cmdn));
            return;
            }
      if (cs && (sc->state() & _sstate) == 0) {
            qDebug("invalid state %04x %04x", sc->state(), _sstate);
            QMessageBox::warning(0,
               tr("Invalid Command"),
               tr("Command %1 not valid in current state").arg(cmdn));
            return;
            }
      if (cmdn == "toggle-palette") {
            showPalette(a->isChecked());
            if (a->isChecked())
                  paletteWidget->activateSearchBox();
            else if (cv)
                  cv->setFocus();
            return;
            }
      if (cmdn == "palette-search") {
            showPalette(true);
            if (paletteWidget)
                  paletteWidget->activateSearchBox();
            return;
            }
      if (cmdn == "apply-current-palette-element") {
            if (paletteWidget)
                  paletteWidget->applyCurrentPaletteElement();
            return;
            }
      if (cmdn == "repeat-cmd") {
            a  = lastCmd;
            sc = lastShortcut;
            if (a == 0)
                  return;
            cmdn = a->data().toString();
            }
      else {
            lastCmd = a;
            lastShortcut = sc;
            }

      if (sc->needsScore() && ! cs) {
            qDebug("no score");
            return;
            }
      if (cs && sc->isCmd()) {
            if (!cv->editMode())
                  cs->startCmd();
            }
      cmd(a, cmdn);
      if (cs && lastShortcut->isCmd())
            cs->endCmd();
      else if (!lastShortcut->isUndoRedo()) // undoRedo() calls endCmd() itself
            endCmd();
      TourHandler::startTour(cmdn);
      }

//---------------------------------------------------------
//   endCmd
//    Called after every command action (including every
//    mouse action).
//    Updates the UI after a possible score change.
//---------------------------------------------------------

void MuseScore::endCmd(bool undoRedo)
      {
#ifdef SCRIPT_INTERFACE
      getPluginEngine()->beginEndCmd(this, undoRedo);
#endif
      if (timeline())
            timeline()->updateGridFromCmdState();
      if (MScore::_error != MS_NO_ERROR)
            showError();
      if (cs) {
            setPos(cs->inputState().tick());
            updateInputState(cs);
            updateUndoRedo();
            dirtyChanged(cs);
            scoreCmpTool->updateDiff();
            Element* e = cs->selection().element();

            // For multiple notes selected check if they all have same pitch and tuning
            bool samePitch = true;
            int pitch    = -1;
            float tuning = 0;
            for (Element* ee : cs->selection().elements()) {
                  if (!ee->isNote()) {
                        samePitch = false;
                        break;
                        }
                  Note* note = toNote(ee);
                  if (pitch == -1) {
                        pitch = note->ppitch();
                        tuning = note->tuning();
                        }
                  else if (note->ppitch() != pitch || fabs(tuning - note->tuning()) > 0.01) {
                        samePitch = false;
                        break;
                        }
                  }
            if (samePitch && pitch >= 0)
                  e = cs->selection().elements()[0];

            NoteEntryMethod entryMethod = cs->noteEntryMethod();
            if (e && (cs->playNote() || cs->playChord())
                        && entryMethod != NoteEntryMethod::REALTIME_AUTO
                        && entryMethod != NoteEntryMethod::REALTIME_MANUAL) {
                  if (cs->playChord() && preferences.getBool(PREF_SCORE_CHORD_PLAYONADDNOTE) &&  e->type() == ElementType::NOTE)
                        play(static_cast<Note*>(e)->chord());
                  else
                        play(e);
                  cs->setPlayNote(false);
                  cs->setPlayChord(false);
                  }
            MasterScore* ms = cs->masterScore();
            if (ms->excerptsChanged()) {
                  if (tab1) {
                        tab1->blockSignals(ctab != tab1);
                        tab1->updateExcerpts();
                        tab1->blockSignals(false);
                        }
                  if (tab2) {
                        tab2->blockSignals(ctab != tab2);
                        tab2->updateExcerpts();
                        tab2->blockSignals(false);
                        }
                  ms->setExcerptsChanged(false);
                  }
            if (ms->instrumentsChanged()) {
                  if (!noSeq && (seq && seq->isRunning()))
                        seq->initInstruments();
                  instrumentChanged();                // update mixer
                  ms->setInstrumentsChanged(false);
                  }
            if (cs->selectionChanged()) {
                  cs->setSelectionChanged(false);
                  SelState ss = cs->selection().state();
                  selectionChanged(ss);
                  }

            if (cv)
                  cv->moveViewportToLastEdit();

            getAction("concert-pitch")->setChecked(cs->styleB(Sid::concertPitch));

            updateViewModeCombo();
            ScoreAccessibility::instance()->updateAccessibilityInfo();
            }
      else {
            selectionChanged(SelState::NONE);
            }
      updateInspector();
      updatePaletteBeamMode();
#ifdef SCRIPT_INTERFACE
      getPluginEngine()->endEndCmd(this);
#endif
      }

//---------------------------------------------------------
//   updateUndoRedo
//---------------------------------------------------------

void MuseScore::updateUndoRedo()
      {
      QAction* a = getAction("undo");
      a->setEnabled(cs ? cs->undoStack()->canUndo() : false);
      a = getAction("redo");
      a->setEnabled(cs ? cs->undoStack()->canRedo() : false);
      }


void MuseScore::setPlayRepeats(bool repeat)
      {
      getAction("repeat")->setChecked(repeat);
      preferences.setPreference(PREF_APP_PLAYBACK_PLAYREPEATS, repeat);
      MScore::playRepeats = repeat;
      if (cs) {
            cs->masterScore()->setExpandRepeats(repeat);
            emit cs->playlistChanged();
            }
      }

void MuseScore::setPanPlayback(bool pan)
      {
      getAction("pan")->setChecked(pan);
      preferences.setPreference(PREF_APP_PLAYBACK_PANPLAYBACK, pan);
      MScore::panPlayback = pan;
      }

//---------------------------------------------------------
//   setPlayPartOnly
//---------------------------------------------------------

void MuseScore::setPlayPartOnly(bool val)
      {
      _playPartOnly = val;
      if (cs)
            cs->masterScore()->setPlaybackScore(_playPartOnly ? cs : cs->masterScore());
      }

//---------------------------------------------------------
//   createScoreTab
//---------------------------------------------------------

ScoreTab* MuseScore::createScoreTab()
      {
      ScoreTab* tab = new ScoreTab(&scoreList, this);
      tab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      connect(tab, SIGNAL(currentScoreViewChanged(ScoreView*)), SLOT(setCurrentScoreView(ScoreView*)));
      connect(tab, SIGNAL(tabCloseRequested(int)), SLOT(removeTab(int)));
      connect(tab, SIGNAL(actionTriggered(QAction*)), SLOT(cmd(QAction*)));
      return tab;
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void MuseScore::cmd(QAction* a, const QString& cmd)
      {
      if (ScriptRecorder* rec = getScriptRecorder())
            rec->recordCommand(cmd);

      if (cmd == "instruments")
            editInstrumentList();
      else if (cmd == "rewind") {
            if (cs) {
                  Fraction tick = loop() ? cs->loopInTick() : Fraction(0,1);
                  seq->seek(tick.ticks());
                  if (cv) {
                        Measure* m = cs->tick2measureMM(tick);
                        if (m)
                              cv->gotoMeasure(m);
                        }
                  if (playPanel)
                        playPanel->heartBeat(0, 0, 0);
                  }
            }
      else if (cmd == "play-next-measure")
            seq->nextMeasure();
      else if (cmd == "play-next-chord")
            seq->nextChord();
      else if (cmd == "play-prev-measure")
            seq->prevMeasure();
      else if (cmd == "play-prev-chord")
            seq->prevChord();
      else if (cmd == "seek-begin")
            seq->rewindStart();
      else if (cmd == "seek-end")
            seq->seekEnd();
      else if (cmd == "keys")
            showKeyEditor();
      else if (cmd == "file-new")
            newFile();
      else if (cmd == "file-open")
            openFiles();
      else if (cmd == "file-close")
            closeScore(cs);
      else if (cmd == "file-save")
            saveFile();
      else if (cmd == "file-save-as")
            saveAs(cs, false);
      else if (cmd == "file-save-a-copy")
            saveAs(cs, true);
      else if (cmd == "file-save-selection")
            saveSelection(cs);
      else if (cmd == saveOnlineMenuItem)
            showUploadScoreDialog();
      else if (cmd == "file-import-pdf")
            importScore();
      else if (cmd == "file-export")
            showExportDialog();
      else if (cmd == "unroll-repeats")
            scoreUnrolled(cs->masterScore());
      else if (cmd == "quit")
            close();
      else if (cmd == "masterpalette")
            showMasterPalette();
      else if (cmd == "key-signatures")
            showMasterPalette(qApp->translate("Palette", "Key Signatures"));
      else if (cmd == "time-signatures")
            showMasterPalette(qApp->translate("Palette", "Time Signatures"));
      else if (cmd == "symbols")
            showMasterPalette(qApp->translate("MasterPalette", "Symbols"));
      else if (cmd == "toggle-statusbar") {
            preferences.setPreference(PREF_UI_APP_SHOWSTATUSBAR, a->isChecked());
            _statusBar->setVisible(a->isChecked());
            }
      else if (cmd == "append-measures")
            cmdAppendMeasures();
      else if (cmd == "insert-measures")
            cmdInsertMeasures();
      else if (cmd == "debugger")
            startDebugger();
      else if (cmd == "album")
            showAlbumManager();
      else if (cmd == "layer" && enableExperimental)
            showLayerManager();
      else if (cmd == "backspace") {
            if (_sstate != STATE_NORMAL )
                  undoRedo(true);
#ifdef Q_OS_MAC
            else if (cv) {
                  cv->cmd(getAction("delete"));
                  return;
                  }
#endif
            }
      else if (cmd == "zoomin")
            zoomBySteps(1.0);
      else if (cmd == "zoomout")
            zoomBySteps(-1.0);
      else if (cmd == "zoom100")
            setZoom(ZoomIndex::ZOOM_100);
      else if (cmd == "zoom-page-width")
            setZoomWithToggle(ZoomIndex::ZOOM_PAGE_WIDTH);
      else if (cmd == "midi-on")
            enableMidiIn(a->isChecked());
      else if (cmd == "undo")
            undoRedo(true);
      else if (cmd == "redo")
            undoRedo(false);
      else if (cmd == "startcenter")
            showStartcenter(a->isChecked());
      else if (cmd == "inspector")
            showInspector(a->isChecked());
#ifdef OMR
      else if (cmd == "omr")
            showOmrPanel(a->isChecked());
#endif
      else if (cmd == "toggle-playpanel")
            showPlayPanel(a->isChecked());
      else if (cmd == "toggle-navigator")
            showNavigator(a->isChecked());
      else if (cmd == "toggle-timeline")
            showTimeline(a->isChecked());
      else if (cmd == "toggle-midiimportpanel")
            importmidiPanel->setVisible(a->isChecked());
      else if (cmd == "toggle-mixer")
            showMixer(a->isChecked());
      else if (cmd == "synth-control")
            showSynthControl(a->isChecked());
      else if (cmd == "toggle-selection-window")
            showSelectionWindow(a->isChecked());
      else if (cmd == "show-keys")
            ;
      else if (cmd == "toggle-fileoperations")
            fileTools->setVisible(!fileTools->isVisible());
      else if (cmd == "toggle-transport")
            transportTools->setVisible(!transportTools->isVisible());
      else if (cmd == "toggle-concertpitch")
            cpitchTools->setVisible(!cpitchTools->isVisible());
      else if (cmd == "toggle-imagecapture")
            fotoTools->setVisible(!fotoTools->isVisible());
      else if (cmd == "toggle-noteinput")
            entryTools->setVisible(!entryTools->isVisible());
#if 0
      else if (cmd == "toggle-feedback")
            feedbackTools->setVisible(!feedbackTools->isVisible());
#endif
      else if (cmd == "toggle-workspaces-toolbar")
            workspacesTools->setVisible(!workspacesTools->isVisible());
      else if (cmd == "create-new-workspace") {
            mscore->createNewWorkspace();
            emit mscore->workspacesChanged();
            }
      else if (cmd == "help")
            showContextHelp();
      else if (cmd == "follow")
            preferences.setPreference(PREF_APP_PLAYBACK_FOLLOWSONG, a->isChecked());
      else if (cmd == "split-h")
            splitWindow(true);
      else if (cmd == "split-v")
            splitWindow(false);
      else if (cmd == "edit-harmony" && enableExperimental)
            editChordStyle();
      else if (cmd == "parts")
            startExcerptsDialog();
      else if (cmd == "fullscreen") {
            _fullscreen = a->isChecked();
            if (_fullscreen)
                  showFullScreen();
            else
                  showNormal();

#ifdef Q_OS_MAC
            // Qt Bug: Toolbar goes into unified mode
            // after switching back from fullscreen
            setUnifiedTitleAndToolBarOnMac(false);
#endif
            }
      else if (cmd == "config-raster")
            editRaster();
      else if (cmd == "hraster" || cmd == "vraster")  // value in [hv]RasterAction already set
            ;
      else if (cmd == "toggle-piano")
            showPianoKeyboard(a->isChecked());
      else if (cmd == "toggle-scorecmp-tool")
            reDisplayDockWidget(scoreCmpTool, a->isChecked());
#ifdef MSCORE_UNSTABLE
      else if (cmd == "toggle-script-recorder")
            scriptRecorder->setVisible(a->isChecked());
#endif
      else if (cmd == "plugin-creator")
            showPluginCreator(a);
      else if (cmd == "plugin-manager")
            showPluginManager();
      else if(cmd == "resource-manager"){
            ResourceManager r(0);
            r.exec();
            }
      else if (cmd == "media" && enableExperimental)
            showMediaDialog();
      else if (cmd == "page-settings")
            showPageSettings();
      else if (cmd == "next-score")
            changeScore(1);
      else if (cmd == "previous-score")
            changeScore(-1);
      else if (cmd == "transpose")
            transpose();
      else if (cmd == "save-style") {
            QString name = getStyleFilename(false);
            if (!name.isEmpty()) {
                  if (!cs->saveStyle(name)) {
                        QMessageBox::critical(this,
                           tr("Save Style"), MScore::lastError);
                        }
                  }
            }
      else if (cmd == "load-style") {
            QString name = mscore->getStyleFilename(true);
            if (!name.isEmpty()) {
                  cs->startCmd();
                  if (!cs->loadStyle(name)) {
                        QMessageBox::StandardButton b = QMessageBox::warning(this, tr("Load Style"),
                           tr("MuseScore may not be able to load this style file: %1").arg(MScore::lastError),
                           QMessageBox::Cancel|QMessageBox::Ignore, QMessageBox::Cancel);
                        if (b == QMessageBox::Ignore)
                              cs->loadStyle(name, true);
                        }
                  cs->endCmd();
                  }
            }
      else if (cmd == "edit-style") {
            showStyleDialog();
            }
      else if (cmd == "edit-info") {
            MetaEditDialog med(cs, 0);
            med.exec();
            }
      else if (cmd == "print")
            printFile();
      else if (cmd == "repeat")
            setPlayRepeats(a->isChecked());
      else if (cmd == "pan")
            setPanPlayback(a->isChecked());
      else if (cmd == "show-invisible") {
            cs->setShowInvisible(a->isChecked());
            cs->update();
            }
      else if (cmd == "show-unprintable") {
            cs->setShowUnprintable(a->isChecked());
            cs->update();
            }
      else if (cmd == "show-frames") {
            cs->setShowFrames(a->isChecked());
            cs->update();
            }
      else if (cmd == "show-pageborders") {
            cs->setShowPageborders(a->isChecked());
            cs->update();
            }
      else if (cmd == "mark-irregular") {
            cs->setMarkIrregularMeasures(a->isChecked());
            cs->update();
            }
      else if (cmd == "tempo")
            addTempo();
      else if (cmd == "loop") {
            if (loop()) {
                  seq->setLoopSelection();
                  }
            }
      else if (cmd == "loop-in") {
            seq->setLoopIn();
            loopAction->setChecked(true);
            }
      else if (cmd == "loop-out") {
            seq->setLoopOut();
            loopAction->setChecked(true);
            }
      else if (cmd == "metronome")  // no action
            ;
      else if (cmd == "countin")    // no action
            ;
      else if (cmd == "playback-speed-increase") {
            createPlayPanel();
            playPanel->increaseSpeed();
            }
      else if (cmd == "playback-speed-decrease") {
            createPlayPanel();
            playPanel->decreaseSpeed();
            }
      else if (cmd == "playback-speed-reset") {
            createPlayPanel();
            playPanel->resetSpeed();
            }
      else if (cmd == "lock") {
            if (_sstate == STATE_LOCK)
                  changeState(STATE_NORMAL);
            else
                  changeState(STATE_LOCK);
            }
      else if (cmd == "find")
            showSearchDialog();
      else if (cmd == "text-b") {
            if (_textTools)
                  _textTools->toggleBold();
            }
      else if (cmd == "text-i") {
            if (_textTools)
                  _textTools->toggleItalic();
            }
      else if (cmd == "text-u") {
            if (_textTools)
                  _textTools->toggleUnderline();
            }
      else if (cmd == "text-s") {
            if (_textTools)
                  _textTools->toggleStrike();
            }
      else if (cmd == "edit-toolbars")
            showToolbarEditor();
      else if (cmd == "viewmode") {
            if (cs) {
                  if (cs->layoutMode() == LayoutMode::PAGE)
                        switchLayoutMode(LayoutMode::LINE);
                  else
                        switchLayoutMode(LayoutMode::PAGE);
                  }
            }
      else if (cmd == "show-tours")
            preferences.setPreference(PREF_UI_APP_STARTUP_SHOWTOURS, a->isChecked());
      else if (cmd == "reset-tours")
            tourHandler()->resetCompletedTours();
      else if (cmd == "report-bug")
            reportBug("panel");
      else if (cmd == "leave-feedback")
            leaveFeedback("panel");
#ifndef NDEBUG
      else if (cmd == "no-horizontal-stretch") {
            MScore::noHorizontalStretch = a->isChecked();
            if (cs) {
                  cs->setLayoutAll();
                  cs->update();
                  }
            }
      else if (cmd == "no-vertical-stretch") {
            MScore::noVerticalStretch = a->isChecked();
            if (cs) {
                  cs->setLayoutAll();
                  cs->update();
                  }
            }
      else if (cmd == "show-segment-shapes") {
            MScore::showSegmentShapes = a->isChecked();
            if (cs) {
                  cs->setLayoutAll();
                  cs->update();
                  }
            }
      else if (cmd == "show-skylines") {
            MScore::showSkylines = a->isChecked();
            if (cs) {
                  cs->setLayoutAll();
                  cs->update();
                  }
            }
      else if (cmd == "show-bounding-rect") {
            MScore::showBoundingRect = a->isChecked();
            if (cs) {
                  cs->setLayoutAll();
                  cs->update();
                  }
            }
      else if (cmd == "show-system-bounding-rect") {
            MScore::showSystemBoundingRect = a->isChecked();
            if (cs) {
                  cs->setLayoutAll();
                  cs->update();
                  }
            }
      else if (cmd == "show-corrupted-measures") {
            MScore::showCorruptedMeasures = a->isChecked();
            if (cs) {
                  cs->setLayoutAll();
                  cs->update();
                  }
            }
      else if (cmd == "qml-reload-source") {
            const QList<QmlDockWidget*> qmlWidgets = findChildren<QmlDockWidget*>();

            const QString oldPrefix = QmlDockWidget::qmlSourcePrefix();
            useSourceQmlFiles = true;
            const QString newPrefix = QmlDockWidget::qmlSourcePrefix();

            getQmlUiEngine()->clearComponentCache();

            for (QmlDockWidget* w : qmlWidgets) {
                  const QString urlString = w->source().toString().replace(oldPrefix, newPrefix);
                  w->setSource(QUrl(urlString));
                  }
            }
#endif
      else {
            if (cv) {
                  //isAncestorOf is called to see if a widget from inspector has focus
                  //if so, the focus doesn't get shifted to the score, unless escape is
                  //pressed, or the user clicks in the score
                  if (!inspector()->isAncestorOf(qApp->focusWidget()) || cmd == "escape")
                        cv->setFocus();
                  cv->cmd(a);
                  }
            else
                  qDebug("2:unknown cmd <%s>", qPrintable(cmd));
            }
      if (debugger)
            debugger->reloadClicked();
      }

//---------------------------------------------------------
//   openExternalLink
//---------------------------------------------------------

void MuseScore::openExternalLink(const QString& url)
      {
      QDesktopServices::openUrl(url);
      }

//---------------------------------------------------------
//   navigator
//---------------------------------------------------------

Navigator* MuseScore::navigator() const
      {
      return _navigator ? static_cast<Navigator*>(_navigator->widget()) : 0;
      }

//---------------------------------------------------------
//   timeline
//---------------------------------------------------------

Timeline* MuseScore::timeline() const
      {
      if (_timeline) {
            QSplitter* s = static_cast<QSplitter *>(_timeline->widget());
            if (s && s->count() > 0)
                  return _timeline ? static_cast<Timeline*>(s->widget(1)) : 0;
            return 0;
            }
      return 0;
      }

//---------------------------------------------------------
//   getScriptRecorder
//---------------------------------------------------------

ScriptRecorder* MuseScore::getScriptRecorder()
      {
#ifdef MSCORE_UNSTABLE
      if (scriptRecorder)
            return &scriptRecorder->scriptRecorder();
#endif
      return nullptr;
      }

//---------------------------------------------------------
//   getSearchDialog
//---------------------------------------------------------

QWidget* MuseScore::searchDialog() const
      {
      return _searchDialog;
      }

//---------------------------------------------------------
//   updateLayer
//---------------------------------------------------------

void MuseScore::updateLayer()
      {
      layerSwitch->clear();
      bool enable;
      if (cs) {
            enable = cs->layer().size() > 1;
            if (enable) {
                  for (Layer& l : cs->layer())
                        layerSwitch->addItem(l.name);
                  layerSwitch->setCurrentIndex(cs->currentLayer());
                  }
            }
      else
           enable = false;
      layerSwitch->setVisible(enable);
      }

//---------------------------------------------------------
//   updatePlayMode
//---------------------------------------------------------

void MuseScore::updatePlayMode()
      {
      bool enable = false;
      if (cs) {
            enable = cs->audio() != 0;
            playMode->setCurrentIndex(int(cs->playMode()));
            }
      playMode->setVisible(enable);
      }

//---------------------------------------------------------
//   closeScore
//---------------------------------------------------------

void MuseScore::closeScore(Score* score)
      {
      // Let's compute maximum count of ports in remaining scores
      if (seq)
            seq->recomputeMaxMidiOutPort();
      removeTab(scoreList.indexOf(score->masterScore()));
      }

//---------------------------------------------------------
//   noteTooShortForTupletDialog
//---------------------------------------------------------

void MuseScore::noteTooShortForTupletDialog()
      {
      QMessageBox::warning(this, tr("Warning"),
        tr("Cannot create tuplet: Note value is too short")
        );
      }

//---------------------------------------------------------
//   instrumentChanged
//---------------------------------------------------------

void MuseScore::instrumentChanged()
      {
      if (mixer)
            mixer->setScore(cs);
      }

//---------------------------------------------------------
//   mixerPreferencesChanged
//---------------------------------------------------------

void MuseScore::mixerPreferencesChanged(bool showMidiControls)
      {
      if (mixer)
            mixer->midiPrefsChanged(showMidiControls);
      }

//---------------------------------------------------------
//   checkForUpdates
//---------------------------------------------------------

void MuseScore::checkForUpdates()
      {
#ifndef MSCORE_NO_UPDATE_CHECKER
      if (hasToCheckForUpdate())
            checkForUpdatesNoUI();
#endif
      }

//---------------------------------------------------------
//   changeScore
//    switch current score
//    step = 1    switch to next score
//    step = -1   switch to previous score
//---------------------------------------------------------

void MuseScore::changeScore(int step)
      {
      int index = tab1->currentIndex();
      int n     = tab1->count();
      if (n == 1)
            return;
      index += step;
      if (index >= n)
            index = 0;
      if (index < 0)
            index = n - 1;
      setCurrentScoreView(index);
      }

//---------------------------------------------------------
//   switchLayoutMode
//    val is index in QComboBox viewModeCombo
//---------------------------------------------------------

void MuseScore::switchLayoutMode(int val)
      {
      if (!cs)
            return;
      switchLayoutMode(static_cast<LayoutMode>(viewModeCombo->itemData(val).toInt()));
      }

void MuseScore::switchLayoutMode(LayoutMode mode)
      {
      // find a measure to use as reference, if possible
      QRectF view = cv->toLogical(QRect(0.0, 0.0, width(), height()));
      Measure* m = cs->firstMeasureMM();
      while (m && !view.intersects(m->canvasBoundingRect()))
            m = m->nextMeasureMM();

      cv->loopUpdate(getAction("loop")->isChecked());

      if (mode != cs->layoutMode()) {
            cs->setLayoutMode(mode);
            cs->doLayout();
            }

      // adjustCanvasPosition often tries to preserve Y position
      // but this doesn't make sense when switching modes
      // also, better positioning is usually achieved if you start from the top
      // and there is really no better place to position canvas if we were all the way off page previously
      cv->pageTop();
      if (m && m != cs->firstMeasureMM())
            cv->adjustCanvasPosition(m, false);
      if (cv->noteEntryMode())
            cv->moveCursor();
      }

//---------------------------------------------------------
//   showDrumTools
//---------------------------------------------------------

void MuseScore::showDrumTools(const Drumset* drumset, Staff* staff)
      {
      if (drumset) {
            if (!_drumTools) {
                  _drumTools = new DrumTools(this);
                  addDockWidget(Qt::BottomDockWidgetArea, _drumTools);
                  }
            if (timelineScrollArea())
                  splitDockWidget(_drumTools, timelineScrollArea(), Qt::Vertical);
            _drumTools->setDrumset(cs, staff, drumset);
            _drumTools->show();
            }
      else {
            if (_drumTools)
                  _drumTools->hide();
            }
      }

//---------------------------------------------------------
//   updateDrumTools
//---------------------------------------------------------

void MuseScore::updateDrumTools(const Drumset* ds)
      {
      if (_drumTools)
            _drumTools->updateDrumset(ds);
      }

//---------------------------------------------------------
//   endSearch
//---------------------------------------------------------

void MuseScore::endSearch()
      {
      _searchDialog->hide();
      if (cv)
            cv->setFocus();
      }

//---------------------------------------------------------
//   showSearchDialog
//---------------------------------------------------------

void MuseScore::showSearchDialog()
      {
      if (_searchDialog == 0) {
            _searchDialog = new QWidget;
            _searchDialog->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
            QHBoxLayout* searchDialogLayout = new QHBoxLayout;
            _searchDialog->setLayout(searchDialogLayout);
            layout->insertWidget(2, _searchDialog);

            QToolButton* searchExit = new QToolButton;
            searchExit->setAutoRaise(true);
            searchExit->setIcon(*icons[int(Icons::close_ICON)]);
            connect(searchExit, SIGNAL(clicked()), SLOT(endSearch()));
            searchDialogLayout->addWidget(searchExit);
            searchDialogLayout->addSpacing(10);

            searchDialogLayout->addWidget(new QLabel(tr("Find / Go to:")));

            searchCombo = new SearchComboBox;
            searchDialogLayout->addWidget(searchCombo);

            searchDialogLayout->addStretch(10);
            _searchDialog->hide();

            qDebug("Line edit %p", searchCombo->lineEdit());

            connect(searchCombo->lineEdit(), SIGNAL(returnPressed()), SLOT(endSearch()));
            }

      searchCombo->clearEditText();
      searchCombo->setFocus();
      _searchDialog->show();
      }



#ifndef SCRIPT_INTERFACE
void MuseScore::pluginTriggered(int) {}
void MuseScore::pluginTriggered(QString path) {}
void MuseScore::loadPlugins() {}
bool MuseScore::loadPlugin(const QString&) { return false;}
void MuseScore::unloadPlugins() {}
#endif


void MuseScore::saveGeometry(QWidget const*const qw)
      {
      QSettings settings;
      QString objectName = qw->objectName();
      Q_ASSERT(!objectName.isEmpty());
      settings.beginGroup("Geometries");
      settings.setValue(objectName, qw->saveGeometry());
      settings.endGroup();
      }

void MuseScore::restoreGeometry(QWidget *const qw)
      {
      if (!useFactorySettings) {
            QSettings settings;
            QString objectName = qw->objectName();
            Q_ASSERT(!objectName.isEmpty());
            settings.beginGroup("Geometries");
            qw->restoreGeometry(settings.value(objectName).toByteArray());
            settings.endGroup();
            }
      }

void MuseScore::updateWindowTitle(Score* score)
      {
      if (!score) {
            setWindowTitle(MUSESCORE_NAME_VERSION);
            setWindowFilePath(QString());
            return;
            }
      QString scoreTitle;
      if (score->isMaster())
            scoreTitle = score->title();
      else
            scoreTitle = score->masterScore()->title() + "-" + score->title();
#ifdef Q_OS_MAC
      setWindowTitle(scoreTitle);
      if (score->masterScore()->created())
            setWindowFilePath(QString());
      else
            setWindowFilePath(score->masterScore()->fileInfo()->absoluteFilePath());
#else
      setWindowTitle(MUSESCORE_NAME_VERSION ": " + scoreTitle + "[*]");
#endif
      }

//---------------------------------------------------------
//   recentScores
//    return a list of recent scores
//    omit loaded scores
//---------------------------------------------------------

QFileInfoList MuseScore::recentScores() const
      {
      QFileInfoList fil;
      for (const QString& s : _recentScores) {
            if (s.isEmpty())
                  continue;
            QFileInfo fi(s);
            bool alreadyLoaded = false;
            QString fp = fi.canonicalFilePath();
            for (Score* sc : qAsConst(mscore->scores())) {
                  if ((sc->masterScore()->fileInfo()->canonicalFilePath() == fp) || (sc->importedFilePath() == fp)) {
                        alreadyLoaded = true;
                        break;
                        }
                  }
            if (!alreadyLoaded && fi.exists())
                  fil.append(fi);
            }
      return fil;
      }

//---------------------------------------------------------
//   createPopupMenu
//---------------------------------------------------------

QMenu* MuseScore::createPopupMenu()
      {
      QMenu* m = QMainWindow::createPopupMenu();
      QList<QAction*> al = m->actions();
      for (QAction* a : qAsConst(al)) {
            // textTool visibility is handled differentlyr
            if (_textTools && a->text() == _textTools->windowTitle())
                  m->removeAction(a);
            }
      return m;
      }

//---------------------------------------------------------
//   editInstrumentList
//---------------------------------------------------------

void MuseScore::editInstrumentList()
      {
      editInstrList();
      if (mixer)
            mixer->setScore(cs);
      }

//---------------------------------------------------------
//   synthesizerState
//---------------------------------------------------------

SynthesizerState MuseScore::synthesizerState() const
      {
      SynthesizerState state;
      return synti ? synti->state() : state;
      }

//---------------------------------------------------------
//   synthesizer
//---------------------------------------------------------

Synthesizer* MuseScore::synthesizer(const QString& name)
      {
      return synti ? synti->synthesizer(name) : nullptr;
      }

//---------------------------------------------------------
//   canSaveMp3
//---------------------------------------------------------

bool MuseScore::canSaveMp3()
      {
#ifndef USE_LAME
      return false;
#else
      MP3Exporter exporter;
      if (!exporter.loadLibrary(MP3Exporter::AskUser::NO)) {
            qDebug("Could not open MP3 encoding library!");
            return false;
            }

      if (!exporter.validLibraryLoaded()) {
            qDebug("Not a valid or supported MP3 encoding library!");
            return false;
            }
      return true;
#endif
      }

//---------------------------------------------------------
//   saveMp3
//---------------------------------------------------------

bool MuseScore::saveMp3(Score* score, const QString& name)
      {
      QFile file(name);
      if (!file.open(QIODevice::WriteOnly)) {
            if (!MScore::noGui) {
                  QMessageBox::warning(0,
                                       tr("Encoding Error"),
                                       tr("Unable to open target file for writing"),
                                       QString(), QString());
                  }
            return false;
            }
      bool wasCanceled = false;
      bool res = saveMp3(score, &file, wasCanceled);
      file.close();
      if (wasCanceled || !res)
            file.remove();
      return res;
      }

bool MuseScore::saveMp3(Score* score, QIODevice* device, bool& wasCanceled)
      {
#ifndef USE_LAME
      Q_UNUSED(score);
      Q_UNUSED(device);
      Q_UNUSED(wasCanceled);
      return false;
#else
      EventMap events;
      // In non-GUI mode current synthesizer settings won't
      // allow single note dynamics. See issue #289947.
      const bool useCurrentSynthesizerState = !MScore::noGui;

      if (useCurrentSynthesizerState) {
            score->renderMidi(&events, synthesizerState());
            if (events.empty())
                  return false;
            }

      MP3Exporter exporter;
      if (!exporter.loadLibrary(MP3Exporter::AskUser::MAYBE)) {
            QSettings set;
            set.setValue("/Export/lameMP3LibPath", "");
            if(!MScore::noGui)
                  QMessageBox::warning(0,
                               tr("Error Opening LAME library"),
                               tr("Could not open MP3 encoding library!"),
                               QString(), QString());
            qDebug("Could not open MP3 encoding library!");
            return false;
            }

      if (!exporter.validLibraryLoaded()) {
            QSettings set;
            set.setValue("/Export/lameMP3LibPath", "");
            if(!MScore::noGui)
                  QMessageBox::warning(0,
                               tr("Error Opening LAME library"),
                               tr("Not a valid or supported MP3 encoding library!"),
                               QString(), QString());
            qDebug("Not a valid or supported MP3 encoding library!");
            return false;
            }

      // Retrieve preferences
//      int highrate = 48000;
//      int lowrate = 8000;
//      int bitrate = 64;
//      int brate = 128;
//      int rmode = MODE_CBR;
//      int vmode = ROUTINE_FAST;
//      int cmode = CHANNEL_STEREO;

      int channels = 2;

      int oldSampleRate = MScore::sampleRate;
      int sampleRate = preferences.getInt(PREF_EXPORT_AUDIO_SAMPLERATE);
      exporter.setBitrate(preferences.getInt(PREF_EXPORT_MP3_BITRATE));

      int inSamples = exporter.initializeStream(channels, sampleRate);
      if (inSamples < 0) {
            if (!MScore::noGui) {
                  QMessageBox::warning(0, tr("Encoding Error"),
                     tr("Unable to initialize MP3 stream"),
                     QString(), QString());
                  }
            qDebug("Unable to initialize MP3 stream");
            MScore::sampleRate = oldSampleRate;
            return false;
            }

      int bufferSize   = exporter.getOutBufferSize();
      uchar* bufferOut = new uchar[bufferSize];
      MasterSynthesizer* synth = synthesizerFactory();
      synth->init();
      synth->setSampleRate(sampleRate);

      const SynthesizerState state = useCurrentSynthesizerState ? mscore->synthesizerState() : score->synthesizerState();
      const bool setStateOk = synth->setState(state);

      if (!setStateOk || !synth->hasSoundFontsLoaded()) {
            synth->init(); // re-initialize master synthesizer with default settings
            synth->setSampleRate(sampleRate);
      }

      MScore::sampleRate = sampleRate;

      if (!useCurrentSynthesizerState) {
            score->masterScore()->rebuildAndUpdateExpressive(synth->synthesizer("Fluid"));
            score->renderMidi(&events, score->synthesizerState());
            if (synti)
                  score->masterScore()->rebuildAndUpdateExpressive(synti->synthesizer("Fluid"));

            if (events.empty()) {
                  delete[] bufferOut;
                  return false;
                  }
            }

      QProgressDialog progress(this);
      progress.setWindowFlags(Qt::WindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowTitleHint));
      progress.setWindowModality(Qt::ApplicationModal);
      //progress.setCancelButton(0);
      progress.setCancelButtonText(tr("Cancel"));
      progress.setLabelText(tr("Exporting…"));
      if (!MScore::noGui)
            progress.show();

      static const int FRAMES = 512;
      float bufferL[FRAMES];
      float bufferR[FRAMES];

      float  peak = 0.0;
      double gain = 1.0;
      EventMap::const_iterator endPos = events.cend();
      --endPos;
      const int et = (score->utick2utime(endPos->first) + 1) * MScore::sampleRate;
      const int maxEndTime = (score->utick2utime(endPos->first) + 3) * MScore::sampleRate;
      progress.setRange(0, et);

      for (int pass = 0; pass < 2; ++pass) {
            EventMap::const_iterator playPos;
            playPos = events.cbegin();
            synth->allSoundsOff(-1);

            //
            // init instruments
            //
            for (Part* part : qAsConst(score->parts())) {
                  const InstrumentList* il = part->instruments();
                  for (auto i = il->begin(); i!= il->end(); i++) {
                        for (const Channel* channel : i->second->channel()) {
                              const Channel* a = score->masterScore()->playbackChannel(channel);
                              for (MidiCoreEvent e : a->initList()) {
                                    if (e.type() == ME_INVALID)
                                          continue;
                                    e.setChannel(a->channel());
                                    int syntiIdx= synth->index(score->masterScore()->midiMapping(a->channel())->articulation()->synti());
                                    synth->play(e, syntiIdx);
                                    }
                              }
                        }
                  }

            int playTime = 0.0;

            for (;;) {
                  unsigned frames = FRAMES;
                  float max = 0;
                  //
                  // collect events for one segment
                  //
                  memset(bufferL, 0, sizeof(float) * FRAMES);
                  memset(bufferR, 0, sizeof(float) * FRAMES);
                  double endTime = playTime + frames;

                  float* l = bufferL;
                  float* r = bufferR;

                  for (; playPos != events.cend(); ++playPos) {
                        double f = score->utick2utime(playPos->first) * MScore::sampleRate;
                        if (f >= endTime)
                              break;
                        int n = f - playTime;
                        if (n) {
                              std::vector<float> vBu(n * 2, 0);   // Default initialized, memset() not required.
                              float* bu = vBu.data();
                              synth->process(n, bu);
                              float* sp = bu;
                              for (int i = 0; i < n; ++i) {
                                    *l++ = *sp++;
                                    *r++ = *sp++;
                                    }
                              playTime  += n;
                              frames    -= n;
                              }
                        const NPlayEvent& e = playPos->second;
                        if (!(!e.velo() && e.discard()) && e.isChannelEvent()) {
                              int channelIdx = e.channel();
                              Channel* c = score->masterScore()->midiMapping(channelIdx)->articulation();
                              if (!c->mute()) {
                                    synth->play(e, synth->index(c->synti()));
                                    }
                              }
                        }
                  if (frames) {
                        std::vector<float> vBu(frames * 2, 0);   // Default initialized, memset() not required.
                        float* bu = vBu.data();
                        synth->process(frames, bu);
                        float* sp = bu;
                        for (unsigned i = 0; i < frames; ++i) {
                              *l++ = *sp++;
                              *r++ = *sp++;
                              }
                        }

                  if (pass == 1) {
                        for (int i = 0; i < FRAMES; ++i) {
                              max = qMax(max, qAbs(bufferL[i]));
                              max = qMax(max, qAbs(bufferR[i]));
                              bufferL[i] *= gain;
                              bufferR[i] *= gain;
                              }
                        long bytes;
                        if (FRAMES < inSamples)
                              bytes = exporter.encodeRemainder(bufferL, bufferR,  FRAMES , bufferOut);
                        else
                              bytes = exporter.encodeBuffer(bufferL, bufferR, bufferOut);
                        if (bytes < 0) {
                              if (MScore::noGui)
                                    qDebug("exportmp3: error from encoder: %ld", bytes);
                              else
                                    QMessageBox::warning(0,
                                       tr("Encoding Error"),
                                       tr("Error %1 returned from MP3 encoder").arg(bytes),
                                       QString(), QString());
                              break;
                              }
                        else
                              device->write((char*)bufferOut, bytes);
                        }
                  else {
                        for (int i = 0; i < FRAMES; ++i) {
                              max = qMax(max, qAbs(bufferL[i]));
                              max = qMax(max, qAbs(bufferR[i]));
                              peak = qMax(peak, qAbs(bufferL[i]));
                              peak = qMax(peak, qAbs(bufferR[i]));
                              }
                        }
                  playTime = endTime;
                  if (!MScore::noGui) {
                        if (progress.wasCanceled())
                              break;
                        progress.setValue((pass * et + playTime) / 2);
                        qApp->processEvents();
                        }
                  if (playTime >= et)
                        synth->allNotesOff(-1);
                  // create sound until the sound decays
                  if (playTime >= et && max * peak < 0.000001)
                        break;
                  // hard limit
                  if (playTime > maxEndTime)
                        break;
                  }
            if (progress.wasCanceled())
                  break;
            if (pass == 0 && qFuzzyIsNull(peak)) {
                  qDebug("song is empty");
                  break;
                  }
            gain = 0.99 / peak;
            }

      long bytes = exporter.finishStream(bufferOut);
      if (bytes > 0L)
            device->write((char*)bufferOut, bytes);
      wasCanceled = progress.wasCanceled();
      progress.close();
      delete synth;
      delete[] bufferOut;
      MScore::sampleRate = oldSampleRate;
      return true;
#endif
      }

#ifndef TELEMETRY_DISABLED

//---------------------------------------------------------
//   tryToRequestTelemetryPermission
//---------------------------------------------------------

void tryToRequestTelemetryPermission()
      {
      static const QString telemetryVersion = "3.4.1";
      QString accessRequestedAtVersion = preferences.getString(PREF_APP_STARTUP_TELEMETRY_ACCESS_REQUESTED);

      if (accessRequestedAtVersion == telemetryVersion)
            return;

      // Disable telemetry until the permission is explicitly confirmed by user
      preferences.setPreference(PREF_APP_TELEMETRY_ALLOWED, false);

      if (MScore::noGui)
            return;

      QEventLoop eventLoop;

      TelemetryPermissionDialog *requestDialog = new TelemetryPermissionDialog(mscore->getQmlUiEngine());
      QObject::connect(requestDialog, &TelemetryPermissionDialog::closeRequested, [&eventLoop] () {
            eventLoop.quit();
            });

      requestDialog->show();
      eventLoop.exec();
      requestDialog->deleteLater();

      preferences.setPreference(PREF_APP_STARTUP_TELEMETRY_ACCESS_REQUESTED, telemetryVersion);
      }
#endif

//---------------------------------------------------------
//   updateUiStyleAndTheme
//---------------------------------------------------------

void MuseScore::updateUiStyleAndTheme()
      {
      // set UI Theme
      QApplication::setStyle(QStyleFactory::create("Fusion"));

#ifdef Q_OS_MAC
      // On Mac, update the color of the window title bars
      CocoaBridge::setWindowAppearanceIsDark(preferences.isThemeDark());
#endif

#if defined(WIN_PORTABLE)
      QString wd = QDir::cleanPath(QString("%1/../../../Data/%2").arg(QCoreApplication::applicationDirPath(), QCoreApplication::applicationName()));
#else
      QString wd = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), QCoreApplication::applicationName());
#endif

      // set UI Color Palette
      QPalette p(QApplication::palette());
      QString jsonPaletteFilename = preferences.isThemeDark() ? "palette_dark_fusion.json" : "palette_light_fusion.json";;
      QFile jsonPalette(QString(":/themes/%1").arg(jsonPaletteFilename));
      // read from Documents TODO: remove this
      if (QFile::exists(QString("%1/%2").arg(wd, "ms_palette.json")))
            jsonPalette.setFileName(QString("%1/%2").arg(wd, "ms_palette.json"));
      if (jsonPalette.open(QFile::ReadOnly | QFile::Text)) {
            QJsonDocument d = QJsonDocument::fromJson(jsonPalette.readAll());
            QJsonObject o = d.object();
            QMetaEnum metaEnum = QMetaEnum::fromType<QPalette::ColorRole>();
            for (int i = 0; i < metaEnum.keyCount(); ++i) {
                  QJsonValue v = o.value(metaEnum.valueToKey(i));
                  if (!v.isUndefined())
                        p.setColor(static_cast<QPalette::ColorRole>(metaEnum.value(i)), QColor(v.toString()));
                  }
            }
      QApplication::setPalette(p);

      // set UI Style
      QString css;
      QString styleFilename = preferences.isThemeDark() ? "style_dark_fusion.css" : "style_light_fusion.css";
      QFile fstyle(QString(":/themes/%1").arg(styleFilename));
      // read from Documents TODO: remove this
      if (QFile::exists(QString("%1/%2").arg(wd, "ms_style.css")))
            fstyle.setFileName(QString("%1/%2").arg(wd, "ms_style.css"));
      if (fstyle.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&fstyle);
            css = in.readAll();
            }

      css.replace("$voice1-bgcolor", MScore::selectColor[0].name(QColor::HexRgb));
      css.replace("$voice2-bgcolor", MScore::selectColor[1].name(QColor::HexRgb));
      css.replace("$voice3-bgcolor", MScore::selectColor[2].name(QColor::HexRgb));
      css.replace("$voice4-bgcolor", MScore::selectColor[3].name(QColor::HexRgb));

      if (preferences.isThemeDark()) {
            css.replace("$buttonHighlightDisabledOff", Ms::preferences.getColor(PREF_UI_BUTTON_HIGHLIGHT_COLOR_DISABLED_DARK_OFF).name().toLatin1());
            css.replace("$buttonHighlightDisabledOn", Ms::preferences.getColor(PREF_UI_BUTTON_HIGHLIGHT_COLOR_DISABLED_DARK_ON).name().toLatin1());
            css.replace("$buttonHighlightEnabledOff", Ms::preferences.getColor(PREF_UI_BUTTON_HIGHLIGHT_COLOR_ENABLED_DARK_OFF).name().toLatin1());
            css.replace("$buttonHighlightEnabledOn", Ms::preferences.getColor(PREF_UI_BUTTON_HIGHLIGHT_COLOR_ENABLED_DARK_ON).name().toLatin1());
            }
      else {
            css.replace("$buttonHighlightDisabledOff", Ms::preferences.getColor(PREF_UI_BUTTON_HIGHLIGHT_COLOR_DISABLED_LIGHT_OFF).name().toLatin1());
            css.replace("$buttonHighlightDisabledOn", Ms::preferences.getColor(PREF_UI_BUTTON_HIGHLIGHT_COLOR_DISABLED_LIGHT_ON).name().toLatin1());
            css.replace("$buttonHighlightEnabledOff", Ms::preferences.getColor(PREF_UI_BUTTON_HIGHLIGHT_COLOR_ENABLED_LIGHT_OFF).name().toLatin1());
            css.replace("$buttonHighlightEnabledOn", Ms::preferences.getColor(PREF_UI_BUTTON_HIGHLIGHT_COLOR_ENABLED_LIGHT_ON).name().toLatin1());
            }

      qApp->setStyleSheet(css);

      QString style = QString("*, QSpinBox { font: %1pt \"%2\" } ")
                  .arg(QString::number(preferences.getDouble(PREF_UI_THEME_FONTSIZE)), preferences.getString(PREF_UI_THEME_FONTFAMILY))
                  + qApp->styleSheet();
      qApp->setStyleSheet(style);

      genIcons();
      Shortcut::refreshIcons();
      }

//---------------------------------------------------------
//   fullVersion
///   \return Full version of MuseScore, including version
///   label (e.g. 3.4.0-Beta).
//---------------------------------------------------------

QString MuseScore::fullVersion()
      {
      QString version(VERSION);
      QString versionLabel(VERSION_LABEL);
      versionLabel = versionLabel.replace(' ', "");
      if (!versionLabel.isEmpty())
            version.append("-").append(versionLabel);
      return version;
      }

//---------------------------------------------------------
//   initApplication
//---------------------------------------------------------

MuseScoreApplication* MuseScoreApplication::initApplication(int& argc, char** argv)
      {
      revision = QString(MUSESCORE_REVISION).trimmed();

      const char* appName;
      const char* appName2;
      if (MuseScore::unstable()) {
            appName2 = "mscore-dev3";
            appName  = "MuseScore3Development";
            }
      else {
            appName2 = "mscore3";
            appName  = "MuseScore3";
            }

      //! NOTE Disable cache for all platforms
      //! Enabled qml cache on MacOS BigSur on ARM leads to a crash at the start
      //! There were also crash problems on some Linux platforms when upgrading
      //! In general, the Qml cache doesn't really matter.
      //! Therefore, it is better to turn it off.
      qputenv("QML_DISABLE_DISK_CACHE", "true");

      MuseScoreApplication* app = new MuseScoreApplication(appName2, argc, argv);
      QCoreApplication::setApplicationName(appName);

      QCoreApplication::setOrganizationName("MuseScore");
      QCoreApplication::setOrganizationDomain("musescore.org");
      QCoreApplication::setApplicationVersion(MuseScore::fullVersion());

#ifdef BUILD_CRASH_REPORTER
      {
      static_assert(sizeof(CRASHREPORTER_EXECUTABLE) > 1,
         "CRASHREPORTER_EXECUTABLE should be defined to build with crash reporter"
         );
      CrashReporter::Handler* crashHandler = new CrashReporter::Handler(QDir::tempPath(), true, CRASHREPORTER_EXECUTABLE);
      QObject::connect(app, &QCoreApplication::aboutToQuit, [crashHandler]() { delete crashHandler; });
      }
#endif

      return app;
      }

//---------------------------------------------------------
//   setCustomConfigFolder
//---------------------------------------------------------

bool MuseScoreApplication::setCustomConfigFolder(const QString& path)
      {
      const QFileInfo pinfo(path);

      if (pinfo.exists() && pinfo.isDir()) {
            QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, path);
            QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, path);
            dataPath = path;
            return true;
            }

      return false;
      }

//---------------------------------------------------------
//   parseCommandLineArguments
//---------------------------------------------------------

MuseScoreApplication::CommandLineParseResult MuseScoreApplication::parseCommandLineArguments(MuseScoreApplication* app)
      {
      QCommandLineParser parser;
      CommandLineParseResult parseResult;

      parser.addHelpOption(); // -?, -h, --help
      parser.addVersionOption(); // -v, --version

    //parser.addOption(QCommandLineOption({"v", "version"}, "Print version")); // see above
      parser.addOption(QCommandLineOption(      "long-version", "Print detailed version information"));
      parser.addOption(QCommandLineOption({"d", "debug"}, "Debug mode"));
      parser.addOption(QCommandLineOption({"L", "layout-debug"}, "Layout debug mode"));
      parser.addOption(QCommandLineOption({"s", "no-synthesizer"}, "No internal synthesizer"));
      parser.addOption(QCommandLineOption({"m", "no-midi"}, "No MIDI"));
      parser.addOption(QCommandLineOption({"a", "use-audio"}, "Use audio driver: jack, alsa, pulse, or portaudio", "driver"));
      parser.addOption(QCommandLineOption({"n", "new-score"}, "Start with new score"));
      parser.addOption(QCommandLineOption({"I", "dump-midi-in"}, "Dump midi input"));
      parser.addOption(QCommandLineOption({"O", "dump-midi-out"}, "Dump midi output"));
      parser.addOption(QCommandLineOption({"o", "export-to"}, "Export to 'file'. Format depends on file's extension", "file"));
      parser.addOption(QCommandLineOption({"u", "unroll-repeats"}, "Unroll repeats", "file"));
      parser.addOption(QCommandLineOption({"r", "image-resolution"}, "Use with '-o <file>.png'. Set output resolution for image export", "DPI"));
      parser.addOption(QCommandLineOption({"T", "trim-image"}, "Use with '-o <file>.png' and '-o <file.svg>'. Trim exported image with specified margin (in pixels)", "margin"));
      parser.addOption(QCommandLineOption({"x", "gui-scaling"}, "Set scaling factor for GUI elements", "factor"));
      parser.addOption(QCommandLineOption({"D", "monitor-resolution"}, "Specify monitor resolution", "DPI"));
      parser.addOption(QCommandLineOption({"S", "style"}, "Load style file", "style"));
      parser.addOption(QCommandLineOption({"p", "plugin"}, "Execute named plugin", "name"));
      parser.addOption(QCommandLineOption(      "template-mode", "Save template mode, no page size")); // and no platform and creationDate tags
      parser.addOption(QCommandLineOption({"F", "factory-settings"}, "Use factory settings"));  // this includes -R, --revert-settimngs
      parser.addOption(QCommandLineOption({"R", "revert-settings"}, "Revert to factory settings, but keep default preferences"));
      parser.addOption(QCommandLineOption({"i", "load-icons"}, "Load icons from INSTALLPATH/icons"));
      parser.addOption(QCommandLineOption({"j", "job"}, "Process a conversion job", "file"));
      parser.addOption(QCommandLineOption({"e", "experimental"}, "Enable experimental features"));
      parser.addOption(QCommandLineOption({"c", "config-folder"}, "Override configuration and settings folder", "dir"));
      parser.addOption(QCommandLineOption({"t", "test-mode"}, "Set test mode flag for all files")); // this includes --template-mode
      parser.addOption(QCommandLineOption(      "run-test-script", "Run script tests listed in the command line arguments"));
      parser.addOption(QCommandLineOption({"M", "midi-operations"}, "Specify MIDI import operations file", "file"));
      parser.addOption(QCommandLineOption({"P", "export-score-parts"}, "Use with '-o <file>.pdf', export score and parts"));
      parser.addOption(QCommandLineOption(      "no-fallback-font", "Don't use a fallback musical font"));
      parser.addOption(QCommandLineOption({"f", "force"}, "Use with '-o <file>', ignore warnings reg. score being corrupted or from wrong version"));
      parser.addOption(QCommandLineOption({"b", "bitrate"}, "Use with '-o <file>.mp3', sets bitrate, in kbps", "bitrate"));
      parser.addOption(QCommandLineOption({"E", "install-extension"}, "Install an extension, load soundfont as default unless -e is passed too", "extension file"));
      parser.addOption(QCommandLineOption(      "save-online", "Upload score(s) to their source URL. Replaces existing online score(s)."));
      parser.addOption(QCommandLineOption(      "score-media", "Export all media (excepting mp3) for a given score in a single JSON file and print it to stdout"));
      parser.addOption(QCommandLineOption(      "highlight-config", "Set highlight to svg, generated from a given score", "highlight-config"));
      parser.addOption(QCommandLineOption(      "score-meta", "Export score metadata to JSON document and print it to stdout"));
      parser.addOption(QCommandLineOption(      "score-mp3", "Generate mp3 for the given score and export the data to a single JSON file, print it to stdout"));
      parser.addOption(QCommandLineOption(      "score-parts", "Generate parts data for the given score and save them to separate mscz files"));
      parser.addOption(QCommandLineOption(      "score-parts-pdf", "Generate parts data for the given score and export the data to a single JSON file, print it to stdout"));
      parser.addOption(QCommandLineOption(      "score-transpose", "Transpose the given score and export the data to a single JSON file, print it to stdout", "options"));
      parser.addOption(QCommandLineOption(      "source-update", "Update the source in the given score"));
      parser.addOption(QCommandLineOption(      "raw-diff", "Print a raw diff for the given scores"));
      parser.addOption(QCommandLineOption(      "diff", "Print a diff for the given scores"));

      parser.addPositionalArgument("scorefiles", "The files to open", "[scorefile...]");

      parser.process(QCoreApplication::arguments());

    //if (parser.isSet("v")) parser.showVersion(); // a) needs Qt >= 5.4 , b) instead we use addVersionOption()
      if (parser.isSet("long-version")) {
            printVersion("MuseScore");
            parseResult.exit = true;
            return parseResult;
            }
      MScore::debugMode = parser.isSet("d");
      MScore::noHorizontalStretch = MScore::noVerticalStretch = parser.isSet("L");
      noSeq = parser.isSet("s");
      noMidi = parser.isSet("m");
      if (parser.isSet("a")) {
            *audioDriver = parser.value("a");
            if (audioDriver->isEmpty())
                  parser.showHelp(EXIT_FAILURE);
            }
      startWithNewScore = parser.isSet("n");
      externalIcons = parser.isSet("i");
      midiInputTrace = parser.isSet("I");
      midiOutputTrace = parser.isSet("O");
      MScore::useFallbackFont = !parser.isSet("no-fallback-font");

      if ((converterMode = parser.isSet("o"))) {
            MScore::noGui = true;
            *outFileName = parser.value("o");
            if (outFileName->isEmpty())
                  parser.showHelp(EXIT_FAILURE);
            }
      if ((unrollRepeats = parser.isSet("u"))) {
            MScore::noGui = true;
            *outFileName = parser.value("u");
            if (outFileName->isEmpty())
                  parser.showHelp(EXIT_FAILURE);
            }
      if ((processJob = parser.isSet("j"))) {
            MScore::noGui = true;
            converterMode = true;
            *jsonFileName = parser.value("j");
            if (jsonFileName->isEmpty()) {
                  fprintf(stderr, "json file name missing\n");
                  parser.showHelp(EXIT_FAILURE);
                  }
            }
      if ((pluginMode = parser.isSet("p"))) {
            MScore::noGui = true;
            *pluginName = parser.value("p");
            if (pluginName->isEmpty())
                  parser.showHelp(EXIT_FAILURE);
            }
      if (parser.isSet("E")) {
            MScore::noGui = true;
            *extensionName = parser.value("E");
            }
      MScore::saveTemplateMode = parser.isSet("template-mode");
      if (parser.isSet("r")) {
            QString temp = parser.value("r");
            if (temp.isEmpty())
                   parser.showHelp(EXIT_FAILURE);
            bool ok = false;
            double res = temp.toDouble(&ok);
            if (ok)
                  preferences.setTemporaryPreference(PREF_EXPORT_PNG_RESOLUTION, res);
            else
                  fprintf(stderr, "PNG resolution value '%s' not recognized, using default setting from preferences instead.\n", qPrintable(temp));
            }
      if (parser.isSet("T")) {
            QString temp = parser.value("T");
            if (temp.isEmpty())
                   parser.showHelp(EXIT_FAILURE);
            bool ok = false;
            trimMargin = temp.toInt(&ok);
            if (!ok) {
                  fprintf(stderr, "Trim margin value '%s' not recognized, so no trimming will be done.\n", qPrintable(temp));
                  trimMargin = -1;
                  }
           }
      if (parser.isSet("x")) {
            QString temp = parser.value("x");
            if (temp.isEmpty())
                   parser.showHelp(EXIT_FAILURE);
            bool ok = false;
            guiScaling = temp.toDouble(&ok);
            if (!ok) {
                  fprintf(stderr, "GUI scaling value '%s' not recognized, so the values detected by Qt are taken.\n", qPrintable(temp));
                  guiScaling = 0.0;
                  }
            }
      if (parser.isSet("D")) {
            QString temp = parser.value("D");
            if (temp.isEmpty())
                   parser.showHelp(EXIT_FAILURE);
            bool ok = false;
            userDPI = temp.toDouble(&ok);
            if (!ok) {
                  fprintf(stderr, "DPI value '%s' not recognized, so the values detected by Qt are taken.\n", qPrintable(temp));
                  userDPI = 0.0;
                  }
            }
      if (parser.isSet("S")) {
            *styleFile = parser.value("S");
            if (styleFile->isEmpty())
                  parser.showHelp(EXIT_FAILURE);
            }
      deletePreferences = parser.isSet("F");
      useFactorySettings = (deletePreferences || parser.isSet("R"));
      enableExperimental = parser.isSet("e");
      if (parser.isSet("c")) {
            QString path = parser.value("c");
            if (path.isEmpty())
                  parser.showHelp(EXIT_FAILURE);
            setCustomConfigFolder(path);
            }
      MScore::testMode = parser.isSet("t");
      if (parser.isSet("M")) {
            QString temp = parser.value("M");
            if (temp.isEmpty())
                  parser.showHelp(EXIT_FAILURE);
            midiImportOperations.setOperationsFile(temp);
            }
      exportScoreParts = parser.isSet("export-score-parts");
      if (exportScoreParts && !converterMode)
            parser.showHelp(EXIT_FAILURE);
      ignoreWarnings = parser.isSet("f");
      if (parser.isSet("b")) {
            QString temp = parser.value("b");
            if (temp.isEmpty())
                   parser.showHelp(EXIT_FAILURE);
            bool ok = false;
            int rate = temp.toInt(&ok);
            if (ok)
                  preferences.setTemporaryPreference(PREF_EXPORT_MP3_BITRATE, rate);
            else
                  fprintf(stderr, "MP3 bitrate value '%s' not recognized, using default setting from preferences instead.\n", qPrintable(temp));
           }

      if (parser.isSet("save-online")) {
            cliSaveOnline = true;
            MScore::noGui = true;

            if (parser.positionalArguments().isEmpty()) {
                  fprintf(stderr, "%s\n", qPrintable(tr("Must specify at least one score to save online.")));
                  ::exit(EXIT_FAILURE);
            }
      }

      if (parser.isSet("score-media")) {
            exportScoreMedia = true;
            MScore::noGui = true;
            converterMode = true;

            if (parser.isSet("highlight-config")) {
                *highlightConfigPath = parser.value("highlight-config");
            }
      }

      if (parser.isSet("score-meta")) {
            exportScoreMeta = true;
            MScore::noGui = true;
            converterMode = true;
            }

      if (parser.isSet("score-mp3")) {
            exportScoreMp3 = true;
            MScore::noGui = true;
            converterMode = true;
            }

      if (parser.isSet("score-parts")) {
            saveScoreParts = true;
            MScore::noGui = true;
            converterMode = true;
            }

      if (parser.isSet("score-parts-pdf")) {
            exportScorePartsPdf = true;
            MScore::noGui = true;
            converterMode = true;
            }

      if (parser.isSet("score-transpose")) {
            exportTransposedScore = true;
            *transposeExportOptions = parser.value("score-transpose");
            MScore::noGui = true;
            converterMode = true;
            }

      if (parser.isSet("source-update")) {
            MScore::noGui = true;
            needUpdateSource = true;
            }

      if (parser.isSet("raw-diff")) {
            MScore::noGui = true;
            rawDiffMode = true;
            }
      if (parser.isSet("diff")) {
            MScore::noGui = true;
            diffMode = true;
            }
      if (parser.isSet("run-test-script")) {
            if (rawDiffMode || diffMode) {
                  fprintf(stderr, "%s\n", qPrintable(tr("--run-test-script is incompatible with --diff and --raw-diff")));
                  ::exit(EXIT_FAILURE);
                  }
            MScore::noGui = true;
            scriptTestMode = true;
            }

      QStringList argv = parser.positionalArguments();

      if (app && !converterMode && !pluginMode) {
            if (!argv.isEmpty()) {
                  int ok = true;
                  for (const QString& message : qAsConst(argv)) {
                        QFileInfo fi(message);
                        if (!app->sendMessage(fi.absoluteFilePath())) {
                              ok = false;
                              break;
                              }

                        }
                  if (ok) {
                        parseResult.exit = true;
                        return parseResult;
                        }
                  }
#if NDEBUG // allow multiple instances when debugging (actually: when built in Debug mode)
            else
                  if (app->sendMessage(QString(""))) {
                        parseResult.exit = true;
                        return parseResult;
                        }
#endif
            }
      if (rawDiffMode || diffMode) {
            if (argv.size() != 2) {
                  fprintf(stderr, "%s\n", qPrintable(tr("Only two scores are needed for performing a comparison")));
                  ::exit(EXIT_FAILURE);
                  }
            }
      if (scriptTestMode && argv.empty()) {
            fprintf(stderr, "%s\n", qPrintable(tr("Please specify scripts to execute")));
            ::exit(EXIT_FAILURE);
            }

      parseResult.argv = argv;
      return parseResult;
      }
} // namespace Ms

//---------------------------------------------------------
//   initZitaResources
///   must be placed outside of namespace
//---------------------------------------------------------

static void initZitaResources()
      {
      Q_INIT_RESOURCE(zita);
      }

namespace Ms {
//---------------------------------------------------------
//   runApplication
//---------------------------------------------------------

int runApplication(int& argc, char** av)
      {
#ifndef NDEBUG
      qSetMessagePattern("%{file}:%{function}: %{message}");
      Ms::checkStyles();
#endif

      QApplication::setDesktopSettingsAware(true);
#ifdef Q_OS_LINUX
      QGuiApplication::setDesktopFileName("mscore");
#endif
      QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
      QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#if defined(QT_DEBUG) && defined(Q_OS_WIN)
      qInstallMessageHandler(mscoreMessageHandler);
#endif

#ifdef Q_OS_WIN
      if (!qEnvironmentVariableIsSet("QT_OPENGL_BUGLIST")) {
            // Set custom OpenGL buglist to work around rendering issues
            // on Windows 7 (#296682). This should also prevent crashes
            // happening for some Intel GPUs due to outdated buglist in Qt 5.9.
            qputenv("QT_OPENGL_BUGLIST", ":/data/win_opengl_buglist.json");
            }
#endif

      qRegisterMetaTypeStreamOperators<SessionStart>("SessionStart");
      qRegisterMetaTypeStreamOperators<MusicxmlExportBreaks>("MusicxmlExportBreaks");
      qRegisterMetaTypeStreamOperators<MuseScorePreferredStyleType>("MuseScorePreferredStyleType");
      qRegisterMetaTypeStreamOperators<MuseScoreEffectiveStyleType>("MuseScoreEffectiveStyleType");

      MuseScoreApplication* app = MuseScoreApplication::initApplication(argc, av);

      QAccessible::installFactory(AccessibleScoreView::ScoreViewFactory);
      QAccessible::installFactory(AccessibleSearchBox::SearchBoxFactory);
      QAccessible::installFactory(Awl::AccessibleAbstractSlider::AbstractSliderFactory);

      initZitaResources();

#ifndef Q_OS_MAC
      QSettings::setDefaultFormat(QSettings::IniFormat);
#endif

      auto cmdLineParseResult = MuseScoreApplication::parseCommandLineArguments(app);

      if (cmdLineParseResult.exit)
            return 0;

      MuseScore::init(cmdLineParseResult.argv);

      if (MScore::noGui) {
#ifdef Q_OS_MAC
            // see issue #28706: Hangup in converter mode with MusicXML source
            qApp->processEvents();
#endif
            const bool ok = processNonGui(cmdLineParseResult.argv);
            return ok ? EXIT_SUCCESS : EXIT_FAILURE;
            }

      QDir::setCurrent(app->applicationDirPath());

      return qApp->exec();
      }

//---------------------------------------------------------
//   showSplashMessage
//---------------------------------------------------------

inline static void showSplashMessage(MsSplashScreen* sc, QString&& message)
      {
      if (sc)
            sc->showMessage(message);
      else
            qInfo("%s", qPrintable(message));
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void MuseScore::init(QStringList& argv)
      {
      mscoreGlobalShare = getSharePath();
      iconPath = externalIcons ? mscoreGlobalShare + QString("icons/") :  QString(":/data/icons/");

#if defined(WIN_PORTABLE)
      if (dataPath.isEmpty()) {
            dataPath = QDir::cleanPath(QString("%1/../../../Data/settings").arg(QCoreApplication::applicationDirPath()).arg(QCoreApplication::applicationName()));
            QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, dataPath);
            QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, dataPath);
            }
#else
      if (dataPath.isEmpty())
            dataPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#endif

      if (useFactorySettings) {
            if (deletePreferences)
                  QDir(dataPath).removeRecursively();
            QSettings settings;
            QFile::remove(settings.fileName() + ".lock"); //forcibly remove lock
            QFile::remove(settings.fileName());
            settings.clear();
            }

      // create local plugin directory
      // if not already there:
      QDir().mkpath(dataPath + "/plugins");

      if (MScore::debugMode)
            qDebug("global share: <%s>", qPrintable(mscoreGlobalShare));

      // Set translator before Shortcuts are initialized to get translations for all shortcuts.
      // Cannot use `preferences` or `localeName()` (which uses `preferences`) to retrieve
      // these values, as `preferences` needs to be initialized after translator is set.
      QString localeName;
      if (useFactorySettings)
            localeName = "system";
      else {
            QSettings s;
            localeName = s.value(PREF_UI_APP_LANGUAGE, "system").toString();
            }

      setMscoreLocale(localeName);

      Shortcut::init();
      preferences.init();

      QNetworkProxyFactory::setUseSystemConfiguration(true);

      MScore::init();         // initialize libmscore
      updateExternalValuesFromPreferences();

      // initialize current page size from default printer
#ifndef QT_NO_PRINTER
      if (!MScore::testMode && !MScore::saveTemplateMode) {
            QPrinter p;
            if (p.isValid()) {
//                  qDebug("set paper size from default printer");
                  QRectF psf = p.paperRect(QPrinter::Inch);
                  MScore::defaultStyle().set(Sid::pageWidth,  psf.width());
                  MScore::defaultStyle().set(Sid::pageHeight, psf.height());
                  MScore::defaultStyle().set(Sid::enableVerticalSpread, true);
                  }
            }
#endif

      if (MScore::debugMode) {
            qDebug("DPI %f", DPI);

            QScreen* screen      = QGuiApplication::primaryScreen();
            qDebug() << "Information for screen:" << screen->name();
            qDebug() << "  Available geometry:" << screen->availableGeometry().x() << screen->availableGeometry().y() << screen->availableGeometry().width() << "x" << screen->availableGeometry().height();
            qDebug() << "  Available size:" << screen->availableSize().width() << "x" << screen->availableSize().height();
            qDebug() << "  Available virtual geometry:" << screen->availableVirtualGeometry().x() << screen->availableVirtualGeometry().y() << screen->availableVirtualGeometry().width() << "x" << screen->availableVirtualGeometry().height();
            qDebug() << "  Available virtual size:" << screen->availableVirtualSize().width() << "x" << screen->availableVirtualSize().height();
            qDebug() << "  Depth:" << screen->depth() << "bits";
            qDebug() << "  Geometry:" << screen->geometry().x() << screen->geometry().y() << screen->geometry().width() << "x" << screen->geometry().height();
            qDebug() << "  Logical DPI:" << screen->logicalDotsPerInch();
            qDebug() << "  Logical DPI X:" << screen->logicalDotsPerInchX();
            qDebug() << "  Logical DPI Y:" << screen->logicalDotsPerInchY();
            qDebug() << "  Physical DPI:" << screen->physicalDotsPerInch();
            qDebug() << "  Physical DPI X:" << screen->physicalDotsPerInchX();
            qDebug() << "  Physical DPI Y:" << screen->physicalDotsPerInchY();
            qDebug() << "  Physical size:" << screen->physicalSize().width() << "x" << screen->physicalSize().height() << "mm";
            qDebug() << "  Refresh rate:" << screen->refreshRate() << "Hz";
            qDebug() << "  Size:" << screen->size().width() << "x" << screen->size().height();
            qDebug() << "  Virtual geometry:" << screen->virtualGeometry().x() << screen->virtualGeometry().y() << screen->virtualGeometry().width() << "x" << screen->virtualGeometry().height();
            qDebug() << "  Virtual size:" << screen->virtualSize().width() << "x" << screen->virtualSize().height();
            }

      if (!MScore::testMode)
            MScore::readDefaultStyle(preferences.getString(PREF_SCORE_STYLE_DEFAULTSTYLEFILE));

      MsSplashScreen* sc = nullptr;
      if (!MScore::noGui && preferences.getBool(PREF_UI_APP_STARTUP_SHOWSPLASHSCREEN)) {
            sc = new MsSplashScreen();
            sc->show();
            qApp->processEvents();
            }

      // Best not to show this since the font used to display the message
      // isn't updated till updateUiStyleAndTheme()
      // showSplashMessage(sc, tr("Updating user interface and theme…"));
      if (!MScore::noGui) {
            MuseScore::updateUiStyleAndTheme();
            }
      else {
            genIcons(); // in GUI mode generated in updateUiStyleAndTheme()
            noSeq = true;
            }

      // Do not create sequencer and audio drivers if run with '-s'
      if (!noSeq) {
            showSplashMessage(sc, tr("Initializing sequencer and audio driver…"));
            seq            = new Seq();
            MScore::seq    = seq;
            Driver* driver = driverFactory(seq, *audioDriver);
            synti          = synthesizerFactory();
            if (driver) {
                  MScore::sampleRate = driver->sampleRate();
                  synti->setSampleRate(MScore::sampleRate);

                  showSplashMessage(sc, tr("Loading SoundFonts…"));
                  synti->init();

                  seq->setDriver(driver);
                  }
            else {
                  // Do not delete the sequencer If we can't load driver.
                  // Allow user to select the working driver later.
                  MScore::sampleRate = 44100;  // Would be changed when user changes driver
                  synti->setSampleRate(MScore::sampleRate);
                  synti->init();
                  }
            seq->setMasterSynthesizer(synti);
            }
      else {
            seq         = 0;
            MScore::seq = 0;
            }
//---
      //
      // avoid font problems by overriding the environment
      //    fall back to "C" locale
      //

      //#ifndef Q_OS_WIN
      //setenv("LANG", "C", 1);
      //#endif
      //QLocale::setDefault(QLocale(QLocale::C));

      if (MScore::debugMode) {
            QStringList sl(QCoreApplication::libraryPaths());
            for (QString& s : sl)
                  qDebug("LibraryPath: <%s>", qPrintable(s));
            }

      // rastral size of font is 20pt = 20/72 inch = 20*DPI/72 dots
      //   staff has 5 lines = 4 * _spatium
      //   _spatium    = SPATIUM20;     // 20.0 / 72.0 * DPI / 4.0;

      if (!MScore::noGui) {
#ifndef Q_OS_MAC
            qApp->setWindowIcon(*icons[int(Icons::window_ICON)]);
#endif
            showSplashMessage(sc, tr("Initializing workspace…"));
            WorkspacesManager::initCurrentWorkspace();
            }

      showSplashMessage(sc, tr("Creating main window…"));
      mscore = new MuseScore();
      // create a score for internal use
      gscore = new MasterScore();
      gscore->setPaletteMode(true);
      gscore->setMovements(new Movements());
      gscore->setStyle(MScore::baseStyle());

      gscore->style().set(Sid::musicalTextFont, QString("Leland Text"));
      ScoreFont* scoreFont = ScoreFont::fontFactory("Leland");
      gscore->setScoreFont(scoreFont);
      gscore->setNoteHeadWidth(scoreFont->width(SymId::noteheadBlack, gscore->spatium()) / SPATIUM20);

#ifndef TELEMETRY_DISABLED
      tryToRequestTelemetryPermission();
#endif

      showSplashMessage(sc, tr("Reading translations…"));
      //read languages list
      mscore->readLanguages(mscoreGlobalShare + "locale/languages.xml");

      if (!MScore::noGui) {
            if (preferences.getBool(PREF_APP_STARTUP_FIRSTSTART)) {
                  mscoreFirstStart = true;
                  showSplashMessage(sc, tr("Initializing startup wizard…"));
                  StartupWizard* sw = new StartupWizard;
                  sw->exec();
                  preferences.setPreference(PREF_APP_STARTUP_FIRSTSTART, false);
                  preferences.setPreference(PREF_APP_KEYBOARDLAYOUT, sw->keyboardLayout());
                  preferences.setPreference(PREF_UI_APP_LANGUAGE, sw->language());
                  setMscoreLocale(sw->language());
                  Workspace::writeGlobalToolBar();
                  Workspace::writeGlobalGUIState();
                  Workspace* targetWorkspace = WorkspacesManager::visibleWorkspaces()[0];
                  if (targetWorkspace)
                        mscore->changeWorkspace(targetWorkspace, true);

                  preferences.setPreference(PREF_UI_APP_STARTUP_SHOWTOURS, sw->showTours());
                  delete sw;

                  showSplashMessage(sc, tr("Initializing preferences…"));
                  // reinitialize preferences so some default values are calculated based on chosen language
                  preferences.init();
                  // store preferences with locale-dependent default values
                  // so that the values from first start will be used later
                  preferences.setToDefaultValue(PREF_APP_PATHS_MYSCORES);
                  preferences.setToDefaultValue(PREF_APP_PATHS_MYSTYLES);
                  preferences.setToDefaultValue(PREF_APP_PATHS_MYIMAGES);
                  preferences.setToDefaultValue(PREF_APP_PATHS_MYTEMPLATES);
                  preferences.setToDefaultValue(PREF_APP_PATHS_MYPLUGINS);
                  preferences.setToDefaultValue(PREF_APP_PATHS_MYSCOREFONTS);
                  preferences.setToDefaultValue(PREF_APP_PATHS_MYSOUNDFONTS);
                  preferences.setToDefaultValue(PREF_APP_PATHS_MYEXTENSIONS);
                  updateExternalValuesFromPreferences();
                  }
            if (!Shortcut::customSource()) {
                  QString keyboardLayout = preferences.getString(PREF_APP_KEYBOARDLAYOUT);
                  StartupWizard::autoSelectShortcuts(keyboardLayout);
                  }
            }

      if (!noSeq) {
            if (!seq->init())
                  qDebug("sequencer init failed");
            }

      QApplication::instance()->installEventFilter(mscore);

#ifndef TELEMETRY_DISABLED
      QApplication::instance()->installEventFilter(ActionEventObserver::instance());
      ActionEventObserver::instance()->setScoreState(mscore->state());
      QObject::connect(mscore, &MuseScore::scoreStateChanged, ActionEventObserver::instance(), &ActionEventObserver::setScoreState);
#endif

      mscore->setRevision(Ms::revision);
      int files = 0;
      bool restoredSession = false;

      if (MScore::noGui)
            return;
      else {
            showSplashMessage(sc, tr("Initializing main window…"));
            mscore->readSettings();
            QObject::connect(qApp, SIGNAL(messageReceived(const QString&)), // const needed here
               mscore, SLOT(handleMessage(const QString&))); // and here, see https://github.com/Jojo-Schmitz/MuseScore/issues/679
            static_cast<QtSingleApplication*>(qApp)->setActivationWindow(mscore, false);
            // count filenames specified on the command line
            // these are the non-empty strings remaining in argv
            for (const QString& name : argv) {
                  if (!name.isEmpty())
                        ++files;
                  }
#ifdef Q_OS_MAC
            // app->paths contains files requested to be loaded by OS X
            // append these to argv and update file count
            for (const QString& name : static_cast<MuseScoreApplication*>(qApp)->paths) {
                  if (!name.isEmpty()) {
                        argv << name;
                        ++files;
                        }
                  }
#endif
            //
            // TODO: delete old session backups
            //
            showSplashMessage(sc, tr("Restoring session…"));
            restoredSession = mscore->restoreSession((preferences.sessionStart() == SessionStart::LAST && (files == 0)));
            }

      errorMessage = new QErrorMessage(mscore);
#ifdef SCRIPT_INTERFACE
      mscore->getPluginManager()->readPluginList();
      mscore->loadPlugins();
#endif
      mscore->writeSessionFile(false);

#ifdef Q_OS_MAC
      // there's a bug in Qt showing the toolbar unified after switching
      // showFullScreen(), showMaximized(), showNormal()...
      mscore->setUnifiedTitleAndToolBarOnMac(false);

      // don't let macOS add "Show Tab Bar" to "View" Menu
      CocoaBridge::setAllowsAutomaticWindowTabbing(false);

      // Observe when macOS's Dark Mode is switched on or off and let MuseScore's
      // theme switch along with it if the user has chosen that setting
      CocoaBridge::observeDarkModeSwitches([]() {
            if (preferences.preferredGlobalStyle() == MuseScorePreferredStyleType::FOLLOW_SYSTEM)
                  MuseScore::updateUiStyleAndTheme();
            });
#endif

      mscore->changeState(mscore->noScore() ? STATE_DISABLED : STATE_NORMAL);
      mscore->show();

      if (!restoredSession || files) {
            showSplashMessage(sc, tr("Loading scores…"));
            loadScores(argv);
            }

      if (mscore->hasToCheckForExtensionsUpdate())
            mscore->checkForExtensionsUpdate();

      if (QWidget* menubar = mscore->menuWidget())
            TourHandler::addWidgetToTour("welcome", menubar, "menubar");

      if (!scoresOnCommandline && preferences.getBool(PREF_UI_APP_STARTUP_SHOWSTARTCENTER) && (!restoredSession || mscore->scores().size() == 0)) {
            showSplashMessage(sc, tr("Initializing start center…"));
#ifdef Q_OS_MAC
// ugly, but on mac we get an event when a file is open.
// We can't get the event when the startcenter is shown.
// So we let the event loop run a bit before showing the start center.
            QTimer* timer = new QTimer();
            timer->setSingleShot(true);
            QObject::connect(timer, &QTimer::timeout, [=]() {
                  if (!scoresOnCommandline) {
                        getAction("startcenter")->setChecked(true);
                        mscore->showStartcenter(true);
                        }
                  timer->deleteLater();
                  } );
            timer->start(500);
#else

            getAction("startcenter")->setChecked(true);
            mscore->showStartcenter(true);
#endif
            }
      else {
            showSplashMessage(sc, tr("Initializing tours…"));
            mscore->tourHandler()->startTour("welcome");
            //otherwise, welcome tour will appear on closing StartCenter
            }

      if (sc) {
            sc->close();
            qApp->processEvents();
            }

      mscore->showPlayPanel(preferences.getBool(PREF_UI_APP_STARTUP_SHOWPLAYPANEL));
      QSettings settings;
      if (settings.value("synthControlVisible", false).toBool())
            mscore->showSynthControl(true);
      }


bool MuseScore::saveScoreParts(const QString& inFilePath, const QString& outFilePath)
{
    MasterScore* score = mscore->readScore(inFilePath);
    if (!score) {
        return false;
    }

    if (!styleFile->isEmpty()) {
        QFile f(*styleFile);
        if (f.open(QIODevice::ReadOnly)) {
            score->style().load(&f);
        }
    }
    score->switchToPageMode();

    // if no parts, generate parts from existing instruments
    if (score->excerpts().isEmpty()) {
        auto excerpts = Excerpt::createAllExcerpt(score);
        for (Excerpt* e : qAsConst(excerpts)) {
              Score* nscore = new Score(e->oscore());
              e->setPartScore(nscore);
              nscore->style().set(Sid::createMultiMeasureRests, true);
              auto excerptCmdFake = new AddExcerpt(e);
              excerptCmdFake->redo(nullptr);
              Excerpt::createExcerpt(e);
        }
    }

    QJsonArray partsObjList;
    QJsonArray partsMetaList;
    QJsonArray partsTitles;

    for (Excerpt* excerpt : qAsConst(score->excerpts())) {
        Score* part = excerpt->partScore();
        QMap<QString, QString> partMetaTags = part->metaTags();

        QJsonValue partTitle(part->title());
        partsTitles << partTitle;

        QVariantMap meta;
        const QList<QString> keys;
        for (const QString& key: keys) {
            meta[key] = partMetaTags[key];
        }

        QJsonValue partMetaObj = QJsonObject::fromVariantMap(meta);
        partsMetaList << partMetaObj;

        QJsonValue partObj(QString::fromLatin1(exportMsczAsJSON(part)));
        partsObjList << partObj;
    }

    QJsonObject json;
    json["parts"] = partsTitles;
    json["partsMeta"] = partsMetaList;
    json["partsBin"] = partsObjList;

    QJsonDocument jsonDoc(json);
    QFile out(outFilePath);

    bool res = out.open(QIODevice::WriteOnly);
    if (res) {
        out.write(jsonDoc.toJson(QJsonDocument::Compact));
        out.close();
    }

    delete score;
    return res;
}

QByteArray MuseScore::exportMsczAsJSON(Score* score)
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);

    QString fileName = saveFilename(score->title()) + ".mscz";
    score->saveCompressedFile(&buffer, fileName, false, true);

    buffer.open(QIODevice::ReadOnly);
    QByteArray scoreData = buffer.readAll();
    buffer.close();

    return scoreData.toBase64();
}

bool MuseScore::exportUnrolled(const QString& inFilePath)
{
      MasterScore* score = mscore->readScore(inFilePath);
      if (!score)
            return false;

      score = score->unrollRepeats();

      QString outPath = inFilePath + QString(".unrolled.mscx");
      QFileInfo fi(outPath);
      bool rv = score->Score::saveFile(fi);
      delete score;
      return rv;
}

//---------------------------------------------------------
//   exportPartsPdfsToJSON
//---------------------------------------------------------

bool MuseScore::exportPartsPdfsToJSON(const QString& inFilePath, const QString& outFilePath)
{
      MasterScore* score = mscore->readScore(inFilePath);
      if (!score)
            return false;

      QString outPath = QFileInfo(inFilePath).path() + "/";

      QJsonObject jsonForPdfs;
      QString outName = outPath + QFileInfo(inFilePath).completeBaseName() + ".pdf";
      jsonForPdfs["score"] = outName;

      //save score pdf
      if (!styleFile->isEmpty()) {
            QFile f(*styleFile);
            if (f.open(QIODevice::ReadOnly))
                  score->style().load(&f);
      }
      score->switchToPageMode();

      jsonForPdfs["scoreBin"] = QString::fromLatin1(mscore->exportPdfAsJSON(score));

      //save extended score+parts and separate parts pdfs
      //if no parts, generate parts from existing instruments
      if (score->excerpts().size() == 0) {
            auto excerpts = Excerpt::createAllExcerpt(score);
            for (Excerpt* e : qAsConst(excerpts)) {
                  Score* nscore = new Score(e->oscore());
                  e->setPartScore(nscore);
                  nscore->style().set(Sid::createMultiMeasureRests, true);
                  auto excerptCmdFake = new AddExcerpt(e);
                  excerptCmdFake->redo(nullptr);
                  Excerpt::createExcerpt(e);
            }
      }

      QList<Score*> scores;
      scores.append(score);
      QJsonArray partsArray;
      QJsonArray partsNamesArray;
      for (Excerpt* e : qAsConst(score->excerpts())) {
            scores.append(e->partScore());
            QJsonValue partNameVal(e->title());
            partsNamesArray.append(partNameVal);
            QJsonValue partVal(QString::fromLatin1(exportPdfAsJSON(e->partScore())));
            partsArray.append(partVal);
      }
      jsonForPdfs["parts"] = partsNamesArray;
      jsonForPdfs["partsBin"] = partsArray;

      jsonForPdfs["scoreFullPostfix"] = QString("-Score_and_parts") + ".pdf";

      QTemporaryFile tempPdf;
      tempPdf.open();

      bool res = mscore->savePdf(scores, tempPdf.fileName());
      QByteArray fullScoreData = tempPdf.readAll();

      jsonForPdfs["scoreFullBin"] = QString::fromLatin1(fullScoreData.toBase64());

      QJsonDocument jsonDoc(jsonForPdfs);
      const QString& jsonPath{outFilePath};
      QFile file(jsonPath);
      res &= file.open(QIODevice::WriteOnly);
      if (res) {
            file.write(jsonDoc.toJson(QJsonDocument::Compact));
            file.close();
      }

      delete score;
      return res;
      }

//---------------------------------------------------------
//   getQmlEngine
//---------------------------------------------------------

MsQmlEngine* MuseScore::getQmlUiEngine()
      {
      if (!_qmlUiEngine)
            _qmlUiEngine = new MsQmlEngine(this);
      return _qmlUiEngine;
      }

//---------------------------------------------------------
//   getPluginEngine
//---------------------------------------------------------

#ifdef SCRIPT_INTERFACE
QmlPluginEngine* MuseScore::getPluginEngine()
      {
      if (!_qmlEngine)
            _qmlEngine = new QmlPluginEngine(this);
      return _qmlEngine;
      }
#endif

//---------------------------------------------------------
//   scoreUnrolled
//    create a version of the given score with repeats unrolled
//---------------------------------------------------------

void MuseScore::scoreUnrolled(MasterScore * original)
      {
      MasterScore * score = original->unrollRepeats();
      setCurrentScoreView(appendScore(score));
      }
} // namespace Ms
