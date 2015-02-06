//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <fenv.h>

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
#include "textstyle.h"
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
#include "importmidi/importmidi_panel.h"
#include "libmscore/chord.h"
#include "mstyle/mstyle.h"
#include "mstyle/mconfig.h"
#include "libmscore/segment.h"
#include "editraster.h"
#include "pianotools.h"
#include "mediadialog.h"
#include "workspace.h"
#include "selectdialog.h"
#include "transposedialog.h"
#include "metaedit.h"
#include "inspector/inspector.h"
#include "omrpanel.h"
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

#include "libmscore/mscore.h"
#include "libmscore/system.h"
#include "libmscore/measure.h"
#include "libmscore/chordlist.h"
#include "libmscore/volta.h"
#include "libmscore/lasso.h"

#include "driver.h"

#include "effects/zita1/zita.h"
#include "effects/noeffect/noeffect.h"
#include "synthesizer/synthesizer.h"
#include "synthesizer/synthesizergui.h"
#include "synthesizer/msynthesizer.h"
#include "fluid/fluid.h"
#include "qmlplugin.h"
#include "accessibletoolbutton.h"
#include "searchComboBox.h"

#include "startcenter.h"
#include "help.h"

#ifdef AEOLUS
extern Ms::Synthesizer* createAeolus();
#endif
#ifdef ZERBERUS
extern Ms::Synthesizer* createZerberus();
#endif

namespace Ms {

MuseScore* mscore;
MuseScoreCore* mscoreCore;
MasterSynthesizer* synti;

bool enableExperimental = false;
bool enableTestMode = false;

QString dataPath;
QString iconPath;

bool converterMode = false;
bool externalIcons = false;
static bool pluginMode = false;
static bool startWithNewScore = false;
double converterDpi = 0;
double guiScaling = 1.0;
int trimMargin = -1;

QString mscoreGlobalShare;

static QString outFileName;
static QString audioDriver;
static QString pluginName;
static QString styleFile;
static bool scoresOnCommandline { false };

QString localeName;
bool useFactorySettings = false;
bool deletePreferences = false;
QString styleName;
QString revision;
QErrorMessage* errorMessage;
const char* voiceActions[] = { "voice-1", "voice-2", "voice-3", "voice-4" };

extern bool savePositions(Score*, const QString& name, bool segments );
extern TextPalette* textPalette;

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
            mscore->currentScoreView()->cmdInsertMeasures(n, Element::Type::MEASURE);
      done(1);
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
      fprintf(stderr, "%s: Music Score Editor\nUnstable Prerelease for Version %s; Build %s\n",
         prog, VERSION, qPrintable(revision));
#else
      fprintf(stderr, "%s: Music Score Editor; Version %s; Build %s\n", prog, VERSION, qPrintable(revision));
#endif
      }

static const int RECENT_LIST_SIZE = 20;

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void MuseScore::closeEvent(QCloseEvent* ev)
      {
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

      writeSettings();

      _loginManager->save();

      ev->accept();
      if (preferences.dirty)
            preferences.write();
      this->deleteLater();     //this is necessary on windows http://musescore.org/node/16713
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

      transportTools->setEnabled(!noSeq && seq && seq->isRunning());
      playId->setEnabled(!noSeq && seq && seq->isRunning());

      getAction("midi-on")->setEnabled(preferences.enableMidiInput);
      _statusBar->setVisible(preferences.showStatusBar);

      updateNewWizard();
      }

//---------------------------------------------------------
//   MuseScore
//---------------------------------------------------------

