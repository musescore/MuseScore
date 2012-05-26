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

class Shortcut;
class ScoreView;
class Element;
class ToolButton;
class PreferenceDialog;
class InstrumentsDialog;
class Instrument;
class MidiFile;
class TextStyleDialog;
class PlayPanel;
class InstrumentListEditor;
class Debugger;
class MeasureListEditor;
class Score;
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
class ScoreView;
class ScoreTab;
class QScriptEngineDebugger;
class Drumset;
class TextTools;
class DrumTools;
class ScriptEngine;
class KeyEditor;
class ChordStyleEditor;
class Navigator;
class PianoTools;
class MediaDialog;
class Profile;
class AlbumManager;
class WebPageDockWidget;
class ChordList;
class EditTempo;
class Capella;
class CapVoice;
class Inspector;
class OmrPanel;
class NScrollArea;
class EditTools;
class Sym;
class MasterPalette;

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

class MuseScore : public QMainWindow {
      Q_OBJECT

      ScoreState _sstate;
      UpdateChecker* ucheck;
      QList<Score*> scoreList;
      Score* cs;              // current score
      ScoreView* cv;          // current viewer

      QVBoxLayout* layout;    // main window layout
      QSplitter* splitter;
      ScoreTab* tab1;
      ScoreTab* tab2;
      NScrollArea* _navigator;
      QSplitter* mainWindow;

      QMenu* menuDisplay;
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
      EditTools* _editTools;
      PianoTools* _pianoTools;
      WebPageDockWidget* _webPage;
      MediaDialog* _mediaDialog;
      DrumTools* _drumTools;
      QToolBar* voiceTools;
      InstrumentsDialog* instrList;
      MeasuresDialog* measuresDialog;
      InsertMeasuresDialog* insertMeasuresDialog;
      MasterPalette* masterPalette;

      QMenu* _fileMenu;
      QMenu* menuEdit;
      QMenu* menuNotes;
      QMenu* menuLayout;
      QMenu* menuStyle;
      AlbumManager* albumManager;

      QWidget* searchDialog;
      QComboBox* searchCombo;

      PlayPanel* playPanel;
      InstrumentListEditor* iledit;
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
      bool _speakerEnabled;
      QString lastOpenPath;
      QList<QString> plugins;
      ScriptEngine* se;
      QString pluginPath;

      QScriptEngineDebugger* scriptDebugger;

      QTimer* autoSaveTimer;
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
      QFileDialog* loadBackgroundDialog;
      QFileDialog* loadScanDialog;
      QFileDialog* loadAudioDialog;
      QFileDialog* loadDrumsetDialog;
      QFileDialog* loadPaletteDialog;
      QFileDialog* savePaletteDialog;
      QFileDialog* saveDrumsetDialog;

      QDialog* editRasterDialog;
      QAction* hRasterAction;
      QAction* vRasterAction;

      QMenu* menuProfiles;
      QActionGroup* profiles;
      QAction* deleteProfileAction;

      bool inChordEditor;

      QComboBox* layerSwitch;
      QComboBox* playMode;
      QNetworkAccessManager* networkManager;
      QAction* lastCmd;
      Shortcut* lastShortcut;
      EditTempo* editTempo;

      QAction* metronomeAction;
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
      void saveFile();
      void fingeringMenu();
      void registerPlugin(const QString& pluginPath);
      void pluginExecuteFunction(int idx, const char* functionName);
      int  pluginIdxFromPath(QString pluginPath);
      void addGlobalObjectToPluginEngine(const char * name, const QString & value );
      void startDebugger();
      void midiinToggled(bool);
      void speakerToggled(bool);
      void undo();
      void redo();
      void showPalette(bool);
      void showInspector(bool);
      void showOmrPanel(bool);
      void showPlayPanel(bool);
      void showNavigator(bool);
      void showMixer(bool);
      void showSynthControl();
      void helpBrowser();
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
      void gotoNextScore();
      void gotoPreviousScore();
      void updateUndoRedo();
      void cmdAddChordName2();
      static void convertCapella(Score*, Capella* cap);
      void changeScore(int);
      virtual void resizeEvent(QResizeEvent*);

   private slots:
      void autoSaveTimerTimeout();
      void helpBrowser1();
      void about();
      void aboutQt();
      void aboutMusicXML();
      void openRecentMenu();
      void selectScore(QAction*);
      void startPreferenceDialog();
      void preferencesChanged();
      void seqStarted();
      void seqStopped();
      void closePlayPanel();
      void cmdAppendMeasures();
      void cmdInsertMeasures();
      void magChanged(int);
      void showPageSettings();
      void removeTab(int);
      void removeTab();
      void cmd(QAction*);
      void cmd(QAction* a, const QString& cmd);
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
      void createNewProfile();
      void deleteProfile();
      void showProfileMenu();
      void changeProfile(QAction*);
      void changeProfile(Profile* p);
      void switchLayer(const QString&);
      void switchPlayMode(int);
      void networkFinished(QNetworkReply*);
      void switchLayoutMode(int);

   public slots:
      void dirtyChanged(Score*);
      void setPos(int tick);
      void searchTextChanged(const QString& s);
      void pluginTriggered(int);
      void handleMessage(const QString& message);
      void setCurrentScoreView(ScoreView*);
      void setCurrentScoreView(int);
      void setNormalState()    { changeState(STATE_NORMAL); }
      void setEditState()      { changeState(STATE_EDIT); }
      void setNoteEntryState() { changeState(STATE_NOTE_ENTRY); }
      void setPlayState()      { changeState(STATE_PLAY); }
      void setSearchState()    { changeState(STATE_SEARCH); }
      void setFotomode()       { changeState(STATE_FOTO); }
      void checkForUpdate();
      void registerPlugin(QAction*);
      QMenu* fileMenu() const  { return _fileMenu; }
      void midiNoteReceived(int channel, int pitch, int velo);
      void midiNoteReceived(int pitch, bool ctrl);
      void instrumentChanged();
      void showMasterPalette();
      void selectionChanged(int);

