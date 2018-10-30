//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: musescore.h 5657 2012-05-21 15:46:06Z lasconic $
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

#ifndef __MUSESCORE_H__
#define __MUSESCORE_H__

#include "config.h"
#include "globals.h"
#include "ui_measuresdialog.h"
#include "ui_insertmeasuresdialog.h"
#include "ui_aboutbox.h"
#include "ui_aboutmusicxmlbox.h"
#include "singleapp/src/QtSingleApplication"
#include "updatechecker.h"
// #include "loginmanager.h"
// #include "uploadscoredialog.h"
#include "libmscore/musescoreCore.h"
#include "libmscore/score.h"
#include "newwizard.h"

namespace Ms {

class UploadScoreDialog;
class LoginManager;
class Shortcut;
class ScoreView;
class Element;
class PreferenceDialog;
class InstrumentsDialog;
class Instrument;
class MidiFile;
class TextStyleDialog;
class PlayPanel;
class Mixer;
class Debugger;
class MeasureListEditor;
class MasterScore;
class Score;
class Tuplet;
class PageSettings;
class PaletteBox;
class Palette;
class PaletteScrollArea;
class SelectionWindow;
class XmlWriter;
class MagBox;
class NewWizard;
class ExcerptsDialog;
class SynthControl;
class PianorollEditor;
class DrumrollEditor;
class Staff;
class ScoreTab;
class Drumset;
class TextTools;
class DrumTools;
class ScriptEngine;
class KeyEditor;
class ChordStyleEditor;
class Navigator;
class Timeline;
class PianoTools;
class MediaDialog;
class Workspace;
class WorkspaceDialog;
class AlbumManager;
class WebPageDockWidget;
class ChordList;
class Capella;
class Inspector;
class OmrPanel;
class NScrollArea;
class TDockWidget;
class Sym;
class MasterPalette;
class PluginCreator;
class PluginManager;
class MasterSynthesizer;
class SynthesizerState;
class Driver;
class Seq;
class ImportMidiPanel;
class ScoreComparisonTool;
class Startcenter;
class HelpBrowser;
class ToolbarEditor;
class TourHandler;

struct PluginDescription;
enum class SelState : char;
enum class IconType : signed char;
enum class MagIdx : char;

extern QString mscoreGlobalShare;
static const int PROJECT_LIST_LEN = 6;
extern const char* voiceActions[];

//---------------------------------------------------------
//   IconActions
//---------------------------------------------------------

struct IconAction {
      IconType subtype;
      const char* action;
      };

//---------------------------------------------------------
//   LanguageItem
//---------------------------------------------------------

struct LanguageItem {
      QString key;
      QString name;
      QString handbook;
      LanguageItem(const QString k, const QString n) {
            key = k;
            name = n;
            handbook = QString::null;
            }
      LanguageItem(const QString k, const QString n, const QString h) {
            key = k;
            name = n;
            handbook = h;
            }
      };

//---------------------------------------------------------
//   AboutBoxDialog
//---------------------------------------------------------

class AboutBoxDialog : public QDialog, Ui::AboutBox {
      Q_OBJECT

   public:
      AboutBoxDialog();

   private slots:
      void copyRevisionToClipboard();
      };

//---------------------------------------------------------
//   AboutMusicXMLBoxDialog
//---------------------------------------------------------

class AboutMusicXMLBoxDialog : public QDialog, Ui::AboutMusicXMLBox {
      Q_OBJECT

   public:
      AboutMusicXMLBoxDialog();
      };

//---------------------------------------------------------
//   InsertMeasuresDialog
//   Added by DK, 05.08.07
//---------------------------------------------------------

class InsertMeasuresDialog : public QDialog, public Ui::InsertMeasuresDialogBase {
      Q_OBJECT

      virtual void hideEvent(QHideEvent*);

   private slots:
      virtual void accept();

   public:
      InsertMeasuresDialog(QWidget* parent = 0);
      };

//---------------------------------------------------------
//   MeasuresDialog
//---------------------------------------------------------

class MeasuresDialog : public QDialog, public Ui::MeasuresDialogBase {
      Q_OBJECT

