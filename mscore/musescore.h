//=============================================================================
//  MuseScore
//  Music Composition & Notation
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
#include "singleapp/src/QtSingleApplication"
#include "updatechecker.h"
#include "libmscore/musescoreCore.h"
#include "libmscore/score.h"
#include "sessionstatusobserver.h"

namespace Ms {

class UploadScoreDialog;
class LoginManager;
class AboutBoxDialog;
class AboutMusicXMLBoxDialog;
class InsertMeasuresDialog;
class MeasuresDialog;
class Shortcut;
class ScoreView;
class Element;
class PreferenceDialog;
class InstrumentsDialog;
class Instrument;
class MidiFile;
class TextStyleDialog;
class PlayPanel;
class IPlayPanel;
class Mixer;
class Debugger;
class MeasureListEditor;
class MasterScore;
class Score;
class Tuplet;
class PageSettings;
class Palette;
class PaletteScrollArea;
class SelectionWindow;
class XmlWriter;
class ZoomBox;
class NewWizard;
class ExcerptsDialog;
class ExportDialog;
class SynthControl;
class PianorollEditor;
class DrumrollEditor;
class Staff;
class ScoreTab;
class Drumset;
class TextTools;
class DrumTools;
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
class MsQmlEngine;
#ifdef SCRIPT_INTERFACE
class PluginManager;
class QmlPluginEngine;
#endif
class MasterSynthesizer;
class SynthesizerState;
class Driver;
class Seq;
class ImportMidiPanel;
class ScoreComparisonTool;
class ScriptRecorder;
class ScriptRecorderWidget;
class Startcenter;
class HelpBrowser;
class ToolbarEditor;
class TourHandler;
class GeneralAutoUpdater;
class EditStyle;

class PalettePanel;
struct PaletteTree;
class PaletteWidget;
class PaletteWorkspace;
class QmlDockWidget;

struct PluginDescription;
enum class SelState : char;
enum class IconType : signed char;
enum class ZoomIndex : char;

extern QString mscoreGlobalShare;
extern QString revision;
static const int PROJECT_LIST_LEN = 6;
extern const char* voiceActions[];
extern bool mscoreFirstStart;

QString localeName();

using NotesColors = QHash<int /* noteIndex */, QColor>;

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
            handbook = QString();
            }
      LanguageItem(const QString k, const QString n, const QString h) {
            key = k;
            name = n;
            handbook = h;
            }
      };

//---------------------------------------------------------
//   SaveReplacePolicy
//---------------------------------------------------------

enum class SaveReplacePolicy {
      NO_CHOICE,
      SKIP_ALL,
      REPLACE_ALL
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

      struct CommandLineParseResult {
            QStringList argv;
            bool exit = false;
            };
      static CommandLineParseResult parseCommandLineArguments(MuseScoreApplication* app);
      static MuseScoreApplication* initApplication(int& argc, char** argv);

      static bool setCustomConfigFolder(const QString& path);
      };


//---------------------------------------------------------
//   MuseScore
//---------------------------------------------------------

class MuseScore : public QMainWindow, public MuseScoreCore {
      Q_OBJECT

      QSettings settings;
      ScoreView* cv                        { 0 };
      ScoreTab* ctab                       { 0 };
      QMap<MasterScore*, bool> scoreWasShown; // whether each score in scoreList has ever been shown
      ScoreState _sstate;
      UpdateChecker* ucheck;
      ExtensionsUpdateChecker* packUChecker = nullptr;

      static const std::list<const char*> _allNoteInputMenuEntries;
      std::list<const char*> _noteInputMenuEntries { _allNoteInputMenuEntries };

      static const std::list<const char*> _allFileOperationEntries;
      std::list<const char*> _fileOperationEntries { _allFileOperationEntries };

      static const std::list<const char*> _allPlaybackControlEntries;
      std::list<const char*> _playbackControlEntries { _allPlaybackControlEntries };

      bool _playPartOnly = true; // play part only vs. full score

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
      ScriptRecorderWidget* scriptRecorder { nullptr };

      ZoomBox* zoomBox                     { nullptr };
      QComboBox* viewModeCombo             { nullptr };
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
      QToolBar* workspacesTools;
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
#ifdef SCRIPT_INTERFACE
      PluginManager* pluginManager         { 0 };
      QmlPluginEngine* _qmlEngine          { 0 };
#endif
      MsQmlEngine* _qmlUiEngine            { 0 };
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
      QMenu* menuStretch;
      QMenu* menuTools;
      QMenu* menuVoices;
      QMenu* menuMeasure;

