//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: musescore.h 5657 2012-05-21 15:46:06Z lasconic $
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

#ifndef __MUSESCORE_H__
#define __MUSESCORE_H__

#include "config.h"
#include "globals.h"
#include "ui_measuresdialog.h"
#include "ui_insertmeasuresdialog.h"
#include "ui_aboutbox.h"
#include "ui_aboutmusicxmlbox.h"
#include "ui_startdialog.h"
#include "singleapp/src/QtSingleApplication"
#include "updatechecker.h"
#include "musescoreCore.h"

namespace Ms {

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
class Score;
class Tuplet;
class PageSettings;
class PaletteBox;
class Palette;
class PaletteScrollArea;
class Xml;
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
class PianoTools;
class MediaDialog;
class Workspace;
class AlbumManager;
class WebPageDockWidget;
class ChordList;
class Capella;
class Inspector;
class OmrPanel;
class NScrollArea;
class Sym;
class MasterPalette;
class PluginCreator;
class PluginManager;
class MasterSynthesizer;
class Driver;
class Seq;
class ImportMidiPanel;

struct PluginDescription;

extern QString mscoreGlobalShare;
static const int PROJECT_LIST_LEN = 6;

//---------------------------------------------------------
//   IconActions
//---------------------------------------------------------

struct IconAction {
      int subtype;
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
//   StartDialog
//---------------------------------------------------------

class StartDialog : public QDialog, public Ui::StartDialog {
      Q_OBJECT

   private slots:
      void createScoreClicked();
      void loadScoreClicked();

   public:
      StartDialog(QWidget* parent = 0);
      };

//---------------------------------------------------------
//   MuseScoreApplication (mac only)
//---------------------------------------------------------

class MuseScoreApplication : public QtSingleApplication {
   public:
      QStringList paths;
      MuseScoreApplication(const QString &id, int &argc, char **argv)
         : QtSingleApplication(id, argc, argv) {
            };
      bool event(QEvent *ev);
      };

//---------------------------------------------------------
//   MuseScore
//---------------------------------------------------------

class MuseScore : public QMainWindow, public MuseScoreCore {
      Q_OBJECT

      ScoreView* cv;
      ScoreState _sstate;
      UpdateChecker* ucheck;

      QVBoxLayout* layout;    // main window layout
      QSplitter* splitter;
      ScoreTab* tab1;
      ScoreTab* tab2;
      NScrollArea* _navigator;
      ImportMidiPanel* importmidiPanel;
      QFrame* importmidiShowPanel;
      QSplitter* mainWindow;

      QMenu* menuView;
      QMenu* openRecent;

      MagBox* mag;
      QComboBox* viewModeCombo;
      QAction* playId;

      QProgressBar* _progressBar;
      PreferenceDialog* preferenceDialog;
      QToolBar* cpitchTools;
      QToolBar* fileTools;
      QToolBar* transportTools;
      QToolBar* entryTools;
      TextTools* _textTools;
      PianoTools* _pianoTools;
      WebPageDockWidget* _webPage;
      MediaDialog* _mediaDialog;
      DrumTools* _drumTools;
      QToolBar* voiceTools;
      InstrumentsDialog* instrList;
      MeasuresDialog* measuresDialog;
      InsertMeasuresDialog* insertMeasuresDialog;
      MasterPalette* masterPalette;
      PluginCreator* pluginCreator;
      PluginManager* pluginManager;

      QMenu* _fileMenu;
      QMenu* menuEdit;
      QMenu* menuNotes;
      QMenu* menuLayout;
      QMenu* menuStyle;
      AlbumManager* albumManager;

      QWidget* searchDialog;
      QComboBox* searchCombo;

      PlayPanel* playPanel;
      Mixer* mixer;
      SynthControl* synthControl;
      Debugger* debugger;
      MeasureListEditor* measureListEdit;
      PageSettings* pageSettings;

      QWidget* symbolDialog;

      PaletteScrollArea* clefPalette;
      PaletteScrollArea* keyPalette;
      KeyEditor* keyEditor;
      ChordStyleEditor* chordStyleEditor;
      QStatusBar* _statusBar;
      QLabel* _modeText;
      QLabel* _positionLabel;
      NewWizard* newWizard;