   private slots:
      virtual void accept();

   public:
      MeasuresDialog(QWidget* parent = 0);
      };


//---------------------------------------------------------
//   MuseScoreApplication (mac only)
//---------------------------------------------------------

class MuseScoreApplication : public QtSingleApplication {
   public:
      QStringList paths;
      MuseScoreApplication(const QString& id, int &argc, char **argv)
         : QtSingleApplication(id, argc, argv) {
            };
      virtual bool event(QEvent *ev) override;
      };


//---------------------------------------------------------
//   MuseScore
//---------------------------------------------------------

class MuseScore : public QMainWindow, public MuseScoreCore {
      Q_OBJECT

      QSettings settings;
      ScoreView* cv                        { 0 };
      ScoreState _sstate;
      UpdateChecker* ucheck;
      ExtensionsUpdateChecker* packUChecker = nullptr;

      static const std::list<const char*> _allNoteInputMenuEntries;
      std::list<const char*> _noteInputMenuEntries { _allNoteInputMenuEntries };

      static const std::list<const char*> _allFileOperationEntries;
      std::list<const char*> _fileOperationEntries { _allFileOperationEntries };

      static const std::list<const char*> _allPlaybackControlEntries;
      std::list<const char*> _playbackControlEntries { _allPlaybackControlEntries };

      QVBoxLayout* layout;    // main window layout
      QSplitter* splitter;
      ScoreTab* tab1;
      ScoreTab* tab2;
      NScrollArea* _navigator;
      TDockWidget* _timeline;
      ImportMidiPanel* importmidiPanel     { 0 };
      QFrame* importmidiShowPanel;
      QSplitter* mainWindow;

      ScoreComparisonTool* scoreCmpTool    { 0 };

      MagBox* mag;
      QComboBox* viewModeCombo;
      QAction* playId;

      QAction* pref;
      QAction* onlineHandbookAction;
      QAction* aboutAction;
      QAction* aboutQtAction;
      QAction* aboutMusicXMLAction;
      QAction* checkForUpdateAction        { 0 };
      QAction* askForHelpAction;
      QAction* reportBugAction;
      QAction* leaveFeedbackAction;
      QAction* revertToFactoryAction;

      QProgressBar* _progressBar           { 0 };
      PreferenceDialog* preferenceDialog   { 0 };
      QToolBar* cpitchTools;
      QToolBar* fotoTools;
      QToolBar* fileTools;
      QToolBar* transportTools;
      QToolBar* entryTools;
      QToolBar* feedbackTools;
      TextTools* _textTools                { 0 };
      PianoTools* _pianoTools              { 0 };
      MediaDialog* _mediaDialog            { 0 };
      DrumTools* _drumTools                { 0 };
      QToolBar* voiceTools;
      InstrumentsDialog* instrList         { 0 };
      MeasuresDialog* measuresDialog       { 0 };
      InsertMeasuresDialog* insertMeasuresDialog { 0 };
      MasterPalette* masterPalette         { 0 };
      PluginCreator* _pluginCreator        { 0 };
      PluginManager* pluginManager         { 0 };
      SelectionWindow* selectionWindow     { 0 };

      QMenu* menuFile;
      QMenu* openRecent;
      QMenu* menuEdit;
      QMenu* menuView;
      QMenu* menuToolbars;
      QMenu* menuWorkspaces;

      QMenu* menuAdd;
      QMenu* menuAddMeasures;
      QMenu* menuAddFrames;
      QMenu* menuAddText;
      QMenu* menuAddLines;
      QMenu* menuAddPitch;
      QMenu* menuAddInterval;
      QMenu* menuTuplet;

      QMenu* menuFormat;
      QMenu* menuTools;
      QMenu* menuVoices;

      QMenu* menuPlugins;
      QMenu* menuHelp;
      AlbumManager* albumManager           { 0 };

      QWidget* _searchDialog               { 0 };
      QComboBox* searchCombo;

      PlayPanel* playPanel                 { 0 };
      Mixer* mixer                         { 0 };
      SynthControl* synthControl           { 0 };
      Debugger* debugger                   { 0 };
      MeasureListEditor* measureListEdit   { 0 };
      PageSettings* pageSettings           { 0 };