      QMenu* menuPlugins;
      QMenu* menuHelp;
      QMenu* menuTours;
#ifndef NDEBUG
      QMenu* menuDebug;
#endif
      AlbumManager* albumManager           { 0 };
      ExportDialog* exportDialog           { 0 };

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

      PaletteWorkspace* paletteWorkspace { nullptr };
      PaletteWidget* paletteWidget { nullptr };

      Inspector* _inspector          { 0 };
      OmrPanel* omrPanel             { 0 };

      QPushButton* showMidiImportButton {0};

      QList<QString> plugins;
      QString pluginPath;

#ifdef SCRIPT_INTERFACE
      void createMenuEntry(PluginDescription*);
      void removeMenuEntry(PluginDescription*);
#endif

      QTimer* autoSaveTimer;
      QList<QAction*> pluginActions;

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

      WorkspaceDialog* _workspaceDialog  { 0 };

      EditStyle* _styleDlg                { nullptr };

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

      QWindow* _lastFocusWindow { nullptr };
      bool _lastFocusWindowIsQQuickView { false };

      std::unique_ptr<GeneralAutoUpdater> autoUpdater;

      SessionStatusObserver sessionStatusObserver;

      //---------------------

      virtual void closeEvent(QCloseEvent*);
      virtual void dragEnterEvent(QDragEnterEvent*);
      virtual void dropEvent(QDropEvent*);
      virtual void changeEvent(QEvent *e);
      virtual void showEvent(QShowEvent *event);

      void retranslate();
      void setMenuTitles();
      void updateMenu(QMenu*& menu, QString menu_id, QString objectName);

      void playVisible(bool flag);
      void launchBrowser(const QString whereTo);

      void loadScoreList();
      void editInstrList();
      void symbolMenu();
      void showKeyEditor();
      bool saveFile(MasterScore* score);
      void fingeringMenu();

#ifdef SCRIPT_INTERFACE
      int  pluginIdxFromPath(QString pluginPath);
#endif
      void startDebugger();
      void enableMidiIn(bool);
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
      void showExportDialog(const QString& type = "");
      void initOsc();
      void editRaster();
      void showPianoKeyboard(bool);
      void showMediaDialog();
      void showAlbumManager();
      void showLayerManager();
      void updateUndoRedo();
      void changeScore(int);
      virtual void resizeEvent(QResizeEvent*);
      void showModeText(const QString& s, bool informScreenReader = true);
      void addRecentScore(const QString& scorePath);

      void setZoom(const ZoomIndex, const qreal logicalFreeZoomLevel = 0.0);
      void setZoomWithToggle(const ZoomIndex index);
      void zoomBySteps(const qreal numSteps);
      void zoomAndSavePrevious(const std::function<void(void)>& zoomFunction);

      void updateViewModeCombo();

      void switchLayoutMode(LayoutMode);
      void setPlayRepeats(bool repeat);
      void setPanPlayback(bool pan);

      void createPlayPanel();

      ScoreTab* createScoreTab();
      void askMigrateScore(Score* score);

      QString getUtmParameters(QString medium) const;

      void checkForUpdatesNoUI();

      void doLoadFiles(const QStringList& filter, bool switchTab, bool singleFile);

      void askAboutApplyingEdwinIfNeed(const QString& fileSuffix);

   signals:
      void windowSplit(bool);
      void musescoreWindowWasShown();
      void workspacesChanged();
      void scoreStateChanged(ScoreState state);

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
      void preferencesChanged(bool fromWorkspace = false, bool changeUI = true);
      void seqStarted();
      void seqStopped();
      void cmdAppendMeasures();
      void cmdInsertMeasures();
      void zoomBoxChanged(const ZoomIndex, const qreal);
      void showPageSettings();
      void removeTab(int);
      void removeTab();
      void clipboardChanged();
      void inputMethodAnchorRectangleChanged();
      void inputMethodAnimatingChanged();
      void inputMethodCursorRectangleChanged();
      void inputMethodInputDirectionChanged(Qt::LayoutDirection newDirection);
      void inputMethodInputItemClipRectangleChanged();
      void inputMethodKeyboardRectangleChanged();
      void inputMethodLocaleChanged();
      void inputMethodVisibleChanged();
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
      void resetWorkspace();
      void showWorkspaceMenu();
      void switchLayer(const QString&);
      void switchPlayMode(int);
      void networkFinished();
      void switchLayoutMode(int);
      void showMidiImportPanel();
      void changeWorkspace(QAction*);
      void onLongOperationFinished();