      PaletteBox* paletteBox;
      Inspector* inspector;
      OmrPanel* omrPanel;

      bool _midiinEnabled;
      QString lastOpenPath;
      QList<QString> plugins;
      ScriptEngine* se;
      QString pluginPath;

      QQmlEngine* _qml;
      void createMenuEntry(PluginDescription*);

      QTimer* autoSaveTimer;
      QList<QAction*> qmlPluginActions;
      QList<QAction*> pluginActions;
      QSignalMapper* pluginMapper;

      PianorollEditor* pianorollEditor;
      DrumrollEditor* drumrollEditor;
      bool _splitScreen;
      bool _horizontalSplit;

      QString rev;

      int _midiRecordId;

      bool _fullscreen;
      QList<LanguageItem> _languages;

      QFileDialog* loadScoreDialog;
      QFileDialog* saveScoreDialog;
      QFileDialog* loadStyleDialog;
      QFileDialog* saveStyleDialog;
      QFileDialog* saveImageDialog;
      QFileDialog* loadChordStyleDialog;
      QFileDialog* saveChordStyleDialog;
      QFileDialog* loadSoundFontDialog;
      QFileDialog* loadSfzFileDialog;
      QFileDialog* loadBackgroundDialog;
      QFileDialog* loadScanDialog;
      QFileDialog* loadAudioDialog;
      QFileDialog* loadDrumsetDialog;
      QFileDialog* loadPluginDialog;
      QFileDialog* loadPaletteDialog;
      QFileDialog* savePaletteDialog;
      QFileDialog* saveDrumsetDialog;
      QFileDialog* savePluginDialog;

      QDialog* editRasterDialog;
      QAction* hRasterAction;
      QAction* vRasterAction;

      QMenu* menuWorkspaces;
      QActionGroup* workspaces;

      bool inChordEditor;

      QComboBox* layerSwitch;
      QComboBox* playMode;
      QNetworkAccessManager* networkManager;
      QAction* lastCmd;
      Shortcut* lastShortcut;

      QAction* metronomeAction;
      QAction* loopAction;
      QAction* loopInAction;
      QAction* loopOutAction;
      QAction* panAction;

      QLabel* cornerLabel;

      //---------------------

      virtual void closeEvent(QCloseEvent*);

      virtual void dragEnterEvent(QDragEnterEvent*);
      virtual void dropEvent(QDropEvent*);

      void playVisible(bool flag);
      void launchBrowser(const QString whereTo);

      void loadScoreList();
      void editInstrList();
      void symbolMenu();
      void showKeyEditor();
      bool saveFile();
      bool saveFile(Score* score);
      void fingeringMenu();
      void registerPlugin(PluginDescription*);
      int  pluginIdxFromPath(QString pluginPath);
      void startDebugger();
      void midiinToggled(bool);
      void undo();
      void redo();
      void showPalette(bool);
      void showInspector(bool);
      void showOmrPanel(bool);
      void showPlayPanel(bool);
      void showNavigator(bool);
      void showMixer(bool);
      void showSynthControl(bool);
      void showSearchDialog();
      void helpBrowser() const;
      void helpBrowser(const QUrl&) const;
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
      void cmdAddChordName2();
      void changeScore(int);
      virtual void resizeEvent(QResizeEvent*);
      void updateInspector();
      void showModeText(const QString&);

   private slots:
      void cmd(QAction* a, const QString& cmd);
      void autoSaveTimerTimeout();
      void helpBrowser1() const;
      void about();
      void aboutQt();
      void aboutMusicXML();
      void reportBug();
      void openRecentMenu();
      void selectScore(QAction*);
      void startPreferenceDialog();
      void preferencesChanged();
      void seqStarted();
      void seqStopped();
      void cmdAppendMeasures();
      void cmdInsertMeasures();
      void magChanged(int);
      void showPageSettings();
      void removeTab(int);
      void removeTab();
      void clipboardChanged();
      void endSearch();
      void saveScoreDialogFilterSelected(const QString&);
#ifdef OSC
      void oscIntMessage(int);
      void oscPlay();
      void oscStop();
      void oscVolume(int val);
      void oscTempo(int val);
      void oscNext();
      void oscNextMeasure();
      void oscGoto(int m);
      void oscSelectMeasure(int m);
      void oscVolChannel(double val);
      void oscPanChannel(double val);
      void oscMuteChannel(double val);
      void oscOpen(QString path);
      void oscCloseAll();
      void oscTriggerPlugin(QString list);
      void oscColorNote(QVariantList list);
#endif
      void createNewWorkspace();
      void deleteWorkspace();
      void undoWorkspace();
      void showWorkspaceMenu();
      void changeWorkspace(QAction*);
      void changeWorkspace(Workspace* p);
      void switchLayer(const QString&);
      void switchPlayMode(int);
      void networkFinished(QNetworkReply*);
      void switchLayoutMode(int);
      void showMidiImportPanel();