      QWidget* symbolDialog                { 0 };

      PaletteScrollArea* clefPalette       { 0 };
      PaletteScrollArea* keyPalette        { 0 };
      KeyEditor* keyEditor                 { 0 };
      ChordStyleEditor* chordStyleEditor   { 0 };
      QStatusBar* _statusBar;
      QLabel* _modeText;
      QLabel* _positionLabel;
      NewWizard* newWizard           { 0 };
      HelpBrowser* helpBrowser       { 0 };
      QDockWidget* manualDock        { 0 };

      PaletteBox* paletteBox         { 0 };
      Inspector* _inspector          { 0 };
      OmrPanel* omrPanel             { 0 };
      QWidget* lastFocusWidget       { 0 };

      QPushButton* showMidiImportButton {0};

      bool _midiinEnabled            { true };
      QList<QString> plugins;
      ScriptEngine* se               { 0 };
      QString pluginPath;

      void createMenuEntry(PluginDescription*);
      void removeMenuEntry(PluginDescription*);

      QTimer* autoSaveTimer;
      QList<QAction*> pluginActions;
      QSignalMapper* pluginMapper        { 0 };

      PianorollEditor* pianorollEditor   { 0 };
      DrumrollEditor* drumrollEditor     { 0 };
      bool _splitScreen                  { false };
      bool _horizontalSplit              { true  };

      QString rev;

      int _midiRecordId                  { -1 };

      bool _fullscreen                   { false };
      QList<LanguageItem> _languages;

      Startcenter* startcenter             { 0 };
      QWidget* loginDialog                 { 0 };
      UploadScoreDialog* uploadScoreDialog { 0 };
      LoginManager* _loginManager        { 0 };
      QFileDialog* loadScoreDialog       { 0 };
      QFileDialog* saveScoreDialog       { 0 };
      QFileDialog* loadStyleDialog       { 0 };
      QFileDialog* saveStyleDialog       { 0 };
      QFileDialog* saveImageDialog       { 0 };
      QFileDialog* loadChordStyleDialog  { 0 };
      QFileDialog* saveChordStyleDialog  { 0 };
      QFileDialog* loadSfzFileDialog     { 0 };
      QFileDialog* loadBackgroundDialog  { 0 };
      QFileDialog* loadScanDialog        { 0 };
      QFileDialog* loadAudioDialog       { 0 };
      QFileDialog* loadDrumsetDialog     { 0 };
      QFileDialog* loadPluginDialog      { 0 };
      QFileDialog* loadPaletteDialog     { 0 };
      QFileDialog* savePaletteDialog     { 0 };
      QFileDialog* saveDrumsetDialog     { 0 };
      QFileDialog* savePluginDialog      { 0 };

      WorkspaceDialog* _workspaceDialog   { 0 };

      QDialog* editRasterDialog          { 0 };

      QAction* hRasterAction;
      QAction* vRasterAction;

      ToolbarEditor* editToolbars        { 0 };
      QActionGroup* workspaces           { 0 };

      bool inChordEditor                 { false };

      QComboBox* layerSwitch;
      QComboBox* playMode;
      QNetworkAccessManager* _networkManager { 0 };
      QAction* lastCmd                       { 0 };
      const Shortcut* lastShortcut           { 0 };
      QHelpEngine* _helpEngine               { 0 };
      int globalX, globalY;       // current mouse position

      QAction* countInAction;
      QAction* metronomeAction;
      QAction* loopAction;
      QAction* loopInAction;
      QAction* loopOutAction;
      QAction* panAction;

      QLabel* cornerLabel;
      QStringList _recentScores;
      QToolButton* _playButton;

      qreal _physicalDotsPerInch;

      QMessageBox* infoMsgBox;
      TourHandler* _tourHandler { 0 };

      //---------------------

      virtual void closeEvent(QCloseEvent*);
      virtual void dragEnterEvent(QDragEnterEvent*);
      virtual void dropEvent(QDropEvent*);
      virtual void changeEvent(QEvent *e);

      void retranslate();