MuseScore::MuseScore()
   : QMainWindow()
      {
      _sstate = STATE_INIT;
      setWindowTitle(QString("MuseScore"));
      setIconSize(QSize(preferences.iconWidth * guiScaling, preferences.iconHeight * guiScaling));

      ucheck = new UpdateChecker();

      setAcceptDrops(true);
      setFocusPolicy(Qt::NoFocus);

      if (!preferences.styleName.isEmpty()) {
            QFile f(preferences.styleName);
            if (f.open(QIODevice::ReadOnly)) {
                  MScore::defaultStyle()->load(&f);
                  f.close();
                  }
            }

      _positionLabel = new QLabel;
      _positionLabel->setObjectName("decoration widget");  // this prevents animations
      _positionLabel->setToolTip(tr("Measure:Beat:Tick"));

      _modeText = new QLabel;
      _modeText->setAutoFillBackground(false);
      _modeText->setObjectName("modeLabel");

      _statusBar = new QStatusBar;

      hRasterAction   = getAction("hraster");
      vRasterAction   = getAction("vraster");
      loopAction      = getAction("loop");
      loopInAction    = getAction("loop-in");
      loopOutAction   = getAction("loop-out");
      metronomeAction = getAction("metronome");
      countInAction   = getAction("countin");
      panAction       = getAction("pan");

      _statusBar->addPermanentWidget(new QWidget(this), 2);
      _statusBar->addPermanentWidget(new QWidget(this), 100);
      _statusBar->addPermanentWidget(_modeText, 0);

      if (enableExperimental) {
            layerSwitch = new QComboBox(this);
            layerSwitch->setToolTip(tr("Switch layer"));
            connect(layerSwitch, SIGNAL(activated(const QString&)), SLOT(switchLayer(const QString&)));
            playMode = new QComboBox(this);
            playMode->addItem(tr("synthesizer"));
            playMode->addItem(tr("audio track"));
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
      mainWindow->setOrientation(Qt::Vertical);

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
      showNavigator(preferences.showNavigator);

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
      QPushButton *b = new QPushButton(tr("Show MIDI import panel"));
      b->setFocusPolicy(Qt::ClickFocus);
      importmidiShowPanel->setVisible(false);
      connect(b, SIGNAL(clicked()), SLOT(showMidiImportPanel()));
      connect(importmidiPanel, SIGNAL(closeClicked()), importmidiShowPanel, SLOT(show()));
      hl->addWidget(b);
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
      a->setEnabled(preferences.enableMidiInput);
      a->setChecked(_midiinEnabled);
#endif

      getAction("undo")->setEnabled(false);
      getAction("redo")->setEnabled(false);
      getAction("paste")->setEnabled(false);
      selectionChanged(SelState::NONE);

      //---------------------------------------------------
      //    File Action
      //---------------------------------------------------

      //---------------------
      //    Tool Bar
      //---------------------

      fileTools = addToolBar(tr("File Operations"));
      fileTools->setObjectName("file-operations");

      for (auto i : { "file-new", "file-open", "file-save", "print", "undo", "redo"})
            fileTools->addWidget(new AccessibleToolButton(fileTools, getAction(i)));

      fileTools->addSeparator();
      mag = new MagBox;
      mag->setFocusPolicy(Qt::StrongFocus);
      mag->setAccessibleName(tr("Zoom"));
      mag->setFixedHeight(preferences.iconHeight + 10);  // hack
      connect(mag, SIGNAL(magChanged(int)), SLOT(magChanged(int)));
      connect(mag->lineEdit(), SIGNAL(editingFinished()), SLOT(magTextChanged()));
      fileTools->addWidget(mag);
      viewModeCombo = new QComboBox(this);
#if defined(Q_OS_MAC)
      viewModeCombo->setFocusPolicy(Qt::StrongFocus);
#else
      viewModeCombo->setFocusPolicy(Qt::TabFocus);
#endif
      viewModeCombo->setAccessibleName(tr("View Mode"));
      viewModeCombo->setFixedHeight(preferences.iconHeight + 8);  // hack
      viewModeCombo->addItem(tr("Page View"));
      viewModeCombo->addItem(tr("Continuous View"));
      connect(viewModeCombo, SIGNAL(activated(int)), SLOT(switchLayoutMode(int)));
      fileTools->addWidget(viewModeCombo);

      transportTools = addToolBar(tr("Transport Tools"));
      transportTools->setObjectName("transport-tools");
#ifdef HAS_MIDI
      transportTools->addWidget(new AccessibleToolButton(transportTools, getAction("midi-on")));
      transportTools->addSeparator();
#endif
      transportTools->addWidget(new AccessibleToolButton(transportTools, getAction("rewind")));
      transportTools->addWidget(new AccessibleToolButton(transportTools, getAction("play")));
      transportTools->addWidget(new AccessibleToolButton(transportTools, getAction("loop")));
      transportTools->addSeparator();
      transportTools->addWidget(new AccessibleToolButton(transportTools, getAction("repeat")));
      transportTools->addWidget(new AccessibleToolButton(transportTools, getAction("pan")));
      transportTools->addWidget(new AccessibleToolButton(transportTools, metronomeAction));

      cpitchTools = addToolBar(tr("Concert Pitch"));
      cpitchTools->setObjectName("pitch-tools");
      cpitchTools->addWidget(new AccessibleToolButton( cpitchTools, getAction("concert-pitch")));

      QToolBar* foto = addToolBar(tr("Screenshot Mode"));
      foto->setObjectName("foto-tools");
      foto->addWidget(new AccessibleToolButton(foto, getAction("fotomode")));

      addToolBarBreak();

      //-------------------------------
      //    Note Entry Tool Bar
      //-------------------------------

      entryTools = addToolBar(tr("Note Entry"));
      entryTools->setObjectName("entry-tools");

      static const char* sl1[] = {
            "note-input",
            "repitch", "pad-note-128", "pad-note-64", "pad-note-32", "pad-note-16",
            "pad-note-8",
            "pad-note-4", "pad-note-2", "pad-note-1", "note-breve", "note-longa",
            "pad-dot",
            "pad-dotdot", "tie", "", "pad-rest", "",
            "sharp2", "sharp", "nat", "flat", "flat2", "flip", ""
            };

      for (auto s : sl1) {
            if (!*s)
                  entryTools->addSeparator();
            else
                  entryTools->addAction(getAction(s));
            }

      for (int i = 0; i < VOICES; ++i) {
            QToolButton* tb = new QToolButton(this);
            tb->setToolButtonStyle(Qt::ToolButtonTextOnly);
            QPalette p(tb->palette());
            p.setColor(QPalette::Base, MScore::selectColor[i]);
            tb->setPalette(p);
            QAction* a = getAction(voiceActions[i]);
            a->setCheckable(true);
            tb->setDefaultAction(a);
            tb->setFocusPolicy(Qt::ClickFocus);
            entryTools->addWidget(tb);
            }


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

      openRecent = _fileMenu->addMenu(tr("Open &Recent"));

      connect(openRecent, SIGNAL(aboutToShow()), SLOT(openRecentMenu()));
      connect(openRecent, SIGNAL(triggered(QAction*)), SLOT(selectScore(QAction*)));

      for (auto i : {
            "", "file-save", "file-save-online", "file-save-as", "file-save-a-copy",
            "file-save-selection", "file-export", "file-part-export",
            "", "file-close", "", "parts", "album" }) {
            if (!*i)
                  _fileMenu->addSeparator();
            else if (i != QString("file-save-online") || enableExperimental)
                  _fileMenu->addAction(getAction(i));
            }
      if (enableExperimental)
            _fileMenu->addAction(getAction("layer"));
      _fileMenu->addSeparator();
      _fileMenu->addAction(getAction("edit-info"));
      if (enableExperimental)
            _fileMenu->addAction(getAction("media"));
      _fileMenu->addSeparator();
      _fileMenu->addAction(getAction("print"));
#ifndef Q_OS_MAC
      _fileMenu->addSeparator();
      _fileMenu->addAction(getAction("quit"));
#endif

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
      menuEdit->addAction(getAction("paste"));

      menuEdit->addSeparator();
      menuEdit->addAction(getAction("select-all"));
      menuEdit->addAction(getAction("select-section"));
      menuEdit->addAction(getAction("find"));

      menuEdit->addSeparator();

      QMenu* menuMeasure = new QMenu(tr("&Measure"));
      for (auto i : { "delete-measures", "split-measure", "join-measure" })
            menuMeasure->addAction(getAction(i));
      menuEdit->addMenu(menuMeasure);

      QMenu* menuTools = new QMenu(tr("&Tools"));
      for (auto i : { "add-remove-breaks", "explode", "implode", "slash-fill", "slash-rhythm", "resequence-rehearsal-marks" })
            menuTools->addAction(getAction(i));
      menuEdit->addMenu(menuTools);

      QMenu* menuVoices = new QMenu(tr("&Voices"));
      for (auto i : { "voice-x12", "voice-x13", "voice-x14", "voice-x23", "voice-x24", "voice-x34" })
            menuVoices->addAction(getAction(i));
      menuEdit->addMenu(menuVoices);

#ifdef NDEBUG
      if (enableExperimental) {
#endif
            menuEdit->addSeparator();
            menuEdit->addAction(getAction("debugger"));
#ifdef NDEBUG
            }
#endif

      menuEdit->addSeparator();
      menuWorkspaces = new QMenu(tr("W&orkspaces"));
      connect(menuWorkspaces, SIGNAL(aboutToShow()), SLOT(showWorkspaceMenu()));
      menuEdit->addMenu(menuWorkspaces);

      QAction* pref = menuEdit->addAction(tr("&Preferences..."), this, SLOT(startPreferenceDialog()));
      pref->setMenuRole(QAction::PreferencesRole);

      //---------------------
      //    Menu View
      //---------------------

      menuView = mb->addMenu(tr("&View"));
      menuView->setObjectName("View");

      a = getAction("startcenter");
      a->setCheckable(true);
      menuView->addAction(a);

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
      a->setChecked(preferences.showNavigator);
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

//      a = getAction("toggle-transport");
//      a->setCheckable(true);
//      a->setChecked(transportTools->isVisible());
//      connect(transportTools, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));
//      menuView->addAction(a);

//      a = getAction("toggle-noteinput");
//      a->setCheckable(true);
//      a->setChecked(true);
//      menuView->addAction(a);

      a = getAction("toggle-statusbar");
      a->setCheckable(true);
      a->setChecked(true);
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
      menuView->addSeparator();
      a = getAction("fullscreen");
      a->setCheckable(true);
      a->setChecked(false);
      menuView->addAction(a);

      //---------------------
      //    Menu Create
      //---------------------

      mb->addMenu(genCreateMenu(mb));

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
      for (auto i : { "duplet", "triplet", "quadruplet", "quintuplet", "sextuplet",
         "septuplet", "octuplet", "nonuplet", "tuplet-dialog" })
            menuNtole->addAction(getAction(i));
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

      for (auto i : { "page-settings", "reset", "stretch+", "stretch-", "reset-stretch", "reset-beammode" })
            menuLayout->addAction(getAction(i));

      //---------------------
      //    Menu Style
      //---------------------

      menuStyle = mb->addMenu(tr("&Style"));
      menuStyle->setObjectName("Style");
      menuStyle->addAction(getAction("edit-style"));
      menuStyle->addAction(getAction("edit-text-style"));
      if (enableExperimental)
            menuStyle->addAction(getAction("edit-harmony"));

      menuStyle->addSeparator();
      menuStyle->addAction(getAction("load-style"));
      menuStyle->addAction(getAction("save-style"));

      //---------------------
      //    Menu Plugins
      //---------------------

      QMenu* menuPlugins = mb->addMenu(tr("&Plugins"));
      menuPlugins->setObjectName("Plugins");

      menuPlugins->addAction(getAction("plugin-manager"));

      a = getAction("plugin-creator");
      a->setCheckable(true);
      menuPlugins->addAction(a);

      menuPlugins->addSeparator();


      //---------------------
      //    Menu Help
      //---------------------

      mb->addSeparator();
      QMenu* menuHelp = mb->addMenu(tr("&Help"));
      menuHelp->setObjectName("Help");

      HelpQuery* hw = new HelpQuery(menuHelp);
      menuHelp->addAction(hw);

      menuHelp->addAction(getAction("local-help"));
      menuHelp->addAction(tr("&Online Handbook"), this, SLOT(helpBrowser1()));

      menuHelp->addSeparator();

      QAction *aboutAction = new QAction(tr("&About"), 0);

      aboutAction->setMenuRole(QAction::AboutRole);
      connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
      menuHelp->addAction(aboutAction);

      QAction *aboutQtAction = new QAction(tr("About &Qt"), 0);
      aboutQtAction->setMenuRole(QAction::AboutQtRole);
      connect(aboutQtAction, SIGNAL(triggered()), this, SLOT(aboutQt()));
      menuHelp->addAction(aboutQtAction);

      QAction *aboutMusicXMLAction = new QAction(tr("About &MusicXML"), 0);
      aboutMusicXMLAction->setMenuRole(QAction::ApplicationSpecificRole);
      connect(aboutMusicXMLAction, SIGNAL(triggered()), this, SLOT(aboutMusicXML()));
      menuHelp->addAction(aboutMusicXMLAction);

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
      menuHelp->addAction(tr("Check for &Update"), this, SLOT(checkForUpdate()));
#endif

#ifdef MSCORE_UNSTABLE
      menuHelp->addSeparator();
      menuHelp->addAction(tr("Report a Bug"), this, SLOT(reportBug()));
#endif

      menuHelp->addSeparator();
      menuHelp->addAction(getAction("resource-manager"));

      //accessibility for menus
      foreach (QMenu* menu, mb->findChildren<QMenu*>()) {
            menu->setAccessibleName(menu->objectName());
            menu->setAccessibleDescription(Shortcut::getMenuShortcutString(menu));
            }

      setCentralWidget(envelope);

      // load cascading instrument templates
      loadInstrumentTemplates(preferences.instrumentList1);
      if (!preferences.instrumentList2.isEmpty())
            loadInstrumentTemplates(preferences.instrumentList2);

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
      _loginManager = new LoginManager(this);
      }

MuseScore::~MuseScore()
      {
      delete synti;
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
//   showHelp
//    show local help
//---------------------------------------------------------

void MuseScore::showHelp(const QUrl& url)
      {
      qDebug("showHelp <%s>", qPrintable(url.toString()));

      if (!helpEngine)
            return;

      QAction* a = getAction("local-help");
      a->blockSignals(true);
      a->setChecked(true);
      a->blockSignals(false);

      if (!helpBrowser) {
            helpBrowser = new HelpBrowser;
            manualDock = new QDockWidget(tr("Manual"), 0);
            manualDock->setObjectName("Manual");

            manualDock->setWidget(helpBrowser);
            Qt::DockWidgetArea area = Qt::RightDockWidgetArea;
            addDockWidget(area, manualDock);
            }
      manualDock->show();
      helpBrowser->setContent(url);
      }

void MuseScore::showHelp(QString s)
      {
      if (s.isEmpty())
            s = "manual";
      QMap<QString,QUrl>list = helpEngine->linksForIdentifier(s);
      if (!list.isEmpty())
            showHelp(*list.begin());
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
      QString help("http://musescore.org/en/handbook-2.0");
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
      help += QString("?utm_source=software&utm_medium=menu&utm_content=r%1&utm_campaign=MuseScore%2").arg(rev.trimmed()).arg(QString(VERSION));
      QDesktopServices::openUrl(QUrl(help));
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
                  Score* score = readScore(a);
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
      updateInspector();
      }

//---------------------------------------------------------
//   updateInspector
//---------------------------------------------------------

void MuseScore::updateInspector()
      {
      if (!_inspector)
            return;
      if (cs) {
            if (state() == STATE_EDIT)
                  _inspector->setElement(cv->getEditObject());
            else if (state() == STATE_FOTO)
                  _inspector->setElement(cv->fotoLasso());
            else {
                  _inspector->setElements(cs->selection().elements());
                  }
            }
      else
            _inspector->setElement(0);
      }

//---------------------------------------------------------
//   appendScore
//    append score to project list
//---------------------------------------------------------

int MuseScore::appendScore(Score* score)
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
      path = score->fileInfo()->absoluteFilePath();
      addRecentScore(path);
      if (startcenter)
            startcenter->updateRecentScores();
      }

void MuseScore::addRecentScore(const QString& scorePath)
      {
      if (scorePath.isEmpty())
            return;
      _recentScores.removeAll(scorePath);
      _recentScores.prepend(scorePath);
      if (_recentScores.size() > RECENT_LIST_SIZE)
            _recentScores.removeLast();
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
#if defined(Q_OS_WIN)
      fprintf(stderr, "Usage: MuseScore.exe flags scorefile\n"
#else
      fprintf(stderr, "Usage: mscore flags scorefile\n"
#endif
        "   Flags:\n"
        "   -v        print version\n"
        "   -d        debug mode\n"
        "   -L        layout debug\n"
        "   -s        no internal synthesizer\n"
        "   -m        no midi\n"
        "   -a driver use audio driver: jack alsa pulse portaudio\n"
        "   -n        start with new score\n"
        "   -I        dump midi input\n"
        "   -O        dump midi output\n"
        "   -o file   export to 'file'; format depends on file extension\n"
        "   -r dpi    set output resolution for image export\n"
        "   -T margin trim exported image with specified margin (in pixels)\n"
        "   -x factor set scaling factor for GUI elements\n"
        "   -S style  load style file\n"
        "   -p name   execute named plugin\n"
        "   -F        use factory settings\n"
        "   -R        revert to default preferences\n"
        "   -i        load icons from INSTALLPATH/icons\n"
        "   -e        enable experimental features\n"
        "   -c dir    override config/settings folder\n"
        "   -t        set testMode flag for all files\n"
        "   -M file   specify MIDI import operations file\n"
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
            if (cv->score() && (cs != cv->score())) {
                  // exit note entry mode
                  if (cv->noteEntryMode()) {
                        cv->cmd(getAction("escape"));
                        qApp->processEvents();
                        }
                  updateInputState(cv->score());
                  }
            cs = cv->score();
            view->setFocusRect();
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
            mixer->updateAll(cs);
#ifdef OMR
      if (omrPanel) {
            if (cv && cv->omrView())
                  omrPanel->setOmrView(cv->omrView());
            else
                  omrPanel->setOmrView(0);
            }
#endif
      if (!cs) {
            setWindowTitle("MuseScore");
            if (_navigator && _navigator->widget()) {
                  navigator()->setScoreView(view);
                  navigator()->setScore(0);
                  }
            if (_inspector)
                  _inspector->setElement(0);
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
      if (cs->layoutMode() == LayoutMode::PAGE)
            viewModeCombo->setCurrentIndex(0);
      else
            viewModeCombo->setCurrentIndex(1);

      selectionChanged(cs->selection().state());

      _sstate = STATE_DISABLED; // defeat optimization
      changeState(view->mscoreState());

      view->setFocus(Qt::OtherFocusReason);
      setFocusProxy(view);

      getAction("file-save")->setEnabled(cs->isSavable());
      getAction("file-part-export")->setEnabled(cs->rootScore()->excerpts().size() > 0);
      getAction("show-invisible")->setChecked(cs->showInvisible());
      getAction("show-unprintable")->setChecked(cs->showUnprintable());
      getAction("show-frames")->setChecked(cs->showFrames());
      getAction("show-pageborders")->setChecked(cs->showPageborders());
      updateUndoRedo();

      if (view->magIdx() == MagIdx::MAG_FREE)
            mag->setMag(view->mag());
      else
            mag->setCurrentIndex(int(view->magIdx()));

      if (cs->parentScore())
            setWindowTitle("MuseScore: " + cs->parentScore()->name() + "-" + cs->name());
      else
            setWindowTitle("MuseScore: " + cs->name());

      QAction* a = getAction("concert-pitch");
      a->setChecked(cs->styleB(StyleIdx::concertPitch));

      setPos(cs->inputPos());
      //showMessage(cs->filePath(), 2000);
      if (_navigator && _navigator->widget())
            navigator()->setScoreView(view);
      ScoreAccessibility::instance()->updateAccessibilityInfo();
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
                        Score* score = readScore(file);
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
      if (noSeq || !(seq && seq->isRunning()))
            return;
      if (playPanel == 0) {
            if (!visible)
                  return;
            playPanel = new PlayPanel(this);
            connect(playPanel, SIGNAL(gainChange(float)),     synti, SLOT(setGain(float)));
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
            mscore->currentScoreView()->cmdAppendMeasures(n, Element::Type::MEASURE);
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
                        case RMIDI_DOT:     a = getAction("pad-dot");  break;
                        case RMIDI_DOTDOT:  a = getAction("pad-dotdot");  break;
                        case RMIDI_TIE:     a = getAction("tie");  break;
                        case RMIDI_UNDO:    a = getAction("undo"); break;
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

      if (velo && processMidiRemote(MIDI_REMOTE_TYPE_NOTEON, pitch)) {
            active = 0;
            return;
            }

      QWidget* w = QApplication::activeModalWidget();
      if (!cv || w) {
            active = 1;
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
            cv->midiNoteReceived(pitch, active > 0);
            ++active;
            }
      else {
      		/*
		* Since a note may be assigned to a midi_remote, don't decrease active below zero
		* on noteoff.
		*/

            if ((channel != 0x09) && (active > 0))
                  --active;
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
            preferences.midiRemote[_midiRecordId].type = MIDI_REMOTE_TYPE_CTRL;
            preferences.midiRemote[_midiRecordId].data = controller;
            _midiRecordId = -1;
            if (preferenceDialog)
                  preferenceDialog->updateRemote();
            return;
            }
      // when value is 0 (usually when a key is released ) nothing happens
      if (value && processMidiRemote(MIDI_REMOTE_TYPE_CTRL, controller))
            return;
      }

//---------------------------------------------------------
//   playEnabled
//---------------------------------------------------------

bool MuseScore::playEnabled() const
      {
      return preferences.playNotes;
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
      static QList<QTranslator*> translatorList;
      QTranslator* translator = new QTranslator;
      QString userPrefix = dataPath + "/locale/"+ filename +"_";
      QString defaultPrefix = mscoreGlobalShare + "locale/"+ filename +"_";
      QString userlp = userPrefix + localeName;

      QString defaultlp = defaultPrefix + localeName;
      QString lp = defaultlp;

      QFileInfo userFi(userlp + ".qm");
      QFileInfo defaultFi(defaultlp + ".qm");

      if(!defaultFi.exists()) { // try with a shorter locale name
      QString shortLocaleName = localeName.left(localeName.lastIndexOf("_"));
            QString shortDefaultlp = defaultPrefix + shortLocaleName;
            QFileInfo shortDefaultFi(shortDefaultlp + ".qm");
            if(shortDefaultFi.exists()) {
                  userlp = userPrefix + shortLocaleName;
                  userFi = QFileInfo(userlp + ".qm");
                  defaultFi = shortDefaultFi;
                  defaultlp = shortDefaultlp;
                  }
            }

      //      qDebug() << userFi.exists();
      //      qDebug() << userFi.lastModified() << defaultFi.lastModified();
      if (userFi.exists()) {
            if (userFi.lastModified() > defaultFi.lastModified())
                  lp = userlp;
            else
                  QFile::remove(userlp + ".qm");
      }

      if (MScore::debugMode) qDebug("load translator <%s>", qPrintable(lp));
      bool success = translator->load(lp);
      if (!success && MScore::debugMode) {
            qDebug("load translator <%s> failed", qPrintable(lp));
      }
      if(success) {
            qApp->installTranslator(translator);
            translatorList.append(translator);
            }
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
      QLocale::setDefault(QLocale(localeName));
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
                        case SessionStart::LAST:
                              {
                              QSettings settings;
                              int n = settings.value("scores", 0).toInt();
                              int c = settings.value("currentScore", 0).toInt();
                              for (int i = 0; i < n; ++i) {
                                    QString s = settings.value(QString("score-%1").arg(i),"").toString();
                                    Score* score = mscore->readScore(s);
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
                              Score* score = mscore->readScore(preferences.startScore);
                              if (preferences.startScore.startsWith(":/") && score)
                                    score->setCreated(true);
                              if (score == 0) {
                                    score = mscore->readScore(":/data/My_First_Score.mscz");
                                    score->setCreated(true);
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
                  Score* score = mscore->readScore(name);
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
            if (!cs)
                  return false;
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
                  return saveXml(cs, fn);
            if (fn.endsWith(".mxl"))
                  return saveMxl(cs, fn);
            if (fn.endsWith(".mid"))
                  return mscore->saveMidi(cs, fn);
            if (fn.endsWith(".pdf"))
                  return mscore->savePdf(fn);
            if (fn.endsWith(".png"))
                  return mscore->savePng(cs, fn);
            if (fn.endsWith(".svg"))
                  return mscore->saveSvg(cs, fn);
#ifdef HAS_AUDIOFILE
            if (fn.endsWith(".wav") || fn.endsWith(".ogg") || fn.endsWith(".flac"))
                  return mscore->saveAudio(cs, fn);
#endif
            if (fn.endsWith(".mp3"))
                  return mscore->saveMp3(cs, fn);
            if (fn.endsWith(".spos"))
                  return savePositions(cs, fn, true);
            if (fn.endsWith(".mpos"))
                  return savePositions(cs, fn, false);
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
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setWindowTitle(tr("MuseScore Startup Dialog"));
      connect(createScore, SIGNAL(clicked()), SLOT(createScoreClicked()));
      connect(loadScore, SIGNAL(clicked()), SLOT(loadScoreClicked()));
      }

//---------------------------------------------------------
//   createScoreClicked
//---------------------------------------------------------

void StartDialog::createScoreClicked()
      {
      done(1);
      }

//---------------------------------------------------------
//   loadScoreClicked
//---------------------------------------------------------

void StartDialog::loadScoreClicked()
      {
      done(2);
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
     case QtDebugMsg:
         cerr << "Debug: " << localMsg.constData() << " ("  << context.file << ":" << context.line << ", " << context.function << ")" << endl;
         break;
     case QtWarningMsg:
         cerr << "Warning: " << localMsg.constData() << " ("  << context.file << ":" << context.line << ", " << context.function << ")" << endl;
         break;
     case QtCriticalMsg:
         cerr << "Critical: " << localMsg.constData() << " ("  << context.file << ":" << context.line << ", " << context.function << ")" << endl;
         break;
     case QtFatalMsg: // set your breakpoint here, if you want to catch the abort
         cerr << "Fatal: " << localMsg.constData() << " ("  << context.file << ":" << context.line << ", " << context.function << ")" << endl;
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
      // ms->registerEffect(0, new Freeverb);
      ms->registerEffect(1, new NoEffect);
      ms->registerEffect(1, new ZitaReverb);
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

bool MuseScoreApplication::event(QEvent *event)
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
                  handleMessage(static_cast<QFileOpenEvent *>(event)->file());
                  return true;
#endif
            case QEvent::KeyPress:
                  {
                  QKeyEvent* e = static_cast<QKeyEvent*>(event);
                  if(obj->isWidgetType() && e->key() == Qt::Key_Escape && e->modifiers() == Qt::NoModifier) {
                        if (isActiveWindow()) {
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
                  }
            default:
                  return QMainWindow::eventFilter(obj, event);
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
                   QWidget::tr("MuseScore: Load Languages Failed:"),
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
// printf("MuseScore::changeState: %s\n", stateName(val));
      if (MScore::debugMode)
            qDebug("MuseScore::changeState: %s", stateName(val));

//      if (_sstate == val)
//            return;

      // disallow change to edit modes if currently in play mode
      if (_sstate == STATE_PLAY && (val == STATE_EDIT || val & STATE_ALLTEXTUAL_EDIT || val & STATE_NOTE_ENTRY))
            return;

      static const char* stdNames[] = {
            "note-longa", "note-breve", "pad-note-1", "pad-note-2", "pad-note-4",
      "pad-note-8", "pad-note-16", "pad-note-32", "pad-note-64", "pad-note-128", "pad-rest", "rest"};
      static const char* tabNames[] = {
            "note-longa-TAB", "note-breve-TAB", "pad-note-1-TAB", "pad-note-2-TAB", "pad-note-4-TAB",
      "pad-note-8-TAB", "pad-note-16-TAB", "pad-note-32-TAB", "pad-note-64-TAB", "pad-note-128-TAB", "pad-rest-TAB", "rest-TAB"};
      bool intoTAB = (_sstate != STATE_NOTE_ENTRY_TAB) && (val == STATE_NOTE_ENTRY_TAB);
      bool fromTAB = (_sstate == STATE_NOTE_ENTRY_TAB) && (val != STATE_NOTE_ENTRY_TAB);
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

      foreach (const Shortcut* s, Shortcut::shortcuts()) {
            QAction* a = s->action();
            if (!a)
                  continue;
            if (enable && (s->key() == "undo"))
                  a->setEnabled((s->state() & val) && (cs ? cs->undo()->canUndo() : false));
            else if (enable && (s->key() == "redo"))
                  a->setEnabled((s->state() & val) && (cs ? cs->undo()->canRedo() : false));
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
            getAction("file-part-export")->setEnabled(cs && cs->rootScore()->excerpts().size() > 0);

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

      menuWorkspaces->setEnabled(enable);

      transportTools->setEnabled(enable && !noSeq && seq && seq->isRunning());
      cpitchTools->setEnabled(enable);
      mag->setEnabled(enable);
      entryTools->setEnabled(enable);

      if (_sstate == STATE_FOTO)
            updateInspector();
      if (_sstate == STATE_NOTE_ENTRY_DRUM)
            showDrumTools(0, 0);

      switch(val) {
            case STATE_DISABLED:
                  showModeText(tr("No score"));
                  if (debugger)
                        debugger->hide();
                  showPianoKeyboard(false);
                  break;
            case STATE_NORMAL:
                  _modeText->hide();
                  break;
            case STATE_NOTE_ENTRY_PITCHED:
                  showModeText(tr("Note entry mode"));
                  break;
            case STATE_NOTE_ENTRY_DRUM:
                  {
                  showModeText(tr("Drum entry mode"));
                  InputState& is = cs->inputState();
                  showDrumTools(is.drumset(), cs->staff(is.track() / VOICES));
                  if (_drumTools)
                        is.setDrumNote(_drumTools->selectedDrumNote());
                  }
                  break;
            case STATE_NOTE_ENTRY_TAB:
                  showModeText(tr("TAB entry mode"));
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
                  showModeText(tr("Screenshot mode"));
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
      bool noteEntry = val == STATE_NOTE_ENTRY_PITCHED || val == STATE_NOTE_ENTRY_TAB || val == STATE_NOTE_ENTRY_DRUM;
      a->setChecked(noteEntry);
      _sstate = val;

      Element* e = 0;
      if (_sstate & STATE_ALLTEXTUAL_EDIT || _sstate == STATE_EDIT) {
            if (cv)
                  e = cv->getEditObject();
            }
      if (!e) {
            textTools()->hide();
            if (textPalette)
                  textPalette->hide();
            }
      else {
            if (e->isText()) {
                  textTools()->setText(static_cast<Text*>(e));
                  textTools()->updateTools();
                  if (e->type() != Element::Type::FIGURED_BASS && e->type() != Element::Type::HARMONY)   // do not show text tools for f.b.
                        textTools()->show();
                  }
            if (_inspector)
                  _inspector->setElement(e);
            }
      }

//---------------------------------------------------------
//   saveDialogState
//---------------------------------------------------------

void MuseScore::saveDialogState(const char* name, QFileDialog* d)
      {
      if (d) {
            settings.beginGroup(name);
            settings.setValue("size",  d->size());
            settings.setValue("pos",   d->pos());
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
      d->resize(settings.value("size", QSize(860,489)).toSize());
      d->restoreState(settings.value("state").toByteArray());
      d->move(settings.value("pos", QPoint(200,200)).toPoint());
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
      int curScore = scoreList.indexOf(cs);
      if (curScore == -1)  // cs removed if new created and not modified
            curScore = 0;
      settings.setValue("currentScore", curScore);

      for (int idx = 0; idx < scoreList.size(); ++idx)
            settings.setValue(QString("score-%1").arg(idx), scoreList[idx]->fileInfo()->absoluteFilePath());

      settings.setValue("lastSaveCopyDirectory", lastSaveCopyDirectory);
      settings.setValue("lastSaveDirectory", lastSaveDirectory);

      settings.beginGroup("MainWindow");
      settings.setValue("size", size());
      settings.setValue("pos", pos());
      settings.setValue("maximized", isMaximized());
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
      if (mixer)
            mixer->writeSettings();
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
            startcenter->writeSettings(settings);
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
            mscore->showInspector(true);
            return;
            }

      settings.beginGroup("MainWindow");
      resize(settings.value("size", QSize(1024, 768)).toSize());
      mainWindow->restoreState(settings.value("debuggerSplitter").toByteArray());
      mainWindow->setOpaqueResize(false);

      move(settings.value("pos", QPoint(10, 10)).toPoint());
      //for some reason when MuseScore starts maximized the screen-reader
      //doesn't respond to QAccessibleEvents
      if (settings.value("maximized", false).toBool() && !QAccessible::isActive())
            showMaximized();
      mscore->showPalette(settings.value("showPanel", "1").toBool());
      mscore->showInspector(settings.value("showInspector", "1").toBool());
      mscore->showPianoKeyboard(settings.value("showPianoKeyboard", "0").toBool());
      mscore->showSelectionWindow(settings.value("showSelectionWindow", "0").toBool());

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

//      QAction* a = getAction("toggle-transport");
//      a->setChecked(!transportTools->isHidden());
//      a = getAction("toggle-noteinput");
//      a->setChecked(!entryTools->isHidden());
      }

//---------------------------------------------------------
//   play
//    play note for preferences.defaultPlayDuration
//---------------------------------------------------------

void MuseScore::play(Element* e) const
      {
      if (noSeq || !(seq && seq->isRunning()) || !mscore->playEnabled())
            return;

      if (e->type() == Element::Type::NOTE) {
            Note* note = static_cast<Note*>(e);
            play(e, note->ppitch());
            }
      else if (e->type() == Element::Type::CHORD) {
            seq->stopNotes();
            Chord* c = static_cast<Chord*>(e);
            Part* part = c->staff()->part();
            int tick = c->segment() ? c->segment()->tick() : 0;
            seq->seek(tick);
            Instrument* instr = part->instr(tick);
            foreach(Note* n, c->notes()) {
                  const Channel& channel = instr->channel(n->subchannel());
                  seq->startNote(channel.channel, n->ppitch(), 80, n->tuning());
                  }
            seq->startNoteTimer(MScore::defaultPlayDuration);
            }
      }

void MuseScore::play(Element* e, int pitch) const
      {
      if (noSeq || !(seq && seq->isRunning()))
            return;
      if (mscore->playEnabled() && e->type() == Element::Type::NOTE) {
            Note* note = static_cast<Note*>(e);
            int tick = note->chord()->tick();
            if (tick < 0)
                  tick = 0;
            Part* part = note->staff()->part();
            Instrument* instr = part->instr(tick);
            const Channel& channel = instr->channel(note->subchannel());
            seq->startNote(channel.channel, pitch, 80, MScore::defaultPlayDuration, note->tuning());
            }
      }

//---------------------------------------------------------
//   reportBug
//---------------------------------------------------------

void MuseScore::reportBug()
      {
      QString url("http://musescore.org/en/node/add/project-issue/musescore?sha=");
      url += revision();
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
      if (preferences.globalStyle == MuseScoreStyleType::DARK)
            museLogo->setPixmap(QPixmap(":/data/musescore_logo_full1.png"));
      else
            museLogo->setPixmap(QPixmap(":/data/musescore_logo_full.png"));

#ifdef MSCORE_UNSTABLE
      versionLabel->setText(tr("Unstable Prerelease for Version: ") + VERSION);
#else
      versionLabel->setText(tr("Version: ") + VERSION);
#endif
      revisionLabel->setText(tr("Revision: %1").arg(revision));
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
#ifdef MSCORE_UNSTABLE
      copyRevisionButton->setIcon(*icons[int(Icons::copy_ICON)]);
      connect(copyRevisionButton, SIGNAL(clicked()), this, SLOT(copyRevisionToClipboard()));
#else
      copyRevisionButton->hide();
#endif
      }

//---------------------------------------------------------
//   copyRevisionToClipboard
//---------------------------------------------------------

void AboutBoxDialog::copyRevisionToClipboard()
      {
      QClipboard* cb = QApplication::clipboard();
      cb->setText(QString("github-musescore-musescore-") + revision);
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
            cv->setMag((MagIdx)idx, mag->getMag(cv));
      }

//---------------------------------------------------------
//   magTextChanged
//---------------------------------------------------------

void MuseScore::magTextChanged()
      {
      if (!cv || mag->currentText().isEmpty())
            return;

      QString s = mag->currentText();
      if (s.right(1) == "%")
            s = s.left(s.length()-1);

      bool ok;
      qreal magVal = s.toFloat(&ok);
      if (ok) {
            mag->setMag((double)(magVal/100.0));
            cv->setMag(MagIdx::MAG_FREE, magVal/100.0);
            }

      //prevent the list from growing
      if (mag->count()-1 > (int)MagIdx::MAG_FREE)
            mag->removeItem(mag->count()-1);
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
            qreal _mag = cv->mag() / 1.7;
            if (_mag < 0.05)
                  _mag = 0.05;
            cv->setMag(MagIdx::MAG_FREE, _mag);
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
      if (_sstate == STATE_EDIT
         || _sstate == STATE_TEXT_EDIT
         || _sstate == STATE_HARMONY_FIGBASS_EDIT
         || _sstate == STATE_LYRICS_EDIT) {
            cv->postCmd("escape");
            qApp->processEvents();
            }
      if (cv)
            cv->startUndoRedo();
      if (cs) {
            if (undo)
                  cs->undo()->undo();
            else
                  cs->undo()->redo();
            }
      if (cv) {
            if (cs->inputState().segment())
                  setPos(cs->inputState().tick());
            if (cs->noteEntryMode() && !cv->noteEntryMode()) {
                  // enter note entry mode
                  cv->postCmd("note-input");
                  }
            else if (!cs->noteEntryMode() && cv->noteEntryMode()) {
                  // leave note entry mode
                  cv->postCmd("escape");
                  }
            cs->endUndoRedo();
            updateInputState(cs);
            }
      endCmd();
      if (_inspector)
            _inspector->reset();
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
      ((QtSingleApplication*)(qApp))->activateWindow();
      Score* score = readScore(message);
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
      Xml xml(&f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      xml.tagE(cleanExit ? "clean" : "dirty");
      foreach(Score* score, scoreList) {
            xml.stag("Score");
            xml.tag("created", score->created());
            xml.tag("dirty", score->dirty());

            QString path;
            if (score->importedFilePath().isEmpty()) {
                              // score was not imported from another format, e.g. MIDI file
                  path = score->fileInfo()->absoluteFilePath();
                  }
            else if (score->fileInfo()->exists()) {   // score was saved
                  path = score->fileInfo()->absoluteFilePath();
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
                        xml.tag("mag", v->mag());
                  else
                        xml.tag("magIdx", int(v->magIdx()));
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
                        if (v->magIdx() == MagIdx::MAG_FREE)
                              xml.tag("mag", v->mag());
                        else
                              xml.tag("magIdx", int(v->magIdx()));
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
      foreach (Score* s, scoreList) {
            if (s->autosaveDirty()) {
                  QString tmp = s->tmpName();
                  if (!tmp.isEmpty()) {
                        QFileInfo fi(tmp);
                        // TODO: cannot catch exeption here:
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
                                          Score* score = readScore(e.readElementText());
                                          if (score) {
                                                if (!name.isEmpty())
                                                      score->setName(name);
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
                              double x = .0, y = .0, vmag = .0;
                              MagIdx magIdx = MagIdx::MAG_FREE;
                              int tab = 0, idx = 0;
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
                                          x = e.readDouble() * MScore::DPMM;
                                    else if (tag == "y")
                                          y = e.readDouble() * MScore::DPMM;
                                    else {
                                          e.unknown();
                                          return false;
                                          }
                                    }
                              if (magIdx != MagIdx::MAG_FREE)
                                    vmag = mag->getMag(cv);
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
            case STATE_DISABLED:           return "STATE_DISABLED";
            case STATE_NORMAL:             return "STATE_NORMAL";
            case STATE_NOTE_ENTRY_PITCHED: return "STATE_NOTE_ENTRY_PITCHED";
            case STATE_NOTE_ENTRY_DRUM:    return "STATE_NOTE_ENTRY_DRUM";
            case STATE_NOTE_ENTRY_TAB:     return "STATE_NOTE_ENTRY_TAB";
            case STATE_NOTE_ENTRY:         return "STATE_NOTE_ENTRY";
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
            if (v && v->score()->rootScore() == s) {
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
      if (_pianoTools == 0) {
            QAction* a = getAction("toggle-piano");
            _pianoTools = new PianoTools(this);
            addDockWidget(Qt::BottomDockWidgetArea, _pianoTools);
            connect(_pianoTools, SIGNAL(keyPressed(int, bool)), SLOT(midiNoteReceived(int, bool)));
            connect(_pianoTools, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));
            }
      if (on) {
            _pianoTools->show();
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
            if (!pluginManager)
                  pluginManager = new PluginManager(0);
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

      Score* score = readScore(tmpName);
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
      if (!networkManager) {
            networkManager = new QNetworkAccessManager(this);
            connect(networkManager, SIGNAL(finished(QNetworkReply*)),
               SLOT(networkFinished(QNetworkReply*)));
            }
      networkManager->get(QNetworkRequest(url));
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
                  foreach(Element* ee, pattern.el)
                        score->select(ee, SelectType::ADD, 0);
                  }
            else if (sd.doSubtract()) {
                  QList<Element*> sl(score->selection().elements());
                  foreach(Element* ee, pattern.el)
                        sl.removeOne(ee);
                  score->select(0, SelectType::SINGLE, 0);
                  foreach(Element* ee, sl)
                        score->select(ee, SelectType::ADD, 0);
                  }
            else if (sd.doAdd()) {
                  QList<Element*> sl(score->selection().elements());
                  foreach(Element* ee, pattern.el) {
                        if(!sl.contains(ee))
                              score->select(ee, SelectType::ADD, 0);
                        }
                  }
            if (score->selectionChanged()) {
                  score->setSelectionChanged(false);
                  SelState ss = score->selection().state();
                  selectionChanged(ss);
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
      int startTick     = 0;
      if (rangeSelection) {
            startStaffIdx = cs->selection().staffStart();
            startTick     = cs->selection().tickStart();
            }
      Staff* staff = cs->staff(startStaffIdx);
      Key key = staff->key(startTick);
      if (!cs->styleB(StyleIdx::concertPitch)) {
            int diff = staff->part()->instr(startTick)->transpose().chromatic;
            if (diff)
                  key = transposeKey(key, diff);
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

      QString cmdn(a->data().toString());

      if (MScore::debugMode)
            qDebug("MuseScore::cmd <%s>", cmdn.toLatin1().data());

      const Shortcut* sc = Shortcut::getShortcut(cmdn.toLatin1().data());
      if (sc == 0) {
            qDebug("MuseScore::cmd(): unknown action <%s>", qPrintable(cmdn));
            return;
            }
      if (cs && (sc->state() & _sstate) == 0) {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: Invalid Command"),
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
      if (cs) {
            setPos(cs->inputState().tick());
            updateInputState(cs);
            updateUndoRedo();
            dirtyChanged(cs);
            Element* e = cs->selection().element();
            if (e && cs->playNote()) {
                  play(e);
                  cs->setPlayNote(false);
                  }
            if (cs->rootScore()->excerptsChanged()) {
                  //Q_ASSERT(cs == cs->rootScore());
                  excerptsChanged(cs->rootScore());
                  cs->rootScore()->setExcerptsChanged(false);
                  }
            if (cs->instrumentsChanged()) {
                  if (!noSeq && (seq && seq->isRunning()))
                        seq->initInstruments();
                  emit instrumentChanged();
                  cs->setInstrumentsChanged(false);
                  }
            if (cs->selectionChanged()) {
                  cs->setSelectionChanged(false);
                  SelState ss = cs->selection().state();
                  selectionChanged(ss);
                  }
            getAction("concert-pitch")->setChecked(cs->styleB(StyleIdx::concertPitch));

            if (e == 0 && cs->noteEntryMode())
                  e = cs->inputState().cr();
            if (cs->layoutMode() == LayoutMode::PAGE)
                  viewModeCombo->setCurrentIndex(0);
            else
                  viewModeCombo->setCurrentIndex(1);

            cs->end();
            ScoreAccessibility::instance()->updateAccessibilityInfo();
            }
      else {
            if (_inspector)
                  _inspector->setElement(0);
            selectionChanged(SelState::NONE);
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
            if (mixer)
                  mixer->updateAll(cs);
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
            showMasterPalette(tr("Key Signatures"));
      else if (cmd == "time-signatures")
            showMasterPalette(tr("Time Signatures"));
      else if (cmd == "symbols")
            showMasterPalette(tr("Symbols"));
      else if (cmd == "toggle-statusbar") {
            preferences.showStatusBar = a->isChecked();
            _statusBar->setVisible(preferences.showStatusBar);
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
//      else if (cmd == "toggle-transport")
//            transportTools->setVisible(!transportTools->isVisible());
//      else if (cmd == "toggle-noteinput")
//            entryTools->setVisible(!entryTools->isVisible());
      else if (cmd == "local-help") {
printf("cmd local help, checked %d\n", a->isChecked());
            if (!a->isChecked()) {
                  if (manualDock)
                        manualDock->hide();
                  a->setChecked(false);
                  }
            else
                  showHelp("manual");
            }
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
                           tr("MuseScore: Save Style"), MScore::lastError);
                        }
                  }
            }
      else if (cmd == "load-style") {
            QString name = mscore->getStyleFilename(true);
            if (!name.isEmpty()) {
                  cs->startCmd();
                  if (!cs->loadStyle(name)) {
                        QMessageBox::critical(this,
                           tr("MuseScore: Load Style"), MScore::lastError);
                        }
                  cs->endCmd();
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
            emit cs->playlistChanged();
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
      else if (cmd == "viewmode") {
            if (cs) {
                  if (cs->layoutMode() == LayoutMode::PAGE)
                        switchLayoutMode(1);
                  else
                        switchLayoutMode(0);
                  }
            }
      else {
            if (cv) {
                  //isAncestorOf is called to see if a widget from inspector has focus
                  //if so, the focus doesn't get shifted to the score, unless escape is
                  //pressed, or the user clicks in the score
                  if(!inspector()->isAncestorOf(qApp->focusWidget()) || cmd == "escape")
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
      removeTab(scoreList.indexOf(score->rootScore()));
      }

//---------------------------------------------------------
//   noteTooShortForTupletDialog
//---------------------------------------------------------

void MuseScore::noteTooShortForTupletDialog()
      {
      QMessageBox::warning(this, tr("MuseScore: Warning"),
        tr("Cannot create tuplet: Note value is too short")
        );
      }

//---------------------------------------------------------
//   instrumentChanged
//---------------------------------------------------------

void MuseScore::instrumentChanged()
      {
      if (mixer)
            mixer->updateAll(cs);
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
//---------------------------------------------------------

void MuseScore::switchLayoutMode(int val)
      {
      if (cs) {
            cs->startCmd();
            LayoutMode mode;
            // find a measure to use as reference, if possible
            QRectF view = cv->toLogical(QRect(0.0, 0.0, width(), height()));
            Measure* m = cs->firstMeasure();
            while (m && !view.intersects(m->canvasBoundingRect()))
                  m = m->nextMeasureMM();
            if (val == 0)
                  mode = LayoutMode::PAGE;
            else
                  mode = LayoutMode::LINE;
            cs->undo(new ChangeLayoutMode(cs, mode));
            cv->loopUpdate(getAction("loop")->isChecked());
            cs->endCmd();
            // adjustCanvasPosition often tries to preserve Y position
            // but this doesn't make sense when switching modes
            // also, better positioning is usually achieved if you start from the top
            // and there is really no better place to position canvas if we were all the way off page previously
            cv->pageTop();
            if (m && m != cs->firstMeasure())
                  cv->adjustCanvasPosition(m, false);
            }
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

void MuseScore::updateDrumTools()
      {
      if (_drumTools)
            _drumTools->updateDrumset();
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

            // does not work: connect(searchCombo->lineEdit(), SIGNAL(returnPressed()), SLOT(endSearch()));
            connect(searchCombo->lineEdit(), SIGNAL(editingFinished()), SLOT(endSearch()));
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
                  if ((s->fileInfo()->canonicalFilePath() == fp) || (s->importedFilePath() == fp)) {
                        alreadyLoaded = true;
                        break;
                        }
                  }
            if (!alreadyLoaded && fi.exists())
                  fil.append(fi);
            }
      return fil;
      }

}

using namespace Ms;

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* av[])
      {
      QApplication::setDesktopSettingsAware(true);
#if defined(QT_DEBUG) && defined(Q_OS_WIN)
      qInstallMessageHandler(mscoreMessageHandler);
#endif

      QFile f(":/revision.h");
      f.open(QIODevice::ReadOnly);
      revision = QString(f.readAll()).trimmed();
      f.close();

#ifdef Q_OS_MAC
      MuseScoreApplication* app = new MuseScoreApplication("mscore-dev", argc, av);
#else
      QtSingleApplication* app = new QtSingleApplication("mscore-dev", argc, av);
#endif

      QCoreApplication::setOrganizationName("MuseScore");
      QCoreApplication::setOrganizationDomain("musescore.org");
      QCoreApplication::setApplicationName("MuseScoreDevelopment");
      QAccessible::installFactory(AccessibleScoreView::ScoreViewFactory);
      QAccessible::installFactory(AccessibleSearchBox::SearchBoxFactory);
      Q_INIT_RESOURCE(zita);

#ifndef Q_OS_MAC
      QSettings::setDefaultFormat(QSettings::IniFormat);
#endif

      QStringList argv =  QCoreApplication::arguments();
      argv.removeFirst();

      for (int i = 0; i < argv.size();) {
            QString s = argv[i];
            if (s[0] != '-') {
                  ++i;
                  continue;
                  }
            switch (s[1].toLatin1()) {
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
                  case 'a':
                        if (argv.size() - i < 2)
                              usage();
                        audioDriver = argv.takeAt(i + 1);
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
                        MScore::noGui = true;
                        if (argv.size() - i < 2)
                              usage();
                        outFileName = argv.takeAt(i + 1);
                        break;
                  case 'p':
                        pluginMode = true;
                        MScore::noGui = true;
                        if (argv.size() - i < 2)
                              usage();
                        pluginName = argv.takeAt(i + 1);
                        break;
                  case 'r':
                        if (argv.size() - i < 2)
                              usage();
                        converterDpi = argv.takeAt(i + 1).toDouble();
                        break;
                  case 'T':
                        if (argv.size() - i < 2)
                              usage();
                        trimMargin = argv.takeAt(i + 1).toInt();
                        break;
                  case 'x':
                        if (argv.size() - i < 2)
                              usage();
                        guiScaling = argv.takeAt(i + 1).toDouble();
                        break;
                  case 'S':
                        if (argv.size() - i < 2)
                              usage();
                        styleFile = argv.takeAt(i + 1);
                        break;
                  case 'F':
                        useFactorySettings = true;
                        deletePreferences = true;
                        break;
                  case 'R':
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
                  case 't':
                        enableTestMode = true;
                        break;
                  case 'M':
                        if (argv.size() - i < 2)
                              usage();
                        preferences.midiImportOperations.setOperationsFile(argv.takeAt(i + 1));
                        break;
                  default:
                        usage();
                  }
            argv.removeAt(i);
            }
      mscoreGlobalShare = getSharePath();
      iconPath = externalIcons ? mscoreGlobalShare + QString("icons/") :  QString(":/data/icons/");

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
                  if (app->sendMessage(QString("")))
                      return 0;
            }

      if (dataPath.isEmpty())
            dataPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);

      if (deletePreferences) {
            QDir(dataPath).removeRecursively();
            QSettings settings;
            QFile::remove(settings.fileName());
            }

      // create local plugin directory
      // if not already there:
      QDir().mkpath(dataPath + "/plugins");

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

      QNetworkProxyFactory::setUseSystemConfiguration(true);

      QScreen* screen = QGuiApplication::primaryScreen();
      MScore::PDPI = screen->physicalDotsPerInch();        // physical resolution
      MScore::DPI  = screen->logicalDotsPerInch();         // logical drawing resolution
      MScore::DPI *= guiScaling;

      MScore::init();                                      // initialize libmscore
      if (!MScore::testMode) {
            QSizeF psf = QPrinter().paperSize(QPrinter::Inch);
            PaperSize ps("system", psf.width(), psf.height());
            PageFormat pf;
            pf.setSize(&ps);
            MScore::defaultStyle()->setPageFormat(pf);
            }

#ifdef SCRIPT_INTERFACE
      qmlRegisterType<QmlPlugin>  ("MuseScore", 1, 0, "MuseScore");
#endif
      if (MScore::debugMode) {
            qDebug("DPI %f", MScore::DPI);

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

      if (!useFactorySettings)
            preferences.read();

      preferences.readDefaultStyle();

      if (converterDpi == 0)
            converterDpi = preferences.pngResolution;

      QSplashScreen* sc = 0;
      QTimer* stimer = 0;
      if (!MScore::noGui && preferences.showSplashScreen) {
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

      if (!converterMode) {
            struct PaletteItem {
                  QPalette::ColorRole role;
                  const char* name;
                  const char* color;
                  };
            MgStyle* st = new MgStyle;
            QApplication::setStyle(st);
            QPalette p(QApplication::palette());
            QSettings s;

            switch (preferences.globalStyle) {
                  case MuseScoreStyleType::DARK: {
                        static const PaletteItem pi[] = {
                              { QPalette::Window,          "WindowColor",          "#525252" },
                              { QPalette::WindowText,      "WindowTextColor",      "#FFFFFF" },
                              { QPalette::Base,            "BaseColor",            "#424242" },
                              { QPalette::AlternateBase,   "AlternateBaseColor",   "#626262" },
                              { QPalette::Text,            "TextColor",            "#FFFFFF" },
                              { QPalette::Button,          "ButtonColor",          "#525252" },
                              { QPalette::ButtonText,      "ButtonTextColor",      "#FFFFFF" },
                              { QPalette::BrightText,      "BrightTextColor",      "#000000" },

//                            { QPalette::Light,           "LightColor",           "#00FF00" },
//                            { QPalette::Midlight,        "MidlightTextColor",    "#00FF00" },
//                            { QPalette::Dark,            "DarkTextColor",        "#00FF00" },
//                            { QPalette::Mid,             "MidColor",             "#00FF00" },
//                            { QPalette::Shadow,          "ShadowColor",          "#00FF00" },
                              { QPalette::Highlight,       "HighlightColor",       "#88bff6" },
//                            { QPalette::HighlightedText, "HighlightedTextColor", "#00FF00" },
//                            { QPalette::Link,            "HighlightedTextColor", "#ffffff" },
//                            { QPalette::LinkVisited,     "HighlightedTextColor", "#00ffff" },
                              { QPalette::ToolTipBase,     "ToolTipBaseColor",     "#808080" },
                              { QPalette::ToolTipText,     "ToolTipTextColor",     "#000000" },
                              };
                        for (auto i : pi)
                              p.setColor(i.role, s.value(i.name, i.color).value<QColor>());
                        break;
                        }
                  case MuseScoreStyleType::LIGHT:
                        static const PaletteItem pi[] = {
                              { QPalette::Window,        "WindowColor",        "#e3e3e3"  },
                              { QPalette::WindowText,    "WindowTextColor",    "#333333"  },
                              { QPalette::Base,          "BaseColor",          "#f9f9f9"  },
                              { QPalette::AlternateBase, "AlternateBaseColor", "#eeeeee"  },
                              { QPalette::Text,          "TextColor",          "#333333"  },
                              { QPalette::Button,        "ButtonColor",        "#c9c9c9"  },
                              { QPalette::ButtonText,    "ButtonTextColor",    "#333333"  },
                              { QPalette::BrightText,    "BrightTextColor",    "#000000"  },
                              { QPalette::ToolTipBase,   "ToolTipBaseColor",   "#fefac2"  },
                              { QPalette::ToolTipText,   "ToolTipTextColor",   "#000000"  },
                              { QPalette::Link,          "LinkColor",          "#3a80c6"  },
                              { QPalette::LinkVisited,   "LinkVisitedColor",   "#3a80c6"  },
                              };
                        for (auto i : pi)
                              p.setColor(i.role, s.value(i.name, i.color).value<QColor>());
                        break;
                  }
            QApplication::setPalette(p);

            qApp->setStyleSheet(
                  "*:disabled {\n"
                  "   color: gray\n"
                  "}\n"
                  "QGroupBox {\n"
                  "font-weight: bold;\n"
                  "}\n"
                  "QGroupBox::title {\n"
                  "   subcontrol-origin: margin; subcontrol-position: top left; padding: 5px 5px;\n"
                  "}\n"
                  "QDockWidget {\n"
                  "   border: 1px solid lightgray;\n"
                  "   titlebar-close-icon: url(:/data/icons/png/window-close.png);\n"
                  "   titlebar-normal-icon: url(:/data/icons/png/window-float.png);\n"
                  "   }\n"
                  "QTabBar::close-button {\n"
                  "   image: url(:/data/icons/png/window-close.png);\n"
              "   }\n"
                  "QTabBar::close-button:hover {\n"
              "   image: url(:/data/icons/png/window-close-hover.png);\n"
              "   }");
            MgStyleConfigData::animationsEnabled = preferences.animations;
            qApp->setAttribute(Qt::AA_UseHighDpiPixmaps);
            }
      else
            noSeq = true;

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
      //   _spatium    = SPATIUM20  * DPI;     // 20.0 / 72.0 * DPI / 4.0;

      genIcons();

      if (!MScore::noGui) {
            qApp->setWindowIcon(*icons[int(Icons::window_ICON)]);
            Workspace::initWorkspace();
            }

      mscore = new MuseScore();
      mscoreCore = mscore;

      // create a score for internal use
      gscore = new Score(MScore::baseStyle());
      gscore->style()->set(StyleIdx::MusicalTextFont, QString("Bravura Text"));
      ScoreFont* scoreFont = ScoreFont::fontFactory("Bravura");
      gscore->setScoreFont(scoreFont);
      gscore->setNoteHeadWidth(scoreFont->width(SymId::noteheadBlack, gscore->spatium()) / (MScore::DPI * SPATIUM20));

      if (!noSeq) {
            if (!seq->init())
                  qDebug("sequencer init failed");
            }

      //read languages list
      mscore->readLanguages(mscoreGlobalShare + "locale/languages.xml");

      QApplication::instance()->installEventFilter(mscore);

      mscore->setRevision(revision);

      int files = 0;
      if (MScore::noGui) {
#ifdef Q_OS_MAC
            // see issue #28706: Hangup in converter mode with MusicXML source
            qApp->processEvents();
#endif
            loadScores(argv);
            exit(processNonGui() ? 0 : -1);
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
#ifdef Q_WS_MAC
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
            if (!mscore->restoreSession((preferences.sessionStart == SessionStart::LAST) && (files == 0)) || files)
                  loadScores(argv);
            }
      errorMessage = new QErrorMessage(mscore);
      mscore->loadPlugins();
      mscore->writeSessionFile(false);

#ifdef Q_OS_MAC
      // there's a bug in Qt showing the toolbar unified after switching showFullScreen(), showMaximized(),
      // showNormal()...
      mscore->setUnifiedTitleAndToolBarOnMac(false);
#endif

      mscore->changeState(mscore->noScore() ? STATE_DISABLED : STATE_NORMAL);
      mscore->show();

      if (mscore->hasToCheckForUpdate())
            mscore->checkForUpdate();

      if (!scoresOnCommandline && preferences.showStartcenter) {
            getAction("startcenter")->setChecked(true);
            mscore->showStartcenter(true);
            }
      return qApp->exec();
      }