   public slots:
      virtual void cmd(QAction* a);
      void dirtyChanged(Score*);
      void setPos(int tick);
      void searchTextChanged(const QString& s);
      void pluginTriggered(int);
      void handleMessage(const QString& message);
      void setCurrentScoreView(ScoreView*);
      void setCurrentScoreView(int);
      void setNormalState()    { changeState(STATE_NORMAL); }
      void setPlayState()      { changeState(STATE_PLAY); }
      void checkForUpdate();
      QMenu* fileMenu() const  { return _fileMenu; }
      void midiNoteReceived(int channel, int pitch, int velo);
      void midiNoteReceived(int pitch, bool ctrl);
      void instrumentChanged();
      void showMasterPalette(const QString& = 0);
      void selectionChanged(int);

   public:
      MuseScore();
      ~MuseScore();
      bool checkDirty(Score*);
      PlayPanel* getPlayPanel() const { return playPanel; }
      QMenu* genCreateMenu(QWidget* parent = 0);
      virtual int appendScore(Score*);
      void midiCtrlReceived(int controller, int value);
      void showElementContext(Element* el);
      void cmdAppendMeasures(int);
      bool midiinEnabled() const;
      bool playEnabled() const;

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
      void showDrumTools(Drumset*, Staff*);
      void updateDrumTools();
      void showWebPanel(bool on);
      void showPluginCreator(QAction*);
      void showPluginManager(QAction*);

      void updateTabNames();
      QProgressBar* showProgressBar();
      void hideProgressBar();
      void updateRecentScores(Score*);
      QFileDialog* saveAsDialog();
      QFileDialog* saveCopyDialog();

      QString lastSaveCopyDirectory;
      QString lastSaveDirectory;
      SynthControl* getSynthControl() const { return synthControl; }
      void editInPianoroll(Staff* staff);
      void editInDrumroll(Staff* staff);
      PianorollEditor* getPianorollEditor() const { return pianorollEditor; }
      DrumrollEditor* getDrumrollEditor() const   { return drumrollEditor; }
      void writeSessionFile(bool);
      bool restoreSession(bool);
      bool splitScreen() const { return _splitScreen; }
      virtual void setCurrentView(int tabIdx, int idx);
      void loadPlugins();
      void unloadPlugins();

      ScoreState state() const { return _sstate; }
      void changeState(ScoreState);
      void updateInputState(Score*);

      bool readLanguages(const QString& path);
      void setRevision(QString& r)  {rev = r;}
      Q_INVOKABLE QString revision()            {return rev;}
      Q_INVOKABLE QString version()            {return VERSION;}
      Q_INVOKABLE void newFile();
      Q_INVOKABLE void loadFile(const QString& url);
      void loadFile(const QUrl&);
      virtual Score* openScore(const QString& fn);
      bool hasToCheckForUpdate();
      static bool unstable();
      bool eventFilter(QObject *, QEvent *);
      void setMidiRecordId(int id) { _midiRecordId = id; }
      int midiRecordId() const { return _midiRecordId; }
      void populatePalette();
      void excerptsChanged(Score*);
      bool processMidiRemote(MidiRemoteType type, int data);
      ScoreTab* getTab1() const { return tab1; }
      ScoreTab* getTab2() const { return tab2; }
      QList<LanguageItem>& languages() { return _languages; }