      void onFocusWindowChanged(QWindow*);

      virtual QMenu* createPopupMenu() override;

      QByteArray exportMsczAsJSON(Score*);
      QByteArray exportPdfAsJSON(Score*);

   public slots:
      virtual void cmd(QAction* a);
      void dirtyChanged(Score*);
      void setPos(const Fraction& tick);
      void pluginTriggered(int);
      void pluginTriggered(QString path);
      void handleMessage(const QString& message);
      void setCurrentScoreView(ScoreView*);
      void setCurrentScoreView(int);
      void setCurrentScores(Score* s1, Score* s2 = nullptr);
      void setNormalState()    { changeState(STATE_NORMAL); }
      void setPlayState()      { changeState(STATE_PLAY); }
      void setNoteEntryState() { changeState(STATE_NOTE_ENTRY); }
      void checkForUpdatesUI();
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
      void checkForUpdates();
      void restartAudioEngine();

   public:
      MuseScore();
      ~MuseScore();
      bool checkDirty(MasterScore*);
      IPlayPanel* playPanelInterface() const;
      PlayPanel* getPlayPanel() const { return playPanel; }
      Mixer* getMixer() const { return mixer; }
      QMenu* genCreateMenu(QWidget* parent = 0);
      virtual int appendScore(MasterScore*);
      void midiCtrlReceived(int controller, int value);
      void showElementContext(Element* el);
      void cmdAppendMeasures(int);
      bool isMidiInEnabled() const;

      void readSettings();
      void writeSettings();
      void play(Element* e) const;
      void play(Element* e, int pitch) const;
      void moveControlCursor();
      bool loadPlugin(const QString& filename);
      QString createDefaultName() const;
      void startAutoSave();
      void updateZoomBox(const ZoomIndex, const qreal logicalLevel);
      bool noScore() const { return scoreList.isEmpty(); }

      TextTools* textTools();
      void showDrumTools(const Drumset*, Staff*);
      void updateDrumTools(const Drumset* ds);
      void showPluginCreator(QAction*);
      void showPluginManager();

//      void updateTabNames();
      void updatePaletteBeamMode();
      QProgressBar* showProgressBar();
      void hideProgressBar();
      void addRecentScore(Score*);
      QFileDialog* saveAsDialog();
      QFileDialog* saveCopyDialog();
      void showStyleDialog(Element* e = nullptr);

      QString lastSaveCopyDirectory;
      QString lastSaveCopyFormat;
      QString lastSaveDirectory;
      QString lastSaveCaptureName;
      bool saveFile();
      SynthControl* getSynthControl() const       { return synthControl; }
      void editInPianoroll(Staff* staff, Position* p = 0);
      void editInDrumroll(Staff* staff);
      PianorollEditor* getPianorollEditor() const { return pianorollEditor; }
      DrumrollEditor* getDrumrollEditor() const   { return drumrollEditor; }
      PianoTools* pianoTools() const              { return _pianoTools; }
#ifdef SCRIPT_INTERFACE
      PluginManager* getPluginManager() const     { return pluginManager; }
      QmlPluginEngine* getPluginEngine();
#endif
      MsQmlEngine* getQmlUiEngine();
      void writeSessionFile(bool);
      bool restoreSession(bool);
      bool splitScreen() const { return _splitScreen; }
      void setSplitScreen(bool val);
      virtual void setCurrentView(int tabIdx, int idx);
      void loadPlugins();
      void unloadPlugins();
#ifdef SCRIPT_INTERFACE
      void addPluginMenuEntries();
#endif

      ScoreState state() const { return _sstate; }
      void changeState(ScoreState);
      void updateInputState(Score*);
      void updateShadowNote();

      bool readLanguages(const QString& path);
      void setRevision(QString& r)  {rev = r;}
      Q_INVOKABLE QString revision()            {return rev;}
      Q_INVOKABLE QString version()            {return VERSION;}
      static QString fullVersion();
      Q_INVOKABLE void newFile();
      MasterScore* getNewFile();
      Q_INVOKABLE void loadFile(const QString& url);
      void loadFile(const QUrl&);
      QTemporaryFile* getTemporaryScoreFileCopy(const QFileInfo& info, const QString& baseNameTemplate);
      QNetworkAccessManager* networkManager();
      virtual Score* openScore(const QString& fn, bool switchTab = true, const bool appendToExistingTabs = true, const QString& withFilename = "") override;
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
      QString getSaveScoreName(const QString& title, QString& name, const QString& filter, bool folder = false, bool askOverwrite = true);
      QString getStyleFilename(bool open, const QString& title = QString());
      QString getFotoFilename(QString& filter, QString *selectedFilter);
      QString getChordStyleFilename(bool open);
      QString getScanFile(const QString&);
      QString getAudioFile(const QString&);
      QString getDrumsetFilename(bool open);
      QString getPluginFilename(bool open);
      QString getPaletteFilename(bool open, const QString& name = "");
      QString getWallpaper(const QString& caption);
      