      void playVisible(bool flag);
      void launchBrowser(const QString whereTo);

      void loadScoreList();
      void editInstrList();
      void symbolMenu();
      void showKeyEditor();
      bool saveFile();
      bool saveFile(MasterScore* score);
      void fingeringMenu();

      int  pluginIdxFromPath(QString pluginPath);
      void startDebugger();
      void midiinToggled(bool);
      void undoRedo(bool undo);
      void showPalette(bool);
      void showInspector(bool);
      void showOmrPanel(bool);
      void showNavigator(bool);
      void showTimeline(bool);
      void showSelectionWindow(bool);
      void showSearchDialog();
      void showToolbarEditor();
      void splitWindow(bool horizontal);
      void removeSessionFile();
      void editChordStyle();
      void startExcerptsDialog();
      void initOsc();
      void editRaster();
      void showPianoKeyboard(bool);
      void showMediaDialog();
      void showAlbumManager();
      void showLayerManager();
      void updateUndoRedo();
      void changeScore(int);
      virtual void resizeEvent(QResizeEvent*);
      void showModeText(const QString&);
      void addRecentScore(const QString& scorePath);

      void updateViewModeCombo();
      void switchLayoutMode(LayoutMode);
      void setPlayRepeats(bool repeat);

      ScoreTab* createScoreTab();

      QString getUtmParameters(QString medium) const;

   signals:
      void windowSplit(bool);

   private slots:
      void cmd(QAction* a, const QString& cmd);
      void autoSaveTimerTimeout();
      void helpBrowser1() const;
      void resetAndRestart();
      void about();
      void aboutQt();
      void aboutMusicXML();
      void reportBug(QString medium);
      void askForHelp();
      void leaveFeedback(QString medium);
      void openRecentMenu();
      void selectScore(QAction*);
      void startPreferenceDialog();
      void preferencesChanged(bool fromWorkspace = false);
      void seqStarted();
      void seqStopped();
      void cmdAppendMeasures();
      void cmdInsertMeasures();
      void magChanged(MagIdx);
      void showPageSettings();
      void removeTab(int);
      void removeTab();
      void clipboardChanged();
      void inputMethodLocaleChanged();
      void endSearch();
      void saveScoreDialogFilterSelected(const QString&);
#ifdef OSC
      void oscIntMessage(int);
      void oscVolume(int val);
      void oscTempo(int val);
      void oscGoto(int m);
      void oscSelectMeasure(int m);
      void oscVolChannel(double val);
      void oscPanChannel(double val);
      void oscMuteChannel(double val);
      void oscOpen(QString path);
      void oscCloseAll();
      void oscTriggerPlugin(QString list);
      void oscColorNote(QVariantList list);
      void oscAction();
#endif
      void deleteWorkspace();
      void undoWorkspace();
      void showWorkspaceMenu();
      void switchLayer(const QString&);
      void switchPlayMode(int);
      void networkFinished();
      void switchLayoutMode(int);
      void showMidiImportPanel();
      void changeWorkspace(QAction*);
      void onLongOperationFinished();

      virtual QMenu* createPopupMenu() override;

   public slots:
      virtual void cmd(QAction* a);
      void dirtyChanged(Score*);
      void setPos(int tick);
      void pluginTriggered(int);
      void handleMessage(const QString& message);
      void setCurrentScoreView(ScoreView*);
      void setCurrentScoreView(int);
      void setCurrentScores(Score* s1, Score* s2 = nullptr);
      void setNormalState()    { changeState(STATE_NORMAL); }
      void setPlayState()      { changeState(STATE_PLAY); }
      void setNoteEntryState() { changeState(STATE_NOTE_ENTRY); }
      void checkForUpdate();
      void checkForExtensionsUpdate();
      void midiNoteReceived(int channel, int pitch, int velo);
      void midiNoteReceived(int pitch, bool ctrl, int velo);
      void instrumentChanged();
      void showMasterPalette(const QString& = 0);
      void selectionChanged(SelState);
      void createNewWorkspace();
      void editWorkspace();
      void changeWorkspace(Workspace* p, bool first=false);
      void mixerPreferencesChanged(bool showMidiControls);

