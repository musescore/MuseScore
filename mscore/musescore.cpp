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

#include <fenv.h>
#include "loginmanager.h"
#include "uploadscoredialog.h"
#include <QStyleFactory>
#include "palettebox.h"
#include "config.h"
#include "musescore.h"
#include "scoreview.h"
#include "libmscore/style.h"
#include "libmscore/score.h"
#include "instrdialog.h"
#include "preferences.h"
#include "prefsdialog.h"
#include "icons.h"
#include "libmscore/xml.h"
#include "seq.h"
#include "libmscore/tempo.h"
#include "libmscore/sym.h"
#include "pagesettings.h"
#include "debugger/debugger.h"
#include "editstyle.h"
#include "playpanel.h"
#include "libmscore/page.h"
#include "mixer.h"
#include "selectionwindow.h"
#include "palette.h"
#include "palettebox.h"
#include "libmscore/part.h"
#include "libmscore/drumset.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/note.h"
#include "libmscore/staff.h"
#include "driver.h"
#include "libmscore/harmony.h"
#include "magbox.h"
#include "libmscore/sig.h"
#include "libmscore/undo.h"
#include "synthcontrol.h"
#include "pianoroll.h"
#include "drumroll.h"
#include "scoretab.h"
#include "timedialog.h"
#include "keyedit.h"
#include "harmonyedit.h"
#include "navigator.h"
#include "timeline.h"
#include "importmidi/importmidi_panel.h"
#include "importmidi/importmidi_operations.h"
#include "libmscore/chord.h"
#include "libmscore/segment.h"
#include "editraster.h"
#include "pianotools.h"
#include "mediadialog.h"
#include "workspace.h"
#include "selectdialog.h"
#include "selectnotedialog.h"
#include "transposedialog.h"
#include "metaedit.h"
#include "inspector/inspector.h"
#ifdef OMR
#include "omrpanel.h"
#endif
#include "shortcut.h"
#ifdef SCRIPT_INTERFACE
#include "pluginCreator.h"
#include "pluginManager.h"
#endif
#include "helpBrowser.h"
#include "drumtools.h"
#include "editstafftype.h"
#include "texttools.h"
#include "textpalette.h"
#include "resourceManager.h"
#include "scoreaccessibility.h"
#include "startupWizard.h"

#include "libmscore/mscore.h"
#include "libmscore/system.h"
#include "libmscore/measure.h"
#include "libmscore/chordlist.h"
#include "libmscore/volta.h"
#include "libmscore/lasso.h"
#include "libmscore/excerpt.h"
#include "libmscore/synthesizerstate.h"
#include "libmscore/utils.h"

#include "driver.h"

#include "effects/zita1/zita.h"
#include "effects/compressor/compressor.h"
#include "effects/noeffect/noeffect.h"
#include "synthesizer/synthesizer.h"
#include "synthesizer/synthesizergui.h"
#include "synthesizer/msynthesizer.h"
#include "fluid/fluid.h"
#include "qmlplugin.h"
#include "accessibletoolbutton.h"
#include "toolbuttonmenu.h"
#include "searchComboBox.h"
#include "startcenter.h"
#include "help.h"
#include "awl/aslider.h"
#include "extension.h"
#include "thirdparty/qzip/qzipreader_p.h"

#ifdef USE_LAME
#include "exportmp3.h"
#endif

#ifdef Q_OS_MAC
#include "macos/cocoabridge.h"
#endif

#ifdef AEOLUS
extern Ms::Synthesizer* createAeolus();
#endif
#ifdef ZERBERUS
extern Ms::Synthesizer* createZerberus();
#endif

#ifdef QT_NO_DEBUG
      Q_LOGGING_CATEGORY(undoRedo, "undoRedo", QtCriticalMsg)
#else
      Q_LOGGING_CATEGORY(undoRedo, "undoRedo", QtCriticalMsg)
//      Q_LOGGING_CATEGORY(undoRedo, "undoRedo")
#endif

namespace Ms {

MuseScore* mscore;
MasterSynthesizer* synti;

bool enableExperimental = false;

QString dataPath;
QString iconPath;

bool converterMode = false;
bool processJob = false;
bool externalIcons = false;
bool pluginMode = false;
static bool startWithNewScore = false;
double guiScaling = 0.0;
static double userDPI = 0.0;
int trimMargin = -1;
bool noWebView = false;
bool exportScoreParts = false;
bool ignoreWarnings = false;

QString mscoreGlobalShare;

static QString outFileName;
static QString jsonFileName;
static QString audioDriver;
static QString pluginName;
static QString styleFile;
static bool scoresOnCommandline { false };

static QList<QTranslator*> translatorList;

QString localeName;
bool useFactorySettings = false;
bool deletePreferences = false;
QString styleName;
QString revision;
QErrorMessage* errorMessage;
const char* voiceActions[] = { "voice-1", "voice-2", "voice-3", "voice-4" };

const std::list<const char*> MuseScore::_allNoteInputMenuEntries {
            "note-input",
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
            "",
            "voice-1",
            "voice-2",
            "voice-3",
            "voice-4"
            };

const std::list<const char*> MuseScore::_advancedNoteInputMenuEntries {
            "note-input",
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
            "",
            "voice-1",
            "voice-2",
            "voice-3",
            "voice-4"
            };

const std::list<const char*> MuseScore::_basicNoteInputMenuEntries {
            "note-input",
            "pad-note-32",
            "pad-note-16",
            "pad-note-8",
            "pad-note-4",
            "pad-note-2",
            "pad-note-1",
            "pad-dot",
            "tie",
            "",
            "pad-rest",
            "",
            "sharp",
            "nat",
            "flat",
            "flip",
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
            "print",
            "redo",
            "undo"
            };

const std::list<const char*> MuseScore::_allPlaybackControlEntries {
            "midi-on",
            "rewind",
            "play",
            "loop",
            "repeat",
            "pan",
            "metronome"
            };

extern bool savePositions(Score*, const QString& name, bool segments );
extern TextPalette* textPalette;

static constexpr double SCALE_MAX  = 16.0;
static constexpr double SCALE_MIN  = 0.05;
static constexpr double SCALE_STEP = 1.7;

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
                  insertMeasuresDialog = new InsertMeasuresDialog;
                  insertMeasuresDialog->show();
                  }
            }
      }

//---------------------------------------------------------
// InsertMeasuresDialog
//---------------------------------------------------------

InsertMeasuresDialog::InsertMeasuresDialog(QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("InsertMeasuresDialog");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      insmeasures->selectAll();
      }

//---------------------------------------------------------
// Insert Measure -->   accept
//---------------------------------------------------------

void InsertMeasuresDialog::accept()
      {
      int n = insmeasures->value();
      if (mscore->currentScore())
            mscore->currentScoreView()->cmdInsertMeasures(n, ElementType::MEASURE);
      done(1);
      }

//---------------------------------------------------------
// InsertMeasuresDialog hideEvent
//---------------------------------------------------------

void InsertMeasuresDialog::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QDialog::hideEvent(event);
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
      for (MasterScore* score : scoreList) {
            if (score->created() && !score->dirty())
                  removeList.append(score);
            else {
                  if (checkDirty(score)) {      // ask user if file is dirty
                        ev->ignore();
                        return;
                        }
                  //
                  // if score is still dirty, then the user has discarded the
                  // score and we can remove it from the list
                  //
                  if (score->created() && score->dirty())
                        removeList.append(score);
                  }
            }

      // remove all new created/not save score so they are
      // note saved as session data
      for (MasterScore* score : removeList)
            scoreList.removeAll(score);

      writeSessionFile(true);
      for (MasterScore* score : scoreList) {
            if (!score->tmpName().isEmpty()) {
                  QFile f(score->tmpName());
                  f.remove();
                  }
            }

      writeSettings();

      _loginManager->save();

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
      MScore::playRepeats = preferences.getBool(PREF_APP_PLAYBACK_PLAYREPEATS);
      MScore::warnPitchRange = preferences.getBool(PREF_SCORE_NOTE_WARNPITCHRANGE);
      MScore::layoutBreakColor = preferences.getColor(PREF_UI_SCORE_LAYOUTBREAKCOLOR);
      MScore::frameMarginColor = preferences.getColor(PREF_UI_SCORE_FRAMEMARGINCOLOR);
      MScore::setVerticalOrientation(preferences.getBool(PREF_UI_CANVAS_SCROLL_VERTICALORIENTATION));

      MScore::selectColor[0] = preferences.getColor(PREF_UI_SCORE_VOICE1_COLOR);
      MScore::selectColor[1] = preferences.getColor(PREF_UI_SCORE_VOICE2_COLOR);
      MScore::selectColor[2] = preferences.getColor(PREF_UI_SCORE_VOICE3_COLOR);
      MScore::selectColor[3] = preferences.getColor(PREF_UI_SCORE_VOICE4_COLOR);

      MScore::setHRaster(preferences.getInt(PREF_UI_APP_RASTER_HORIZONTAL));
      MScore::setVRaster(preferences.getInt(PREF_UI_APP_RASTER_VERTICAL));

      MScore::setNudgeStep(.1);         // cursor key (default 0.1)
      MScore::setNudgeStep10(1.0);      // Ctrl + cursor key (default 1.0)
      MScore::setNudgeStep50(0.01);     // Alt  + cursor key (default 0.01)

      //Create directories if they are missing
      QDir dir;
      dir.mkpath(preferences.getString(PREF_APP_PATHS_MYSCORES));
      dir.mkpath(preferences.getString(PREF_APP_PATHS_MYSTYLES));
      dir.mkpath(preferences.getString(PREF_APP_PATHS_MYIMAGES));
      dir.mkpath(preferences.getString(PREF_APP_PATHS_MYTEMPLATES));
      dir.mkpath(preferences.getString(PREF_APP_PATHS_MYEXTENSIONS));
      dir.mkpath(preferences.getString(PREF_APP_PATHS_MYPLUGINS));
      foreach (QString path, preferences.getString(PREF_APP_PATHS_MYSOUNDFONTS).split(";"))
            dir.mkpath(path);

      }

//---------------------------------------------------------
//   preferencesChanged
//---------------------------------------------------------

void MuseScore::preferencesChanged()
      {
      updateExternalValuesFromPreferences();

      setPlayRepeats(MScore::playRepeats);
      getAction("pan")->setChecked(MScore::panPlayback);
      getAction("follow")->setChecked(preferences.getBool(PREF_APP_PLAYBACK_FOLLOWSONG));
      getAction("midi-on")->setEnabled(preferences.getBool(PREF_IO_MIDI_ENABLEINPUT));
      getAction("toggle-statusbar")->setChecked(preferences.getBool(PREF_UI_APP_SHOWSTATUSBAR));
      _statusBar->setVisible(preferences.getBool(PREF_UI_APP_SHOWSTATUSBAR));

      MuseScore::updateUiStyleAndTheme();

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
      if (preferences.getString(PREF_APP_WORKSPACE) != Workspace::currentWorkspace->name()) {
            Workspace* workspace = 0;
            for (Workspace* w: Workspace::workspaces()) {
                  if (w->name() == preferences.getString(PREF_APP_WORKSPACE)) {
                        workspace = w;
                        break;
                        }
                  }

            if (workspace != 0) {
                  mscore->changeWorkspace(workspace);
                  mscore->getPaletteBox()->updateWorkspaces();
                  }
            }

      delete newWizard;
      newWizard = 0;
      reloadInstrumentTemplates();
      updateNewWizard();
      updateInstrumentDialog();
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

                        connect(noteEntryMethods, SIGNAL(triggered(QAction*)), this, SLOT(cmd(QAction*)));

                        w = new ToolButtonMenu(tr("Note Entry Methods"),
                           ToolButtonMenu::TYPES::ICON_CHANGED,
                           getAction("note-input"),
                           noteEntryMethods,
                           this);
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
                  else
                        w = new AccessibleToolButton(entryTools, a);
                  entryTools->addWidget(w);
                  }
            }
      }

//---------------------------------------------------------
//   importExtension
//---------------------------------------------------------