      QStringList getOpenScoreNames(QString& dir, const QString& filter, const QString& title);
      QString getSaveScoreName(const QString& title, QString& name, const QString& filter, bool folder = false);
      QString getStyleFilename(bool open, const QString& title = QString());
      QString getFotoFilename(QString& filter, QString *selectedFilter);
      QString getChordStyleFilename(bool open);
      QString getScanFile(const QString&);
      QString getAudioFile(const QString&);
      QString getDrumsetFilename(bool open);
      QString getPluginFilename(bool open);
      QString getPaletteFilename(bool open);
      QString getWallpaper(const QString& caption);

      bool hRaster() const { return hRasterAction->isChecked(); }
      bool vRaster() const { return vRasterAction->isChecked(); }

      PaletteBox* getPaletteBox();
      void disableCommands(bool val) { inChordEditor = val; }

      Tuplet* tupletDialog();
      void selectSimilar(Element*, bool);
      void selectElementDialog(Element* e);
      void transpose();

      Q_INVOKABLE void openExternalLink(const QString&);
      Q_INVOKABLE void closeWebPanelPermanently();

      void endCmd();
      void printFile();
      bool exportFile();
      bool exportParts();
      bool saveAs(Score*, bool saveCopy, const QString& path, const QString& ext);
      bool savePdf(const QString& saveName);
      bool savePdf(Score* cs, const QString& saveName);

      Score* readScore(const QString& name);

      bool saveAs(Score*, bool saveCopy = false);
      bool saveSelection(Score*);
      void addImage(Score*, Element*);

      bool savePng(Score*, const QString& name, bool screenshot, bool transparent, double convDpi, QImage::Format format);
      bool saveAudio(Score*, const QString& name, const QString& type);
      bool saveMp3(Score*, const QString& name);
      bool saveSvg(Score*, const QString& name);
      bool savePng(Score*, const QString& name);
//      bool saveLilypond(Score*, const QString& name);
      bool saveMidi(Score* score, const QString& name);

      void closeScore(Score* score);

      void addTempo();
      void addMetronome();

      Q_INVOKABLE QString getLocaleISOCode() const;
      Navigator* navigator() const;
      NScrollArea* navigatorScrollArea() const { return _navigator; }
      void updateLayer();
      void updatePlayMode();
      bool loop() const         	 { return loopAction->isChecked(); }
      bool metronome() const         { return metronomeAction->isChecked(); }
      bool panDuringPlayback() const { return panAction->isChecked(); }
      void noteTooShortForTupletDialog();
      void loadFiles();
                  // midi panel functions
      void midiPanelOnSwitchToFile(const QString &file);
      void midiPanelOnCloseFile(const QString &file);
      void allowShowMidiPanel(const QString &file);
      void setMidiPrefOperations(const QString &file);

      static Palette* newTempoPalette();
      static Palette* newTextPalette();
      static Palette* newTimePalette();
      static Palette* newRepeatsPalette();
      static Palette* newBreaksPalette();
      static Palette* newBeamPalette();
      static Palette* newDynamicsPalette();
      static Palette* newFramePalette();
      static Palette* newFingeringPalette();
      static Palette* newTremoloPalette();
      static Palette* newNoteHeadsPalette();
      static Palette* newArticulationsPalette();
      static Palette* newBracketsPalette();
      static Palette* newBreathPalette();
      static Palette* newArpeggioPalette();
      static Palette* newClefsPalette();
      static Palette* newGraceNotePalette();
      static Palette* newBagpipeEmbellishmentPalette();
      static Palette* newKeySigPalette();
      static Palette* newAccidentalsPalette();
      static Palette* newBarLinePalette();
      static Palette* newLinesPalette();

      Inspector* getInspector()           { return inspector; }
      QQmlEngine* qml();
      PluginCreator* getPluginCreator()   { return pluginCreator; }
      ScoreView* currentScoreView() const { return cv; }
      void showMessage(const QString& s, int timeout);
      };

extern MuseScore* mscore;
extern MuseScoreCore* mscoreCore;
extern QString dataPath;
extern MasterSynthesizer* synti;
MasterSynthesizer* synthesizerFactory();
Driver* driverFactory(Seq*, QString driver);

extern QAction* getAction(const char*);
extern Shortcut* midiActionMap[128];
extern void setMscoreLocale(QString localeName);
extern QPixmap sym2pixmap(const Sym* s, qreal mag);

extern bool saveMxl(Score*, const QString& name);
extern bool saveXml(Score*, const QString& name);

} // namespace Ms
#endif