   public:
      MuseScore();
      ~MuseScore();
      bool checkDirty(MasterScore*);
      PlayPanel* getPlayPanel() const { return playPanel; }
      Mixer* getMixer() const { return mixer; }
      QMenu* genCreateMenu(QWidget* parent = 0);
      virtual int appendScore(MasterScore*);
      void midiCtrlReceived(int controller, int value);
      void showElementContext(Element* el);
      void cmdAppendMeasures(int);
      bool midiinEnabled() const;

      void incMag();
      void decMag();
      void readSettings();
      void writeSettings();
      void play(Element* e) const;
      void play(Element* e, int pitch) const;
      bool loadPlugin(const QString& filename);
      QString createDefaultName() const;
      void startAutoSave();
      double getMag(ScoreView*) const;
      void setMag(double);
      bool noScore() const { return scoreList.isEmpty(); }

      TextTools* textTools();
      void showDrumTools(const Drumset*, Staff*);
      void updateDrumTools(const Drumset* ds);
      void showPluginCreator(QAction*);
      void showPluginManager();

//      void updateTabNames();
      QProgressBar* showProgressBar();
      void hideProgressBar();
      void addRecentScore(Score*);
      QFileDialog* saveAsDialog();
      QFileDialog* saveCopyDialog();

      QString lastSaveCopyDirectory;
      QString lastSaveCopyFormat;
      QString lastSaveDirectory;
      QString lastSaveCaptureName;
      SynthControl* getSynthControl() const       { return synthControl; }
      void editInPianoroll(Staff* staff, Position* p = 0);
      void editInDrumroll(Staff* staff);
      PianorollEditor* getPianorollEditor() const { return pianorollEditor; }
      DrumrollEditor* getDrumrollEditor() const   { return drumrollEditor; }
      PianoTools* pianoTools() const              { return _pianoTools; }
      PluginManager* getPluginManager() const     { return pluginManager; }
      void writeSessionFile(bool);
      bool restoreSession(bool);
      bool splitScreen() const { return _splitScreen; }
      void setSplitScreen(bool val);
      virtual void setCurrentView(int tabIdx, int idx);
      void loadPlugins();
      void unloadPlugins();

      ScoreState state() const { return _sstate; }
      void changeState(ScoreState);
      void updateInputState(Score*);
      void updateShadowNote();

      bool readLanguages(const QString& path);
      void setRevision(QString& r)  {rev = r;}
      Q_INVOKABLE QString revision()            {return rev;}
      Q_INVOKABLE QString version()            {return VERSION;}
      Q_INVOKABLE void newFile();
      MasterScore* getNewFile();
      Q_INVOKABLE void loadFile(const QString& url);
      void loadFile(const QUrl&);
      QTemporaryFile* getTemporaryScoreFileCopy(const QFileInfo& info, const QString& baseNameTemplate);
      QNetworkAccessManager* networkManager();
      virtual Score* openScore(const QString& fn, bool switchTab = true);
      bool hasToCheckForUpdate();
      bool hasToCheckForExtensionsUpdate();
      static bool unstable();
      bool eventFilter(QObject *, QEvent *);
      void setMidiRecordId(int id) { _midiRecordId = id; }
      int midiRecordId() const { return _midiRecordId; }
      void setDefaultPalette();
      void scorePageLayoutChanged();
      bool processMidiRemote(MidiRemoteType type, int data, int value);
      ScoreTab* getTab1() const { return tab1; }
      ScoreTab* getTab2() const { return tab2; }
      QList<LanguageItem>& languages() { return _languages; }

      QStringList getOpenScoreNames(const QString& filter, const QString& title, bool singleFile = false);
      QString getSaveScoreName(const QString& title, QString& name, const QString& filter, bool folder = false);
      QString getStyleFilename(bool open, const QString& title = QString());
      QString getFotoFilename(QString& filter, QString *selectedFilter);
      QString getChordStyleFilename(bool open);
      QString getScanFile(const QString&);
      QString getAudioFile(const QString&);
      QString getDrumsetFilename(bool open);
      QString getPluginFilename(bool open);
      QString getPaletteFilename(bool open, const QString& name = "");
      QString getWallpaper(const QString& caption);