bool MuseScore::importExtension(QString path)
      {
      MQZipReader zipFile(path);
      // compute total unzipped size
      qint64 totalZipSize = 0;
      for (auto fi : zipFile.fileInfoList())
            totalZipSize += fi.size;

      // check if extension path is writable and has enough space
      QStorageInfo storage = QStorageInfo(preferences.getString(PREF_APP_PATHS_MYTEMPLATES));
      if (storage.isReadOnly()) {
            if (!MScore::noGui)
                  QMessageBox::critical(mscore, QWidget::tr("Import Extension File"), QWidget::tr("Cannot import extension on read-only storage:%1").arg(storage.displayName()));
            return false;
            }
      if (totalZipSize >= storage.bytesAvailable()) {
            if (!MScore::noGui)
                  QMessageBox::critical(mscore, QWidget::tr("Import Extension File"), QWidget::tr("Cannot import extension: storage %1 is full").arg(storage.displayName()));
            return false;
            }
      // Check structure of the extension
      bool hasMetadata = false;
      bool hasAlienDirectory = false;
      bool hasAlienFiles = false;
      QSet<QString> acceptableFolders = { "sfzs", "soundfonts", "templates", "instruments" };
      for (auto fi : zipFile.fileInfoList()) {
            if (fi.filePath == "metadata.json")
                  hasMetadata = true;
            else {
                  // get folders
                  auto path = QDir::cleanPath(fi.filePath);
                  QStringList folders(path);
                  while ((path = QFileInfo(path).path()).length() < folders.last().length())
                        folders << path;
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
      QDir dir(preferences.getString(PREF_APP_PATHS_MYTEMPLATES));
      auto dirList = dir.entryList(QStringList(extensionId), QDir::Dirs | QDir::NoDotAndDotDot);
      bool newerVersion = false;
      if (dirList.contains(extensionId)) {
            QString extDirName = QString("%1/%2").arg(preferences.getString(PREF_APP_PATHS_MYTEMPLATES)).arg(extensionId);
            QDir extDir(extDirName);
            auto versionDirList = extDir.entryList(QDir::Dirs);
            if (versionDirList.size() > 0) {
                  // potentially other versions
                  // is there a more recent version?
                  for (auto versionDir : versionDirList) {
                        if (compareVersion(version, versionDir)) {
                              qDebug() << "There is a newer version. We don't install";
                              if (!MScore::noGui)
                                    QMessageBox::critical(mscore, QWidget::tr("Import Extension File"), QWidget::tr("A newer version is already installed"));
                              newerVersion = true;
                              return false;
                              }
                        }
                  }
            if (!newerVersion) {
                  qDebug() << "found already install extension without newer version: deleting it";
                  QDir d(QString("%1/%2").arg(preferences.getString(PREF_APP_PATHS_MYTEMPLATES)).arg(extensionId));
                  if (!d.removeRecursively()) {
                        if (!MScore::noGui)
                              QMessageBox::critical(mscore, QWidget::tr("Import Extension File"), QWidget::tr("Error while deleting previous version of the extension: %1").arg(extensionId));
                        return false;
                        }
                  }
            }
      // Unzip the extension
      MQZipReader zipFile3(path);
      zipFile3.extractAll(QString("%1/%2/%3").arg(preferences.getString(PREF_APP_PATHS_MYTEMPLATES)).arg(extensionId).arg(version));
      zipFile3.close();

      mscore->reloadInstrumentTemplates();
      mscore->updateNewWizard();
      mscore->updateInstrumentDialog();
      //TODO After install: add soundfont to synth ?
      return true;
      }

//---------------------------------------------------------
//   MuseScore
//---------------------------------------------------------

MuseScore::MuseScore()
   : QMainWindow()
      {
      QScreen* screen = QGuiApplication::primaryScreen();
      if (userDPI == 0.0) {
#if defined(Q_OS_WIN)
      if (QSysInfo::WindowsVersion <= QSysInfo::WV_WINDOWS7)
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
      if (guiScaling == 0.0) {
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

      ucheck = new UpdateChecker();

      setAcceptDrops(true);
      setFocusPolicy(Qt::NoFocus);

#ifdef SCRIPT_INTERFACE
      pluginManager = new PluginManager(0);
#endif

      if (!converterMode && !pluginMode) {
            _loginManager = new LoginManager(this);
#if 0
            // initialize help engine
            QString lang = mscore->getLocaleISOCode();
            if (lang == "en_US")    // HACK
                  lang = "en";

            QString s = getSharePath() + "manual/doc_" + lang + ".qhc";
            _helpEngine = new QHelpEngine(s, this);
            if (!_helpEngine->setupData()) {
                  qDebug("cannot setup data for help engine: %s", qPrintable(_helpEngine->error()));
                  delete _helpEngine;
                  _helpEngine = 0;
                  }
#endif
            }

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
            connect(layerSwitch, SIGNAL(activated(const QString&)), SLOT(switchLayer(const QString&)));
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
      //   must be added somewere to work
      QActionGroup* ag = Shortcut::getActionGroupForWidget(MsWidget::MAIN_WINDOW);
      ag->setParent(this);
      addActions(ag->actions());
      connect(ag, SIGNAL(triggered(QAction*)), SLOT(cmd(QAction*)));

      mainWindow = new QSplitter;
      mainWindow->setChildrenCollapsible(false);

      QWidget* mainScore = new QWidget;
      mainScore->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      mainWindow->addWidget(mainScore);

      layout = new QVBoxLayout;
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

      mainWindow->setStretchFactor(0, 1);
      mainWindow->setStretchFactor(1, 0);
      mainWindow->setSizes(QList<int>({500, 50}));

      QSplitter* envelope = new QSplitter;
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
      tab1 = new ScoreTab(&scoreList, this);
      tab1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      connect(tab1, SIGNAL(currentScoreViewChanged(ScoreView*)), SLOT(setCurrentScoreView(ScoreView*)));
      connect(tab1, SIGNAL(tabCloseRequested(int)), SLOT(removeTab(int)));
      connect(tab1, SIGNAL(actionTriggered(QAction*)), SLOT(cmd(QAction*)));
      splitter->addWidget(tab1);

      if (splitScreen()) {
            tab2 = new ScoreTab(&scoreList, this);
            tab2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            connect(tab2, SIGNAL(currentScoreViewChanged(ScoreView*)), SLOT(setCurrentScoreView(ScoreView*)));
            connect(tab2, SIGNAL(tabCloseRequested(int)), SLOT(removeTab(int)));
            connect(tab2, SIGNAL(actionTriggered(QAction*)), SLOT(cmd(QAction*)));
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
      a->setEnabled(preferences.getBool(PREF_IO_MIDI_ENABLEINPUT));
      a->setChecked(_midiinEnabled);
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
      if (qApp->layoutDirection() == Qt::LayoutDirection::LeftToRight) {
            for (auto i : { "file-new", "file-open", "file-save", "print", "undo", "redo"})
                  fileTools->addWidget(new AccessibleToolButton(fileTools, getAction(i)));
            }
      else {
            for (auto i : { "undo", "redo", "print", "file-save", "file-open", "file-new"})
                  fileTools->addWidget(new AccessibleToolButton(fileTools, getAction(i)));
            }

      fileTools->addSeparator();
      mag = new MagBox;
      connect(mag, SIGNAL(magChanged(MagIdx)), SLOT(magChanged(MagIdx)));
      fileTools->addWidget(mag);
      viewModeCombo = new QComboBox(this);
#if defined(Q_OS_MAC)
      viewModeCombo->setFocusPolicy(Qt::StrongFocus);
#else
      viewModeCombo->setFocusPolicy(Qt::TabFocus);
#endif
      viewModeCombo->setFixedHeight(preferences.getInt(PREF_UI_THEME_ICONHEIGHT) + 8);  // hack
      viewModeCombo->addItem("",       int(LayoutMode::PAGE));
      viewModeCombo->addItem("", int(LayoutMode::LINE));
      viewModeCombo->addItem("", int(LayoutMode::SYSTEM));
      connect(viewModeCombo, SIGNAL(activated(int)), SLOT(switchLayoutMode(int)));
      fileTools->addWidget(viewModeCombo);

      //---------------------
      //    Transport Tool Bar
      //---------------------

      transportTools = addToolBar("");
      transportTools->setObjectName("transport-tools");
#ifdef HAS_MIDI
      transportTools->addWidget(new AccessibleToolButton(transportTools, getAction("midi-on")));
      transportTools->addSeparator();
#endif
      transportTools->addWidget(new AccessibleToolButton(transportTools, getAction("rewind")));
      _playButton = new AccessibleToolButton(transportTools, getAction("play"));
      transportTools->addWidget(_playButton);
      transportTools->addWidget(new AccessibleToolButton(transportTools, getAction("loop")));
      transportTools->addSeparator();
      QAction* repeatAction = getAction("repeat");
      repeatAction->setChecked(preferences.getBool(PREF_APP_PLAYBACK_PLAYREPEATS));
      transportTools->addWidget(new AccessibleToolButton(transportTools, repeatAction));
      transportTools->addWidget(new AccessibleToolButton(transportTools, getAction("pan")));
      transportTools->addWidget(new AccessibleToolButton(transportTools, metronomeAction));

      //-------------------------------
      //    Concert Pitch Tool Bar
      //-------------------------------

      cpitchTools = addToolBar("");
      cpitchTools->setObjectName("pitch-tools");
      a = getAction("concert-pitch");
      a->setCheckable(true);
      cpitchTools->addWidget(new AccessibleToolButton(cpitchTools, a));

      //-------------------------------
      //    Image Capture Tool Bar
      //-------------------------------

      fotoTools = addToolBar("");
      fotoTools->setObjectName("foto-tools");
      fotoTools->addWidget(new AccessibleToolButton(fotoTools, getAction("fotomode")));

      addToolBarBreak();

      //-------------------------------
      //    Note Input Tool Bar
      //-------------------------------

      entryTools = addToolBar("");
      entryTools->setObjectName("entry-tools");

      populateNoteInputMenu();

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

      for (auto i : {
            "",
            "file-save",
            "file-save-as",
            "file-save-a-copy",
            "file-save-selection",
            "file-save-online",
            "file-export",
            "file-part-export",
            "file-import-pdf",
            "",
            "file-close",
            "",
            "parts",
            "album" }) {
            if (!*i)
                  menuFile->addSeparator();
            else if (enableExperimental || strcmp(i,"file-save-online") != 0)
                  menuFile->addAction(getAction(i));
            }
      if (enableExperimental)
            menuFile->addAction(getAction("layer"));
      menuFile->addSeparator();
      menuFile->addAction(getAction("edit-info"));
      if (enableExperimental)
            menuFile->addAction(getAction("media"));
      menuFile->addSeparator();
      menuFile->addAction(getAction("print"));
#ifndef Q_OS_MAC
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
      pref = menuEdit->addAction("", this, SLOT(startPreferenceDialog()));
      pref->setMenuRole(QAction::PreferencesRole);

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
            sprintf(buffer, "note-%c", "cdefgab"[i]);
            a = getAction(buffer);
            menuAddPitch->addAction(a);
            }
      menuAddPitch->addSeparator();
      for (int i = 0; i < 7; ++i) {
            char buffer[8];
            sprintf(buffer, "chord-%c", "cdefgab"[i]);
            a = getAction(buffer);
            menuAddPitch->addAction(a);
            }
      menuAdd->addMenu(menuAddPitch);

      menuAddInterval = new QMenu();
      for (int i = 1; i < 10; ++i) {
            char buffer[16];
            sprintf(buffer, "interval%d", i);
            a = getAction(buffer);
            menuAddInterval->addAction(a);
            }
      menuAddInterval->addSeparator();
      for (int i = 2; i < 10; ++i) {
            char buffer[16];
            sprintf(buffer, "interval-%d", i);
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

      QMenu* menuStretch = new QMenu(tr("&Stretch"));
      for (auto i : { "stretch+", "stretch-", "reset-stretch" })
            menuStretch->addAction(getAction(i));
      menuFormat->addMenu(menuStretch);
      menuFormat->addSeparator();

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

      menuVoices = new QMenu("");
      for (auto i : { "voice-x12", "voice-x13", "voice-x14", "voice-x23", "voice-x24", "voice-x34" })
            menuVoices->addAction(getAction(i));
      menuTools->addMenu(menuVoices);
      menuTools->addSeparator();

      menuTools->addAction(getAction("slash-fill"));
      menuTools->addAction(getAction("slash-rhythm"));
      menuTools->addSeparator();

      menuTools->addAction(getAction("pitch-spell"));
      menuTools->addAction(getAction("reset-groupings"));
      menuTools->addAction(getAction("resequence-rehearsal-marks"));
      menuTools->addSeparator();

      menuTools->addAction(getAction("copy-lyrics-to-clipboard"));
      menuTools->addAction(getAction("fotomode"));

      menuTools->addAction(getAction("del-empty-measures"));

      //---------------------
      //    Menu Plugins
      //---------------------

      menuPlugins = mb->addMenu("");
      menuPlugins->setObjectName("Plugins");

      menuPlugins->addAction(getAction("plugin-manager"));

      a = getAction("plugin-creator");
      a->setCheckable(true);
      menuPlugins->addAction(a);

      menuPlugins->addSeparator();

      //---------------------
      //    Menu Debug
      //---------------------

#ifndef NDEBUG
      QMenu* menuDebug = mb->addMenu("Debug");
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
      a = getAction("show-measure-shapes");
      a->setCheckable(true);
      menuDebug->addAction(a);
      a = getAction("show-bounding-rect");
      a->setCheckable(true);
      menuDebug->addAction(a);
      a = getAction("show-corrupted-measures");
      a->setCheckable(true);
      a->setChecked(true);
      menuDebug->addAction(a);
      a = getAction("relayout");
      menuDebug->addAction(a);
      a = getAction("autoplace-slurs");
      a->setCheckable(true);
      a->setChecked(MScore::autoplaceSlurs);
      menuDebug->addAction(a);
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
      onlineHandbookAction = menuHelp->addAction("", this, SLOT(helpBrowser1()));

      menuHelp->addSeparator();

      aboutAction = new QAction("", 0);

      aboutAction->setMenuRole(QAction::AboutRole);
      connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
      menuHelp->addAction(aboutAction);

      aboutQtAction = new QAction("", 0);
      aboutQtAction->setMenuRole(QAction::AboutQtRole);
      connect(aboutQtAction, SIGNAL(triggered()), this, SLOT(aboutQt()));
      menuHelp->addAction(aboutQtAction);

      aboutMusicXMLAction = new QAction("", 0);
      aboutMusicXMLAction->setMenuRole(QAction::ApplicationSpecificRole);
      connect(aboutMusicXMLAction, SIGNAL(triggered()), this, SLOT(aboutMusicXML()));
      menuHelp->addAction(aboutMusicXMLAction);

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
#if not defined(FOR_WINSTORE)
      checkForUpdateAction = menuHelp->addAction("", this, SLOT(checkForUpdate()));
#endif
#endif
      menuHelp->addSeparator();
      askForHelpAction = menuHelp->addAction("", this, SLOT(askForHelp()));
      reportBugAction = menuHelp->addAction("", this, SLOT(reportBug()));

      menuHelp->addSeparator();
      menuHelp->addAction(getAction("resource-manager"));
      menuHelp->addSeparator();
      revertToFactoryAction = menuHelp->addAction("", this, SLOT(resetAndRestart()));

      if (!MScore::noGui) {
            retranslate(true);
            //accessibility for menus
            for (QMenu* menu : mb->findChildren<QMenu*>()) {
                  menu->setAccessibleName(menu->objectName());
                  menu->setAccessibleDescription(Shortcut::getMenuShortcutString(menu));
                  }
            }

      setCentralWidget(envelope);

      reloadInstrumentTemplates();

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

      QInputMethod* im = QGuiApplication::inputMethod();
      connect(im, SIGNAL(localeChanged()), SLOT(inputMethodLocaleChanged()));

      if (enableExperimental) {
            cornerLabel = new QLabel(this);
            cornerLabel->setScaledContents(true);
            cornerLabel->setPixmap(QPixmap(":/data/mscore.png"));
            cornerLabel->setGeometry(width() - 48, 0, 48, 48);
            }
      }

MuseScore::~MuseScore()
      {
      delete synti;
      }

//---------------------------------------------------------
//   showError
//---------------------------------------------------------

void MuseScore::showError()
      {
      static QErrorMessage* msg = 0;
      if (msg == 0)
            msg = new QErrorMessage(this);
      msg->showMessage(tr(MScore::errorMessage()), MScore::errorGroup());
      MScore::setError(MS_NO_ERROR);
      }

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void MuseScore::retranslate(bool firstStart)
      {
      _positionLabel->setToolTip(tr("Measure:Beat:Tick"));

      // retranslate the menu
      menuFile->setTitle(tr("&File"));
      openRecent->setTitle(tr("Open &Recent"));
      menuEdit->setTitle(tr("&Edit"));
      menuView->setTitle(tr("&View"));
      menuToolbars->setTitle(tr("&Toolbars"));
      menuWorkspaces->setTitle(tr("W&orkspaces"));
      pref->setText(tr("&Preferences..."));
      menuAdd->setTitle(tr("&Add"));
      menuAddMeasures->setTitle(tr("&Measures"));
      menuAddFrames->setTitle(tr("&Frames"));
      menuAddText->setTitle(tr("&Text"));
      menuAddLines->setTitle(tr("&Lines"));
      menuAddPitch->setTitle(tr("N&otes"));
      menuAddInterval->setTitle(tr("&Intervals"));
      menuTuplet->setTitle(tr("T&uplets"));
      menuFormat->setTitle(tr("F&ormat"));
      menuTools->setTitle(tr("&Tools"));
      menuVoices->setTitle(tr("&Voices"));
      menuPlugins->setTitle(tr("&Plugins"));
      menuHelp->setTitle(tr("&Help"));

      aboutAction->setText(tr("&About..."));
      aboutQtAction->setText(tr("About &Qt..."));
      aboutMusicXMLAction->setText(tr("About &MusicXML..."));
      onlineHandbookAction->setText(tr("&Online Handbook"));
      if (checkForUpdateAction)
            checkForUpdateAction->setText(tr("Check for &Update"));
      askForHelpAction->setText(tr("Ask for Help"));
      reportBugAction->setText(tr("Report a Bug"));
      revertToFactoryAction->setText(tr("Revert to Factory Settings"));

      fileTools->setWindowTitle(tr("File Operations"));
      transportTools->setWindowTitle(tr("Playback Controls"));
      cpitchTools->setWindowTitle(tr("Concert Pitch"));
      fotoTools->setWindowTitle(tr("Image Capture"));
      entryTools->setWindowTitle(tr("Note Input"));

      viewModeCombo->setAccessibleName(tr("View Mode"));
      viewModeCombo->setItemText(viewModeCombo->findData(int(LayoutMode::PAGE)), tr("Page View"));
      viewModeCombo->setItemText(viewModeCombo->findData(int(LayoutMode::LINE)), tr("Continuous View"));
      viewModeCombo->setItemText(viewModeCombo->findData(int(LayoutMode::SYSTEM)), tr("Single Page"));

      showMidiImportButton->setText(tr("Show MIDI import panel"));

      Shortcut::retranslate();
      if (!firstStart && Workspace::currentWorkspace->readOnly()) {
            changeWorkspace(Workspace::currentWorkspace);
            }
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
      QString lang;
      if (localeName.toLower() == "system")
            lang = QLocale::system().name();
      else
            lang = localeName;
      return lang;
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
      foreach (LanguageItem item, _languages) {
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
            foreach (LanguageItem item, _languages) {
                  if (item.key == lang){
                      QString handbook = item.handbook;
                      if (!handbook.isNull())
                          help = item.handbook;
                      break;
                      }
                  }
            }

      //track visits. see: http://www.google.com/support/googleanalytics/bin/answer.py?answer=55578
      help += QString("&utm_source=desktop&utm_medium=menu&utm_content=%1&utm_campaign=MuseScore%2").arg(rev.trimmed()).arg(QString(VERSION));
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
      QString a = action->data().toString();
      if (!a.isEmpty()) {
            if (a == "clear-recent") {
                  _recentScores.clear();
                  if (startcenter)
                        startcenter->updateRecentScores();
                  }
            else {
                  MasterScore* score = readScore(a);
                  if (score) {
                        setCurrentScoreView(appendScore(score));
                        addRecentScore(score);
                        writeSessionFile(false);
                        }
                  }
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
            if (cs)
                  _pianoTools->changeSelection(cs->selection());
            else
                  _pianoTools->clearSelection();
            }
      if (_inspector)
            updateInspector();
      }

//---------------------------------------------------------
//   updateInspector
//---------------------------------------------------------

void MuseScore::updateInspector()
      {
      if (_inspector)
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
      tab1->blockSignals(true);
      if (tab2)
            tab2->blockSignals(true);
      tab1->insertTab(score);
      if (tab2)
            tab2->insertTab(score);
      tab1->blockSignals(false);
      if (tab2)
            tab2->blockSignals(false);
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
      if (_recentScores.size() > RECENT_LIST_SIZE)
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
            if (!path.isEmpty() && QFileInfo(path).exists()) {
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
      bool one = false;
      for (const QFileInfo& fi : recentScores()) {
            QAction* action = openRecent->addAction(fi.fileName().replace("&", "&&"));  // show filename only
            QString data(fi.canonicalFilePath());
            action->setData(data);
            action->setToolTip(data);
            one = true;
            }
      if (one) {
            openRecent->addSeparator();
            QAction* action = openRecent->addAction(tr("Clear Recent Files"));
            action->setData("clear-recent");
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
      QStringList extensionDir = Extension::getDirectoriesByType("instruments");
      QStringList filter("*.xml");
      for (QString s : extensionDir) {
            QDir extDir(s);
            extDir.setNameFilters(filter);
            auto instFiles = extDir.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::Readable);
            for (auto instFile : instFiles)
                  loadInstrumentTemplates(instFile.absoluteFilePath());
            }
      }

//---------------------------------------------------------
//   setCurrentView
//---------------------------------------------------------

void MuseScore::setCurrentScoreView(int idx)
      {
      setCurrentView(0, idx);
      if (tab2)
            setCurrentView(1, idx);
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
            mixer->updateAll(cs ? cs->masterScore() : nullptr);
#ifdef OMR
      if (omrPanel) {
            if (cv && cv->omrView())
                  omrPanel->setOmrView(cv->omrView());
            else
                  omrPanel->setOmrView(0);
            }
#endif
      if (!cs) {
            setWindowTitle(MUSESCORE_NAME_VERSION);
            if (_navigator && _navigator->widget()) {
                  navigator()->setScoreView(cv);
                  navigator()->setScore(0);
                  }
            if (timeline()) {
                  timeline()->setScoreView(cv);
                  timeline()->setScore(0);
                  }
            if (_inspector)
                  _inspector->update(0);
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
      getAction("file-part-export")->setEnabled(cs->masterScore()->excerpts().size() > 0);
      getAction("show-invisible")->setChecked(cs->showInvisible());
      getAction("show-unprintable")->setChecked(cs->showUnprintable());
      getAction("show-frames")->setChecked(cs->showFrames());
      getAction("show-pageborders")->setChecked(cs->showPageborders());
      getAction("mark-irregular")->setChecked(cs->markIrregularMeasures());
      getAction("fotomode")->setChecked(cv->fotoMode());
      getAction("join-measures")->setEnabled(cs->masterScore()->excerpts().size() == 0);
      getAction("split-measure")->setEnabled(cs->masterScore()->excerpts().size() == 0);
      updateUndoRedo();

      MagIdx midx = cv->magIdx();
      if (midx == MagIdx::MAG_FREE)
            mag->setMag(view->lmag());
      else {
            mag->setMagIdx(midx);
            magChanged(midx);
            }

      updateWindowTitle(cs);
      setWindowModified(cs->dirty());

      QAction* a = getAction("concert-pitch");
      a->setChecked(cs->styleB(Sid::concertPitch));

      setPos(cs->inputPos());
      //showMessage(cs->filePath(), 2000);
      if (_navigator && _navigator->widget()) {
            navigator()->setScore(cs);
            navigator()->setScoreView(view);
            }
      if (timeline()) {
            timeline()->setScore(cs);
            timeline()->setScoreView(view);
            }
      ScoreAccessibility::instance()->updateAccessibilityInfo();
      }

//---------------------------------------------------------
//   updateViewModeCombo
//---------------------------------------------------------

void MuseScore::updateViewModeCombo()
      {
      int idx;
      switch (cs->layoutMode()) {
            case LayoutMode::PAGE:
            case LayoutMode::FLOAT:
                  idx = 0;
                  break;
            case LayoutMode::LINE:
                  idx = 1;
                  break;
            case LayoutMode::SYSTEM:
                  idx = 2;
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
      const QMimeData* data = event->mimeData();
      if (data->hasUrls()) {
            QList<QUrl>ul = event->mimeData()->urls();
            foreach(const QUrl& u, ul) {
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
      const QMimeData* data = event->mimeData();
      if (data->hasUrls()) {
            int view = -1;
            foreach(const QUrl& u, event->mimeData()->urls()) {
                  if (u.scheme() == "file") {
                        QString file = u.toLocalFile();
                        MasterScore* score = readScore(file);
                        if (score) {
                              view = appendScore(score);
                              addRecentScore(score);
                              }
                        }
                  }
            if (view != -1) {
                  setCurrentScoreView(view);
                  writeSessionFile(false);
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
//   showPageSettings
//---------------------------------------------------------

void MuseScore::showPageSettings()
      {
      if (pageSettings == 0)
            pageSettings = new PageSettings();
      pageSettings->setScore(cs->masterScore());
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
//   showPlayPanel
//---------------------------------------------------------

void MuseScore::showPlayPanel(bool visible)
      {
      if (noSeq || !(seq && seq->isRunning()))
            return;
      if (playPanel == 0) {
            if (!visible)
                  return;
            playPanel = new PlayPanel(this);
            connect(playPanel, SIGNAL(gainChange(float)),     synti, SLOT(setGain(float)));
            connect(playPanel, SIGNAL(metronomeGainChanged(float)), seq, SLOT(setMetronomeGain(float)));
            connect(playPanel, SIGNAL(relTempoChanged(double)),seq, SLOT(setRelTempo(double)));
            connect(playPanel, SIGNAL(posChange(int)),         seq, SLOT(seek(int)));
            connect(playPanel, SIGNAL(closed(bool)),          playId,   SLOT(setChecked(bool)));
            connect(synti,     SIGNAL(gainChanged(float)), playPanel, SLOT(setGain(float)));
            playPanel->setGain(synti->gain());
            playPanel->setScore(cs);
            mscore->stackUnder(playPanel);
            }
      playPanel->setVisible(visible);
      playId->setChecked(visible);
      }

//---------------------------------------------------------
//   cmdAppendMeasures
//---------------------------------------------------------

void MuseScore::cmdAppendMeasures()
      {
      if (cs) {
            if (measuresDialog == 0)
                  measuresDialog = new MeasuresDialog;
            measuresDialog->show();
            }
      }

//---------------------------------------------------------
//   MeasuresDialog
//---------------------------------------------------------

MeasuresDialog::MeasuresDialog(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      measures->selectAll();
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void MeasuresDialog::accept()
      {
      int n = measures->value();
      if (mscore->currentScore())
            mscore->currentScoreView()->cmdAppendMeasures(n, ElementType::MEASURE);
      done(1);
      }

//---------------------------------------------------------
//   midiinToggled
//---------------------------------------------------------

void MuseScore::midiinToggled(bool val)
      {
      _midiinEnabled = val;
      }

//---------------------------------------------------------
//   midiinEnabled
//---------------------------------------------------------

bool MuseScore::midiinEnabled() const
      {
      return preferences.getBool(PREF_IO_MIDI_ENABLEINPUT) && _midiinEnabled;
      }

//---------------------------------------------------------
//   processMidiRemote
//    return if midi remote command detected
//---------------------------------------------------------

bool MuseScore::processMidiRemote(MidiRemoteType type, int data, int value)
      {
      if (!preferences.getBool(PREF_IO_MIDI_USEREMOTECONTROL))
            return false;
      if (!value) {
            // This was a "NoteOff" or "CtrlOff" event. Most MidiRemote actions should only
            // be triggered by an "On" event, so we need to check if this is one of those.
            if (!preferences.getBool(PREF_IO_MIDI_ADVANCEONRELEASE)
                    || type != preferences.midiRemote(RMIDI_REALTIME_ADVANCE).type
                    || data != preferences.midiRemote(RMIDI_REALTIME_ADVANCE).data)
                  return false;
            }
      for (int i = 0; i < MIDI_REMOTES; ++i) {
            if (preferences.midiRemote(i).type == type && preferences.midiRemote(i).data == data) {
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

      if (!midiinEnabled())
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
      if (!midiinEnabled())
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

      if (checkDirty(score))
            return;
      if (seq && seq->score() == score) {
            seq->stopWait();
            seq->setScoreView(0);
            }

      int idx1      = tab1->currentIndex();
      bool firstTab = tab1->view(idx1) == cv;

      midiPanelOnCloseFile(score->importedFilePath());
      scoreList.removeAt(i);

      tab1->blockSignals(true);
      tab1->removeTab(i);
      tab1->blockSignals(false);

      if (tab2) {
            tab2->blockSignals(true);
            tab2->removeTab(i);
            tab2->blockSignals(false);
            }

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
//   loadTranslation
//---------------------------------------------------------

void loadTranslation(QString filename, QString localeName)
      {
      QString userPrefix    = dataPath + "/locale/"+ filename +"_";
      QString defaultPrefix = mscoreGlobalShare + "locale/"+ filename +"_";
      QString userlp        = userPrefix + localeName;
      QString defaultlp     = defaultPrefix + localeName;
      QString lp            = defaultlp;

      QFileInfo userFi(userlp + ".qm");
      QFileInfo defaultFi(defaultlp + ".qm");

      if (!defaultFi.exists()) {     // try with a shorter locale name
            QString shortLocaleName = localeName.left(localeName.lastIndexOf("_"));
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
            translatorList.append(translator);
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

void setMscoreLocale(QString localeName)
      {
      for (QTranslator* t : translatorList) {
            qApp->removeTranslator(t);
            delete t;
            }
      translatorList.clear();

      if (MScore::debugMode)
            qDebug("configured localeName <%s>", qPrintable(localeName));
      if (localeName.toLower() == "system") {
            localeName = QLocale::system().name();
            if (MScore::debugMode)
                  qDebug("real localeName <%s>", qPrintable(localeName));
            if (localeName == "en_AU") {
                  localeName = "en_GB"; // otherwise Australia would fall back to US English
                  if (MScore::debugMode)
                        qDebug("modified localeName <%s>", qPrintable(localeName));
                  }
            }

      // find the most recent translation file
      // try to replicate QTranslator.load algorithm in our particular case
      loadTranslation("mscore", localeName);
      loadTranslation("instruments", localeName);

      QString resourceDir;
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
      resourceDir = mscoreGlobalShare + "locale/";
#else
      resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif
      QTranslator* qtTranslator = new QTranslator;
      if (MScore::debugMode)
            qDebug("load translator <qt_%s> from <%s>",
               qPrintable(localeName), qPrintable(resourceDir));

      if (!qtTranslator->load(QLatin1String("qt_") + localeName, resourceDir) && MScore::debugMode)
            qDebug("load translator <qt_%s> failed", qPrintable(localeName));
      else {
            qApp->installTranslator(qtTranslator);
            translatorList.append(qtTranslator);
            }
      QLocale locale(localeName);
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
                              {
                              QSettings settings;
                              int n = settings.value("scores", 0).toInt();
                              int c = settings.value("currentScore", 0).toInt();
                              for (int i = 0; i < n; ++i) {
                                    QString s = settings.value(QString("score-%1").arg(i),"").toString();
                                    MasterScore* score = mscore->readScore(s);
                                    if (score) {
                                          int view = mscore->appendScore(score);
                                          if (i == c)
                                                currentScoreView = view;
                                          }
                                    }
                              }
                              break;
                        case SessionStart::EMPTY:
                              break;
                        case SessionStart::NEW:
                              mscore->newFile();
                              break;
                        case SessionStart::SCORE:
                              {
                              QString startScore = preferences.getString(PREF_APP_STARTUP_STARTSCORE);
                              MasterScore* score = mscore->readScore(startScore);
                              if (startScore.startsWith(":/") && score) {
                                    score->setName(mscore->createDefaultName());
                                    // TODO score->setPageFormat(*MScore::defaultStyle().pageFormat());
                                    score->doLayout();
                                    score->setCreated(true);
                                    }
                              if (score == 0) {
                                    score = mscore->readScore(":/data/My_First_Score.mscz");
                                    if (score) {
                                          score->setName(mscore->createDefaultName());
                                          // TODO score->setPageFormat(*MScore::defaultStyle().pageFormat());
                                          score->doLayout();
                                          score->setCreated(true);
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
            foreach(const QString& name, argv) {
                  if (name.isEmpty())
                        continue;
                  MasterScore* score = mscore->readScore(name);
                  if (score) {
                        mscore->appendScore(score);
                        scoresOnCommandline = true;
                        if(!MScore::noGui) {
                              mscore->addRecentScore(score);
                              mscore->writeSessionFile(false);
                              }
                        }
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

static bool doConvert(Score* cs, QString fn, QString plugin = "")
      {
      bool rv = true;
      LayoutMode layoutMode = cs->layoutMode();
      cs->setLayoutMode(LayoutMode::PAGE);
      if (cs->layoutMode() != layoutMode) {
            cs->setLayoutMode(LayoutMode::PAGE);
            cs->doLayout();
            }

      if (!styleFile.isEmpty()) {
            QFile f(styleFile);
            if (f.open(QIODevice::ReadOnly))
                  cs->style().load(&f);
            }
      if (!plugin.isEmpty()) {
            mscore->setCurrentScore(cs);
            LayoutMode layoutMode = cs->layoutMode();
            if (layoutMode != LayoutMode::PAGE) {
                  cs->setLayoutMode(LayoutMode::PAGE);
                  cs->doLayout();
                  }
            if (mscore->loadPlugin(plugin))
                  mscore->pluginTriggered(0);
            mscore->unloadPlugins();
            if (layoutMode != cs->layoutMode()) {
                  cs->setLayoutMode(layoutMode);
                  cs->doLayout();
                  }
            }
      if (fn.endsWith(".mscx")) {
            QFileInfo fi(fn);
            if (!cs->saveFile(fi))
                  return false;
            return true;
            }
      else if (fn.endsWith(".mscz")) {
            QFileInfo fi(fn);
            if (!cs->saveCompressedFile(fi, false))
                  return false;
            return true;
            }
      else if (fn.endsWith(".xml") || fn.endsWith(".musicxml")) {
            rv = saveXml(cs, fn);
            }
      else if (fn.endsWith(".mxl")) {
            rv = saveMxl(cs, fn);
            }
      else if (fn.endsWith(".mid"))
            return mscore->saveMidi(cs, fn);
      else if (fn.endsWith(".pdf")) {
            if (!exportScoreParts) {
                  rv = mscore->savePdf(cs, fn);
                  }
            else {
                  if (cs->excerpts().size() == 0) {
                        auto excerpts = Excerpt::createAllExcerpt(cs->masterScore());

                        for (Excerpt* e : excerpts) {
                              Score* nscore = new Score(e->oscore());
                              e->setPartScore(nscore);
                              nscore->style().set(Sid::createMultiMeasureRests, true);
                              Excerpt::createExcerpt(e);
                              cs->startCmd();
                              cs->undo(new AddExcerpt(e));
                              cs->endCmd();
                              }
                        }
                  QList<Score*> scores;
                  scores.append(cs);
                  for (Excerpt* e : cs->excerpts())
                        scores.append(e->partScore());
                  return mscore->savePdf(scores, fn);
                  }
            }
      else if (fn.endsWith(".png")) {
            if (!exportScoreParts)
                  return mscore->savePng(cs, fn);
            else {
                  if (cs->excerpts().size() == 0) {
                        auto excerpts = Excerpt::createAllExcerpt(cs->masterScore());

                        for (Excerpt* e: excerpts) {
                              Score* nscore = new Score(e->oscore());
                              e->setPartScore(nscore);
                              nscore->setExcerpt(e);
                              // nscore->setName(e->title()); // needed before AddExcerpt
                              nscore->style().set(Sid::createMultiMeasureRests, true);
                              Excerpt::createExcerpt(e);
                              cs->startCmd();
                              cs->undo(new AddExcerpt(e));
                              cs->endCmd();
                              }
                        }
                  if (!mscore->savePng(cs, fn))
                        return false;
                  int idx = 0;
                  int padding = QString("%1").arg(cs->excerpts().size()).size();
                  for (Excerpt* e: cs->excerpts()) {
                        QString suffix = QString("__excerpt__%1.png").arg(idx, padding, 10, QLatin1Char('0'));
                        QString excerptFn = fn.left(fn.size() - 4) + suffix;
                        if (!mscore->savePng(e->partScore(), excerptFn))
                              return false;
                        idx++;
                        }
                  return true;
                  }
            }
      else if (fn.endsWith(".svg")) {
            rv = mscore->saveSvg(cs, fn);
            }
#ifdef HAS_AUDIOFILE
            else if (fn.endsWith(".wav") || fn.endsWith(".ogg") || fn.endsWith(".flac"))
                  return mscore->saveAudio(cs, fn);
#endif
#ifdef USE_LAME
            else if (fn.endsWith(".mp3"))
                  return mscore->saveMp3(cs, fn);
#endif
      else if (fn.endsWith(".spos")) {
            rv = savePositions(cs, fn, true);
            }
      else if (fn.endsWith(".mpos")) {
            rv = savePositions(cs, fn, false);
            }
      else if (fn.endsWith(".mlog"))
            return cs->sanityCheck(fn);
      else if (plugin.isEmpty()) {
            qDebug("don't know how to convert to %s", qPrintable(outFileName));
            return false;
            }
      if (layoutMode != cs->layoutMode()) {
            cs->setLayoutMode(layoutMode);
            cs->doLayout();
            }
      return rv;
      }

//---------------------------------------------------------
//   convert
//---------------------------------------------------------

static bool convert(const QString& inFile, const QString& outFile, const QString& plugin = "")
      {
      if (inFile.isEmpty() || (outFile.isEmpty() && plugin.isEmpty())) {
            fprintf(stderr, "cannot convert <%s> to <%s>\n", qPrintable(inFile), qPrintable(outFile));
            return false;
            }
      fprintf(stderr, "convert <%s> to <%s>\n", qPrintable(inFile), qPrintable(outFile));
      MasterScore* score = mscore->readScore(inFile);
      if (!score)
            return false;
      if (!doConvert(score, outFile, plugin)) {
            delete score;
            return false;
            }
      delete score;
      return true;
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
      for (const auto i : a) {
            QString inFile;
            QString outFile;
            QString plugin;
            if (!i.isObject()) {
                  fprintf(stderr, "array value is not an object\n");
                  return false;
                  }
            QJsonObject obj = i.toObject();
            for (const auto& key : obj.keys()) {
                  QString val = obj.value(key).toString();
                  if (key == "in")
                        inFile = val;
                  else if (key == "out")
                        outFile = val;
                  else if (key == "plugin")
                        plugin = val;
                  else {
                        fprintf(stderr, "unknown key <%s>\n", qPrintable(key));
                        return false;
                        }
                  }
            if (!convert(inFile, outFile, plugin))
                  return false;
            }
      return true;
      }

//---------------------------------------------------------
//   processNonGui
//---------------------------------------------------------

static bool processNonGui(const QStringList& argv)
      {
      if (pluginMode) {
            loadScores(argv);
            QString pn(pluginName);
            bool res = false;
            if (mscore->loadPlugin(pn)){
                  Score* cs = mscore->currentScore();
                  if (!styleFile.isEmpty()) {
                        QFile f(styleFile);
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
            if (!converterMode)
                  return res;
            }
      bool rv = true;
      if (converterMode) {
            if (processJob)
                  return doProcessJob(jsonFileName);
            else
                  return convert(argv[0], outFileName);
            }
      return rv;
      }

//---------------------------------------------------------
//   Message handler
//---------------------------------------------------------

#if defined(QT_DEBUG) && defined(Q_OS_WIN)
static void mscoreMessageHandler(QtMsgType type, const QMessageLogContext &context,const QString &msg)
     {
     QTextStream cerr(stderr);
     QByteArray localMsg = msg.toLocal8Bit();

     switch (type) {
     case QtInfoMsg:
         cerr << "Info: "     << localMsg.constData() << " ("  << context.file << ":" << context.line << ", " << context.function << ")" << endl;
         break;
     case QtDebugMsg:
         cerr << "Debug: "    << localMsg.constData() << " ("  << context.file << ":" << context.line << ", " << context.function << ")" << endl;
         break;
     case QtWarningMsg:
         cerr << "Warning: "  << localMsg.constData() << " ("  << context.file << ":" << context.line << ", " << context.function << ")" << endl;
         break;
     case QtCriticalMsg: // same as QtSystemMsg
         cerr << "Critical: " << localMsg.constData() << " ("  << context.file << ":" << context.line << ", " << context.function << ")" << endl;
         break;
     case QtFatalMsg: // set your breakpoint here, if you want to catch the abort
         cerr << "Fatal: "    << localMsg.constData() << " ("  << context.file << ":" << context.line << ", " << context.function << ")" << endl;
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
      ms->registerEffect(0, new ZitaReverb);
      ms->registerEffect(0, new Compressor);
      // ms->registerEffect(0, new Freeverb);
      ms->registerEffect(1, new NoEffect);
      ms->registerEffect(1, new ZitaReverb);
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
#ifdef MSCORE_UNSTABLE
      return true;
#else
      return false;
#endif
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
                              if(currentScoreView())
                                    currentScoreView()->setFocus();
                              else
                                    mscore->setFocus();
                              return true;
                              }

                        QWidget* w = static_cast<QWidget*>(obj);
                        if(getPaletteBox()->isAncestorOf(w) ||
                           inspector()->isAncestorOf(w) ||
                           (selectionWindow && selectionWindow->isAncestorOf(w))) {
                              activateWindow();
                              if(currentScoreView())
                                    currentScoreView()->setFocus();
                              else
                                    mscore->setFocus();
                              return true;
                              }
                        }
                  break;
                  }
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
//   checkForUpdate
//---------------------------------------------------------

void MuseScore::checkForUpdate()
      {
      if (ucheck)
            ucheck->check(version(), sender() != 0);
      }

//---------------------------------------------------------
//   readLanguages
//---------------------------------------------------------

bool MuseScore::readLanguages(const QString& path)
      {
      //: The default language of the operating system. NOT a music system.
      _languages.append(LanguageItem("system", tr("System")));
      QFile qf(path);
      if (qf.exists()){
          QDomDocument doc;
          int line, column;
          QString err;
          if (!doc.setContent(&qf, false, &err, &line, &column)) {
                QString error;
                error.sprintf(qPrintable(tr("Error reading language file %s at line %d column %d: %s\n")),
                   qPrintable(qf.fileName()), line, column, qPrintable(err));
                QMessageBox::warning(0,
                   QWidget::tr("Load Languages Failed:"),
                   error,
                   QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
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
      getAction("swap")->setEnabled(flag);
      }

//---------------------------------------------------------
//   inputMethodLocaleChanged
//---------------------------------------------------------

void MuseScore::inputMethodLocaleChanged()
      {
      qDebug("Input method QLocale::Script enum now #%d.", QGuiApplication::inputMethod()->locale().script());
      }

//---------------------------------------------------------
//   showModeText
//---------------------------------------------------------

void MuseScore::showModeText(const QString& s)
      {
      _modeText->setText(s);
      _modeText->show();
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
      "pad-note-8", "pad-note-16", "pad-note-32", "pad-note-64", "pad-note-128", "pad-rest", "rest"};
      static const char* tabNames[] = {
            "note-longa-TAB", "note-breve-TAB", "pad-note-1-TAB", "pad-note-2-TAB", "pad-note-4-TAB",
      "pad-note-8-TAB", "pad-note-16-TAB", "pad-note-32-TAB", "pad-note-64-TAB", "pad-note-128-TAB", "pad-rest-TAB", "rest-TAB"};
      bool intoTAB = (_sstate != STATE_NOTE_ENTRY_STAFF_TAB) && (val == STATE_NOTE_ENTRY_STAFF_TAB);
      bool fromTAB = (_sstate == STATE_NOTE_ENTRY_STAFF_TAB) && (val != STATE_NOTE_ENTRY_STAFF_TAB);
      // if activating TAB note entry, swap "pad-note-...-TAB" shorctuts into "pad-note-..." actions
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
            else if (enable && (s->key() == "synth-control")) {
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

      if (getAction("file-part-export")->isEnabled())
            getAction("file-part-export")->setEnabled(cs && cs->masterScore()->excerpts().size() > 0);
      if (getAction("join-measures")->isEnabled())
            getAction("join-measures")->setEnabled(cs && cs->masterScore()->excerpts().size() == 0);
      if (getAction("split-measure")->isEnabled())
            getAction("split-measure")->setEnabled(cs && cs->masterScore()->excerpts().size() == 0);

      //getAction("split-measure")->setEnabled(cs->masterScore()->excerpts().size() == 0);

      // disabling top level menu entries does not
      // work for MAC

      QList<QObject*> ol = menuBar()->children();
      foreach(QObject* o, ol) {
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
      mag->setEnabled(enable);
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
                  _modeText->hide();
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
                  else {
                        showModeText(tr("Steptime note input mode"));
                        cs->setNoteEntryMethod(NoteEntryMethod::STEPTIME);
                        val = STATE_NOTE_ENTRY_METHOD_STEPTIME;
                        }
                  break;
            case STATE_NOTE_ENTRY_STAFF_DRUM:
                  {
                  showModeText(tr("Drum input mode"));
                  InputState& is = cs->inputState();
                  showDrumTools(is.drumset(), cs->staff(is.track() / VOICES));
                  if (_drumTools)
                        is.setDrumNote(_drumTools->selectedDrumNote());
                  }
                  break;
            case STATE_NOTE_ENTRY_STAFF_TAB:
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
                  showModeText(tr("Play"));
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
      if (paletteBox)
            paletteBox->setDisabled(val == STATE_PLAY || val == STATE_DISABLED);
      if (selectionWindow)
            selectionWindow->setDisabled(val == STATE_PLAY || val == STATE_DISABLED);
      QAction* a = getAction("note-input");
      bool noteEntry = val & STATE_NOTE_ENTRY;
      a->setChecked(noteEntry);
      _sstate = val;

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
      settings.setValue("showPanel", paletteBox && paletteBox->isVisible());
      settings.setValue("showInspector", _inspector && _inspector->isVisible());
      settings.setValue("showPianoKeyboard", _pianoTools && _pianoTools->isVisible());
      settings.setValue("showSelectionWindow", selectionWindow && selectionWindow->isVisible());
      settings.setValue("state", saveState());
      settings.setValue("splitScreen", _splitScreen);
      settings.setValue("debuggerSplitter", mainWindow->saveState());
      settings.setValue("split", _horizontalSplit);
      settings.setValue("splitter", splitter->saveState());
      settings.endGroup();

      Workspace::currentWorkspace->save();
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
      if (mixer)
            mixer->writeSettings();
      settings.setValue("mixerVisible", mixer && mixer->isVisible());
      if (seq) {
            seq->stopWait();
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
      const QSize screenSize = screen->availableVirtualSize();
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
            int tick   = c->segment() ? c->segment()->tick() : 0;
            seq->seek(tick);
            Instrument* instr = part->instrument(tick);
            for (Note* n : c->notes()) {
                  const Channel* channel = instr->channel(n->subchannel());
                  seq->startNote(channel->channel, n->ppitch(), 80, n->tuning());
                  }
            seq->startNoteTimer(MScore::defaultPlayDuration);
            }
      }

void MuseScore::play(Element* e, int pitch) const
      {
      if (noSeq || !(seq && seq->isRunning()))
            return;
      if (preferences.getBool(PREF_SCORE_NOTE_PLAYONCLICK) && e->isNote()) {
            Note* note = static_cast<Note*>(e);
            int tick = note->chord()->tick();
            if (tick < 0)
                  tick = 0;
            Instrument* instr = note->part()->instrument(tick);
            const Channel* channel = instr->channel(note->subchannel());
            seq->startNote(channel->channel, pitch, 80, MScore::defaultPlayDuration, note->tuning());
            }
      }

//---------------------------------------------------------
//   reportBug
//---------------------------------------------------------

void MuseScore::reportBug()
      {
      QString url = QString("https://musescore.org/redirect/post/bug-report?sha=%1&locale=%2").arg(revision()).arg(getLocaleISOCode());
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
//   about
//---------------------------------------------------------

void MuseScore::about()
      {
      AboutBoxDialog ab;
      ab.show();
      ab.exec();
      }

//---------------------------------------------------------
//   AboutBoxDialog
//---------------------------------------------------------

AboutBoxDialog::AboutBoxDialog()
      {
      setupUi(this);
      museLogo->setPixmap(QPixmap(preferences.isThemeDark() ?
            ":/data/musescore-logo-transbg-m.png" : ":/data/musescore_logo_full.png"));

      if (MuseScore::unstable())
            versionLabel->setText(tr("Unstable Prerelease for Version: %1").arg(VERSION));
      else
            versionLabel->setText(tr("Version: %1").arg(VERSION));
      revisionLabel->setText(tr("Revision: %1").arg(revision));
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      copyrightLabel->setText(QString("<span style=\"font-size:10pt;\">%1</span>")
                              .arg(tr(   "Visit %1www.musescore.org%2 for new versions and more information.\n"
                                         "Support MuseScore with your %3donation%4.\n\n"
                                         "Copyright &copy; 1999-2018 Werner Schweer and Others.\n"
                                         "Published under the GNU General Public License.")
                                   .arg("<a href=\"http://www.musescore.org/\">")
                                   .arg("</a>")
                                   .arg("<a href=\"http://www.musescore.org/donate\">")
                                   .arg("</a>")
                                   .replace("\n","<br/>")));
      connect(copyRevisionButton, SIGNAL(clicked()), this, SLOT(copyRevisionToClipboard()));
      }

//---------------------------------------------------------
//   copyRevisionToClipboard
//---------------------------------------------------------

void AboutBoxDialog::copyRevisionToClipboard()
      {
      QClipboard* cb = QApplication::clipboard();
      QString sysinfo = "OS: ";
      sysinfo += QSysInfo::prettyProductName();
      sysinfo += ", Arch.: ";
      sysinfo += QSysInfo::currentCpuArchitecture();
      // endianness?
      sysinfo += ", MuseScore version (";
      sysinfo += QSysInfo::WordSize==32?"32":"64";
      sysinfo += "-bit): " VERSION ", revision: ";
      sysinfo += "github-musescore-musescore-";
      sysinfo += revision;
      cb->setText(sysinfo);
      }

//---------------------------------------------------------
//   AboutBoxDialog
//---------------------------------------------------------

AboutMusicXMLBoxDialog::AboutMusicXMLBoxDialog()
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      label->setText(QString("<span style=\"font-size:10pt;\">%1<br/></span>")
                     .arg(tr(   "MusicXML is an open file format for exchanging digital sheet music,\n"
                                "supported by many applications.\n"
                                "Copyright © 2004-2017 the Contributors to the MusicXML\n"
                                "Specification, published by the W3C Music Notation Community\n"
                                "Group under the W3C Community Contributor License Agreement\n"
                                "(CLA):\n%1\n"
                                "A human-readable summary is available:\n%2")
                          .arg( "\n&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"https://www.w3.org/community/about/agreements/cla/\">https://www.w3.org/community/about/agreements/cla/</a>\n",
                                "\n&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"https://www.w3.org/community/about/agreements/cla-deed/\">https://www.w3.org/community/about/agreements/cla-deed/</a>\n")
                          .replace("\n","<br/>")));
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
//   magChanged
//---------------------------------------------------------

void MuseScore::magChanged(MagIdx idx)
      {
      if (cv)
            cv->setMag(idx, mag->getLMag(cv));
      }

//---------------------------------------------------------
//   incMag
//---------------------------------------------------------

void MuseScore::incMag()
      {
      if (cv) {
            qreal _mag = cv->lmag() * SCALE_STEP;
            if (_mag > SCALE_MAX)
                  _mag = SCALE_MAX;
            cv->setMag(MagIdx::MAG_FREE, _mag);
            setMag(_mag);
            }
      }

//---------------------------------------------------------
//   decMag
//---------------------------------------------------------

void MuseScore::decMag()
      {
      if (cv) {
            qreal _mag = cv->lmag() / SCALE_STEP;
            if (_mag < SCALE_MIN)
                  _mag = SCALE_MIN;
            cv->setMag(MagIdx::MAG_FREE, _mag);
            setMag(_mag);
            }
      }

//---------------------------------------------------------
//   getMag
//    return physical scale
//---------------------------------------------------------

double MuseScore::getMag(ScoreView* canvas) const
      {
      return mag->getMag(canvas);
      }

//---------------------------------------------------------
//   setMag
//    set logical scale
//---------------------------------------------------------

void MuseScore::setMag(double d)
      {
      mag->setMag(d);
      mag->setMagIdx(MagIdx::MAG_FREE);
      }

//---------------------------------------------------------
//   setPos
//    set position label
//---------------------------------------------------------

void MuseScore::setPos(int t)
      {
      if (cs == 0 || t < 0)
            return;

      TimeSigMap* s = cs->sigmap();
      int bar, beat, tick;
      s->tickValues(t, &bar, &beat, &tick);
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
      endCmd();
      if (_inspector)
            _inspector->update();
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
      MasterScore* score = readScore(message);
      if (score) {
            setCurrentScoreView(appendScore(score));
            addRecentScore(score);
            writeSessionFile(false);
            }
      }

//---------------------------------------------------------
//   editInPianoroll
//---------------------------------------------------------

void MuseScore::editInPianoroll(Staff* staff)
      {
      if (pianorollEditor == 0)
            pianorollEditor = new PianorollEditor;
      pianorollEditor->setScore(staff->score());
      pianorollEditor->setStaff(staff);
      pianorollEditor->show();
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
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      xml.tagE(cleanExit ? "clean" : "dirty");

      foreach(MasterScore* score, scoreList) {
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
                  if (v->magIdx() == MagIdx::MAG_FREE)
                        xml.tag("mag", v->lmag());
                  else
                        xml.tag("magIdx", int(v->magIdx()));
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
                        if (v->magIdx() == MagIdx::MAG_FREE)
                              xml.tag("mag", v->lmag());
                        else
                              xml.tag("magIdx", int(v->magIdx()));
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
      for (MasterScore* s : scoreList) {
            if (s->autosaveDirty()) {
                  QString tmp = s->tmpName();
                  if (!tmp.isEmpty()) {
                        QFileInfo fi(tmp);
                        // TODO: cannot catch exception here:
                        s->saveCompressedFile(fi, false);
                        }
                  else {
                        QDir dir;
                        dir.mkpath(dataPath);
                        QTemporaryFile tf(dataPath + "/scXXXXXX.mscz");
                        tf.setAutoRemove(false);
                        if (!tf.open()) {
                              qDebug("autoSaveTimerTimeout(): create temporary file failed");
                              return;
                              }
                        s->setTmpName(tf.fileName());
                        QFileInfo info(tf.fileName());
                        s->saveCompressedFile(&tf, info, false, false);  // no thumbnail
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

//---------------------------------------------------------
//   restoreSession
//    Restore last session. If "always" is true, then restore
//    last session even on a clean exit else only if last
//    session ended abnormal.
//    Return true if a session was found and restored.
//---------------------------------------------------------

bool MuseScore::restoreSession(bool always)
      {
      QFile f(dataPath + "/session");
      if (!f.exists())
            return false;
      if (!f.open(QIODevice::ReadOnly)) {
            qDebug("Cannot open session file <%s>", qPrintable(f.fileName()));
            return false;
            }
      XmlReader e(&f);
      int tab = 0;
      int idx = -1;
      bool cleanExit = false;
      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  /* QString version = e.attribute(QString("version"));
                  QStringList sl  = version.split('.');
                  int v           = sl[0].toInt() * 100 + sl[1].toInt();
                  */
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "clean") {
                              if (!always)
                                    return false;
                              cleanExit = true;
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
                                    const QStringRef& tag(e.name());
                                    if (tag == "name")
                                          name = e.readElementText();
                                    else if (tag == "created")
                                          created = e.readInt();
                                    else if (tag == "dirty")
                                          /*int dirty =*/ e.readInt();
                                    else if (tag == "path") {
                                          MasterScore* score = readScore(e.readElementText());
                                          if (score) {
                                                if (!name.isEmpty()) {
                                                      QFileInfo* fi = score->masterScore()->fileInfo();
                                                      fi->setFile(name);
                                                      // TODO: what if that path is no longer valid?
                                                      }
                                                if (cleanExit) {
                                                      // override if last session did a clean exit
                                                      created = false;
                                                      }
                                                appendScore(score);
                                                score->setCreated(created);
                                                }
                                          }
                                    else {
                                          e.unknown();
                                          return false;
                                          }
                                    }
                              }
                        else if (tag == "ScoreView") {
                              qreal x       = .0;
                              qreal y       = .0;
                              qreal vmag    = 1.0;
                              MagIdx magIdx = MagIdx::MAG_FREE;
                              int tab       = 0;
                              int idx       = 0;
                              while (e.readNextStartElement()) {
                                    const QStringRef& tag(e.name());
                                    if (tag == "tab")
                                          tab = e.readInt();
                                    else if (tag == "idx")
                                          idx = e.readInt();
                                    else if (tag == "mag")
                                          vmag = e.readDouble();
                                    else if (tag == "magIdx")
                                          magIdx = MagIdx(e.readInt());
                                    else if (tag == "x")
                                          x = e.readDouble() * DPMM;
                                    else if (tag == "y")
                                          y = e.readDouble() * DPMM;
                                    else {
                                          e.unknown();
                                          return false;
                                          }
                                    }
                              (tab == 0 ? tab1 : tab2)->initScoreView(idx, vmag, magIdx, x, y);
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
                  tab2 = new ScoreTab(&scoreList, this);
                  tab2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                  connect(tab2, SIGNAL(currentScoreViewChanged(ScoreView*)), SLOT(setCurrentScoreView(ScoreView*)));
                  connect(tab2, SIGNAL(tabCloseRequested(int)), SLOT(removeTab(int)));
                  connect(tab2, SIGNAL(actionTriggered(QAction*)), SLOT(cmd(QAction*)));
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
            }
      else {
            if (_horizontalSplit == horizontal) {
                  _splitScreen = false;
                  tab2->setVisible(false);
                  setCurrentView(0, tab1->currentIndex());
                  }
            else {
                  _horizontalSplit = horizontal;
                  QAction* a;
                  if (_horizontalSplit)
                        a = getAction("split-v");
                  else
                        a = getAction("split-h");
                  a->setChecked(false);
                  splitter->setOrientation(_horizontalSplit ? Qt::Horizontal : Qt::Vertical);
                  }
            }
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

void MuseScore::showPianoKeyboard(bool on)
      {
      if (_pianoTools == 0) {
            QAction* a = getAction("toggle-piano");
            _pianoTools = new PianoTools(this);
            addDockWidget(Qt::BottomDockWidgetArea, _pianoTools);
            connect(_pianoTools, SIGNAL(keyPressed(int, bool, int)), SLOT(midiNoteReceived(int, bool, int)));
            connect(_pianoTools, SIGNAL(keyReleased(int, bool, int)), SLOT(midiNoteReceived(int, bool, int)));
            connect(_pianoTools, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));
            }
      if (on) {
            _pianoTools->show();
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
//   getPaletteBox
//---------------------------------------------------------

PaletteBox* MuseScore::getPaletteBox()
      {
      if (paletteBox == 0) {
            paletteBox = new PaletteBox(this);
            QAction* a = getAction("toggle-palette");
            connect(paletteBox, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));
            addDockWidget(Qt::LeftDockWidgetArea, paletteBox);
            }
      return paletteBox;
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

void MuseScore::networkFinished(QNetworkReply* reply)
      {
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

      QByteArray data = reply->readAll();
      QString tmpName = QDir::tempPath () + "/"+ name;
      QFile f(tmpName);
      f.open(QIODevice::WriteOnly);
      f.write(data);
      f.close();

      reply->deleteLater();

      MasterScore* score = readScore(tmpName);
      if (!score) {
            qDebug("readScore failed");
            return;
            }
      score->setCreated(true);
      setCurrentScoreView(appendScore(score));
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
      QNetworkReply* nr = networkManager()->get(QNetworkRequest(url));
      connect(nr, SIGNAL(finished(QNetworkReply*)),
               SLOT(networkFinished(QNetworkReply*)));
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
                        for (Note* ee : pattern.el)
                              score->select(ee, SelectType::ADD, 0);
                        }
                  else if (sd.doSubtract()) {
                        QList<Element*> sl(score->selection().elements());
                        for (Note* ee : pattern.el)
                              sl.removeOne(ee);
                        score->select(0, SelectType::SINGLE, 0);
                        for (Element* ee : sl)
                              score->select(ee, SelectType::ADD, 0);
                        }
                  else if (sd.doAdd()) {
                        QList<Element*> sl(score->selection().elements());
                        for (Note* ee : pattern.el) {
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
                        for (Element* ee : pattern.el)
                              score->select(ee, SelectType::ADD, 0);
                        }
                  else if (sd.doSubtract()) {
                        QList<Element*> sl(score->selection().elements());
                        for (Element* ee : pattern.el)
                              sl.removeOne(ee);
                        score->select(0, SelectType::SINGLE, 0);
                        for (Element* ee : sl)
                              score->select(ee, SelectType::ADD, 0);
                        }
                  else if (sd.doAdd()) {
                        QList<Element*> sl(score->selection().elements());
                        for (Element* ee : pattern.el) {
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
      p.setPen(QPen(QColor(MScore::selectColor[0]), 2.0/p.matrix().m11()));
      if (active)
            p.setBrush(MScore::selectColor[0]);
      else
            p.setBrush(Qt::NoBrush);
      qreal w = 8.0 / p.matrix().m11();
      qreal h = 8.0 / p.matrix().m22();

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

      // TRANSPOSE_BY_KEY and "transpose keys" is only possible if selection state is SelState::RANGE
      td.enableTransposeKeys(rangeSelection);
      td.enableTransposeByKey(rangeSelection);
      td.enableTransposeChordNames(rangeSelection);

      int startStaffIdx = 0;
      int endStaffIdx   = 0;
      int startTick     = 0;
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
                              key = transposeKey(key, diff);
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
      if (sc == 0) {
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
      if (cmdn == "palette-search") {
            PaletteBox* pb = getPaletteBox();
            QLineEdit* sb = pb->searchBox();
            sb->setFocus();
            if (pb->noSelection())
                  pb->setKeyboardNavigation(false);
            else
                  pb->setKeyboardNavigation(true);
            return;
            }
      if (cmdn == "apply-current-palette-element") {
            PaletteBox* pb = getPaletteBox();
            for (Palette* p : pb->palettes()) {
                  if (p->getCurrentIdx() != -1) {
                        p->applyPaletteElement();
                        break;
                        }
                  }
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
      if (sc->isCmd()) {
            if (!cv->editMode())
                  cs->startCmd();
            }
      cmd(a, cmdn);
      if (lastShortcut->isCmd())
            cs->endCmd();
      endCmd();
      }

//---------------------------------------------------------
//   endCmd
//    Called after every command action (including every
//    mouse action).
//    Updates the UI after a possible score change.
//---------------------------------------------------------

void MuseScore::endCmd()
      {
      if (timeline())
            timeline()->updateGrid();
      if (MScore::_error != MS_NO_ERROR)
            showError();
      if (cs) {
            setPos(cs->inputState().tick());
            updateInputState(cs);
            updateUndoRedo();
            dirtyChanged(cs);
            Element* e = cs->selection().element();

            // For multiple notes selected check if they all have same pitch and tuning
            bool samePitch = true;
            int pitch    = -1;
            float tuning = 0;
            for (Element* e : cs->selection().elements()) {
                  if (!e->isNote()) {
                        samePitch = false;
                        break;
                        }
                  Note* note = toNote(e);
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
                  if (tab2) {
//                      ScoreView* v = tab2->view();
//                      if (v && v->score() == ms) {
                              tab2->updateExcerpts();
//                            }
                        }
                  if (tab1) {
                        ScoreView* v = tab1->view();
                        if (v && v->score()->masterScore() == ms) {
                              tab1->updateExcerpts();
                              }
                        else if (v == 0) {
                              tab1->setExcerpt(0);
                              tab1->updateExcerpts();
                              }
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
            getAction("concert-pitch")->setChecked(cs->styleB(Sid::concertPitch));

            if (e == 0 && cs->noteEntryMode())
                  e = cs->inputState().cr();
            updateViewModeCombo();
            ScoreAccessibility::instance()->updateAccessibilityInfo();
            }
      else {
            selectionChanged(SelState::NONE);
            }
      updateInspector();
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
            cs->updateRepeatList(repeat);
            emit cs->playlistChanged();
            }
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void MuseScore::cmd(QAction* a, const QString& cmd)
      {
      if (cmd == "instruments") {
            editInstrList();
            if (mixer)
                  mixer->updateAll(cs->masterScore());
            }
      else if (cmd == "rewind") {
            seq->rewindStart();
            if (playPanel)
                  playPanel->heartBeat(0, 0, 0);
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
      else if (cmd == "file-open")
            loadFiles();
      else if (cmd == "file-save")
            saveFile();
      else if (cmd == "file-save-online")
            showUploadScoreDialog();
      else if (cmd == "file-export")
            exportFile();
      else if (cmd == "file-part-export")
            exportParts();
      else if (cmd == "file-import-pdf")
            openExternalLink("https://musescore.com/import");
      else if (cmd == "file-close")
            closeScore(cs);
      else if (cmd == "file-save-as")
            saveAs(cs, false);
      else if (cmd == "file-save-selection")
            saveSelection(cs);
      else if (cmd == "file-save-a-copy")
            saveAs(cs, true);
      else if (cmd == "file-new")
            newFile();
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
      else if (cmd == "layer")
            showLayerManager();
      else if (cmd == "backspace") {
            if (_sstate != STATE_NORMAL )
                  undoRedo(true);
#ifdef Q_OS_MAC
            else if (cs) {
                  cs->startCmd();
                  cs->cmdDeleteSelection();
                  cs->endCmd();
                  }
#endif
            }
      else if (cmd == "zoomin")
            incMag();
      else if (cmd == "zoomout")
            decMag();
      else if (cmd == "zoom100") {
            if (cv)
                  cv->setMag(MagIdx::MAG_100, 1.0);
            setMag(1.0);
            }
      else if (cmd == "midi-on")
            midiinToggled(a->isChecked());
      else if (cmd == "undo")
            undoRedo(true);
      else if (cmd == "redo")
            undoRedo(false);
      else if (cmd == "toggle-palette")
            showPalette(a->isChecked());
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
      else if (cmd == "help")
            showContextHelp();
      else if (cmd == "follow")
            preferences.setPreference(PREF_APP_PLAYBACK_FOLLOWSONG, a->isChecked());
      else if (cmd == "split-h")
            splitWindow(true);
      else if (cmd == "split-v")
            splitWindow(false);
      else if (cmd == "edit-harmony")
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
      else if (cmd == "plugin-creator")
            showPluginCreator(a);
      else if (cmd == "plugin-manager")
            showPluginManager();
      else if(cmd == "resource-manager"){
            ResourceManager r(0);
            r.exec();
            }
      else if (cmd == "media")
            showMediaDialog();
      else if (cmd == "page-settings")
            showPageSettings();
      else if (cmd == "next-score")
            changeScore(1);
      else if (cmd == "previous-score")
            changeScore(1);
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
                        QMessageBox::critical(this,
                           tr("Load Style"), MScore::lastError);
                        }
                  cs->endCmd();
                  }
            }
      else if (cmd == "edit-style") {
            EditStyle es(cs, this);
            es.exec();
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
            MScore::panPlayback = !MScore::panPlayback;
      else if (cmd == "show-invisible")
            cs->setShowInvisible(a->isChecked());
      else if (cmd == "show-unprintable")
            cs->setShowUnprintable(a->isChecked());
      else if (cmd == "show-frames")
            cs->setShowFrames(a->isChecked());
      else if (cmd == "show-pageborders")
            cs->setShowPageborders(a->isChecked());
      else if (cmd == "mark-irregular")
            cs->setMarkIrregularMeasures(a->isChecked());
      else if (cmd == "tempo")
            addTempo();
      else if (cmd == "loop") {
            if (loop()) {
                  if (cs->selection().isRange())
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
      else if (cmd == "show-measure-shapes") {
            MScore::showMeasureShapes = a->isChecked();
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
      else if (cmd == "show-corrupted-measures") {
            MScore::showCorruptedMeasures = a->isChecked();
            if (cs) {
                  cs->setLayoutAll();
                  cs->update();
                  }
            }
      else if (cmd == "autoplace-slurs") {
            MScore::autoplaceSlurs = a->isChecked();
            if (cs) {
                  cs->setLayoutAll();
                  cs->update();
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
                  foreach(const Layer& l, cs->layer())
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
            mixer->updateAll(cs->masterScore());
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
      Measure* m = cs->firstMeasure();
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
      if (m && m != cs->firstMeasure())
            cv->adjustCanvasPosition(m, false);
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

            searchDialogLayout->addWidget(new QLabel(tr("Go To: ")));

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
#ifdef Q_OS_MAC
      if (!cs->isMaster())
            setWindowTitle(cs->masterScore()->title() + "-" + cs->title());
      else
            setWindowTitle(cs->title());
      if (score->masterScore()->created())
            setWindowFilePath(QString());
      else
            setWindowFilePath(score->masterScore()->fileInfo()->absoluteFilePath());
#else
      if (!cs->isMaster())
            setWindowTitle(MUSESCORE_NAME_VERSION ": " + cs->masterScore()->title() + "-" + cs->title() + "[*]");
      else
            setWindowTitle(MUSESCORE_NAME_VERSION ": " + score->title() + "[*]");
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
            QString data(s);
            QFileInfo fi(s);
            bool alreadyLoaded = false;
            QString fp = fi.canonicalFilePath();
            for (Score* s : mscore->scores()) {
                  if ((s->masterScore()->fileInfo()->canonicalFilePath() == fp) || (s->importedFilePath() == fp)) {
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
      for (QAction* a : al) {
            // textTool visibility is handled differentlyr
            if (_textTools && a->text() == _textTools->windowTitle())
                  m->removeAction(a);
            }
      return m;
      }

//---------------------------------------------------------
//   synthesizerState
//---------------------------------------------------------

SynthesizerState MuseScore::synthesizerState()
      {
      SynthesizerState state;
      return synti ? synti->state() : state;
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
#ifndef USE_LAME
      return false;
#else
      EventMap events;
      score->renderMidi(&events);
      if(events.size() == 0)
            return false;

      MP3Exporter exporter;
      if (!exporter.loadLibrary(MP3Exporter::AskUser::MAYBE)) {
            QSettings settings;
            settings.setValue("/Export/lameMP3LibPath", "");
            if(!MScore::noGui)
                  QMessageBox::warning(0,
                               tr("Error Opening LAME library"),
                               tr("Could not open MP3 encoding library!"),
                               QString::null, QString::null);
            qDebug("Could not open MP3 encoding library!");
            return false;
            }

      if (!exporter.validLibraryLoaded()) {
            QSettings settings;
            settings.setValue("/Export/lameMP3LibPath", "");
            if(!MScore::noGui)
                  QMessageBox::warning(0,
                               tr("Error Opening LAME library"),
                               tr("Not a valid or supported MP3 encoding library!"),
                               QString::null, QString::null);
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
                     QString::null, QString::null);
                  }
            qDebug("Unable to initialize MP3 stream");
            MScore::sampleRate = oldSampleRate;
            return false;
            }

      QFile file(name);
      if (!file.open(QIODevice::WriteOnly)) {
            if (!MScore::noGui) {
                  QMessageBox::warning(0,
                     tr("Encoding Error"),
                     tr("Unable to open target file for writing"),
                     QString::null, QString::null);
                  }
            MScore::sampleRate = oldSampleRate;
            return false;
            }

      int bufferSize   = exporter.getOutBufferSize();
      uchar* bufferOut = new uchar[bufferSize];
      MasterSynthesizer* synti = synthesizerFactory();
      synti->init();
      synti->setSampleRate(sampleRate);
      if (MScore::noGui) { // use score settings if possible
            bool r = synti->setState(score->synthesizerState());
            if (!r)
                  synti->init();
            }
      else { // use current synth settings
            bool r = synti->setState(mscore->synthesizerState());
            if (!r)
                  synti->init();
            }

      MScore::sampleRate = sampleRate;

      QProgressDialog progress(this);
      progress.setWindowFlags(Qt::WindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowTitleHint));
      progress.setWindowModality(Qt::ApplicationModal);
      //progress.setCancelButton(0);
      progress.setCancelButtonText(tr("Cancel"));
      progress.setLabelText(tr("Exporting..."));
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
            synti->allSoundsOff(-1);

            //
            // init instruments
            //
            foreach(Part* part, score->parts()) {
                  const InstrumentList* il = part->instruments();
                  for(auto i = il->begin(); i!= il->end(); i++) {
                        foreach(const Channel* a, i->second->channel()) {
                              a->updateInitList();
                              foreach(MidiCoreEvent e, a->init) {
                                    if (e.type() == ME_INVALID)
                                          continue;
                                    e.setChannel(a->channel);
                                    int syntiIdx= synti->index(score->masterScore()->midiMapping(a->channel)->articulation->synti);
                                    synti->play(e, syntiIdx);
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
                              float bu[n * 2];
                              memset(bu, 0, sizeof(float) * 2 * n);

                              synti->process(n, bu);
                              float* sp = bu;
                              for (int i = 0; i < n; ++i) {
                                    *l++ = *sp++;
                                    *r++ = *sp++;
                                    }
                              playTime  += n;
                              frames    -= n;
                              }
                        const NPlayEvent& e = playPos->second;
                        if (e.isChannelEvent()) {
                              int channelIdx = e.channel();
                              Channel* c = score->masterScore()->midiMapping(channelIdx)->articulation;
                              if (!c->mute) {
                                    synti->play(e, synti->index(c->synti));
                                    }
                              }
                        }
                  if (frames) {
                        float bu[frames * 2];
                        memset(bu, 0, sizeof(float) * 2 * frames);
                        synti->process(frames, bu);
                        float* sp = bu;
                        for (unsigned i = 0; i < frames; ++i) {
                              *l++ = *sp++;
                              *r++ = *sp++;
                              }
                        playTime += frames;
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
                                       QString::null, QString::null);
                              break;
                              }
                        else
                              file.write((char*)bufferOut, bytes);
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
                        synti->allNotesOff(-1);
                  // create sound until the sound decays
                  if (playTime >= et && max * peak < 0.000001)
                        break;
                  // hard limit
                  if (playTime > maxEndTime)
                        break;
                  }
            if (progress.wasCanceled())
                  break;
            if (pass == 0 && peak == 0.0) {
                  qDebug("song is empty");
                  break;
                  }
            gain = 0.99 / peak;
            }

      long bytes = exporter.finishStream(bufferOut);
      if (bytes > 0L)
            file.write((char*)bufferOut, bytes);

      bool wasCanceled = progress.wasCanceled();
      progress.close();
      delete synti;
      delete[] bufferOut;
      file.close();
      if (wasCanceled)
            file.remove();
      MScore::sampleRate = oldSampleRate;
      return true;
#endif
      }

void MuseScore::updateUiStyleAndTheme()
      {
      // set UI Theme
      QApplication::setStyle(QStyleFactory::create("Fusion"));

      QString wd      = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).arg(QCoreApplication::applicationName());
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
      qApp->setStyleSheet(css);

      genIcons();
      Shortcut::refreshIcons();
      }
}

using namespace Ms;

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* av[])
      {
#ifndef NDEBUG
      qSetMessagePattern("%{file}:%{function}: %{message}");
      Ms::checkStyles();
#endif

      QApplication::setDesktopSettingsAware(true);
      QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
      QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#if defined(QT_DEBUG) && defined(Q_OS_WIN)
      qInstallMessageHandler(mscoreMessageHandler);
#endif

      qRegisterMetaTypeStreamOperators<SessionStart>("SessionStart");
      qRegisterMetaTypeStreamOperators<MusicxmlExportBreaks>("MusicxmlExportBreaks");
      qRegisterMetaTypeStreamOperators<MuseScoreStyleType>("MuseScoreStyleType");

      QFile f(":/revision.h");
      f.open(QIODevice::ReadOnly);
      revision = QString(f.readAll()).trimmed();
      f.close();

      const char* appName;
      const char* appName2;
      if (MuseScore::unstable()) {
            appName2 = "mscore-dev3";
            appName  = "MuseScoreDevelopment";
            }
      else {
            appName2 = "mscore3";
            appName  = "MuseScore3";
            }

      MuseScoreApplication* app = new MuseScoreApplication(appName2, argc, av);
      QCoreApplication::setApplicationName(appName);

      QCoreApplication::setOrganizationName("MuseScore");
      QCoreApplication::setOrganizationDomain("musescore.org");
      QCoreApplication::setApplicationVersion(VERSION);

      QAccessible::installFactory(AccessibleScoreView::ScoreViewFactory);
      QAccessible::installFactory(AccessibleSearchBox::SearchBoxFactory);
      QAccessible::installFactory(Awl::AccessibleAbstractSlider::AbstractSliderFactory);

      Q_INIT_RESOURCE(zita);

#ifndef Q_OS_MAC
      QSettings::setDefaultFormat(QSettings::IniFormat);
#endif

      QCommandLineParser parser;

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
      parser.addOption(QCommandLineOption({"r", "image-resolution"}, "Used with '-o <file>.png'. Set output resolution for image export", "DPI"));
      parser.addOption(QCommandLineOption({"T", "trim-image"}, "Used with '-o <file>.png' and '-o <file.svg>'. Trim exported image with specified margin (in pixels)", "margin"));
      parser.addOption(QCommandLineOption({"x", "gui-scaling"}, "Set scaling factor for GUI elements", "factor"));
      parser.addOption(QCommandLineOption({"D", "monitor-resolution"}, "Specify monitor resolution", "DPI"));
      parser.addOption(QCommandLineOption({"S", "style"}, "Load style file", "style"));
      parser.addOption(QCommandLineOption({"p", "plugin"}, "Execute named plugin", "name"));
      parser.addOption(QCommandLineOption(      "template-mode", "Save template mode, no page size"));
      parser.addOption(QCommandLineOption({"F", "factory-settings"}, "Use factory settings"));
      parser.addOption(QCommandLineOption({"R", "revert-settings"}, "Revert to default preferences"));
      parser.addOption(QCommandLineOption({"i", "load-icons"}, "Load icons from INSTALLPATH/icons"));
      parser.addOption(QCommandLineOption({"j", "job"}, "Process a conversion job", "file"));
      parser.addOption(QCommandLineOption({"e", "experimental"}, "Enable experimental features"));
      parser.addOption(QCommandLineOption({"c", "config-folder"}, "Override configuration and settings folder", "dir"));
      parser.addOption(QCommandLineOption({"t", "test-mode"}, "Set test mode flag for all files"));
      parser.addOption(QCommandLineOption({"M", "midi-operations"}, "Specify MIDI import operations file", "file"));
      parser.addOption(QCommandLineOption({"w", "no-webview"}, "No web view in start center"));
      parser.addOption(QCommandLineOption({"P", "export-score-parts"}, "Used with '-o <file>.pdf', export score and parts"));
      parser.addOption(QCommandLineOption(      "no-fallback-font", "Don't use Bravura as fallback musical font"));
      parser.addOption(QCommandLineOption({"f", "force"}, "Used with '-o <file>', ignore warnings reg. score being corrupted or from wrong version"));
      parser.addOption(QCommandLineOption({"b", "bitrate"}, "Used with '-o <file>.mp3', sets bitrate, in kbps", "bitrate"));

      parser.addPositionalArgument("scorefiles", "The files to open", "[scorefile...]");

      parser.process(QCoreApplication::arguments());

    //if (parser.isSet("v")) parser.showVersion(); // a) needs Qt >= 5.4 , b) instead we use addVersionOption()
      if (parser.isSet("long-version")) {
            printVersion("MuseScore");
            return EXIT_SUCCESS;
            }
      MScore::debugMode = parser.isSet("d");
      MScore::noHorizontalStretch = MScore::noVerticalStretch = parser.isSet("L");
      noSeq = parser.isSet("s");
      noMidi = parser.isSet("m");
      if (parser.isSet("a")) {
            audioDriver = parser.value("a");
            if (audioDriver.isEmpty())
                  parser.showHelp(EXIT_FAILURE);
            }
      startWithNewScore = parser.isSet("n");
      externalIcons = parser.isSet("i");
      midiInputTrace = parser.isSet("I");
      midiOutputTrace = parser.isSet("O");
      MScore::useFallbackFont = !parser.isSet("no-fallback-font");

      if ((converterMode = parser.isSet("o"))) {
            MScore::noGui = true;
            outFileName = parser.value("o");
            if (outFileName.isEmpty())
                  parser.showHelp(EXIT_FAILURE);
            }
      if ((processJob = parser.isSet("j"))) {
            MScore::noGui = true;
            converterMode = true;
            jsonFileName = parser.value("j");
            if (jsonFileName.isEmpty()) {
                  fprintf(stderr, "json file name missing\n");
                  parser.showHelp(EXIT_FAILURE);
                  }
            }
      if ((pluginMode = parser.isSet("p"))) {
            MScore::noGui = true;
            pluginName = parser.value("p");
            if (pluginName.isEmpty())
                  parser.showHelp(EXIT_FAILURE);
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
            styleFile = parser.value("S");
            if (styleFile.isEmpty())
                  parser.showHelp(EXIT_FAILURE);
            }
      useFactorySettings = parser.isSet("F");
      deletePreferences = (useFactorySettings || parser.isSet("R"));
      enableExperimental = parser.isSet("e");
      if (parser.isSet("c")) {
            QString path = parser.value("c");
            if (path.isEmpty())
                  parser.showHelp(EXIT_FAILURE);
            QDir dir;
            if (dir.exists(path)) {
                  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, path);
                  QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, path);
                  dataPath = path;
                  }
            }
      MScore::testMode = parser.isSet("t");
      if (parser.isSet("M")) {
            QString temp = parser.value("M");
            if (temp.isEmpty())
                  parser.showHelp(EXIT_FAILURE);
            midiImportOperations.setOperationsFile(temp);
            }
      noWebView = parser.isSet("w");
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

      QStringList argv = parser.positionalArguments();

      mscoreGlobalShare = getSharePath();
      iconPath = externalIcons ? mscoreGlobalShare + QString("icons/") :  QString(":/data/icons/");

      if (!converterMode && !pluginMode) {
            if (!argv.isEmpty()) {
                  int ok = true;
                  for (const QString& message : argv) {
                        QFileInfo fi(message);
                        if (!app->sendMessage(fi.absoluteFilePath())) {
                              ok = false;
                              break;
                              }

                        }
                  if (ok)
                        return 0;
                  }
            else
                  if (app->sendMessage(QString("")))
                      return 0;
            }

      if (dataPath.isEmpty())
            dataPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);

      if (deletePreferences) {
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

      // set translator before Shortcut are initialized to get translations for all shortcuts
      // can not use preferences to retrieve these values as it needs to be initialized after translator is set
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
      if (!MScore::testMode) {
            QPrinter p;
            if (p.isValid()) {
//                  qDebug("set paper size from default printer");
                  QRectF psf = p.paperRect(QPrinter::Inch);
                  MScore::defaultStyle().set(Sid::pageWidth,  psf.width());
                  MScore::defaultStyle().set(Sid::pageHeight, psf.height());
                  MScore::defaultStyle().set(Sid::pagePrintableWidth, psf.width()-20.0/INCH);
                  }
            }
#endif

#ifdef SCRIPT_INTERFACE
      if (-1 == qmlRegisterType<QmlPlugin>  ("MuseScore", 3, 0, "MuseScore"))
            qDebug("qmlRegisterType failed: MuseScore");
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

      QSplashScreen* sc = 0;
      QTimer* stimer = 0;
      if (!MScore::noGui && preferences.getBool(PREF_UI_APP_STARTUP_SHOWSPLASHSCREEN)) {
            QPixmap pm(":/data/splash.png");
            sc = new QSplashScreen(pm);
            sc->setWindowTitle(QString("MuseScore Startup"));
#ifdef Q_OS_MAC // to have session dialog on top of splashscreen on mac
            sc->setWindowFlags(Qt::FramelessWindowHint);
#endif
            // show splash screen for 5 sec
            stimer = new QTimer(0);
            qApp->connect(stimer, SIGNAL(timeout()), sc, SLOT(close()));
            stimer->start(5000);
            sc->show();
            qApp->processEvents();
            }

      if (!MScore::noGui)
            MuseScore::updateUiStyleAndTheme();
      else
            noSeq = true;

      genIcons();

      // Do not create sequencer and audio drivers if run with '-s'
      if (!noSeq) {
            seq            = new Seq();
            MScore::seq    = seq;
            Driver* driver = driverFactory(seq, audioDriver);
            synti          = synthesizerFactory();
            if (driver) {
                  MScore::sampleRate = driver->sampleRate();
                  synti->setSampleRate(MScore::sampleRate);
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
            foreach(const QString& s, sl)
                  qDebug("LibraryPath: <%s>", qPrintable(s));
            }

      // rastral size of font is 20pt = 20/72 inch = 20*DPI/72 dots
      //   staff has 5 lines = 4 * _spatium
      //   _spatium    = SPATIUM20;     // 20.0 / 72.0 * DPI / 4.0;

      if (!MScore::noGui) {
#ifndef Q_OS_MAC
            qApp->setWindowIcon(*icons[int(Icons::window_ICON)]);
#endif
            Workspace::initWorkspace();
            }

      mscore = new MuseScore();
      // create a score for internal use
      gscore = new MasterScore();
      gscore->setMovements(new Movements());
      gscore->setStyle(MScore::baseStyle());

      gscore->style().set(Sid::MusicalTextFont, QString("Bravura Text"));
      ScoreFont* scoreFont = ScoreFont::fontFactory("Bravura");
      gscore->setScoreFont(scoreFont);
      gscore->setNoteHeadWidth(scoreFont->width(SymId::noteheadBlack, gscore->spatium()) / SPATIUM20);

      if (!noSeq) {
            if (!seq->init())
                  qDebug("sequencer init failed");
            }

      //read languages list
      mscore->readLanguages(mscoreGlobalShare + "locale/languages.xml");

      if (!MScore::noGui) {
            if (preferences.getBool(PREF_APP_STARTUP_FIRSTSTART)) {
                  StartupWizard* sw = new StartupWizard;
                  sw->exec();
                  preferences.setPreference(PREF_APP_STARTUP_FIRSTSTART, false);
                  preferences.setPreference(PREF_APP_KEYBOARDLAYOUT, sw->keyboardLayout());
                  preferences.setPreference(PREF_UI_APP_LANGUAGE, sw->language());
                  setMscoreLocale(sw->language());
                  for (auto ws : Workspace::workspaces()) {
                        if (ws->name().compare(sw->workspace()) == 0) {
                              mscore->changeWorkspace(ws);
                              preferences.setPreference(PREF_APP_WORKSPACE, ws->name());
                              mscore->getPaletteBox()->updateWorkspaces();
                              }
                        }
                  delete sw;
                  }
            QString keyboardLayout = preferences.getString(PREF_APP_KEYBOARDLAYOUT);
            StartupWizard::autoSelectShortcuts(keyboardLayout);
            }

      QApplication::instance()->installEventFilter(mscore);

      mscore->setRevision(revision);
      int files = 0;
      bool restoredSession = false;
      if (MScore::noGui) {
#ifdef Q_OS_MAC
            // see issue #28706: Hangup in converter mode with MusicXML source
            qApp->processEvents();
#endif
            exit(processNonGui(argv) ? 0 : EXIT_FAILURE);
            }
      else {
            mscore->readSettings();
            QObject::connect(qApp, SIGNAL(messageReceived(const QString&)),
               mscore, SLOT(handleMessage(const QString&)));

            static_cast<QtSingleApplication*>(qApp)->setActivationWindow(mscore, false);
            // count filenames specified on the command line
            // these are the non-empty strings remaining in argv
            foreach(const QString& name, argv) {
                  if (!name.isEmpty())
                        ++files;
                  }
#ifdef Q_OS_MAC
            // app->paths contains files requested to be loaded by OS X
            // append these to argv and update file count
            foreach(const QString& name, app->paths) {
                  if (!name.isEmpty()) {
                        argv << name;
                        ++files;
                        }
                  }
#endif
            //
            // TODO: delete old session backups
            //
            restoredSession = mscore->restoreSession((preferences.sessionStart() == SessionStart::LAST && (files == 0)));
            if (!restoredSession || files)
                  loadScores(argv);
            }

      errorMessage = new QErrorMessage(mscore);
      mscore->getPluginManager()->readPluginList();
      mscore->loadPlugins();
      mscore->writeSessionFile(false);

#ifdef Q_OS_MAC
      // there's a bug in Qt showing the toolbar unified after switching showFullScreen(), showMaximized(),
      // showNormal()...
      mscore->setUnifiedTitleAndToolBarOnMac(false);

      // don't let macOS add "Show Tab Bar" to "View" Menu
      CocoaBridge::setAllowsAutomaticWindowTabbing(false);
#endif

      mscore->changeState(mscore->noScore() ? STATE_DISABLED : STATE_NORMAL);
      mscore->show();

#ifndef MSCORE_NO_UPDATE_CHECKER
      if (mscore->hasToCheckForUpdate())
            mscore->checkForUpdate();
#endif

      if (!scoresOnCommandline && preferences.getBool(PREF_UI_APP_STARTUP_SHOWSTARTCENTER) && (!restoredSession || mscore->scores().size() == 0)) {
#ifdef Q_OS_MAC
// ugly, but on mac we get an event when a file is open.
// We can't get the event when the startcenter is shown.
// So we let the event loop run a bit before showing the start center.
            QTimer *timer = new QTimer();
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

      mscore->showPlayPanel(preferences.getBool(PREF_UI_APP_STARTUP_SHOWPLAYPANEL));
      QSettings settings;
      if (settings.value("synthControlVisible", false).toBool())
            mscore->showSynthControl(true);
      if (settings.value("mixerVisible", false).toBool())
            mscore->showMixer(true);

      return qApp->exec();
      }