   public:
      MuseScore();
      ~MuseScore();
      bool checkDirty(Score*);
      PlayPanel* getPlayPanel() const { return playPanel; }
      QMenu* genCreateMenu(QWidget* parent = 0);
      int appendScore(Score*);
      void midiCtrlReceived(int controller, int value);
      void showElementContext(Element* el);
	    void cmdAppendMeasures(int);
      bool midiinEnabled() const;
      bool playEnabled() const;

      Score* currentScore() const         { return cs; }
      ScoreView* currentScoreView() const { return cv; }

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
      EditTools* editTools();
      void showDrumTools(Drumset*, Staff*);
      void showWebPanel(bool on);

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
      void setCurrentView(int tabIdx, int idx);
      void loadPlugins();
      void unloadPlugins();
      ScoreState state() const { return _sstate; }
      void changeState(ScoreState);
      bool readLanguages(const QString& path);
      void setRevision(QString& r){rev = r;}
      QString revision() {return rev;}
      Q_INVOKABLE void newFile();
      Q_INVOKABLE void loadFile(const QString& url);
      void loadFile(const QUrl&);
      void openScore(const QString& fn);
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
      void readScoreError(const QString&) const;
      QList<LanguageItem>& languages() { return _languages; }

      QStringList getOpenScoreNames(QString& dir, const QString& filter);
      QString getSaveScoreName(const QString& title,
         QString& name, const QString& filter, QString* selectedFilter);
      QString getSaveScoreName(const QString& title,
         QString& name, const QString& filter, QString* selectedFilter, bool selectFolder);
      QString getStyleFilename(bool open, const QString& title = QString());
      QString getFotoFilename();
      QString getChordStyleFilename(bool open);
      QStringList getSoundFont(const QString&);
      QString getScanFile(const QString&);
      QString getAudioFile(const QString&);
      QString getDrumsetFilename(bool open);
      QString getPaletteFilename(bool open);
      QString getWallpaper(const QString& caption);

      bool hRaster() const { return hRasterAction->isChecked(); }
      bool vRaster() const { return vRasterAction->isChecked(); }

      PaletteBox* getPaletteBox();
      void disableCommands(bool val) { inChordEditor = val; }

      void updateInputState(Score*);

      void tupletDialog();
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
      bool savePsPdf(const QString& saveName, QPrinter::OutputFormat format);
      bool savePsPdf(Score* cs, const QString& saveName, QPrinter::OutputFormat format);
      bool readScore(Score*, QString name);
      bool saveAs(Score*, bool saveCopy = false);
      bool saveSelection(Score*);
      void addImage(Score*, Element*);
      bool savePng(Score*, const QString& name, bool screenshot, bool transparent, double convDpi, QImage::Format format);
      bool saveAudio(Score*, const QString& name, const QString& type);
      bool saveMp3(Score*, const QString& name);
      bool saveMxl(Score*, const QString& name);
      bool saveXml(Score*, const QString& name);
      bool saveSvg(Score*, const QString& name);
      bool savePng(Score*, const QString& name);
      bool saveLilypond(Score*, const QString& name);

//      static bool importPdf(Score*, const QString& path);
      static bool importGTP(Score*, const QString& name);
      static bool importBww(Score*, const QString& path);
      static bool importMusicXml(Score*, const QString&);
      static bool importCompressedMusicXml(Score*, const QString&);
      static bool importMuseData(Score*, const QString& name);
      static bool importLilypond(Score*, const QString& name);
      static bool importBB(Score*, const QString& name);
      static bool importCapella(Score*, const QString& name);
      static bool importOve(Score*, const QString& name);

      void closeScore(Score* score);

      void addTempo();
      void addMetronome();

      Q_INVOKABLE QString getLocaleISOCode();
      Navigator* navigator() const;
      NScrollArea* navigatorScrollArea() const { return _navigator; }
      void updateLayer();
      void updatePlayMode();
      bool metronome() const         { return metronomeAction->isChecked(); }
      bool panDuringPlayback() const { return panAction->isChecked(); }
      void noteTooShortForTupletDialog();
      void enableInputToolbar(bool enableInput);
      void loadFiles();

      static Palette* newTextPalette();
      static Palette* newRepeatsPalette();
      static Palette* newBreaksPalette();
      static Palette* newBeamPalette();
      static Palette* newDynamicsPalette();
      static Palette* newFramePalette();
      static Palette* newFingeringPalette();
      static Palette* newFallDoitPalette();
      static Palette* newTremoloPalette();
      static Palette* newNoteHeadsPalette();
      static Palette* newArticulationsPalette();
      static Palette* newBracketsPalette();
      static Palette* newBreathPalette();
      static Palette* newArpeggioPalette();
      static Palette* newClefsPalette();
      static Palette* newGraceNotePalette();
      static Palette* newKeySigPalette();
      static Palette* newAccidentalsPalette();
      static Palette* newBarLinePalette();
      static Palette* newLinesPalette();
      };

extern MuseScore* mscore;
extern QString dataPath;

extern QAction* getAction(const char*);
extern Shortcut* midiActionMap[128];
extern void setMscoreLocale(QString localeName);
extern QPixmap sym2pixmap(const Sym* s, qreal mag);

extern bool saveMidi(Score*, const QString& name);
extern bool importMidi(Score*, const QString& name);
extern void convertMidi(Score*, MidiFile* mf);


#endif