      bool hRaster() const { return hRasterAction->isChecked(); }
      bool vRaster() const { return vRasterAction->isChecked(); }

      PaletteBox* getPaletteBox();
      void disableCommands(bool val) { inChordEditor = val; }

      Tuplet* tupletDialog();
      void selectSimilar(Element*, bool);
      void selectSimilarInRange(Element* e);
      void selectElementDialog(Element* e);
      void transpose();

      Q_INVOKABLE void openExternalLink(const QString&);

      virtual void endCmd() override;
      void printFile();
      void exportFile();
      bool exportParts();
      virtual bool saveAs(Score*, bool saveCopy, const QString& path, const QString& ext);
      bool savePdf(const QString& saveName);
      bool savePdf(Score* cs, const QString& saveName);
      bool savePdf(QList<Score*> cs, const QString& saveName);


      MasterScore* readScore(const QString& name);

      bool saveAs(Score*, bool saveCopy = false);
      bool saveSelection(Score*);
      void addImage(Score*, Element*);

      bool savePng(Score*, const QString& name, bool screenshot, bool transparent, double convDpi, int trimMargin, QImage::Format format);
      bool saveAudio(Score*, QIODevice *device, std::function<bool(float)> updateProgress = nullptr);
      bool saveAudio(Score*, const QString& name);
      bool canSaveMp3();
      bool saveMp3(Score*, const QString& name);
      bool saveSvg(Score*, const QString& name);
      bool savePng(Score*, const QString& name);
      bool saveMidi(Score* score, const QString& name);

      virtual void closeScore(Score* score);

      void addTempo();
      void addMetronome();

      SynthesizerState synthesizerState();

      Q_INVOKABLE QString getLocaleISOCode() const;
      Navigator* navigator() const;
      NScrollArea* navigatorScrollArea() const { return _navigator; }
      Timeline* timeline() const;
      TDockWidget* timelineScrollArea() const { return _timeline; }
      QWidget*   searchDialog() const;
      SelectionWindow* getSelectionWindow() const { return selectionWindow; }
      void updateLayer();
      void updatePlayMode();
      bool loop() const              { return loopAction->isChecked(); }
      bool metronome() const         { return metronomeAction->isChecked(); }
      bool countIn() const           { return countInAction->isChecked(); }
      bool panDuringPlayback() const { return panAction->isChecked(); }
      void noteTooShortForTupletDialog();
      void loadFiles(bool switchTab = true, bool singleFile = false);
                  // midi panel functions
      void midiPanelOnSwitchToFile(const QString &file);
      void midiPanelOnCloseFile(const QString &file);
      void allowShowMidiPanel(const QString &file);
      void setMidiReopenInProgress(const QString &file);

      static Palette* newTempoPalette(bool defaultPalette = false);
      static Palette* newTextPalette();
      static Palette* newTimePalette();
      static Palette* newRepeatsPalette();
      static Palette* newBreaksPalette();
      static Palette* newBeamPalette();
      static Palette* newDynamicsPalette(bool defaultPalette = false);
      static Palette* newFramePalette();
      static Palette* newFingeringPalette();
      static Palette* newTremoloPalette();
      static Palette* newNoteHeadsPalette();
      static Palette* newArticulationsPalette();
      static Palette* newOrnamentsPalette();
      static Palette* newAccordionPalette();
      static Palette* newBracketsPalette();
      static Palette* newBreathPalette();
      static Palette* newArpeggioPalette();
      static Palette* newClefsPalette(bool defaultPalette = false);
      static Palette* newGraceNotePalette();
      static Palette* newBagpipeEmbellishmentPalette();
      static Palette* newKeySigPalette();
      static Palette* newAccidentalsPalette(bool defaultPalette = false);
      static Palette* newBarLinePalette();
      static Palette* newLinesPalette();
      static Palette* newFretboardDiagramPalette();

      WorkspaceDialog* workspaceDialog() { return _workspaceDialog; }
      void updateIcons();