      int askOverwriteAll(QString&);

      bool hRaster() const { return hRasterAction->isChecked(); }
      bool vRaster() const { return vRasterAction->isChecked(); }

      PaletteWorkspace* getPaletteWorkspace();
      PaletteWidget* getPaletteWidget() { return paletteWidget; }
      std::vector<QmlDockWidget*> qmlDockWidgets();
      void changeWorkspace(const QString& name);

      void disableCommands(bool val) { inChordEditor = val; }

      Tuplet* tupletDialog();
      void selectSimilar(Element*, bool);
      void selectSimilarInRange(Element* e);
      void selectElementDialog(Element* e);
      void transpose();
      void realizeChordSymbols();

      Q_INVOKABLE void openExternalLink(const QString&);

      void endCmd(bool undoRedo);
      void endCmd() override { endCmd(false); };
      void printFile();
      virtual bool saveAs(Score*, bool saveCopy, const QString& path, const QString& ext, SaveReplacePolicy* replacePolicy = nullptr);
      QString saveFilename(QString fn);
      bool savePdf(const QString& saveName);
      bool savePdf(Score* cs, const QString& saveName);
      bool savePdf(QList<Score*> cs, const QString& saveName);
      bool savePdf(Score* cs, QPrinter& printer);


      MasterScore* readScore(const QString& name);
      NotesColors readNotesColors(const QString& filePath) const;

      bool saveAs(Score*, bool saveCopy = false);
      bool saveSelection(Score*);
      void addImage(Score*, Element*);

      bool saveAudio(Score*, QIODevice*, std::function<bool(float)> updateProgress = nullptr);
      bool saveAudio(Score*, const QString& name);
      bool canSaveMp3();
      bool saveMp3(Score*, const QString& name);
      bool saveMp3(Score*, QIODevice*, bool& wasCanceled);
      bool saveSvg(Score*, const QString& name, const NotesColors& notesColors = NotesColors(), SaveReplacePolicy* replacePolicy = nullptr);
      bool saveSvg(Score*, QIODevice*, int pageNum = 0, bool drawPageBackground = false, const NotesColors& notesColors = NotesColors());
      bool savePng(Score*, const QString& name, SaveReplacePolicy* replacePolicy = nullptr);
      bool savePng(Score*, QIODevice*, int pageNum = 0, bool drawPageBackground = false);
      bool saveMidi(Score*, const QString& name);
      bool saveMidi(Score*, QIODevice*);
      bool savePositions(Score*, const QString& name, bool segments);
      bool savePositions(Score*, QIODevice*, bool segments);
      bool saveMetadataJSON(Score*, const QString& name);
      QJsonObject saveMetadataJSON(Score*);
      bool saveOnline(const QStringList& inFilePaths);

      /////The methods are used in the musescore.com backend
      bool exportAllMediaFiles(const QString& inFilePath, const QString& highlightConfigPath, const QString& outFilePath = "/dev/stdout");
      bool exportScoreMetadata(const QString& inFilePath, const QString& outFilePath = "/dev/stdout");
      bool exportMp3AsJSON(const QString& inFilePath, const QString& outFilePath = "/dev/stdout");
      bool saveScoreParts(const QString& inFilePath, const QString& outFilePath = "/dev/stdout");
      bool exportPartsPdfsToJSON(const QString& inFilePath, const QString& outFilePath = "/dev/stdout");
      bool exportTransposedScoreToJSON(const QString& inFilePath, const QString& transposeOptions, const QString& outFilePath = "/dev/stdout");
      bool updateSource(const QString& scorePath, const QString& newSource);
      /////////////////////////////////////////////////

      void scoreUnrolled(MasterScore* original);
      
      virtual void closeScore(Score* score);

      void addTempo();
      void addMetronome();

      void editInstrumentList();

