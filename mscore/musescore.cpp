//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: musescore.cpp 5652 2012-05-19 18:22:24Z wschweer $
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

#include <fenv.h>

#include "musescore.h"
#include "scoreview.h"
#include "libmscore/style.h"
#include "libmscore/score.h"
#include "instrdialog.h"
#include "preferences.h"
#include "prefsdialog.h"
#include "icons.h"
#include "textstyle.h"
#include "libmscore/xml.h"
#include "seq.h"
#include "libmscore/tempo.h"
#include "libmscore/sym.h"
#include "pagesettings.h"
#include "debugger.h"
#include "editstyle.h"
#include "playpanel.h"
#include "libmscore/page.h"
#include "mixer.h"
#include "palette.h"
#include "libmscore/part.h"
#include "libmscore/drumset.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/note.h"
#include "libmscore/staff.h"
#include "driver.h"
#include "libmscore/harmony.h"
#include "magbox.h"
#include "voiceselector.h"
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
#include "libmscore/chord.h"
#include "mstyle/mstyle.h"
#include "libmscore/segment.h"
#include "editraster.h"
#include "pianotools.h"
#include "mediadialog.h"
#include "profile.h"
#include "webpage.h"
#include "selectdialog.h"
#include "transposedialog.h"
#include "metaedit.h"
#include "chordedit.h"
#include "edittempo.h"
#include "inspector.h"
#include "omrpanel.h"
#include "shortcut.h"

#include "libmscore/mscore.h"
#include "libmscore/system.h"
#include "libmscore/measurebase.h"
#include "libmscore/chordlist.h"
#include "libmscore/volta.h"

#include "msynth/synti.h"

//import qt bindings for plugin framework
#if ( defined(BUILD_SCRIPTGEN) && defined(STATIC_SCRIPT_BINDINGS) )
Q_IMPORT_PLUGIN(com_trolltech_qt_gui_ScriptPlugin)
Q_IMPORT_PLUGIN(com_trolltech_qt_core_ScriptPlugin)
Q_IMPORT_PLUGIN(com_trolltech_qt_network_ScriptPlugin)
Q_IMPORT_PLUGIN(com_trolltech_qt_uitools_ScriptPlugin)
Q_IMPORT_PLUGIN(com_trolltech_qt_xml_ScriptPlugin)
Q_IMPORT_PLUGIN(com_trolltech_qt_webkit_ScriptPlugin)
#endif

MuseScore* mscore;

bool enableExperimental = false;

QString dataPath;
QString iconPath, iconGroup;

bool converterMode = false;
bool noGui = false;
bool externalIcons = false;
static bool pluginMode = false;
static bool startWithNewScore = false;
double converterDpi = 0;

QString mscoreGlobalShare;
static QStringList recentScores;
static QString outFileName;
static QString pluginName;
static QString styleFile;
static QString localeName;
bool useFactorySettings = false;
QString styleName;
QString revision;

extern void initStaffTypes();

// Mac-Applications don't have menubar icons:
#ifdef Q_WS_MAC
extern void qt_mac_set_menubar_icons(bool b);
#endif


//---------------------------------------------------------
// cmdInsertMeasure
//---------------------------------------------------------

void MuseScore::cmdInsertMeasures()
      {
	if (cs) {
		insertMeasuresDialog = new InsertMeasuresDialog;
		insertMeasuresDialog->show();
            }
      }

//---------------------------------------------------------
// InsertMeasuresDialog
//---------------------------------------------------------

InsertMeasuresDialog::InsertMeasuresDialog(QWidget* parent)
   : QDialog(parent)
      {
	setupUi(this);
      }

//---------------------------------------------------------
// Insert Measure -->   accept
//---------------------------------------------------------

void InsertMeasuresDialog::accept()
      {
	int n = insmeasures->value();
	if (mscore->currentScore())
            mscore->currentScoreView()->cmdInsertMeasures(n, MEASURE);
	done(1);
      }

//---------------------------------------------------------
//   getSharePath
//---------------------------------------------------------

static QString getSharePath()
      {
#ifdef __MINGW32__
      QDir dir(QCoreApplication::applicationDirPath() + QString("/../" INSTALL_NAME));
      return dir.absolutePath() + "/";
#else
#ifdef Q_WS_MAC
      QDir dir(QCoreApplication::applicationDirPath() + QString("/../Resources"));
      return dir.absolutePath() + "/";
#else
      return QString( INSTPREFIX "/share/" INSTALL_NAME);
#endif
#endif
      }

//---------------------------------------------------------
//   printVersion
//---------------------------------------------------------

static void printVersion(const char* prog)
      {
#ifdef MSCORE_UNSTABLE
      qDebug("%s: Music Score Editor\nUnstable Prerelease for Version %s; Build %s",
         prog, VERSION, qPrintable(revision));
#else
     qDebug("%s: Music Score Editor; Version %s; Build %s", prog, VERSION, qPrintable(revision));
#endif
      }