      Inspector* inspector()           { return _inspector; }
      PluginCreator* pluginCreator()   { return _pluginCreator; }
      ScoreView* currentScoreView() const { return cv; }
      QToolButton* playButton()        { return _playButton;    }
      void showMessage(const QString& s, int timeout);
      void showHelp(QString);
      void showContextHelp();
      void showHelp(const QUrl&);

      TourHandler* tourHandler()       { return _tourHandler; }

      void registerPlugin(PluginDescription*);
      void unregisterPlugin(PluginDescription*);

      Q_INVOKABLE void showStartcenter(bool);
      void showPlayPanel(bool);

      QFileInfoList recentScores() const;
      void saveDialogState(const char* name, QFileDialog* d);
      void restoreDialogState(const char* name, QFileDialog* d);

      QPixmap extractThumbnail(const QString& name);

      void showLoginDialog();
      void showUploadScoreDialog();
      LoginManager* loginManager()     { return _loginManager; }
      QHelpEngine*  helpEngine() const { return _helpEngine;   }

      virtual void updateInspector() override;
      void updateInstrumentDialog();
      void reloadInstrumentTemplates();
      void showSynthControl(bool);
      void showMixer(bool);

      qreal physicalDotsPerInch() const                              { return _physicalDotsPerInch; }
      static const std::list<const char*>& allNoteInputMenuEntries() { return _allNoteInputMenuEntries; }
      std::list<const char*>* noteInputMenuEntries()                 { return &_noteInputMenuEntries; }
      void setNoteInputMenuEntries(std::list<const char*> l)         { _noteInputMenuEntries = l; }
      void populateNoteInputMenu();

      static const std::list<const char*>& allFileOperationEntries() { return _allFileOperationEntries; }
      std::list<const char*>* fileOperationEntries()                 { return &_fileOperationEntries; }
      void setFileOperationEntries(std::list<const char*> l)         { _fileOperationEntries = l; }
      void populateFileOperations();

      static const std::list<const char*>& allPlaybackControlEntries() { return _allPlaybackControlEntries; }
      std::list<const char*>* playbackControlEntries()               { return &_playbackControlEntries; }
      void setPlaybackControlEntries(std::list<const char*> l)       { _playbackControlEntries = l; }
      void populatePlaybackControls();

      static void updateUiStyleAndTheme();

      void showError();

      static void saveGeometry(QWidget const*const qw);
      static void restoreGeometry(QWidget*const qw);

      void updateWindowTitle(Score* score);
      bool importExtension(QString path);
      bool uninstallExtension(QString extensionId);
      Q_INVOKABLE bool isInstalledExtension(QString extensionId);
      };

extern MuseScore* mscore;
extern QStringList recentScores;
extern QString dataPath;
extern MasterSynthesizer* synti;
MasterSynthesizer* synthesizerFactory();
Driver* driverFactory(Seq*, QString driver);

extern QAction* getAction(const char*);
extern Shortcut* midiActionMap[128];
extern void loadTranslation(QString fileName, QString localeName);
extern void setMscoreLocale(QString localeName);
extern bool saveMxl(Score*, const QString& name);
extern bool saveXml(Score*, const QString& name);

struct PluginDescription;
extern void collectPluginMetaInformation(PluginDescription*);
extern QString getSharePath();

extern Score::FileError importMidi(MasterScore*, const QString& name);
extern Score::FileError importGTP(MasterScore*, const QString& name);
extern Score::FileError importBww(MasterScore*, const QString& path);
extern Score::FileError importMusicXml(MasterScore*, const QString&);
extern Score::FileError importCompressedMusicXml(MasterScore*, const QString&);
extern Score::FileError importMuseData(MasterScore*, const QString& name);
extern Score::FileError importLilypond(MasterScore*, const QString& name);
extern Score::FileError importBB(MasterScore*, const QString& name);
extern Score::FileError importCapella(MasterScore*, const QString& name);
extern Score::FileError importCapXml(MasterScore*, const QString& name);
extern Score::FileError readScore(MasterScore* score, QString name, bool ignoreVersionError);

} // namespace Ms

extern Ms::Score::FileError importOve(Ms::MasterScore*, const QString& name);

#endif