      SynthesizerState synthesizerState() const;
      static Synthesizer* synthesizer(const QString& name);

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
      void openFiles(bool switchTab = true, bool singleFile = false);
      void importScore(bool switchTab = true, bool singleFile = false);
                  // midi panel functions
      void midiPanelOnSwitchToFile(const QString &file);
      void midiPanelOnCloseFile(const QString &file);
      void allowShowMidiPanel(const QString &file);
      void setMidiReopenInProgress(const QString &file);

      static Palette* newTempoPalette(bool defaultPalette = false);
      static Palette* newTextPalette(bool defaultPalette = false);
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

      static PalettePanel* newTempoPalettePanel(bool defaultPalette = false);
      static PalettePanel* newTextPalettePanel(bool defaultPalette = false);
      static PalettePanel* newTimePalettePanel();
      static PalettePanel* newRepeatsPalettePanel();
      static PalettePanel* newBreaksPalettePanel();
      static PalettePanel* newBeamPalettePanel();
      static PalettePanel* newDynamicsPalettePanel(bool defaultPalette = false);
      static PalettePanel* newFramePalettePanel();
      static PalettePanel* newFingeringPalettePanel();
      static PalettePanel* newTremoloPalettePanel();
      static PalettePanel* newNoteHeadsPalettePanel();
      static PalettePanel* newArticulationsPalettePanel();
      static PalettePanel* newOrnamentsPalettePanel();
      static PalettePanel* newAccordionPalettePanel();
      static PalettePanel* newBracketsPalettePanel();
      static PalettePanel* newBreathPalettePanel();
      static PalettePanel* newArpeggioPalettePanel();
      static PalettePanel* newClefsPalettePanel(bool defaultPalette = false);
      static PalettePanel* newGraceNotePalettePanel();
      static PalettePanel* newBagpipeEmbellishmentPalettePanel();
      static PalettePanel* newKeySigPalettePanel();
      static PalettePanel* newAccidentalsPalettePanel(bool defaultPalette = false);
      static PalettePanel* newBarLinePalettePanel();
      static PalettePanel* newLinesPalettePanel();
      static PalettePanel* newFretboardDiagramPalettePanel();
      static PaletteTree* newMasterPaletteTree();

      WorkspaceDialog* workspaceDialog() { return _workspaceDialog; }
      void updateIcons();
      void updateMenus();

      Inspector* inspector()           { return _inspector; }
      PluginCreator* pluginCreator()   { return _pluginCreator; }
      ScoreView* currentScoreView() const { return cv; }
      ScoreTab* currentScoreTab() const { return ctab; }
      QToolButton* playButton()        { return _playButton;    }
      void showMessage(const QString& s, int timeout);
      void showHelp(QString);
      void showContextHelp();
      void showHelp(const QUrl&);

      TourHandler* tourHandler()       { return _tourHandler; }

      bool isModalDialogOpen() { return QApplication::activeModalWidget() != nullptr || QApplication::modalWindow() != nullptr; }

#ifdef SCRIPT_INTERFACE
      void registerPlugin(PluginDescription*);
      void unregisterPlugin(PluginDescription*);
#endif

      Q_INVOKABLE void showStartcenter(bool);
      void reDisplayDockWidget(QDockWidget* widget, bool visible);
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

      bool playPartOnly() const { return _playPartOnly; }
      void setPlayPartOnly(bool val);

      static void updateUiStyleAndTheme();

      void showError();

      static void saveGeometry(QWidget const*const qw);
      static void restoreGeometry(QWidget*const qw);

      void updateWindowTitle(Score* score);
      bool importExtension(QString path);
      bool uninstallExtension(QString extensionId);
      Q_INVOKABLE bool isInstalledExtension(QString extensionId);

      void focusScoreView();

      void notifyElementDraggedToScoreView();

      ScriptRecorder* getScriptRecorder();
      bool runTestScripts(const QStringList& scripts);

      static void init(QStringList& argv);

      friend class TestWorkspaces;
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
extern bool saveMxl(Score*, QIODevice*);
extern bool saveXml(Score*, QIODevice*);
extern bool saveXml(Score*, const QString& name);

extern QString getSharePath();

#ifdef AVSOMR
extern Score::FileError importMSMR(MasterScore*, const QString& name);
extern Score::FileError loadAndImportMSMR(MasterScore*, const QString& name);
#endif
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

int runApplication(int& argc, char** argv);
} // namespace Ms

extern Ms::Score::FileError importOve(Ms::MasterScore*, const QString& name);

#endif