static const int RECENT_LIST_SIZE = 10;

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void MuseScore::closeEvent(QCloseEvent* ev)
      {
      if (cs)
            cs->setSyntiState(synti->state());
      unloadPlugins();
      QList<Score*> removeList;
      foreach(Score* score, scoreList) {
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
      foreach(Score* score, removeList)
            scoreList.removeAll(score);

      writeSessionFile(true);
      foreach(Score* score, scoreList) {
            if (!score->tmpName().isEmpty()) {
                  QFile f(score->tmpName());
                  f.remove();
                  }
            }

      // save score list
      QSettings settings;
      for (int i = 0; i < RECENT_LIST_SIZE; ++i)
            settings.setValue(QString("recent-%1").arg(i), recentScores.value(i));

      settings.setValue("scores", scoreList.size());
      int curScore = scoreList.indexOf(cs);
      if (curScore == -1)  // cs removed if new created and not modified
            curScore = 0;
      settings.setValue("currentScore", curScore);

      int idx = 0;
      foreach(Score* s, scoreList) {
            settings.setValue(QString("score-%1").arg(idx), s->fileInfo()->absoluteFilePath());
            ++idx;
            }

      settings.setValue("lastSaveCopyDirectory", lastSaveCopyDirectory);
      settings.setValue("lastSaveDirectory", lastSaveDirectory);

      if (playPanel)
            preferences.playPanelPos = playPanel->pos();

      if (synthControl)
            synthControl->updatePreferences();

      writeSettings();
      if (debugger)
            debugger->writeSettings();

      seq->stopWait();
      seq->exit();
      ev->accept();
      if (preferences.dirty)
            preferences.write();
//      this->deleteLater();     !?
      qApp->quit();
      }

//---------------------------------------------------------
//   preferencesChanged
//---------------------------------------------------------

void MuseScore::preferencesChanged()
      {
      for (int i = 0; i < tab1->count(); ++i) {
            ScoreView* canvas = tab1->view(i);
            if (canvas == 0)
                  continue;
            if (preferences.bgUseColor)
                  canvas->setBackground(MScore::bgColor);
            else {
                  QPixmap* pm = new QPixmap(preferences.bgWallpaper);
                  canvas->setBackground(pm);
                  }
            if (preferences.fgUseColor)
                  canvas->setForeground(preferences.fgColor);
            else {
                  QPixmap* pm = new QPixmap(preferences.fgWallpaper);
                  if (pm == 0 || pm->isNull())
                        qDebug("no valid pixmap %s", preferences.fgWallpaper.toLatin1().data());
                  canvas->setForeground(pm);
                  }
            }
      if (tab2) {
            for (int i = 0; i < tab2->count(); ++i) {
                  ScoreView* canvas = tab2->view(i);
                  if (canvas == 0)
                        continue;
                  if (preferences.bgUseColor)
                        canvas->setBackground(MScore::bgColor);
                  else {
                        QPixmap* pm = new QPixmap(preferences.bgWallpaper);
                        canvas->setBackground(pm);
                        }
                  if (preferences.fgUseColor)
                        canvas->setForeground(preferences.fgColor);
                  else {
                        QPixmap* pm = new QPixmap(preferences.fgWallpaper);
                        if (pm == 0 || pm->isNull())
                              qDebug("no valid pixmap %s", preferences.fgWallpaper.toLatin1().data());
                        canvas->setForeground(pm);
                        }
                  }
            }

      transportTools->setEnabled(!noSeq);
      playId->setEnabled(!noSeq);

      getAction("midi-on")->setEnabled(preferences.enableMidiInput);
      _statusBar->setShown(preferences.showStatusBar);
      }

//---------------------------------------------------------
//   MuseScore
//---------------------------------------------------------

MuseScore::MuseScore()
   : QMainWindow()
      {
      _sstate = STATE_INIT;
      setWindowTitle(QString("MuseScore"));
      setIconSize(QSize(preferences.iconWidth, preferences.iconHeight));

      ucheck                = new UpdateChecker();

      setAcceptDrops(true);
      cs                    = 0;
      cv                    = 0;
      se                    = 0;    // script engine
      pluginMapper          = 0;
      debugger              = 0;
      instrList             = 0;
      playPanel             = 0;
      preferenceDialog      = 0;
      measuresDialog        = 0;
      insertMeasuresDialog  = 0;
      masterPalette         = 0;
      iledit                = 0;
      synthControl          = 0;
      debugger              = 0;
      measureListEdit       = 0;
      symbolDialog          = 0;
      clefPalette           = 0;
      keyPalette            = 0;
      keyEditor             = 0;
      pageSettings          = 0;
      paletteBox            = 0;
      inspector             = 0;
      omrPanel              = 0;
      _midiinEnabled        = true;
      _speakerEnabled       = true;
      newWizard             = 0;
      lastOpenPath          = preferences.myScoresPath;
      _textTools            = 0;
      _editTools            = 0;
      _pianoTools           = 0;
      _webPage              = 0;
      _mediaDialog          = 0;
      _drumTools            = 0;
      pianorollEditor       = 0;
      drumrollEditor        = 0;
      _splitScreen          = false;
      _horizontalSplit      = true;
      chordStyleEditor      = 0;
      albumManager          = 0;

      loadScoreDialog       = 0;
      saveScoreDialog       = 0;
      loadStyleDialog       = 0;
      saveStyleDialog       = 0;
      saveImageDialog       = 0;
      loadSoundFontDialog   = 0;
      loadBackgroundDialog  = 0;
      loadScanDialog        = 0;
      loadAudioDialog       = 0;
      loadChordStyleDialog  = 0;
      saveChordStyleDialog  = 0;
      loadDrumsetDialog     = 0;
      loadPaletteDialog     = 0;
      savePaletteDialog     = 0;
      saveDrumsetDialog     = 0;

      editRasterDialog      = 0;
      inChordEditor         = false;
      networkManager        = 0;

      profiles              = 0;

      _midiRecordId         = -1;
      _fullscreen           = false;
      lastCmd               = 0;
      lastShortcut          = 0;
      editTempo             = 0;

      if (!preferences.styleName.isEmpty()) {
            QFile f(preferences.styleName);
            if (f.open(QIODevice::ReadOnly)) {
                  MScore::defaultStyle()->load(&f);
                  f.close();
                  }
            }

      _positionLabel = new QLabel;
      _positionLabel->setObjectName("decoration widget");  // this prevents animations

      _positionLabel->setText("001:01:000");

      _modeText = new QLabel;
      _modeText->setAutoFillBackground(true);
      _statusBar = new QStatusBar;

      hRasterAction = getAction("hraster");
      hRasterAction->setCheckable(true);
      vRasterAction = getAction("vraster");
      vRasterAction->setCheckable(true);

      metronomeAction = getAction("metronome");
      metronomeAction->setCheckable(true);
      metronomeAction->setChecked(false);

      _statusBar->addPermanentWidget(new QWidget(this), 2);
      _statusBar->addPermanentWidget(new QWidget(this), 100);
      _statusBar->addPermanentWidget(_modeText, 0);
      layerSwitch = new QComboBox(this);
      layerSwitch->setToolTip(tr("switch layer"));
      connect(layerSwitch, SIGNAL(activated(const QString&)), SLOT(switchLayer(const QString&)));

      playMode = new QComboBox(this);
      playMode->addItem(tr("synthesizer"));
      playMode->addItem(tr("audio track"));
      playMode->setToolTip(tr("switch play mode"));
      connect(playMode, SIGNAL(activated(int)), SLOT(switchPlayMode(int)));

      _statusBar->addPermanentWidget(playMode);
      _statusBar->addPermanentWidget(layerSwitch);
      _statusBar->addPermanentWidget(_positionLabel, 0);

      setStatusBar(_statusBar);

      _progressBar = 0;

      // otherwise unused actions:
      //   must be added somewere to work

      QActionGroup* ag = new QActionGroup(this);
      ag->setExclusive(false);
      foreach(const Shortcut* s, Shortcut::shortcuts()) {
            QAction* a = s->action();
            ag->addAction(a);
            }
      addActions(ag->actions());
      connect(ag, SIGNAL(triggered(QAction*)), SLOT(cmd(QAction*)));

      mainWindow = new QSplitter;
      mainWindow->setOrientation(Qt::Vertical);
      QLayout* mlayout = new QVBoxLayout;
      mlayout->setMargin(0);
      mlayout->setSpacing(0);
      mainWindow->setLayout(mlayout);

      QWidget* mainScore = new QWidget;
      mainScore->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      mainWindow->addWidget(mainScore);

      layout = new QVBoxLayout;
      layout->setMargin(0);
      layout->setSpacing(0);
      mainScore->setLayout(layout);

      _navigator = new NScrollArea;
      mainWindow->addWidget(_navigator);
      showNavigator(preferences.showNavigator);

      QList<int> sizes;
      sizes << 500 << 500;
      mainWindow->setSizes(sizes);

      splitter = new QSplitter;

      tab1 = new ScoreTab(&scoreList);
      tab1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      connect(tab1, SIGNAL(currentScoreViewChanged(ScoreView*)), SLOT(setCurrentScoreView(ScoreView*)));
      connect(tab1, SIGNAL(tabCloseRequested(int)), SLOT(removeTab(int)));
      splitter->addWidget(tab1);

      if (splitScreen()) {
            tab2 = new ScoreTab(&scoreList);
            tab2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            connect(tab2, SIGNAL(currentScoreViewChanged(ScoreView*)), SLOT(setCurrentScoreView(ScoreView*)));
            connect(tab2, SIGNAL(tabCloseRequested(int)), SLOT(removeTab(int)));
            splitter->addWidget(tab2);
            tab2->setVisible(false);
            }
      else
            tab2 = 0;
      layout->addWidget(splitter);

      searchDialog = 0;

      //---------------------------------------------------
      //    Transport Action
      //---------------------------------------------------

      QAction* a;
#ifdef HAS_MIDI
      a  = getAction("midi-on");
      a->setCheckable(true);
      a->setEnabled(preferences.enableMidiInput);
      a->setChecked(_midiinEnabled);
#endif
      a = getAction("sound-on");
      a->setCheckable(true);
      a->setEnabled(preferences.playNotes);
      a->setChecked(_speakerEnabled);

      getAction("play")->setCheckable(true);
      a = getAction("repeat");
      a->setCheckable(true);
      a->setChecked(true);

      panAction = getAction("pan");
      panAction->setCheckable(true);
      panAction->setChecked(true);

      //---------------------------------------------------
      //    File Action
      //---------------------------------------------------

      //---------------------
      //    Tool Bar
      //---------------------

      fileTools = addToolBar(tr("File Operations"));
      fileTools->setObjectName("file-operations");
      fileTools->addAction(getAction("file-new"));
      fileTools->addAction(getAction("file-open"));
      fileTools->addAction(getAction("file-save"));
      fileTools->addAction(getAction("print"));
      fileTools->addAction(getAction("musescore-connect"));
      fileTools->addSeparator();

      a = getAction("undo");
      a->setEnabled(false);
      fileTools->addAction(a);

      a = getAction("redo");
      a->setEnabled(false);
      fileTools->addAction(a);

      fileTools->addSeparator();

      transportTools = addToolBar(tr("Transport Tools"));
      transportTools->setObjectName("transport-tools");
      transportTools->addAction(getAction("sound-on"));
#ifdef HAS_MIDI
      transportTools->addAction(getAction("midi-on"));
#endif
      transportTools->addSeparator();
      transportTools->addAction(getAction("rewind"));
      transportTools->addAction(getAction("play"));
      transportTools->addSeparator();
      a = getAction("repeat");
      a->setChecked(MScore::playRepeats);
      transportTools->addAction(a);
      a = getAction("pan");
      a->setChecked(MScore::panPlayback);
      transportTools->addAction(a);

      transportTools->addAction(metronomeAction);

      mag = new MagBox;
      connect(mag, SIGNAL(magChanged(int)), SLOT(magChanged(int)));
      fileTools->addWidget(mag);
      viewModeCombo = new QComboBox(this);
      viewModeCombo->addItem(tr("Page View"));
      viewModeCombo->addItem(tr("Continuous View"));
      connect(viewModeCombo, SIGNAL(activated(int)), SLOT(switchLayoutMode(int)));
      fileTools->addWidget(viewModeCombo);
      addToolBarBreak();

      cpitchTools = addToolBar(tr("Concert Pitch"));
      cpitchTools->setObjectName("pitch-tools");
      cpitchTools->addAction(getAction("concert-pitch"));

      QToolBar* foto = addToolBar(tr("Foto Mode"));
      foto->setObjectName("foto-tools");
      a = getAction("fotomode");
      a->setCheckable(true);
      foto->addAction(a);

      //-------------------------------
      //    Note Entry Tool Bar
      //-------------------------------

      entryTools = addToolBar(tr("Note Entry"));
      entryTools->setObjectName("entry-tools");

      a = getAction("note-input");
      a->setCheckable(true);
      entryTools->addAction(a);

      QStringList sl1;
      sl1 << "repitch" << "pad-note-128" << "pad-note-64" << "pad-note-32" << "pad-note-16"
         << "pad-note-8"
         << "pad-note-4" << "pad-note-2" << "pad-note-1" << "note-breve" << "note-longa"
         << "pad-dot"
         << "pad-dotdot" << "tie" << "pad-rest";

      foreach(const QString& s, sl1) {
            QToolButton* nb = new QToolButton;
            QAction* a = getAction(qPrintable(s));
            if (s != "tie")
                  a->setCheckable(true);
            nb->setDefaultAction(a);
            entryTools->addWidget(nb);
            if (s == "tie" || s == "pad-rest")
                  entryTools->addSeparator();
            }
      QStringList sl2;
      sl2 << "sharp2" << "sharp" << "nat" << "flat"  <<"flat2";
      foreach(const QString& s, sl2) {
            QToolButton* nb = new QToolButton;
            QAction* a = getAction(qPrintable(s));
            nb->setDefaultAction(a);
            entryTools->addWidget(nb);
            }

      sl1.clear();
      sl1 << "appoggiatura" << "acciaccatura" << "grace4" <<"grace16" << "grace32"
          << "beam-start" << "beam-mid" << "no-beam" << "beam32" << "auto-beam"
          << "show-invisible" << "show-unprintable" << "show-frames" << "show-pageborders";

      foreach(const QString& s, sl1) {
            QAction* a = getAction(s.toLatin1().data());
            a->setCheckable(true);
            }

      a = getAction("flip");
      entryTools->addAction(a);
      entryTools->addSeparator();

      VoiceSelector* vw = new VoiceSelector;
      entryTools->addWidget(vw);
      connect(vw, SIGNAL(triggered(QAction*)), SLOT(cmd(QAction*)));

      //---------------------
      //    Menus
      //---------------------

      QMenuBar* mb = menuBar();

      //---------------------
      //    Menu File
      //---------------------

      _fileMenu = mb->addMenu(tr("&File"));
      _fileMenu->setObjectName("File");

      _fileMenu->addAction(getAction("file-new"));
      _fileMenu->addAction(getAction("file-open"));
      openRecent = _fileMenu->addMenu(*icons[fileOpen_ICON], tr("Open &Recent"));
      connect(openRecent, SIGNAL(aboutToShow()), SLOT(openRecentMenu()));
      connect(openRecent, SIGNAL(triggered(QAction*)), SLOT(selectScore(QAction*)));
      _fileMenu->addSeparator();
      _fileMenu->addAction(getAction("file-save"));
      _fileMenu->addAction(getAction("file-save-as"));
      _fileMenu->addAction(getAction("file-save-a-copy"));
      _fileMenu->addAction(getAction("file-save-selection"));
      _fileMenu->addAction(getAction("file-export"));
      _fileMenu->addAction(getAction("file-part-export"));
      _fileMenu->addSeparator();
      _fileMenu->addAction(getAction("file-reload"));
      _fileMenu->addSeparator();
      _fileMenu->addAction(getAction("file-close"));

      _fileMenu->addSeparator();
      _fileMenu->addAction(getAction("parts"));
      _fileMenu->addAction(getAction("album"));
      _fileMenu->addAction(getAction("layer"));

      _fileMenu->addSeparator();
      _fileMenu->addAction(getAction("edit-info"));
      _fileMenu->addAction(getAction("media"));

      _fileMenu->addSeparator();
      _fileMenu->addAction(getAction("print"));
      _fileMenu->addSeparator();
      _fileMenu->addAction(getAction("quit"));

      //---------------------
      //    Menu Edit
      //---------------------

      menuEdit = mb->addMenu(tr("&Edit"));
      menuEdit->setObjectName("Edit");
      menuEdit->addAction(getAction("undo"));
      menuEdit->addAction(getAction("redo"));

      menuEdit->addSeparator();

      menuEdit->addAction(getAction("cut"));
      menuEdit->addAction(getAction("copy"));
      a = getAction("paste");
      a->setEnabled(false);
      menuEdit->addAction(a);
      selectionChanged(SEL_NONE);
      menuEdit->addSeparator();
      menuEdit->addAction(getAction("select-all"));
      menuEdit->addAction(getAction("select-section"));
      menuEdit->addAction(getAction("find"));
      menuEdit->addSeparator();

      QMenu* menuMeasure = new QMenu(tr("&Measure"));
      menuMeasure->addAction(getAction("delete-measures"));
      menuMeasure->addAction(getAction("split-measure"));
      menuMeasure->addAction(getAction("join-measure"));
      menuEdit->addMenu(menuMeasure);

      menuEdit->addSeparator();

      QMenu* menuVoices = new QMenu(tr("&Voices"));
      menuVoices->addAction(getAction("voice-x12"));
      menuVoices->addAction(getAction("voice-x13"));
      menuVoices->addAction(getAction("voice-x14"));
      menuVoices->addAction(getAction("voice-x23"));
      menuVoices->addAction(getAction("voice-x24"));
      menuVoices->addAction(getAction("voice-x34"));
      menuEdit->addMenu(menuVoices);

      menuEdit->addSeparator();
      menuEdit->addAction(getAction("debugger"));
      menuEdit->addSeparator();

      menuProfiles = new QMenu(tr("Pr&ofiles"));
      connect(menuProfiles, SIGNAL(aboutToShow()), SLOT(showProfileMenu()));
      menuEdit->addMenu(menuProfiles);

      QAction* pref = menuEdit->addAction(tr("&Preferences..."), this, SLOT(startPreferenceDialog()));
      pref->setMenuRole(QAction::PreferencesRole);

      //---------------------
      //    Menu Create
      //---------------------

      QMenu* menuCreate = genCreateMenu(mb);
      mb->setObjectName("Create");
      mb->addMenu(menuCreate);

      //---------------------
      //    Menu Notes
      //---------------------

      menuNotes = mb->addMenu(qApp->translate("MenuNotes", "&Notes"));
      menuNotes->setObjectName("Notes");

      menuNotes->addAction(getAction("note-input"));
      menuNotes->addAction(getAction("pitch-spell"));
      menuNotes->addSeparator();

      QMenu* menuAddPitch = new QMenu(tr("Add N&ote"));
      for (int i = 0; i < 7; ++i) {
            char buffer[8];
            sprintf(buffer, "note-%c", "cdefgab"[i]);
            a = getAction(buffer);
            menuAddPitch->addAction(a);
            }
      menuNotes->addMenu(menuAddPitch);

      for (int i = 0; i < 7; ++i) {
            char buffer[8];
            sprintf(buffer, "chord-%c", "cdefgab"[i]);
            a = getAction(buffer);
            menuAddPitch->addAction(a);
            }

      QMenu* menuAddInterval = new QMenu(tr("Add &Interval"));
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
      menuNotes->addMenu(menuAddInterval);

      QMenu* menuNtole = new QMenu(tr("T&uplets"));
      menuNtole->addAction(getAction("duplet"));
      menuNtole->addAction(getAction("triplet"));
      menuNtole->addAction(getAction("quadruplet"));
      menuNtole->addAction(getAction("quintuplet"));
      menuNtole->addAction(getAction("sextuplet"));
      menuNtole->addAction(getAction("septuplet"));
      menuNtole->addAction(getAction("octuplet"));
      menuNtole->addAction(getAction("nonuplet"));
      menuNtole->addAction(getAction("tuplet-dialog"));
      menuNotes->addMenu(menuNtole);

      menuNotes->addSeparator();
      menuNotes->addAction(getAction("transpose"));
      a = getAction("concert-pitch");
      a->setCheckable(true);
      menuNotes->addAction(a);

      //---------------------
      //    Menu Layout
      //---------------------

      menuLayout = mb->addMenu(tr("&Layout"));
      menuLayout->setObjectName("Layout");

      menuLayout->addAction(getAction("page-settings"));

      menuLayout->addAction(getAction("reset"));
      menuLayout->addAction(getAction("stretch+"));
      menuLayout->addAction(getAction("stretch-"));

      menuLayout->addAction(getAction("reset-stretch"));
      menuLayout->addAction(getAction("reset-beammode"));
//      menuLayout->addAction(tr("Breaks && Spacer..."), this, SLOT(showLayoutBreakPalette()));

      //---------------------
      //    Menu Style
      //---------------------

      menuStyle = mb->addMenu(tr("&Style"));
      menuStyle->setObjectName("Style");
      menuStyle->addAction(getAction("edit-style"));
      menuStyle->addAction(getAction("edit-text-style"));
      menuStyle->addAction(getAction("edit-harmony"));
      menuStyle->addSeparator();
      menuStyle->addAction(getAction("load-style"));
      menuStyle->addAction(getAction("save-style"));
      menuStyle->addAction(getAction("save-default-style"));

      //---------------------
      //    Menu Display
      //---------------------

      menuDisplay = mb->addMenu(tr("&Display"));
      menuDisplay->setObjectName("Display");

      a = getAction("toggle-palette");
      a->setCheckable(true);
      menuDisplay->addAction(a);

      a = getAction("masterpalette");
      a->setCheckable(true);
      menuDisplay->addAction(a);

      a = getAction("inspector");
      a->setCheckable(true);
      menuDisplay->addAction(a);

#ifdef OMR
      a = getAction("omr");
      a->setCheckable(true);
      menuDisplay->addAction(a);
#endif

      playId = getAction("toggle-playpanel");
      playId->setCheckable(true);
      menuDisplay->addAction(playId);

      a = getAction("toggle-navigator");
      a->setCheckable(true);
      a->setChecked(preferences.showNavigator);
      menuDisplay->addAction(a);

      a = getAction("toggle-mixer");
      a->setCheckable(true);
      menuDisplay->addAction(a);

      a = getAction("synth-control");
      a->setCheckable(true);
      menuDisplay->addAction(a);

      a = getAction("toogle-piano");
      a->setCheckable(true);
      menuDisplay->addAction(a);

      a = getAction("musescore-connect");
      a->setCheckable(true);
      menuDisplay->addAction(a);

      menuDisplay->addSeparator();
      menuDisplay->addAction(getAction("zoomin"));
      menuDisplay->addAction(getAction("zoomout"));
      menuDisplay->addSeparator();

      a = getAction("toggle-transport");
      a->setCheckable(true);
      a->setChecked(transportTools->isVisible());
      menuDisplay->addAction(a);

      a = getAction("toggle-noteinput");
      a->setCheckable(true);
      a->setChecked(true);
      menuDisplay->addAction(a);

      a = getAction("toggle-statusbar");
      a->setCheckable(true);
      a->setChecked(true);
      menuDisplay->addAction(a);

      menuDisplay->addSeparator();
      a = getAction("split-h");
      a->setCheckable(true);
      a->setChecked(false);
      menuDisplay->addAction(a);
      a = getAction("split-v");
      a->setCheckable(true);
      a->setChecked(false);
      menuDisplay->addAction(a);

      menuDisplay->addSeparator();
      menuDisplay->addAction(getAction("show-invisible"));
      menuDisplay->addAction(getAction("show-unprintable"));
      menuDisplay->addAction(getAction("show-frames"));
      menuDisplay->addAction(getAction("show-pageborders"));
      menuDisplay->addSeparator();
      a = getAction("fullscreen");
      a->setCheckable(true);
      a->setChecked(false);
      menuDisplay->addAction(a);

      //---------------------
      //    Menu Help
      //---------------------

      mb->addSeparator();
      QMenu* menuHelp = mb->addMenu(tr("&Help"));
      menuHelp->setObjectName("Help");

      menuHelp->addAction(getAction("local-help"));
      menuHelp->addAction(tr("&Online Handbook"), this, SLOT(helpBrowser1()));

      menuHelp->addSeparator();

      QAction *aboutAction = new QAction(tr("&About"), 0);
      aboutAction->setMenuRole(QAction::AboutRole);
      connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
      menuHelp->addAction(aboutAction);

      menuHelp->addAction(tr("About &Qt"), this, SLOT(aboutQt()));

      QAction *aboutMusicXMLAction = new QAction(tr("About &MusicXML"), 0);
      aboutMusicXMLAction->setMenuRole(QAction::ApplicationSpecificRole);
      connect(aboutMusicXMLAction, SIGNAL(triggered()), this, SLOT(aboutMusicXML()));
      menuHelp->addAction(aboutMusicXMLAction);

#if defined(Q_WS_MAC) || defined(Q_WS_WIN)
      menuHelp->addAction(tr("Check for &Update"), this, SLOT(checkForUpdate()));
#endif
      menuHelp->addSeparator();

      a = getAction("script-debug");
      a->setCheckable(true);
      a->setChecked(scriptDebug);
      menuHelp->addAction(a);
      a->setEnabled(false);

      setCentralWidget(mainWindow);

      loadInstrumentTemplates(preferences.instrumentList);
      preferencesChanged();
      if (seq) {
            connect(seq, SIGNAL(started()), SLOT(seqStarted()));
            connect(seq, SIGNAL(stopped()), SLOT(seqStopped()));
            }
      loadScoreList();

      showPlayPanel(preferences.showPlayPanel);

      QClipboard* cb = QApplication::clipboard();
      connect(cb, SIGNAL(dataChanged()), SLOT(clipboardChanged()));
      connect(cb, SIGNAL(selectionChanged()), SLOT(clipboardChanged()));
      autoSaveTimer = new QTimer(this);
      autoSaveTimer->setSingleShot(true);
      connect(autoSaveTimer, SIGNAL(timeout()), this, SLOT(autoSaveTimerTimeout()));
      initOsc();
      startAutoSave();
      if (enableExperimental) {
            cornerLabel = new QLabel(this);
            cornerLabel->setScaledContents(true);
            cornerLabel->setPixmap(QPixmap(":/data/mscore.png"));
            cornerLabel->setGeometry(width() - 48, 0, 48, 48);
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
//   MuseScore
//---------------------------------------------------------

MuseScore::~MuseScore()
      {
      }

//---------------------------------------------------------
//   startAutoSave
//---------------------------------------------------------

void MuseScore::startAutoSave()
      {
      if (preferences.autoSave) {
            int t = preferences.autoSaveTime * 60 * 1000;
            autoSaveTimer->start(t);
            }
      else
            autoSaveTimer->stop();
      }

//---------------------------------------------------------
//   getLocaleISOCode
//---------------------------------------------------------

QString MuseScore::getLocaleISOCode()
      {
      QString lang;
      if (localeName.toLower() == "system")
            lang = QLocale::system().name();
      else
            lang = localeName;
      return lang;
      }

//---------------------------------------------------------
//   helpBrowser
//    show local help
//---------------------------------------------------------

void MuseScore::helpBrowser()
      {
      QString lang = getLocaleISOCode();

      if (MScore::debugMode)
            qDebug("open handbook for language <%s>", qPrintable(lang));

      QFileInfo mscoreHelp(mscoreGlobalShare + QString("man/MuseScore-") + lang + QString(".pdf"));
      if (!mscoreHelp.isReadable()) {
            if (MScore::debugMode) {
                  qDebug("cannot open doc <%s>", qPrintable(mscoreHelp.filePath()));
                  }
            lang = lang.left(2);
            mscoreHelp.setFile(mscoreGlobalShare + QString("man/MuseScore-") + lang + QString(".pdf"));
            if(!mscoreHelp.isReadable()){
                mscoreHelp.setFile(mscoreGlobalShare + QString("man/MuseScore-en.pdf"));
                if (!mscoreHelp.isReadable()) {
                      QString info(tr("MuseScore handbook not found at: \n"));
                      info += mscoreHelp.filePath();
                      info += tr("\n\nFrom the \"Help\" menu try choosing \"Online Handbook\" instead.");
                      QMessageBox::critical(this, tr("MuseScore: Open Help"), info);
                      return;
                      }
                }
            }
      QString p = mscoreHelp.filePath();
#ifndef __MINGW32__
      p = p.replace(" ", "%20");    // HACK: why does'nt fromLocalFile() do this?
#endif
      QUrl url(QUrl::fromLocalFile(p));
      QDesktopServices::openUrl(url);
      }

//---------------------------------------------------------
//   helpBrowser1
//    show online help
//---------------------------------------------------------

void MuseScore::helpBrowser1()
      {
      QString lang = getLocaleISOCode();

      if (MScore::debugMode)
            qDebug("open online handbook for language <%s>", qPrintable(lang));
      QString help("http://www.musescore.org/en/handbook");
      //try to find an exact match
      bool found = false;
      foreach (LanguageItem item, _languages) {
            if (item.key == lang){
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
      help += QString("?utm_source=software&utm_medium=menu&utm_content=r%1&utm_campaign=MuseScore%2").arg(rev.trimmed()).arg(QString(VERSION));
      QUrl url(help);
      QDesktopServices::openUrl(url);
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
            Score* score = new Score(MScore::defaultStyle());
            if (!readScore(score, a)) {
                  readScoreError(a);
                  delete score;
                  }
            else
                  setCurrentScoreView(appendScore(score));
            }
      }

//---------------------------------------------------------
//   selectionChanged
//---------------------------------------------------------

void MuseScore::selectionChanged(int state)
      {
      bool enable = state != SEL_NONE;
      getAction("cut")->setEnabled(enable);
      getAction("copy")->setEnabled(enable);
      if (pianorollEditor)
            pianorollEditor->changeSelection(state);
      if (drumrollEditor)
            drumrollEditor->changeSelection(state);
      if (inspector) {
            if (cs) {
                  if (cs->selection().isSingle())
                        inspector->setElement(cs->selection().element());
                  else if (cs->selection().state() == SEL_NONE)
                        inspector->setElement(0);
                  else
                        inspector->setElementList(cs->selection().elements());
                  }
            else
                  inspector->setElement(0);
            }
      }

//---------------------------------------------------------
//   appendScore
//    append score to project list
//---------------------------------------------------------

int MuseScore::appendScore(Score* score)
      {
      int index = scoreList.size();
      for (int i = 0; i < scoreList.size(); ++i) {
            if (scoreList[i]->filePath() == score->filePath()) {
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
//   updateRecentScores
//---------------------------------------------------------

void MuseScore::updateRecentScores(Score* score)
      {
      QString path = score->fileInfo()->absoluteFilePath();
      recentScores.removeAll(path);
      recentScores.prepend(path);
      }

//---------------------------------------------------------
//   updateTabNames
//---------------------------------------------------------

void MuseScore::updateTabNames()
      {
      for (int i = 0; i < tab1->count(); ++i) {
            ScoreView* view = tab1->view(i);
            if (view)
                  tab1->setTabText(i, view->score()->name());
            }
      if (tab2) {
            for (int i = 0; i < tab2->count(); ++i) {
                  ScoreView* view = tab2->view(i);
                  if (view)
                        tab2->setTabText(i, view->score()->name());
                  }
            }
      }

//---------------------------------------------------------
//   usage
//---------------------------------------------------------

static void usage()
      {
      printVersion("MuseScore");
      fprintf(stderr, "usage: mscore flags scorefile\n   Flags:\n");
      fprintf(stderr,
        "   -v        print version\n"
        "   -d        debug mode\n"
        "   -L        layout debug\n"
        "   -D        enable plugin script debugger\n"
        "   -s        no internal synthesizer\n"
        "   -m        no midi\n"
        "   -n        start with new score\n"
        "   -I        dump midi input\n"
        "   -O        dump midi output\n"
        "   -o file   export to 'file'; format depends on file extension\n"
        "   -r dpi    set output resolution for image export\n"
        "   -S style  load style file\n"
        "   -p name   execute named plugin\n"
        "   -F        use factory settings\n"
        "   -i        load icons from INSTALLPATH/icons\n"
        "   -e        enable experimental features\n"
        "   -c dir    override config/settings directory\n"
        );
      exit(-1);
      }

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
                  recentScores.removeAll(path);
                  recentScores.prepend(path);
                  }
            }
      }

//---------------------------------------------------------
//   openRecentMenu
//---------------------------------------------------------

void MuseScore::openRecentMenu()
      {
      openRecent->clear();
      foreach(QString s, recentScores) {
            if (s.isEmpty())
                  break;
            // QFileInfo fi(s);
            // QAction* action = openRecent->addAction(fi.completeBaseName());
            QAction* action = openRecent->addAction(s);  // show complete path
            action->setData(s);
            }
      }

//---------------------------------------------------------
//   setCurrentView
//---------------------------------------------------------

void MuseScore::setCurrentScoreView(int idx)
      {
      setCurrentView(0, idx);
      }

void MuseScore::setCurrentView(int tabIdx, int idx)
      {
      if (idx == -1)
            setCurrentScoreView((ScoreView*)0);
      else {
            ScoreTab* tab = tabIdx ? tab2 : tab1;
            if (tab) {
                  tab->setCurrentIndex(idx);
                  }
            }
      }

//---------------------------------------------------------
//   setCurrentScoreView
//---------------------------------------------------------

void MuseScore::setCurrentScoreView(ScoreView* view)
      {
      //
      // save current synthesizer setting to score
      //
      if (cs)
            cs->setSyntiState(synti->state());

      cv = view;
      if (cv) {
            if (cv->score() && (cs != cv->score()))
                  updateInputState(cv->score());
            cs = cv->score();
            view->setFocusRect();
            cs->end();  // do layout if necessary
            }
      else
            cs = 0;
      updateLayer();
      updatePlayMode();
      if (seq)
            seq->setScoreView(cv);
      if (playPanel)
            playPanel->setScore(cs);
      if (synthControl)
            synthControl->setScore(cs);
      if (iledit)
            iledit->updateAll(cs);
#ifdef OMR
      if (omrPanel) {
            if (cv && cv->omrView())
                  omrPanel->setOmrView(cv->omrView());
            else
                  omrPanel->setOmrView(0);
            }
#endif
      if (!cs) {
            changeState(STATE_DISABLED);
            setWindowTitle("MuseScore");
            if (_navigator && _navigator->widget())
                  static_cast<Navigator*>(_navigator->widget())->setScore(0);
            if (inspector)
                  inspector->setElement(0);
            viewModeCombo->setEnabled(false);
            return;
            }
      viewModeCombo->setEnabled(true);
      if (cs->layoutMode() == LayoutPage)
            viewModeCombo->setCurrentIndex(0);
      else
            viewModeCombo->setCurrentIndex(1);

      selectionChanged(cs->selection().state());
      changeState(view->mscoreState());

      view->setFocus(Qt::OtherFocusReason);

      getAction("file-save")->setEnabled(cs->isSavable());
      getAction("show-invisible")->setChecked(cs->showInvisible());
      getAction("show-unprintable")->setChecked(cs->showUnprintable());
      getAction("show-frames")->setChecked(cs->showFrames());
      getAction("show-pageborders")->setChecked(cs->showPageborders());
      updateUndoRedo();

      if (view->magIdx() == MAG_FREE)
            mag->setMag(view->mag());
      else
            mag->setMagIdx(view->magIdx());

      setWindowTitle("MuseScore: " + cs->name());

      QAction* a = getAction("concert-pitch");
      a->setChecked(cs->styleB(ST_concertPitch));

      setPos(cs->inputPos());
      _statusBar->showMessage(cs->filePath(), 2000);
      if (_navigator && _navigator->widget())
            static_cast<Navigator*>(_navigator->widget())->setScoreView(view);
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
                        qDebug("drag Url: %s", qPrintable(u.toString()));
                  if (u.scheme() == "file") {
                        QFileInfo fi(u.toLocalFile());
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
                        Score* score = new Score(MScore::defaultStyle());
                        if (readScore(score, u.toLocalFile()))
                              view = appendScore(score);
                        else
                              delete score;
                        }
                  }
            if(view != -1)
                  setCurrentScoreView(view);
            else {
                  QMessageBox::critical(0,
                        tr("MuseScore: Load error"),
                        tr("Open failed: unknown file extension or broken file"));
                  }

            event->acceptProposedAction();
            }
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
//   showPlayPanel
//---------------------------------------------------------

void MuseScore::showPlayPanel(bool visible)
      {
      if (noSeq)
            return;
      if (playPanel == 0) {
            if (!visible)
                  return;
            playPanel = new PlayPanel(this);
            connect(playPanel, SIGNAL(gainChange(float)),    seq, SLOT(setGain(float)));
            connect(playPanel, SIGNAL(relTempoChanged(double)),seq, SLOT(setRelTempo(double)));
            connect(playPanel, SIGNAL(posChange(int)),         seq, SLOT(seek(int)));
            connect(playPanel, SIGNAL(closed()),                 SLOT(closePlayPanel()));
            connect(seq,       SIGNAL(gainChanged(float)), playPanel, SLOT(setGain(float)));

            playPanel->setGain(seq->gain());
            playPanel->setScore(cs);
            playPanel->move(preferences.playPanelPos);
            }
      playPanel->setVisible(visible);
      playId->setChecked(visible);
      }

//---------------------------------------------------------
//   closePlayPanel
//---------------------------------------------------------

void MuseScore::closePlayPanel()
      {
      playId->setChecked(false);
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
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void MeasuresDialog::accept()
	{
	int n = measures->value();
      if (mscore->currentScore())
            mscore->currentScoreView()->cmdAppendMeasures(n, MEASURE);
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
      return preferences.enableMidiInput && _midiinEnabled;
      }

//---------------------------------------------------------
//   processMidiRemote
//    return if midi remote command detected
//---------------------------------------------------------

bool MuseScore::processMidiRemote(MidiRemoteType type, int data)
      {
      if (!preferences.useMidiRemote)
            return false;
      for (int i = 0; i < MIDI_REMOTES; ++i) {
            if (preferences.midiRemote[i].type == type && preferences.midiRemote[i].data == data) {
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
                        case RMIDI_DOT:     a = getAction("dot");  break;
                        case RMIDI_DOTDOT:  a = getAction("dotdot");  break;
                        case RMIDI_TIE:     a = getAction("tie");  break;
                        case RMIDI_NOTE_EDIT_MODE: a = getAction("note-input");  break;
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
      static const int THRESHOLD = 3; // iterations required before consecutive drum notes
                                     // are not considered part of a chord
      static int active = 0;
      static int iter = 0;

      if (!midiinEnabled())
            return;

// qDebug("midiNoteReceived %d %d %d", channel, pitch, velo);

      if (_midiRecordId != -1) {
            preferences.midiRemote[_midiRecordId].type = MIDI_REMOTE_TYPE_NOTEON;
            preferences.midiRemote[_midiRecordId].data = pitch;
            _midiRecordId = -1;
            if (preferenceDialog)
                  preferenceDialog->updateRemote();
            return;
            }
      if (processMidiRemote(MIDI_REMOTE_TYPE_NOTEON, pitch)) {
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
                  if (iter >= THRESHOLD)
                        active = 0;
                  iter = 0;
                  }
// qDebug("    midiNoteReceived %d active %d", pitch, active);
            cv->midiNoteReceived(pitch, active);
            ++active;
            }
      else {
            if (channel != 0x09)
                  --active;
            }
      }

//---------------------------------------------------------
//   midiCtrlReceived
//---------------------------------------------------------

void MuseScore::midiCtrlReceived(int controller, int /*value*/)
      {
      if (!midiinEnabled())
            return;
      if (_midiRecordId != -1) {
            preferences.midiRemote[_midiRecordId].type = MIDI_REMOTE_TYPE_CTRL;
            preferences.midiRemote[_midiRecordId].data = controller;
            _midiRecordId = -1;
            if (preferenceDialog)
                  preferenceDialog->updateRemote();
            return;
            }
      if (processMidiRemote(MIDI_REMOTE_TYPE_CTRL, controller))
            return;
      }

//---------------------------------------------------------
//   speakerToggled
//---------------------------------------------------------

void MuseScore::speakerToggled(bool val)
      {
      _speakerEnabled = val;
      }

//---------------------------------------------------------
//   playEnabled
//---------------------------------------------------------

bool MuseScore::playEnabled() const
      {
      return preferences.playNotes && _speakerEnabled;
      }

//---------------------------------------------------------
//   removeTab
//---------------------------------------------------------

void MuseScore::removeTab()
      {
      int n = scoreList.indexOf(cs);
      if (n == -1) {
            qDebug("removeTab: %p not found", cs);
            return;
            }
      removeTab(n);
      }

void MuseScore::removeTab(int i)
      {
      Score* score = scoreList.value(i);
      if (score == 0)
            return;

      if (checkDirty(score))
            return;
      if (seq->score() == score)
            seq->setScoreView(0);

      int idx1      = tab1->currentIndex();
      bool firstTab = tab1->view(idx1) == cv;

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
      if (n == 0) {
            setCurrentScoreView((ScoreView*)0);
            }
      else {
            setCurrentScoreView((firstTab ? tab1 : tab2)->view());
            }
      writeSessionFile(false);
      if (!score->tmpName().isEmpty()) {
            QFile f(score->tmpName());
            f.remove();
            }
      delete score;
      }

//---------------------------------------------------------
//   setLocale
//---------------------------------------------------------

void setMscoreLocale(QString localeName)
      {
      static QList<QTranslator*> translatorList;

      foreach(QTranslator* t, translatorList) {
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
            }

      QTranslator* translator = new QTranslator;
      QString lp = mscoreGlobalShare + "locale/" + QString("mscore_") + localeName;
      if (MScore::debugMode)
            qDebug("load translator <%s>", qPrintable(lp));

      if (!translator->load(lp) && MScore::debugMode)
            qDebug("load translator <%s> failed", qPrintable(lp));
      else {
            qApp->installTranslator(translator);
            translatorList.append(translator);
            }

      QString resourceDir;
#if defined(Q_WS_MAC) || defined(Q_WS_WIN)
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
                  switch (preferences.sessionStart) {
                        case LAST_SESSION:
                              {
                              QSettings settings;
                              int n = settings.value("scores", 0).toInt();
                              int c = settings.value("currentScore", 0).toInt();
                              for (int i = 0; i < n; ++i) {
                                    QString s = settings.value(QString("score-%1").arg(i),"").toString();
                                    Score* score = new Score(MScore::defaultStyle());
                                    if (mscore->readScore(score, s)) {
                                          int view = mscore->appendScore(score);
                                          if (i == c)
                                                currentScoreView = view;
                                          }
                                    else
                                          delete score;
                                    }
                              }
                              break;
                        case EMPTY_SESSION:
                              break;
                        case NEW_SESSION:
                              mscore->newFile();
                              break;
                        case SCORE_SESSION:
                              {
                              Score* score = new Score(MScore::defaultStyle());
                              if (mscore->readScore(score, preferences.startScore))
                                    currentScoreView = mscore->appendScore(score);
                              else {
                                    if (mscore->readScore(score, ":/data/Promenade_Example.mscx")) {
                                          preferences.startScore = ":/data/Promenade_Example.mscx";
                                          currentScoreView = mscore->appendScore(score);
                                          }
                                    else
                                          delete score;
                                    }
                              }
                              break;
                        }
                  }
            }
      else {
            foreach(const QString& name, argv) {
                  if (name.isEmpty())
                        continue;
                  Score* score = new Score(MScore::defaultStyle());
                  if (!mscore->readScore(score, name)) {
                        mscore->readScoreError(name);
                        QMessageBox::warning(0,
                              QWidget::tr("MuseScore"),
                              QWidget::tr("reading file <")
                                 + name + QWidget::tr("> failed: ") +
                              QString(strerror(errno)),
                              QString::null, QWidget::tr("&Quit"), QString::null, 0, 1);
                        delete score;
                        }
                  else {
                        mscore->appendScore(score);
                        }
                  }
            }

      if (mscore->noScore())
            currentScoreView = -1;
      mscore->setCurrentView(0, currentScoreView);
      mscore->setCurrentView(1, currentScoreView);
      }

//---------------------------------------------------------
//   processNonGui
//---------------------------------------------------------

static bool processNonGui()
      {
      if (pluginMode) {
            QString pn(pluginName);
            bool res = false;
            if (mscore->loadPlugin(pn)){
                  Score* cs = mscore->currentScore();
                  if (!styleFile.isEmpty()) {
                        QFile f(styleFile);
                        if (f.open(QIODevice::ReadOnly))
                              cs->style()->load(&f);
                        }
                  cs->startCmd();
                  cs->setLayoutAll(true);
                  cs->endCmd();
                  mscore->pluginTriggered(0);
                  res = true;
                  }
            if (!converterMode)
                  return res;
            }

      if (converterMode) {
            QString fn(outFileName);
            Score* cs = mscore->currentScore();
            if (!styleFile.isEmpty()) {
                  QFile f(styleFile);
                  if (f.open(QIODevice::ReadOnly)) {
                        cs->style()->load(&f);
                        }
                  }
            if (fn.endsWith(".mscx")) {
                  QFileInfo fi(fn);
                  try {
                        cs->saveFile(fi);
                        }
                  catch(QString) {
                        return false;
                        }
                  return true;
                  }
            if (fn.endsWith(".mscz")) {
                  QFileInfo fi(fn);
                  try {
                        cs->saveCompressedFile(fi, false);
                        }
                  catch(QString) {
                        return false;
                        }
                  return true;
                  }
            if (fn.endsWith(".xml"))
                  return mscore->saveXml(cs, fn);
            if (fn.endsWith(".mxl"))
                  return mscore->saveMxl(cs, fn);
            if (fn.endsWith(".mid"))
                  return saveMidi(cs, fn);
            if (fn.endsWith(".pdf"))
                  return mscore->savePsPdf(fn, QPrinter::PdfFormat);
            if (fn.endsWith(".ps"))
                  return mscore->savePsPdf(fn, QPrinter::PostScriptFormat);
            if (fn.endsWith(".png"))
                  return mscore->savePng(cs, fn);
            if (fn.endsWith(".svg"))
                  return mscore->saveSvg(cs, fn);
            if (fn.endsWith(".ly"))
                  return mscore->saveLilypond(cs, fn);
#ifdef HAS_AUDIOFILE
            if (fn.endsWith(".wav"))
                  return mscore->saveAudio(cs, fn, "wav");
            if (fn.endsWith(".ogg"))
                  return mscore->saveAudio(cs, fn, "ogg");
            if (fn.endsWith(".flac"))
                  return mscore->saveAudio(cs, fn, "flac");
#endif
            if (fn.endsWith(".mp3"))
                  return mscore->saveMp3(cs, fn);
            else {
                  qDebug("dont know how to convert to %s", qPrintable(outFileName));
                  return false;
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   StartDialog
//---------------------------------------------------------

StartDialog::StartDialog(QWidget* parent)
  : QDialog(parent)
      {
	setupUi(this);
      setWindowTitle(tr("MuseScore Startup Dialog"));
      connect(createScore, SIGNAL(clicked()), SLOT(createScoreClicked()));
      connect(loadScore, SIGNAL(clicked()), SLOT(loadScoreClicked()));
      }

//---------------------------------------------------------
//   createScoreClicked
//---------------------------------------------------------

void StartDialog::createScoreClicked()
      {
      done(0);
      }

//---------------------------------------------------------
//   loadScoreClicked
//---------------------------------------------------------

void StartDialog::loadScoreClicked()
      {
      done(1);
      }

//---------------------------------------------------------
//   Message handler
//---------------------------------------------------------

#if defined(QT_DEBUG) && defined(Q_WS_WIN)
static void mscoreMessageHandler(QtMsgType type, const char *msg)
     {
     QTextStream cout(stdout);

     switch (type) {
     case QtDebugMsg:
         cout << "Debug: " << msg << endl;
         break;
     case QtWarningMsg:
         cout << "Warning: " << msg << endl;
         break;
     case QtCriticalMsg:
         cout << "Critical: " << msg << endl;
         break;
     case QtFatalMsg:
         //
         // set your breakpoint here, if you want to catch the abort
         //
         cout << "Fatal: " << msg << endl;
         abort();
         }
     }
#endif

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* av[])
      {
#if defined(QT_DEBUG) && defined(Q_WS_WIN)
      qInstallMsgHandler(mscoreMessageHandler);
#endif

      QFile f(":/revision.h");
      f.open(QIODevice::ReadOnly);
      revision = QString(f.readAll());
      f.close();

#ifdef Q_WS_MAC
      MuseScoreApplication* app = new MuseScoreApplication("mscore-dev", argc, av);
#else
      QtSingleApplication* app = new QtSingleApplication("mscore-dev", argc, av);
#endif

      QCoreApplication::setOrganizationName("MuseScore");
      QCoreApplication::setOrganizationDomain("musescore.org");
      QCoreApplication::setApplicationName("MuseScoreDevelopment");

#ifndef Q_WS_MAC
      // Save the preferences in QSettings::NativeFormat
      QSettings::setDefaultFormat(QSettings::IniFormat);
#endif

      if (!QFontDatabase::supportsThreadedFontRendering()) {
            qDebug("Your computer does not support threaded font rendering!");
            exit(-1);
            }

      QStringList argv =  QCoreApplication::arguments();
      argv.removeFirst();

      for (int i = 0; i < argv.size();) {
            QString s = argv[i];
            if (s[0] != '-') {
                  ++i;
                  continue;
                  }
            switch(s[1].toAscii()) {
                  case 'v':
                        printVersion("MuseScore");
                        return 0;
                   case 'd':
                        MScore::debugMode = true;
                        break;
                  case 'L':
                        MScore::layoutDebug = true;
                        break;
                  case 's':
                        noSeq = true;
                        break;
                  case 'm':
                        noMidi = true;
                        break;
                  case 'n':
                        startWithNewScore = true;
                        break;
                  case 'i':
                        externalIcons = true;
                        break;
                  case 'I':
                        midiInputTrace = true;
                        break;
                  case 'O':
                        midiOutputTrace = true;
                        break;
                  case 'o':
                        converterMode = true;
                        noGui = true;
                        if (argv.size() - i < 2)
                              usage();
                        outFileName = argv.takeAt(i + 1);
                        break;
                  case 'p':
                        pluginMode = true;
                        noGui = true;
                        if (argv.size() - i < 2)
                              usage();
                        pluginName = argv.takeAt(i + 1);
                        break;
                  case 'r':
                        if (argv.size() - i < 2)
                              usage();
                        converterDpi = argv.takeAt(i + 1).toDouble();
                        break;
                  case 'S':
                        if (argv.size() - i < 2)
                              usage();
                        styleFile = argv.takeAt(i + 1);
                        break;
                  case 'D':
                        scriptDebug = true;
                        break;
                  case 'F':
                        useFactorySettings = true;
                        break;
                  case 'e':
                        enableExperimental = true;
                        break;
                  case 'c':
                        {
                        if (argv.size() - i < 2)
                              usage();
                        QString path = argv.takeAt(i + 1);
                        QDir dir;
                        if (dir.exists(path)) {
                              QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, path);
                              QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, path);
                              dataPath = path;
                              }
                        }
                        break;
                  default:
                        usage();
                  }
            argv.removeAt(i);
            }
      mscoreGlobalShare = getSharePath();
      iconPath = externalIcons ? mscoreGlobalShare + QString("icons/") :  QString(":/data/");
      iconGroup = "icons-dark/";

      if (!converterMode) {
            if (!argv.isEmpty()) {
                  int ok = true;
                  foreach(QString message, argv) {
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
                  if (app->sendMessage(QString(""))) {
                      return 0;
                      }
            }

/**/
      if (dataPath.isEmpty())
            dataPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);

      // create local plugin directory
      // if not already there:
      QDir dir;
      dir.mkpath(dataPath + "/plugins");

      if (MScore::debugMode)
            qDebug("global share: <%s>", qPrintable(mscoreGlobalShare));

      // set translator before preferences are read to get
      //    translations for all shortcuts
      //
      if (useFactorySettings)
            localeName = "system";
      else {
            QSettings s;
            localeName = s.value("language", "system").toString();
            }

      setMscoreLocale(localeName);

      Shortcut::init();
      preferences.init();

      QWidget wi(0);
      MScore::PDPI = wi.logicalDpiX();         // physical resolution
      MScore::DPI  = MScore::PDPI;             // logical drawing resolution
      MScore::init();                          // initialize libmscore
      preferences.readDefaultStyle();

      if (!useFactorySettings)
            preferences.read();

      if (converterDpi == 0)
            converterDpi = preferences.pngResolution;

      QSplashScreen* sc = 0;
      if (!noGui && preferences.showSplashScreen) {
            QPixmap pm(":/data/splash.jpg");
            sc = new QSplashScreen(pm);
            sc->setWindowTitle(QString("MuseScore Startup"));
            sc->setWindowFlags(Qt::FramelessWindowHint);
            sc->show();
            qApp->processEvents();
            }

      if (!converterMode) {
            switch(preferences.globalStyle) {
                  case STYLE_DARK: {
                        QApplication::setStyle(new MgStyle);
                        qApp->setStyleSheet(appStyleSheet());
                        QPalette p(QApplication::palette());
                        p.setColor(QPalette::Window,        QColor(0x52, 0x52, 0x52));
                        p.setColor(QPalette::WindowText,    Qt::white);
                        p.setColor(QPalette::Base,          QColor(0x42, 0x42, 0x42));
                        p.setColor(QPalette::AlternateBase, QColor(0x62, 0x62, 0x62));
                        p.setColor(QPalette::Text,          Qt::white);
                        p.setColor(QPalette::Button,        QColor(0x52, 0x52, 0x52));
                        p.setColor(QPalette::ButtonText,    Qt::white);
                        p.setColor(QPalette::BrightText,    Qt::black);
                        QApplication::setPalette(p);
                        break;
                        }
                  case STYLE_LIGHT: {
                        QApplication::setStyle(new MgStyle);
                        qApp->setStyleSheet(appStyleSheet());
                        QPalette p(QApplication::palette());
                        p.setColor(QPalette::Window,        QColor(0xdc, 0xdc, 0xdc));
                        p.setColor(QPalette::WindowText,    Qt::black);
                        p.setColor(QPalette::Base,          QColor(0x82, 0x82, 0x82));
                        p.setColor(QPalette::AlternateBase, QColor(0xa2, 0xa2, 0xa2));
                        p.setColor(QPalette::Text,          Qt::black);
                        p.setColor(QPalette::Button,        QColor(0xdc, 0xdc, 0xdc));
                        p.setColor(QPalette::ButtonText,    Qt::black);
                        p.setColor(QPalette::BrightText,    Qt::black);  //??
                        QApplication::setPalette(p);
                        break;
                        }
                  case STYLE_NATIVE:
                        break;
                  }
            }

      synti = new MasterSynth();
      seq   = new Seq();
      MScore::seq = seq;

      //
      // avoid font problems by overriding the environment
      //    fall back to "C" locale
      //

#ifndef __MINGW32__
      setenv("LANG", "C", 1);
#endif
      QLocale::setDefault(QLocale(QLocale::C));

      if (MScore::debugMode) {
            QStringList sl(QCoreApplication::libraryPaths());
            foreach(const QString& s, sl)
                  qDebug("LibraryPath: <%s>", qPrintable(s));
            }

      // rastral size of font is 20pt = 20/72 inch = 20*DPI/72 dots
      //   staff has 5 lines = 4 * _spatium
      //   _spatium    = SPATIUM20  * DPI;     // 20.0 / 72.0 * DPI / 4.0;

      genIcons();

      if (!converterMode)
            qApp->setWindowIcon(*icons[window_ICON]);
      initProfile();
      mscore = new MuseScore();
      gscore = new Score(MScore::defaultStyle());

      if (!noSeq) {
            if (!seq->init()) {
                  qDebug("sequencer init failed");
                  noSeq = true;
                  }
            }

      //read languages list
      mscore->readLanguages(mscoreGlobalShare + "locale/languages.xml");

#ifdef Q_WS_MAC
      QApplication::instance()->installEventFilter(mscore);

      // Mac-Applications don't have menubar icons
      qt_mac_set_menubar_icons(false);
#endif
      mscore->setRevision(revision);

      int files = 0;
      if (noGui) {
            loadScores(argv);
            exit(processNonGui() ? 0 : -1);
            }
      else {
            mscore->readSettings();
            QObject::connect(qApp, SIGNAL(messageReceived(const QString&)),
               mscore, SLOT(handleMessage(const QString&)));

            mscore->showWebPanel(preferences.showWebPanel);
            static_cast<QtSingleApplication*>(qApp)->setActivationWindow(mscore, false);
            foreach(const QString& name, argv) {
                  if (!name.isEmpty())
                        ++files;
                  }
#ifdef Q_WS_MAC
            if (!mscore->restoreSession(preferences.sessionStart == LAST_SESSION))
                  loadScores(static_cast<MuseScoreApplication*>(qApp)->paths);
#else
            //
            // TODO: delete old session backups
            //
            if (!mscore->restoreSession((preferences.sessionStart == LAST_SESSION) && (files == 0)) || files)
                  loadScores(argv);
#endif
            }
      mscore->loadPlugins();
      mscore->writeSessionFile(false);

#ifdef Q_WS_MAC
      // there's a bug in Qt showing the toolbar unified after switching showFullScreen(), showMaximized(),
      // showNormal()...
      mscore->setUnifiedTitleAndToolBarOnMac(false);
#endif

      mscore->show();

      if (sc)
            sc->finish(mscore);
      if (mscore->hasToCheckForUpdate())
            mscore->checkForUpdate();

      if (preferences.sessionStart == EMPTY_SESSION && files == 0) {
            QDialog* start = new StartDialog(0);
            switch(start->exec()) {
                  case 0:
                        mscore->newFile();
                        break;
                  case 1:
                        mscore->loadFiles();
                        break;
                  }
            }
      return qApp->exec();
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

bool MuseScoreApplication::event(QEvent *event)
      {
      switch(event->type()) {
            case QEvent::FileOpen:
                  paths.append(static_cast<QFileOpenEvent *>(event)->file());
                  return true;
            default:
                  return QApplication::event(event);
            }
      }

//---------------------------------------------------------
//   eventFilter (mac only)
//---------------------------------------------------------

bool MuseScore::eventFilter(QObject *obj, QEvent *event)
      {
      switch(event->type()) {
            case QEvent::FileOpen:
                  handleMessage(static_cast<QFileOpenEvent *>(event)->file());
                  return true;
            default:
                  return QObject::eventFilter(obj, event);
            }
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
            ucheck->check(revision(), sender() != 0);
      }

//---------------------------------------------------------
//   readLanguages
//---------------------------------------------------------

bool MuseScore::readLanguages(const QString& path)
      {
      _languages.append(LanguageItem("system", tr("System")));
      QFile qf(path);
      if (qf.exists()){
          QDomDocument doc;
          int line, column;
          QString err;
          if (!doc.setContent(&qf, false, &err, &line, &column)) {
                QString error;
                error.sprintf("error reading language file  %s at line %d column %d: %s\n",
                   qPrintable(qf.fileName()), line, column, qPrintable(err));
                QMessageBox::warning(0,
                   QWidget::tr("MuseScore: Load languages failed:"),
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
      }

//---------------------------------------------------------
//   changeState
//---------------------------------------------------------

void MuseScore::changeState(ScoreState val)
      {
      if (MScore::debugMode)
            qDebug("MuseScore::changeState: %s", stateName(val));

      if (_sstate == val)
            return;
      foreach (const Shortcut* s, Shortcut::shortcuts()) {
            QAction* a = s->action();
            if (!a)
                  continue;
            if (strcmp(s->key(), "undo") == 0)
                  a->setEnabled((s->state() & val) && (cs ? cs->undo()->canUndo() : false));
            else if (strcmp(s->key(), "redo") == 0)
                  a->setEnabled((s->state() & val) && (cs ? cs->undo()->canRedo() : false));
            else if (strcmp(s->key(), "cut") == 0)
                  a->setEnabled(cs && cs->selection().state());
            else if (strcmp(s->key(), "copy") == 0)
                  a->setEnabled(cs && cs->selection().state());
            else if (strcmp(s->key(), "synth-control") == 0) {
                  Driver* driver = seq ? seq->getDriver() : 0;
                  // a->setEnabled(driver && driver->getSynth());
                  if (MScore::debugMode)
                        qDebug("disable synth control");
                  a->setEnabled(driver);
                  }
            else {
                  bool enable = s->state() & val;
                  a->setEnabled(enable);
                  }
            }
      if (val != STATE_SEARCH && searchDialog)
            searchDialog->hide();

      bool enable = val != STATE_DISABLED;

      // disabling top level menu entries does not
      // work for MAC

      QList<QObject*> ol = menuBar()->children();
      foreach(QObject* o, ol) {
            QMenu* menu = qobject_cast<QMenu*>(o);
            if (!menu)
                  continue;
            QString s(menu->objectName());
            if (s == "File" || s == "Help" || s == "Edit" || s == "Plugins")
                  continue;
            menu->setEnabled(enable);
            }

      menuProfiles->setEnabled(enable);
//      foreach (QAction* a, pluginActions)     allow for "create score" plugins
//            a->setEnabled(true);

      transportTools->setEnabled(enable && !noSeq);
      cpitchTools->setEnabled(enable);
      mag->setEnabled(enable);
      entryTools->setEnabled(enable);

      switch(val) {
            case STATE_DISABLED:
                  _modeText->setText(tr("no score"));
                  _modeText->show();
                  if (debugger)
                        debugger->hide();
                  showDrumTools(0, 0);
                  showPianoKeyboard(false);
                  break;
            case STATE_NORMAL:
                  _modeText->hide();
                  if (searchDialog)
                        searchDialog->hide();
                  break;
            case STATE_NOTE_ENTRY:
                  _modeText->setText(tr("note entry mode"));
                  _modeText->show();
                  break;
            case STATE_EDIT:
            case STATE_LYRICS_EDIT:
                  _modeText->setText(tr("edit mode"));
                  _modeText->show();
                  break;
            case STATE_PLAY:
                  _modeText->setText(tr("play"));
                  _modeText->show();
                  break;
            case STATE_FOTO:
                  _modeText->setText(tr("foto mode"));
                  _modeText->show();
                  break;
            case STATE_SEARCH:
                  if (searchDialog == 0) {
                        searchDialog = new QWidget;
                        searchDialog->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
                        QHBoxLayout* searchDialogLayout = new QHBoxLayout;
                        searchDialog->setLayout(searchDialogLayout);
                        layout->insertWidget(2, searchDialog);

                        QToolButton* searchExit = new QToolButton;
                        searchExit->setIcon(QIcon(":/data/cancel.png"));
                        connect(searchExit, SIGNAL(clicked()), SLOT(endSearch()));
                        searchDialogLayout->addWidget(searchExit);

                        searchDialogLayout->addWidget(new QLabel(tr("Go To: ")));

                        searchCombo = new QComboBox;
                        searchCombo->setEditable(true);
                        searchCombo->setInsertPolicy(QComboBox::InsertAtTop);
                        searchDialogLayout->addWidget(searchCombo);

                        searchDialogLayout->addStretch(10);
                        searchDialog->hide();

                        connect(searchCombo, SIGNAL(editTextChanged(const QString&)),
                           SLOT(searchTextChanged(const QString&)));
                        }

                  searchCombo->clearEditText();
                  searchCombo->setFocus();
                  searchDialog->show();
                  _modeText->setText(tr("Search"));
                  _modeText->show();
                  break;
            default:
                  qDebug("MuseScore::changeState: illegal state %d", val);
                  break;
            }
      if (paletteBox)
            paletteBox->setDisabled(val == STATE_PLAY || val == STATE_DISABLED);
      QAction* a = getAction("note-input");
      a->setChecked(val == STATE_NOTE_ENTRY);
      _sstate = val;
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void MuseScore::writeSettings()
      {
      QSettings settings;
      settings.beginGroup("MainWindow");
      settings.setValue("size", size());
      settings.setValue("pos", pos());
      settings.setValue("maximized", isMaximized());
      settings.setValue("showPanel", paletteBox && paletteBox->isVisible());
      settings.setValue("state", saveState());
      settings.setValue("splitScreen", _splitScreen);
      settings.setValue("debuggerSplitter", mainWindow->saveState());
//      if (_splitScreen) {
            settings.setValue("split", _horizontalSplit);
            settings.setValue("splitter", splitter->saveState());
//            }
      settings.endGroup();
      profile->save();
      if (keyEditor && keyEditor->dirty())
            keyEditor->save();
      if (chordStyleEditor)
            chordStyleEditor->save();
      if (loadScoreDialog)
            settings.setValue("loadScoreDialog", loadScoreDialog->saveState());
      if (saveScoreDialog)
            settings.setValue("saveScoreDialog", saveScoreDialog->saveState());
      if (loadStyleDialog)
            settings.setValue("loadStyleDialog", loadStyleDialog->saveState());
      if (saveStyleDialog)
            settings.setValue("saveStyleDialog", saveStyleDialog->saveState());
      if (saveImageDialog)
            settings.setValue("saveImageDialog", saveImageDialog->saveState());
      if (loadChordStyleDialog)
            settings.setValue("loadChordStyleDialog", loadChordStyleDialog->saveState());
      if (saveChordStyleDialog)
            settings.setValue("saveChordStyleDialog", saveChordStyleDialog->saveState());
      if (loadSoundFontDialog)
            settings.setValue("loadSoundFontDialog", loadSoundFontDialog->saveState());
      if (loadScanDialog)
            settings.setValue("loadScanDialog", loadScanDialog->saveState());
      if (loadAudioDialog)
            settings.setValue("loadAudioDialog", loadAudioDialog->saveState());
      if (loadDrumsetDialog)
            settings.setValue("loadDrumsetDialog", loadDrumsetDialog->saveState());
      if (loadPaletteDialog)
            settings.setValue("loadPaletteDialog", loadPaletteDialog->saveState());
      if (saveDrumsetDialog)
            settings.setValue("saveDrumsetDialog", saveDrumsetDialog->saveState());
      if (savePaletteDialog)
            settings.setValue("savePaletteDialog", savePaletteDialog->saveState());
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void MuseScore::readSettings()
      {
      if (useFactorySettings) {
            resize(QSize(1024, 768));
            QList<int> sizes;
            sizes << 500 << 100;
            mainWindow->setSizes(sizes);
            mscore->showPalette(true);
            return;
            }
      QSettings settings;

      settings.beginGroup("MainWindow");
      resize(settings.value("size", QSize(1024, 768)).toSize());
      mainWindow->restoreState(settings.value("debuggerSplitter").toByteArray());
      move(settings.value("pos", QPoint(10, 10)).toPoint());
      if (settings.value("maximized", false).toBool())
            showMaximized();
      mscore->showPalette(settings.value("showPanel", "1").toBool());

      restoreState(settings.value("state").toByteArray());
      _horizontalSplit = settings.value("split", true).toBool();
      bool splitScreen = settings.value("splitScreen", false).toBool();
      if (splitScreen) {
            splitWindow(_horizontalSplit);
            QAction* a = getAction(_horizontalSplit ? "split-h" : "split-v");
            a->setChecked(true);
            }
      splitter->restoreState(settings.value("splitter").toByteArray());
      settings.endGroup();

      QAction* a = getAction("toggle-transport");
      a->setChecked(!transportTools->isHidden());
      a = getAction("toggle-noteinput");
      a->setChecked(!entryTools->isHidden());
      }

//---------------------------------------------------------
//   play
//    play note for preferences.defaultPlayDuration
//---------------------------------------------------------

void MuseScore::play(Element* e) const
      {
      if (mscore->playEnabled()) {
            if (e->type() == NOTE) {
                  Note* note = static_cast<Note*>(e);
                  play(e, note->ppitch());
                  }
            else if (e->type() == CHORD) {
                  seq->stopNotes();
                  Chord* c = static_cast<Chord*>(e);
                  Part* part = c->staff()->part();
                  int tick = c->segment() ? c->segment()->tick() : 0;
                  Instrument* instr = part->instr(tick);
                  foreach(Note* n, c->notes()) {
                        const Channel& channel = instr->channel(n->subchannel());
                        seq->startNote(channel, n->ppitch(), 80, n->tuning());
                        }
                  seq->startNoteTimer(MScore::defaultPlayDuration);
                  }
            }
      }

void MuseScore::play(Element* e, int pitch) const
      {
      if (mscore->playEnabled() && e->type() == NOTE) {
            Note* note = static_cast<Note*>(e);
            Part* part = note->staff()->part();
            int tick = note->chord()->segment() ? note->chord()->segment()->tick() : 0;
            Instrument* instr = part->instr(tick);
            const Channel& channel = instr->channel(note->subchannel());
            seq->startNote(channel, pitch, 80, MScore::defaultPlayDuration, note->tuning());
            }
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
#ifdef MSCORE_UNSTABLE
      versionLabel->setText(tr("Unstable Prerelease for Version: ") + VERSION);
#else
      versionLabel->setText(tr("Version: ") + VERSION);
#endif
      revisionLabel->setText(tr("Revision: %1").arg(revision));
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      }

//---------------------------------------------------------
//   AboutBoxDialog
//---------------------------------------------------------

AboutMusicXMLBoxDialog::AboutMusicXMLBoxDialog()
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      }

//---------------------------------------------------------
//   dirtyChanged
//---------------------------------------------------------

void MuseScore::dirtyChanged(Score* s)
      {
      Score* score = s->rootScore();

      int idx = scoreList.indexOf(score);
      if (idx == -1) {
            qDebug("score not in list");
            return;
            }
      QString label(score->name());
      if (score->dirty())
            label += "*";
      tab1->setTabText(idx, label);
      if (tab2)
            tab2->setTabText(idx, label);
      }

//---------------------------------------------------------
//   magChanged
//---------------------------------------------------------

void MuseScore::magChanged(int idx)
      {
      if (cv)
            cv->setMag(idx, mag->getMag(cv));
      }

//---------------------------------------------------------
//   incMag
//---------------------------------------------------------

void MuseScore::incMag()
      {
      if (cv) {
            qreal _mag = cv->mag() * 1.7;
            if (_mag > 16.0)
                  _mag = 16.0;
            cv->setMag(MAG_FREE, _mag);
            setMag(_mag);
            }
      }

//---------------------------------------------------------
//   decMag
//---------------------------------------------------------

void MuseScore::decMag()
      {
      if (cv) {
            qreal _mag = cv->mag() / 1.7;
            if (_mag < 0.05)
                  _mag = 0.05;
            cv->setMag(MAG_FREE, _mag);
            setMag(_mag);
            }
      }

//---------------------------------------------------------
//   getMag
//---------------------------------------------------------

double MuseScore::getMag(ScoreView* canvas) const
      {
      return mag->getMag(canvas);
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void MuseScore::setMag(double d)
      {
      mag->setMag(d);
      mag->setMagIdx(MAG_FREE);
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
      _positionLabel->setText(tr("Bar %1 Beat %2.%3")
         .arg(bar + 1,  3, 10, QLatin1Char(' '))
         .arg(beat + 1, 2, 10, QLatin1Char(' '))
         .arg(tick,     3, 10, QLatin1Char('0'))
         );
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void MuseScore::undo()
      {
      if (_sstate == STATE_EDIT || _sstate == STATE_LYRICS_EDIT) {
            cv->postCmd("escape");
            qApp->processEvents();
            }
      if (cv)
            cv->startUndoRedo();
      if (cs)
            cs->undo()->undo();
      if (cv) {
            if (cs->inputState().segment())
                  setPos(cs->inputState().tick());
            if (cs->noteEntryMode() && !cv->noteEntryMode()) {
                  // enter note entry mode
                  cv->postCmd("note-input");
                  }
            else if (!cs->inputState().noteEntryMode && cv->noteEntryMode()) {
                  // leave note entry mode
                  cv->postCmd("escape");
                  }
            cs->endUndoRedo();
            updateInputState(cs);
            }
      endCmd();
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void MuseScore::redo()
      {
      if (_sstate == STATE_EDIT || _sstate == STATE_LYRICS_EDIT) {
            cv->postCmd("escape");
            qApp->processEvents();
            }
      if (cv)
            cv->startUndoRedo();
      if (cs)
            cs->undo()->redo();
      if (cv) {
            if (cs->inputState().segment())
                  setPos(cs->inputState().tick());
            if (cs->noteEntryMode() && !cv->noteEntryMode()) {
                  // enter note entry mode
                  cv->postCmd("note-input");
                  }
            else if (!cs->inputState().noteEntryMode && cv->noteEntryMode()) {
                  // leave note entry mode
                  cv->postCmd("escape");
                  }
            cs->endUndoRedo();
            updateInputState(cs);
            }
      endCmd();
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
//   searchTextChanged
//---------------------------------------------------------

void MuseScore::searchTextChanged(const QString& s)
      {
      if (cv == 0)
            return;
      cv->search(s);
      }

//---------------------------------------------------------
//   endSearch
//---------------------------------------------------------

void MuseScore::endSearch()
      {
      if (cv)
            cv->postCmd("escape");
      }

//---------------------------------------------------------
//   handleMessage
//---------------------------------------------------------

void MuseScore::handleMessage(const QString& message)
      {
      if (message.isEmpty())
            return;
      ((QtSingleApplication*)(qApp))->activateWindow();
      Score* score = new Score(MScore::defaultStyle());
      if (readScore(score, message)) {
            setCurrentScoreView(appendScore(score));
            lastOpenPath = score->fileInfo()->path();
            }
      else
            delete score;
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
      Xml xml(&f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      xml.tagE(cleanExit ? "clean" : "dirty");
      foreach(Score* score, scoreList) {
            xml.stag("Score");
            xml.tag("created", score->created());
            xml.tag("dirty", score->dirty());
// qDebug("  %d <%s>", score->dirty(), qPrintable(score->fileInfo()->absoluteFilePath()));
            if (score->tmpName().isEmpty()) {
                  xml.tag("path", score->fileInfo()->absoluteFilePath());
                  }
            else {
                  xml.tag("name", score->fileInfo()->absoluteFilePath());
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
                  if (v->magIdx() == MAG_FREE)
                        xml.tag("mag", v->mag());
                  else
                        xml.tag("magIdx", v->magIdx());
                  xml.tag("x",   v->xoffset() / MScore::DPMM);
                  xml.tag("y",   v->yoffset() / MScore::DPMM);
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
                        if (v->magIdx() == MAG_FREE)
                              xml.tag("mag", v->mag());
                        else
                              xml.tag("magIdx", v->magIdx());
                        xml.tag("x",   v->xoffset() / MScore::DPMM);
                        xml.tag("y",   v->yoffset() / MScore::DPMM);
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
      foreach(Score* s, scoreList) {
            if (s->autosaveDirty()) {
                  QString tmp = s->tmpName();
                  if (!tmp.isEmpty()) {
                        QFileInfo fi(tmp);
                        // TODO: cannot catch exeption here:
                        cs->saveCompressedFile(fi, false);
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
                        s->saveCompressedFile(&tf, info, false);
                        tf.close();
                        sessionChanged = true;
                        }
                  s->setAutosaveDirty(false);
                  }
            }
      if (sessionChanged)
            writeSessionFile(false);
      if (preferences.autoSave) {
            int t = preferences.autoSaveTime * 60 * 1000;
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
            qDebug("cannot open session file <%s>", qPrintable(f.fileName()));
            return false;
            }
      QDomDocument doc;
      int line, column;
      QString err;
      docName = f.fileName();
      if (!doc.setContent(&f, false, &err, &line, &column)) {
            QString error;
            error.sprintf("error reading session file %s at line %d column %d: %s\n",
               qPrintable(docName), line, column, qPrintable(err));
            return false;
            }
      int tab = 0;
      int idx = -1;
      bool cleanExit = false;
      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  /* QString version = e.attribute(QString("version"));
                  QStringList sl  = version.split('.');
                  int v           = sl[0].toInt() * 100 + sl[1].toInt();
                  */
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull();  ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        if (tag == "clean") {
                              if (!always) {
                                    f.close();
                                    return false;
                                    }
                              cleanExit = true;
                              }
                        else if (tag == "dirty") {
                              QMessageBox::StandardButton b = QMessageBox::question(0,
                                 tr("MuseScore"),
                                 tr("The previous session quit unexpectedly.\n\nRestore session?"),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::Yes
                                 );
                              if (b != QMessageBox::Yes) {
                                    f.close();
                                    return false;
                                    }
                              }
                        else if (tag == "Score") {
                              QString name;
                              bool created = false;
                              bool dirty = false;
                              for (QDomElement eee = ee.firstChildElement(); !eee.isNull();  eee = eee.nextSiblingElement()) {
                                    QString tag(eee.tagName());
                                    QString val(eee.text());
                                    if (tag == "name")
                                          name = val;
                                    else if (tag == "created")
                                          created = val.toInt();
                                    else if (tag == "dirty")
                                          dirty = val.toInt();
                                    else if (tag == "path") {
                                          Score* score = new Score(MScore::defaultStyle());
                                          if (!readScore(score, val)) {
                                                f.close();
                                                delete score;
                                                continue;
                                                }
                                          else {
                                                if (!name.isEmpty())
                                                      score->setName(name);
                                                if (cleanExit) {
                                                      // override if last session did a clean exit
                                                      dirty = false;
                                                      created = false;
                                                      }
                                                appendScore(score);
                                                score->setDirty(dirty);
                                                score->setCreated(created);
                                                }
                                          }
                                    else {
                                          domError(eee);
                                          f.close();
                                          return false;
                                          }
                                    }
                              }
                        else if (tag == "ScoreView") {
                              double x = .0, y = .0, vmag = .0;
                              int magIdx = MAG_FREE;
                              int tab = 0, idx = 0;
                              for (QDomElement eee = ee.firstChildElement(); !eee.isNull();  eee = eee.nextSiblingElement()) {
                                    QString tag(eee.tagName());
                                    QString val(eee.text());
                                    if (tag == "tab")
                                          tab = val.toInt();
                                    else if (tag == "idx")
                                          idx = val.toInt();
                                    else if (tag == "mag")
                                          vmag = val.toDouble();
                                    else if (tag == "magIdx")
                                          magIdx = val.toInt();
                                    else if (tag == "x")
                                          x = val.toDouble() * MScore::DPMM;
                                    else if (tag == "y")
                                          y = val.toDouble() * MScore::DPMM;
                                    else {
                                          domError(eee);
                                          f.close();
                                          return false;
                                          }
                                    }
                              if (magIdx != MAG_FREE)
                                    vmag = mag->getMag(cv);
                              (tab == 0 ? tab1 : tab2)->initScoreView(idx, vmag, magIdx, x, y);
                              }
                        else if (tag == "tab")
                              tab = ee.text().toInt();
                        else if (tag == "idx")
                              idx = ee.text().toInt();
                        else {
                              domError(ee);
                              f.close();
                              return false;
                              }
                        }
                  }
            else {
                  domError(e);
                  f.close();
                  return false;
                  }
            }
      setCurrentView(tab, idx);
      f.close();
      return true;
      }

//---------------------------------------------------------
//   splitWindow
//---------------------------------------------------------

void MuseScore::splitWindow(bool horizontal)
      {
      if (!_splitScreen) {
            if (tab2 == 0) {
                  tab2 = new ScoreTab(&scoreList);
                  tab2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                  connect(tab2, SIGNAL(currentScoreViewChanged(ScoreView*)), SLOT(setCurrentScoreView(ScoreView*)));
                  connect(tab2, SIGNAL(tabCloseRequested(int)), SLOT(removeTab(int)));
                  splitter->addWidget(tab2);
                  }
            tab2->setVisible(true);
            _splitScreen = true;
            _horizontalSplit = horizontal;
            splitter->setOrientation(_horizontalSplit ? Qt::Horizontal : Qt::Vertical);
            if (!scoreList.isEmpty()) {
                  tab2->setCurrentIndex(0);
                  Score* s = scoreList[0];
                  s->setLayoutAll(true);
                  s->end();
                  setCurrentView(1, 0);
                  }
            }
      else {
            if (_horizontalSplit == horizontal) {
                  _splitScreen = false;
                  tab2->setVisible(false);
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
            case STATE_DISABLED:   return "STATE_DISABLED";
            case STATE_NORMAL:     return "STATE_NORMAL";
            case STATE_NOTE_ENTRY: return "STATE_NOTE_ENTRY";
            case STATE_EDIT:       return "STATE_EDIT";
            case STATE_LYRICS_EDIT: return "STATE_LYRICS_EDIT";
            case STATE_PLAY:       return "STATE_PLAY";
            case STATE_SEARCH:     return "STATE_SEARCH";
            case STATE_FOTO:       return "STATE_FOTO";
            default:               return "??";
            }
      }

//---------------------------------------------------------
//   excerptsChanged
//---------------------------------------------------------

void MuseScore::excerptsChanged(Score* s)
      {
      if (tab2) {
//            ScoreView* v = tab2->view();
//            if (v && v->score() == s) {
                  tab2->updateExcerpts();
//                  }
            }
      if (tab1) {
            ScoreView* v = tab1->view();
            if (v && v->score() == s) {
                  tab1->updateExcerpts();
                  }
            else if (v == 0) {
                  tab1->setExcerpt(0);
                  tab1->updateExcerpts();
                  }
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
      if (on) {
            if (_pianoTools == 0) {
                  QAction* a = getAction("toogle-piano");
                  _pianoTools = new PianoTools(this);
                  addDockWidget(Qt::BottomDockWidgetArea, _pianoTools);
                  connect(_pianoTools, SIGNAL(keyPressed(int, bool)), SLOT(midiNoteReceived(int, bool)));
                  connect(_pianoTools, SIGNAL(pianoVisible(bool)), a, SLOT(setChecked(bool)));
                  }
            _pianoTools->show();
            }
      else {
            if (_pianoTools)
                  _pianoTools->hide();
            }
      }

//---------------------------------------------------------
//   showWeb
//---------------------------------------------------------

void MuseScore::showWebPanel(bool on)
      {
      QAction* a = getAction("musescore-connect");
      if (on) {
            if (_webPage == 0) {
                  _webPage = new WebPageDockWidget(this, this);
                  connect(_webPage, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));
                  addDockWidget(Qt::RightDockWidgetArea, _webPage);
                  }
            _webPage->show();
            }
      else {
            if (_webPage)
                  _webPage->hide();
            }
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
            connect(paletteBox, SIGNAL(paletteVisible(bool)), a, SLOT(setChecked(bool)));
            addDockWidget(Qt::LeftDockWidgetArea, paletteBox);

#if 0
            if (!useFactorySettings) {
                  QFile f(dataPath + "/" + "mscore-palette.xml");
                  if (f.exists()) {
                        if (paletteBox->read(&f))
                              return paletteBox;
                        }
                  }
#endif
            populatePalette();
            }
      return paletteBox;
      }

//---------------------------------------------------------
//   midiNoteReceived
//---------------------------------------------------------

void MuseScore::midiNoteReceived(int pitch, bool ctrl)
      {
      if (cv)
            cv->midiNoteReceived(pitch, ctrl);
      }

//---------------------------------------------------------
//   switchLayer
//---------------------------------------------------------

void MuseScore::switchLayer(const QString& s)
      {
      if (cs->switchLayer(s)) {
            cs->setLayoutAll(true);
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
            if(path.endsWith(".pdf") || path.endsWith(".mscz"))
                  name = path.section('/',-1);
        	  else
                  name = "unknown.mscz";
            }

      // attachment; filename="Bilder_einer_Ausstellung.mscz"

      qDebug("header <%s>", qPrintable(s));
      qDebug("name <%s>", qPrintable(name));

      QByteArray data = reply->readAll();
      QString tmpName = QDir::tempPath () + "/"+ name;
      QFile f(tmpName);
      f.open(QIODevice::WriteOnly);
      f.write(data);
      f.close();

      Score* score = new Score(MScore::defaultStyle());
      if (!readScore(score, tmpName)) {
            qDebug("readScore failed");
            delete score;
            return;
            }
      score->setCreated(true);
      score->setDirty(true);
      setCurrentScoreView(appendScore(score));
      lastOpenPath = score->fileInfo()->path();
      writeSessionFile(false);
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
      if (!networkManager) {
            networkManager = new QNetworkAccessManager(this);
            connect(networkManager, SIGNAL(finished(QNetworkReply*)),
               SLOT(networkFinished(QNetworkReply*)));
            }
      networkManager->get(QNetworkRequest(url));
      }

//---------------------------------------------------------
//   gotoNextScore
//---------------------------------------------------------

void MuseScore::gotoNextScore()
      {
      int idx = tab1->currentIndex();
      int n   = tab1->count();
      if (idx >= (n-1))
            idx = 0;
      else
            ++idx;
      tab1->setCurrentIndex(idx);
      }

//---------------------------------------------------------
//   gotoPreviousScore
//---------------------------------------------------------

void MuseScore::gotoPreviousScore()
      {
      int idx = tab1->currentIndex();
      if (idx == 0)
            idx = tab1->count() -1;
      else
            --idx;
      tab1->setCurrentIndex(idx);
      }

//---------------------------------------------------------
//   collectMatch
//---------------------------------------------------------

static void collectMatch(void* data, Element* e)
      {
      ElementPattern* p = static_cast<ElementPattern*>(data);
/*      if (p->type == e->type() && p->subtype != e->subtype())
            qDebug("%s subtype %d does not match", e->name(), e->subtype());
      */
//TODO      if ((p->type != e->type()) || (p->subtypeValid && p->subtype != e->subtype()))
      if (p->type != e->type())
            return;
      if ((p->staff != -1) && (p->staff != e->staffIdx()))
            return;
      if (e->type() == CHORD || e->type() == REST || e->type() == NOTE || e->type() == LYRICS) {
            if (p->voice != -1 && p->voice != e->voice())
                  return;
            }
      if (p->system) {
            Element* ee = e;
            do {
                  if (ee->type() == SYSTEM) {
                        if (p->system != ee)
                              return;
                        break;
                        }
                  ee = ee->parent();
                  } while (ee);
            }
      p->el.append(e);
      }

//---------------------------------------------------------
//   selectSimilar
//---------------------------------------------------------

void MuseScore::selectSimilar(Element* e, bool sameStaff)
      {
      ElementType type = e->type();
//TODO      int subtype      = e->subtype();

      ElementPattern pattern;
      pattern.subtypeValid = true;
//TODO      if (type == VOLTA_SEGMENT) {
            // Volta* volta = static_cast<VoltaSegment*>(e)->volta();
            // type    = volta->type();
            // subtype = volta->subtype();
            pattern.subtypeValid = false;
//            }

      Score* score = e->score();
      pattern.type    = type;
      pattern.subtype = 0; // TODO subtype;
      pattern.staff   = sameStaff ? e->staffIdx() : -1;
      pattern.voice   = -1;
      pattern.system  = 0;

      score->scanElements(&pattern, collectMatch);

      score->select(0, SELECT_SINGLE, 0);
      foreach(Element* e, pattern.el) {
            score->select(e, SELECT_ADD, 0);
            }
      }

//---------------------------------------------------------
//   selectElementDialog
//---------------------------------------------------------

void MuseScore::selectElementDialog(Element* e)
      {
      Score* score = e->score();
      SelectDialog sd(e, 0);
      if (sd.exec()) {
            ElementPattern pattern;
            sd.setPattern(&pattern);
            score->scanElements(&pattern, collectMatch);
            if (sd.doReplace()) {
                  score->select(0, SELECT_SINGLE, 0);
                  foreach(Element* ee, pattern.el)
                        score->select(ee, SELECT_ADD, 0);
                  }
            else if (sd.doSubtract()) {
                  QList<Element*> sl(score->selection().elements());
                  foreach(Element* ee, pattern.el)
                        sl.removeOne(ee);
                  score->select(0, SELECT_SINGLE, 0);
                  foreach(Element* ee, sl)
                        score->select(ee, SELECT_ADD, 0);
                  }
            else if (sd.doAdd()) {
                  foreach(Element* ee, pattern.el)
                        score->select(ee, SELECT_ADD, 0);
                  }
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
      setToolTip(tr("record"));
      }

//---------------------------------------------------------
//   GreendotButton
//---------------------------------------------------------

GreendotButton::GreendotButton(QWidget* parent)
   : SimpleButton(":/data/greendot.svg", ":/data/darkgreendot.svg", parent)
      {
      setCheckable(true);
      setToolTip(tr("record"));
      }

//---------------------------------------------------------
//   drawHandle
//---------------------------------------------------------

QRectF drawHandle(QPainter& p, const QPointF& pos, bool active)
      {
      p.save();
      p.setPen(QPen(QColor(Qt::blue), 2.0/p.matrix().m11()));
      if (active)
            p.setBrush(Qt::blue);
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
      if (cs->selection().state() != SEL_RANGE) {
            QMessageBox::StandardButton sb = QMessageBox::question(mscore,
               tr("MuseScore: transpose"),
               tr("There is nothing selected. Transpose whole score?"),
               QMessageBox::Yes | QMessageBox::Cancel,
               QMessageBox::Yes
            );
            if (sb == QMessageBox::Cancel)
                  return;
            //
            // select all
            //
            cs->selection().setState(SEL_RANGE);
            cs->selection().setStartSegment(cs->tick2segment(0));
            cs->selection().setEndSegment(
               cs->tick2segment(cs->last()->tick() + cs->last()->ticks())
               );
            cs->selection().setStaffStart(0);
            cs->selection().setStaffEnd(cs->nstaves());
            }
      bool rangeSelection = cs->selection().state() == SEL_RANGE;
      TransposeDialog td;

      // TRANSPOSE_BY_KEY and "transpose keys" is only possible if selection state is SEL_RANGE
      td.enableTransposeKeys(rangeSelection);
      td.enableTransposeByKey(rangeSelection);

      int startStaffIdx = 0;
      int startTick     = 0;
      if (rangeSelection) {
            startStaffIdx = cs->selection().staffStart();
            startTick     = cs->selection().tickStart();
            }
      KeyList* km = cs->staff(startStaffIdx)->keymap();
      int key     = km->key(startTick).accidentalType();
      td.setKey(key);
      if (!td.exec())
            return;
      cs->transpose(td.mode(), td.direction(), td.transposeKey(), td.transposeInterval(),
         td.getTransposeKeys(), td.getTransposeChordNames(), td.useDoubleSharpsFlats());
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void MuseScore::cmd(QAction* a)
      {
      if (inChordEditor)      // HACK
            return;

      QString cmdn(a->data().toString());

      if (MScore::debugMode)
            qDebug("MuseScore::cmd <%s>", cmdn.toAscii().data());

      Shortcut* sc = Shortcut::getShortcut(cmdn.toAscii().data());
      if (sc == 0) {
            qDebug("MuseScore::cmd(): unknown action <%s>", qPrintable(cmdn));
            return;
            }
      if (cs && (sc->state() & _sstate) == 0) {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: invalid command"),
               QString("Command %1 not valid in current state").arg(cmdn));
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

      if ((sc->flags() & A_SCORE) && ! cs) {
            qDebug("no score");
            return;
            }
      if (sc->flags() & A_CMD) {
            if (!cv->editMode())
                  cs->startCmd();
            }
      cmd(a, cmdn);
      if (lastShortcut->flags() & A_CMD)
            cs->endCmd();
      endCmd();
      }

//---------------------------------------------------------
//   endCmd
//    called after every command action (including every
//    mouse action)
//---------------------------------------------------------

void MuseScore::endCmd()
      {
      bool enableInput = false;
      if (cs) {
            if (cv->noteEntryMode())
                  cs->moveCursor();
            setPos(cs->inputState().tick());
            updateInputState(cs);
            updateUndoRedo();
            cs->setDirty(!cs->undo()->isClean());
            dirtyChanged(cs);
            Element* e = cs->selection().element();
            if (e && cs->playNote()) {
                  play(e);
                  cs->setPlayNote(false);
                  }
            if (cs->excerptsChanged()) {
                  Q_ASSERT(cs == cs->rootScore());
                  excerptsChanged(cs);
                  cs->setExcerptsChanged(false);
                  }
            if (cs->instrumentsChanged()) {
                  seq->initInstruments();
                  cs->setInstrumentsChanged(false);
                  }
            if (cs->selectionChanged()) {
                  cs->setSelectionChanged(false);
                  SelState ss = cs->selection().state();
                  selectionChanged(ss);
                  }
            QAction* action = getAction("concert-pitch");
            action->setChecked(cs->styleB(ST_concertPitch));

            if (e == 0 && cs->inputState().noteEntryMode)
                  e = cs->inputState().cr();
            enableInput = e && (e->type() == NOTE || e->isChordRest());
            cs->end();
            }
      else {
            if (inspector)
                  inspector->setElement(0);
            selectionChanged(SEL_NONE);
            }

      enableInputToolbar(enableInput);
      }

//---------------------------------------------------------
//   enableInputToolbar
//---------------------------------------------------------

void MuseScore::enableInputToolbar(bool enableInput)
      {
      static const char* actionNames[] = {
            "pad-rest", "pad-dot", "pad-dotdot", "note-longa",
            "note-breve", "pad-note-1", "pad-note-2", "pad-note-4",
            "pad-note-8", "pad-note-16", "pad-note-32", "pad-note-64",
            "pad-note-128",
//            "voice-1", "voice-2", "voice-3", "voice-4",
            "acciaccatura", "appoggiatura", "grace4", "grace16",
            "grace32", "beam-start", "beam-mid", "no-beam", "beam32",
            "auto-beam"
            };
      for (unsigned i = 0; i < sizeof(actionNames)/sizeof(*actionNames); ++i) {
            getAction(actionNames[i])->setEnabled(enableInput);
            }
      }

//---------------------------------------------------------
//   updateUndoRedo
//---------------------------------------------------------

void MuseScore::updateUndoRedo()
      {
      QAction* a = getAction("undo");
      a->setEnabled(cs ? cs->undo()->canUndo() : false);
      a = getAction("redo");
      a->setEnabled(cs ? cs->undo()->canRedo() : false);
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void MuseScore::cmd(QAction* a, const QString& cmd)
      {
      if (cmd == "instruments") {
            editInstrList();
            if (iledit)
                  iledit->updateAll(cs);
            }
      else if (cmd == "rewind") {
            seq->rewindStart();
            if (playPanel)
                  playPanel->heartBeat(0, 0);
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
      else if (cmd == "file-export")
            exportFile();
      else if (cmd == "file-part-export")
            exportParts();
      else if (cmd == "file-reload") {
            if (!cs->created() && !checkDirty(cs)) {
                  if (cv->editMode()) {
                        cv->postCmd("escape");
                        qApp->processEvents();
                        }
                  readScore(cs, cs->filePath());
                  // hack: so we don't get another checkDirty in appendScore
                  cs->setDirty(false);
                  setCurrentScoreView(appendScore(cs));
                  }
            }
      else if (cmd == "file-close")
            closeScore(cs);
      else if (cmd == "file-save-as") {
            cs->setSyntiState(synti->state());
            saveAs(cs, false);
            }
      else if (cmd == "file-save-selection") {
            cs->setSyntiState(synti->state());
            saveSelection(cs);
            }
      else if (cmd == "file-save-a-copy") {
            cs->setSyntiState(synti->state());
            saveAs(cs, true);
            }
      else if (cmd == "file-new")
            newFile();
      else if (cmd == "quit") {
            close();
            }
      else if (cmd == "masterpalette")
            showMasterPalette();
      else if (cmd == "toggle-statusbar") {
            preferences.showStatusBar = a->isChecked();
            _statusBar->setShown(preferences.showStatusBar);
            preferences.write();
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
      else if (cmd == "script-debug")
            scriptDebug = a->isChecked();
      else if (cmd == "backspace")
            undo();
      else if (cmd == "zoomin")
            incMag();
      else if (cmd == "zoomout")
            decMag();
      else if (cmd == "midi-on")
            midiinToggled(a->isChecked());
      else if (cmd == "sound-on")
            speakerToggled(a->isChecked());
      else if (cmd == "undo") {
            undo();
            if (inspector)
                  inspector->reset();
            }
      else if (cmd == "redo") {
            redo();
            if (inspector)
                  inspector->reset();
            }
      else if (cmd == "toggle-palette")
            showPalette(a->isChecked());
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
      else if (cmd == "toggle-mixer")
            showMixer(a->isChecked());
      else if (cmd == "synth-control")
            showSynthControl();
      else if (cmd == "show-keys")
            ;
      else if (cmd == "toggle-transport")
            transportTools->setVisible(!transportTools->isVisible());
      else if (cmd == "toggle-noteinput")
            entryTools->setVisible(!entryTools->isVisible());
      else if (cmd == "local-help")
            helpBrowser();
      else if (cmd == "follow")
            preferences.followSong = a->isChecked();
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

#ifdef Q_WS_MAC
            // Qt Bug: Toolbar goes into unified mode
            // after switching back from fullscreen
            setUnifiedTitleAndToolBarOnMac(false);
#endif
            }
      else if (cmd == "config-raster")
            editRaster();
      else if (cmd == "hraster" || cmd == "vraster")  // value in [hv]RasterAction already set
            ;
      else if (cmd == "toogle-piano")
            showPianoKeyboard(a->isChecked());
      else if (cmd == "musescore-connect")
            showWebPanel(a->isChecked());
      else if (cmd == "media")
            showMediaDialog();
      else if (cmd == "page-settings")
            showPageSettings();
      else if (cmd == "next-score")
            gotoNextScore();
      else if (cmd == "previous-score")
            gotoPreviousScore();
      else if (cmd == "transpose")
            transpose();
      else if (cmd == "tuplet-dialog")
            tupletDialog();
      else if (cmd == "save-style") {
            QString name = getStyleFilename(false);
            if (!name.isEmpty()) {
                  if (!cs->saveStyle(name)) {
                        QMessageBox::critical(this,
                           tr("MuseScore: save style"), MScore::lastError);
                        }
                  }
            }
      else if (cmd == "save-default-style") {
            QString name = getStyleFilename(false);
            if (!name.isEmpty()) {
                  if (!cs->saveStyle(name)) {
                        QMessageBox::critical(this,
                           tr("MuseScore: save style"), MScore::lastError);
                        }
                  else {
printf("set preferences to <%s>\n", qPrintable(name));
                        QFileInfo info(name);
                        if (info.suffix().isEmpty())
                              info.setFile(info.filePath() + ".mss");
                        preferences.defaultStyleFile = info.filePath();
                        }
                  }
            }
      else if (cmd == "load-style") {
            QString name = mscore->getStyleFilename(true);
            if (!name.isEmpty()) {
                  cs->startCmd();
                  if (!cs->loadStyle(name)) {
                        QMessageBox::critical(this,
                           tr("MuseScore: load style"), MScore::lastError);
                        }
                  cs->endCmd();
                  endCmd();
                  }
            }
      else if (cmd == "edit-style") {
            EditStyle es(cs, this);
            es.exec();
            }
      else if (cmd == "edit-text-style") {
            TextStyleDialog es(0, cs);
            es.exec();
            }
      else if (cmd == "edit-info") {
            MetaEditDialog med(cs, 0);
            med.exec();
            }
      else if (cmd == "print")
            printFile();
      else if (cmd == "repeat") {
            MScore::playRepeats = !MScore::playRepeats;
            cs->updateRepeatList(MScore::playRepeats);
            }
      else if (cmd == "pan")
            MScore::panPlayback = !MScore::panPlayback;
      else if (cmd == "show-invisible")
            cs->setShowInvisible(a->isChecked());
      else if (cmd == "show-unprintable")
            cs->setShowUnprintable(a->isChecked());
      else if (cmd == "show-frames")
            cs->setShowFrames(getAction(cmd.toLatin1().data())->isChecked());
      else if (cmd == "show-pageborders")
            cs->setShowPageborders(getAction(cmd.toLatin1().data())->isChecked());
      else if (cmd == "harmony-properties")
            cmdAddChordName2();
      else if (cmd == "tempo")
            addTempo();
      else if (cmd == "metronome")  // no action
            ;
      else if (cmd == "next-score")
            changeScore(1);
      else if (cmd == "prev-score")
            changeScore(-1);
      else if (cmd == "viewmode") {
            if (cs) {
                  if (cs->layoutMode() == LayoutPage) {
                        cs->setLayoutMode(LayoutLine);
                        viewModeCombo->setCurrentIndex(1);
                        }
                  else {
                        cs->setLayoutMode(LayoutPage);
                        viewModeCombo->setCurrentIndex(0);
                        }
                  }
            }
      else {
            if (cv) {
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
//   cmdAddChordName2
//---------------------------------------------------------

void MuseScore::cmdAddChordName2()
      {
      if (cs == 0 || !cs->checkHasMeasures())
            return;
      ChordRest* cr = cs->getSelectedChordRest();
      if (!cr)
            return;
      int rootTpc = 14;
      if (cr->type() == CHORD) {
            Chord* chord = static_cast<Chord*>(cr);
            rootTpc = chord->downNote()->tpc();
            }
      Harmony* s = 0;
      Segment* segment = cr->segment();

      foreach(Element* e, segment->annotations()) {
            if (e->type() == HARMONY && (e->track() == cr->track())) {
                  s = static_cast<Harmony*>(e);
                  break;
                  }
            }

      bool created = false;
      if (s == 0) {
            s = new Harmony(cs);
            s->setTrack(cr->track());
            s->setParent(segment);
            s->setRootTpc(rootTpc);
            created = true;
            }
      ChordEdit ce(cs);
      ce.setHarmony(s);
      int rv = ce.exec();
      if (rv) {
            const Harmony* h = ce.harmony();
            s->setRootTpc(h->rootTpc());
            s->setBaseTpc(h->baseTpc());
            s->setId(h->id());
            s->clearDegrees();
            for (int i = 0; i < h->numberOfDegrees(); i++)
                  s->addDegree(h->degree(i));
            s->render();
            cs->select(s, SELECT_SINGLE, 0);
            cs->undoAddElement(s);
            cs->setLayoutAll(true);
            }
      else {
            if (created)
                  delete s;
            }
      }

//---------------------------------------------------------
//   openExternalLink
//---------------------------------------------------------

void MuseScore::openExternalLink(const QString& url)
      {
      QDesktopServices::openUrl(url);
      }

//---------------------------------------------------------
//   closeWebPanelPermanently
//---------------------------------------------------------

void MuseScore::closeWebPanelPermanently()
      {
      showWebPanel(false);
      preferences.showWebPanel = false;
      preferences.dirty  = true;
      }

//---------------------------------------------------------
//   navigator
//---------------------------------------------------------

Navigator* MuseScore::navigator() const
      {
      return _navigator ? static_cast<Navigator*>(_navigator->widget()) : 0;
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
      removeTab(scoreList.indexOf(score->rootScore()));
      }

//---------------------------------------------------------
//   noteTooShortForTupletDialog
//---------------------------------------------------------

void MuseScore::noteTooShortForTupletDialog()
      {
      QMessageBox::warning(this, tr("MuseScore: warning"),
        tr("Cannot create tuplet: note value too short")
        );
      }

//---------------------------------------------------------
//   instrumentChanged
//---------------------------------------------------------

void MuseScore::instrumentChanged()
      {
      if (iledit)
            iledit->updateAll(cs);
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
//   sym2pixmap
//---------------------------------------------------------

QPixmap sym2pixmap(const Sym* s, qreal mag)
      {
      QString string = s->toString();
      QRectF  bb(s->getBbox());
      bb.setRect(bb.x() * mag, bb.y() * mag, bb.width() * mag, bb.height() * mag);

      bb.adjust(-5, -5, 5, 5);
      int w = lrint(bb.width());
      int h = lrint(bb.height());
      QPixmap pm(w, h);
      pm.fill(QColor(0, 0, 0, 0));
      QPainter painter;
      painter.begin(&pm);
      QPalette p(mscore->palette());
      painter.setPen(p.text().color());
      s->draw(&painter, mag, -bb.topLeft() + QPointF(2.0, 2.0));
      painter.end();
      return pm;
      }

//---------------------------------------------------------
//   switchLayoutMode
//---------------------------------------------------------

void MuseScore::switchLayoutMode(int val)
      {
      if (cs) {
            if (val == 0)
                  cs->setLayoutMode(LayoutPage);
            else
                  cs->setLayoutMode(LayoutLine);
            cv->update();
            }
      }

