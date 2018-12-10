//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2017 Werner Schweer and others
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

#include "musescore.h"
#include "preferences.h"
#include "prefsdialog.h"
#include "seq.h"
#include "shortcutcapturedialog.h"
#include "scoreview.h"
#include "pa.h"
#include "shortcut.h"
#include "workspace.h"

#ifdef USE_PORTMIDI
#include "pm.h"
#endif

#include "pathlistdialog.h"
#include "resourceManager.h"
#include "synthesizer/msynthesizer.h"

namespace Ms {

//---------------------------------------------------------
//   startPreferenceDialog
//---------------------------------------------------------

void MuseScore::startPreferenceDialog()
      {
      if (!preferenceDialog) {
            preferenceDialog = new PreferenceDialog(this);
            connect(preferenceDialog, SIGNAL(preferencesChanged()),
               SLOT(preferencesChanged()));
            connect(preferenceDialog, SIGNAL(mixerPreferencesChanged(bool)),
               SLOT(mixerPreferencesChanged(bool)));
            }
      preferenceDialog->start();
      }

//---------------------------------------------------------
//   PreferenceDialog
//---------------------------------------------------------

PreferenceDialog::PreferenceDialog(QWidget* parent)
   : AbstractDialog(parent)
      {
      setObjectName("PreferenceDialog");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setModal(true);
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
      connect(fgColorButton, SIGNAL(toggled(bool)), SLOT(updateFgView(bool)));

      QButtonGroup* bgButtons = new QButtonGroup(this);
      bgButtons->setExclusive(true);
      bgButtons->addButton(bgColorButton);
      bgButtons->addButton(bgWallpaperButton);
      connect(bgColorButton, SIGNAL(toggled(bool)), SLOT(updateBgView(bool)));

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
      connect(saveShortcutList,  SIGNAL(clicked()), SLOT(saveShortcutListClicked()));
      connect(loadShortcutList,  SIGNAL(clicked()), SLOT(loadShortcutListClicked()));
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

      connect(recordButtons,          SIGNAL(buttonClicked(int)), SLOT(recordButtonClicked(int)));
      connect(midiRemoteControlClear, SIGNAL(clicked()), SLOT(midiRemoteControlClearClicked()));
      connect(portaudioDriver, SIGNAL(toggled(bool)), SLOT(exclusiveAudioDriver(bool)));
      connect(pulseaudioDriver, SIGNAL(toggled(bool)), SLOT(exclusiveAudioDriver(bool)));
      connect(alsaDriver, SIGNAL(toggled(bool)), SLOT(exclusiveAudioDriver(bool)));
      connect(jackDriver, SIGNAL(toggled(bool)), SLOT(exclusiveAudioDriver(bool)));
      connect(useJackAudio, SIGNAL(toggled(bool)), SLOT(nonExclusiveJackDriver(bool)));
      connect(useJackMidi,  SIGNAL(toggled(bool)), SLOT(nonExclusiveJackDriver(bool)));
      updateRemote();

      advancedWidget = new PreferencesListWidget();
      QVBoxLayout* l = static_cast<QVBoxLayout*> (tabAdvanced->layout());
      l->insertWidget(0, advancedWidget);
      advancedWidget->loadPreferences();
      connect(advancedSearch, &QLineEdit::textChanged, this, &PreferenceDialog::filterAdvancedPreferences);
      connect(resetPreference, &QPushButton::clicked, this, &PreferenceDialog::resetAdvancedPreferenceToDefault);

      MuseScore::restoreGeometry(this);
#if !defined(Q_OS_MAC) && (!defined(Q_OS_WIN) || defined(FOR_WINSTORE))
      General->removeTab(General->indexOf(tabUpdate)); // updateTab not needed on Linux and not wanted in Windows Store
#endif
      }

//---------------------------------------------------------
//   start
//---------------------------------------------------------

void PreferenceDialog::start()
      {
      updateValues();
      show();
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

void PreferenceDialog::recordButtonClicked(int val)
      {
      for (QAbstractButton* b : recordButtons->buttons()) {
            b->setChecked(recordButtons->id(b) == val);
            }
      mscore->setMidiRecordId(val);
      }

//---------------------------------------------------------
//   updateRemote
//---------------------------------------------------------

void PreferenceDialog::updateRemote()
      {
      rewindActive->setChecked(preferences.midiRemote(RMIDI_REWIND).type != -1);
      togglePlayActive->setChecked(preferences.midiRemote(RMIDI_TOGGLE_PLAY).type   != -1);
      playActive->setChecked(preferences.midiRemote(RMIDI_PLAY).type         != -1);
      stopActive->setChecked(preferences.midiRemote(RMIDI_STOP).type         != -1);
      rca2->setChecked(preferences.midiRemote(RMIDI_NOTE1).type        != -1);
      rca3->setChecked(preferences.midiRemote(RMIDI_NOTE2).type        != -1);
      rca4->setChecked(preferences.midiRemote(RMIDI_NOTE4).type        != -1);
      rca5->setChecked(preferences.midiRemote(RMIDI_NOTE8).type        != -1);
      rca6->setChecked(preferences.midiRemote(RMIDI_NOTE16).type       != -1);
      rca7->setChecked(preferences.midiRemote(RMIDI_NOTE32).type       != -1);
      rca8->setChecked(preferences.midiRemote(RMIDI_NOTE64).type      != -1);
      rca9->setChecked(preferences.midiRemote(RMIDI_REST).type        != -1);
      rca10->setChecked(preferences.midiRemote(RMIDI_DOT).type         != -1);
      rca11->setChecked(preferences.midiRemote(RMIDI_DOTDOT).type      != -1);
      rca12->setChecked(preferences.midiRemote(RMIDI_TIE).type        != -1);
      recordUndoActive->setChecked(preferences.midiRemote(RMIDI_UNDO).type != -1);
      editModeActive->setChecked(preferences.midiRemote(RMIDI_NOTE_EDIT_MODE).type != -1);
      realtimeAdvanceActive->setChecked(preferences.midiRemote(RMIDI_REALTIME_ADVANCE).type != -1);

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

void PreferenceDialog::updateValues(bool useDefaultValues)
      {
      if (useDefaultValues)
            preferences.setReturnDefaultValuesMode(true);

      advancedWidget->updatePreferences();

      rcGroup->setChecked(preferences.getBool(PREF_IO_MIDI_USEREMOTECONTROL));
      advanceOnRelease->setChecked(preferences.getBool(PREF_IO_MIDI_ADVANCEONRELEASE));

      fgWallpaper->setText(preferences.getString(PREF_UI_CANVAS_FG_WALLPAPER));
      bgWallpaper->setText(preferences.getString(PREF_UI_CANVAS_BG_WALLPAPER));

      bool useBgColor = preferences.getBool(PREF_UI_CANVAS_BG_USECOLOR);
      updateBgView(useBgColor);

      bool useFgColor = preferences.getBool(PREF_UI_CANVAS_FG_USECOLOR);
      updateFgView(useFgColor);

      iconWidth->setValue(preferences.getInt(PREF_UI_THEME_ICONWIDTH));
      iconHeight->setValue(preferences.getInt(PREF_UI_THEME_ICONHEIGHT));
      fontFamily->setCurrentFont(QFont(preferences.getString(PREF_UI_THEME_FONTFAMILY)));
      fontSize->setValue(preferences.getInt(PREF_UI_THEME_FONTSIZE));

      enableMidiInput->setChecked(preferences.getBool(PREF_IO_MIDI_ENABLEINPUT));
      realtimeDelay->setValue(preferences.getInt(PREF_IO_MIDI_REALTIMEDELAY));
      playNotes->setChecked(preferences.getBool(PREF_SCORE_NOTE_PLAYONCLICK));
      playChordOnAddNote->setChecked(preferences.getBool(PREF_SCORE_CHORD_PLAYONADDNOTE));

      checkUpdateStartup->setChecked(preferences.getBool(PREF_UI_APP_STARTUP_CHECKUPDATE));

      navigatorShow->setChecked(preferences.getBool(PREF_UI_APP_STARTUP_SHOWNAVIGATOR));
      playPanelShow->setChecked(preferences.getBool(PREF_UI_APP_STARTUP_SHOWPLAYPANEL));
      showSplashScreen->setChecked(preferences.getBool(PREF_UI_APP_STARTUP_SHOWSPLASHSCREEN));
      showStartcenter->setChecked(preferences.getBool(PREF_UI_APP_STARTUP_SHOWSTARTCENTER));
      showTours->setChecked(preferences.getBool(PREF_UI_APP_STARTUP_SHOWTOURS));

      alsaDriver->setChecked(preferences.getBool(PREF_IO_ALSA_USEALSAAUDIO));
      jackDriver->setChecked(preferences.getBool(PREF_IO_JACK_USEJACKAUDIO) || preferences.getBool(PREF_IO_JACK_USEJACKMIDI));
      useJackAudio->setChecked(preferences.getBool(PREF_IO_JACK_USEJACKAUDIO));
      portaudioDriver->setChecked(preferences.getBool(PREF_IO_PORTAUDIO_USEPORTAUDIO));
      pulseaudioDriver->setChecked(preferences.getBool(PREF_IO_PULSEAUDIO_USEPULSEAUDIO));
      useJackMidi->setChecked(preferences.getBool(PREF_IO_JACK_USEJACKMIDI));
      useJackTransport->setChecked(preferences.getBool(PREF_IO_JACK_USEJACKTRANSPORT));
      becomeTimebaseMaster->setChecked(preferences.getBool(PREF_IO_JACK_TIMEBASEMASTER));

      alsaDevice->setText(preferences.getString(PREF_IO_ALSA_DEVICE));

      int index = alsaSampleRate->findData(preferences.getInt(PREF_IO_ALSA_SAMPLERATE));
      alsaSampleRate->setCurrentIndex(index);

      index = alsaPeriodSize->findData(preferences.getInt(PREF_IO_ALSA_PERIODSIZE));
      alsaPeriodSize->setCurrentIndex(index);

      alsaFragments->setValue(preferences.getInt(PREF_IO_ALSA_FRAGMENTS));
      drawAntialiased->setChecked(preferences.getBool(PREF_UI_CANVAS_MISC_ANTIALIASEDDRAWING));
      limitScrollArea->setChecked(preferences.getBool(PREF_UI_CANVAS_SCROLL_LIMITSCROLLAREA));
      switch(preferences.sessionStart()) {
            case SessionStart::EMPTY:  emptySession->setChecked(true); break;
            case SessionStart::LAST:   lastSession->setChecked(true); break;
            case SessionStart::NEW:    newSession->setChecked(true); break;
            case SessionStart::SCORE:  scoreSession->setChecked(true); break;
            }
      sessionScore->setText(preferences.getString(PREF_APP_STARTUP_STARTSCORE));
      expandRepeats->setChecked(preferences.getBool(PREF_IO_MIDI_EXPANDREPEATS));
      normalize->setChecked(preferences.getBool(PREF_EXPORT_AUDIO_NORMALIZE));
      exportRPNs->setChecked(preferences.getBool(PREF_IO_MIDI_EXPORTRPNS));
      instrumentList1->setText(preferences.getString(PREF_APP_PATHS_INSTRUMENTLIST1));
      instrumentList2->setText(preferences.getString(PREF_APP_PATHS_INSTRUMENTLIST2));

      importLayout->setChecked(preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT));
      importBreaks->setChecked(preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTBREAKS));
      if (preferences.getBool(PREF_EXPORT_MUSICXML_EXPORTLAYOUT)) {
            exportAllLayouts->setChecked(true);
            }
      else {
            switch(preferences.musicxmlExportBreaks()) {
                  case MusicxmlExportBreaks::ALL:     exportAllBreaks->setChecked(true); break;
                  case MusicxmlExportBreaks::MANUAL:  exportManualBreaks->setChecked(true); break;
                  case MusicxmlExportBreaks::NO:      exportNoBreaks->setChecked(true); break;
                  }
            }

      rememberLastMidiConnections->setChecked(preferences.getBool(PREF_IO_JACK_REMEMBERLASTCONNECTIONS));
      proximity->setValue(preferences.getInt(PREF_UI_CANVAS_MISC_SELECTIONPROXIMITY));
      autoSave->setChecked(preferences.getBool(PREF_APP_AUTOSAVE_USEAUTOSAVE));
      autoSaveTime->setValue(preferences.getInt(PREF_APP_AUTOSAVE_AUTOSAVETIME));
      pngResolution->setValue(preferences.getDouble(PREF_EXPORT_PNG_RESOLUTION));
      pngTransparent->setChecked(preferences.getBool(PREF_EXPORT_PNG_USETRANSPARENCY));
      language->blockSignals(true);
      for (int i = 0; i < language->count(); ++i) {
            if (language->itemText(i).startsWith(preferences.getString(PREF_UI_APP_LANGUAGE))) {
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
      for(const Shortcut* s : Shortcut::shortcuts())
            localShortcuts[s->key()] = new Shortcut(*s);
      updateSCListView();

      //Generate the filtered Shortcut List
      filterShortcutsTextChanged(filterShortcuts->text());

      //
      // initialize portaudio
      //
#ifdef USE_PORTAUDIO
      if (portAudioIsUsed) {
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
                              if (midiInputs.at(i) == preferences.getString(PREF_IO_PORTMIDI_INPUTDEVICE))
                                    curMidiInIdx = i;
                              }
                        portMidiInput->setCurrentIndex(curMidiInIdx);

                        QStringList midiOutputs = midiDriver->deviceOutList();
                        int curMidiOutIdx = -1; // do not set a midi out device if user never selected one
                        portMidiOutput->clear();
                        portMidiOutput->addItem("", -1);
                        for(int i = 0; i < midiOutputs.size(); ++i) {
                              portMidiOutput->addItem(midiOutputs.at(i), i);
                              if (midiOutputs.at(i) == preferences.getString(PREF_IO_PORTMIDI_OUTPUTDEVICE))
                                    curMidiOutIdx = i + 1;
                              }
                        portMidiOutput->setCurrentIndex(curMidiOutIdx);

                        portMidiOutputLatencyMilliseconds->setValue(preferences.getInt(PREF_IO_PORTMIDI_OUTPUTLATENCYMILLISECONDS));
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
      scale->setValue(preferences.getDouble(PREF_SCORE_MAGNIFICATION) * 100.0);
      showMidiControls->setChecked(preferences.getBool(PREF_IO_MIDI_SHOWCONTROLSINMIXER));

      defaultPlayDuration->setValue(preferences.getInt(PREF_SCORE_NOTE_DEFAULTPLAYDURATION));

      int shortestNoteIndex = 2;
      int nn = (preferences.getInt(PREF_IO_MIDI_SHORTESTNOTE) * 16)/MScore::division;
      switch(nn) {
            case 16: shortestNoteIndex = 0; break;
            case 8:  shortestNoteIndex = 1; break;
            case 4:  shortestNoteIndex = 2; break;
            case 2:  shortestNoteIndex = 3; break;
            case 1:  shortestNoteIndex = 4; break;
            }
      shortestNote->setCurrentIndex(shortestNoteIndex);

      QString styleFile = preferences.getString(PREF_IMPORT_STYLE_STYLEFILE);
      importStyleFile->setText(styleFile);
      useImportBuiltinStyle->setChecked(styleFile.isEmpty());
      useImportStyleFile->setChecked(!styleFile.isEmpty());

      QList<QByteArray> charsets = QTextCodec::availableCodecs();
      qSort(charsets.begin(), charsets.end());
      int idx = 0;
      importCharsetListOve->clear();
      importCharsetListGP->clear();
      for (QByteArray charset : charsets) {
            importCharsetListOve->addItem(charset);
            importCharsetListGP->addItem(charset);
            if (charset == preferences.getString(PREF_IMPORT_OVERTURE_CHARSET))
                  importCharsetListOve->setCurrentIndex(idx);
            if (charset == preferences.getString(PREF_IMPORT_GUITARPRO_CHARSET))
                  importCharsetListGP->setCurrentIndex(idx);
            idx++;
            }

      warnPitchRange->setChecked(preferences.getBool(PREF_SCORE_NOTE_WARNPITCHRANGE));

      language->blockSignals(true);
      language->clear();
      QString lang = preferences.getString(PREF_UI_APP_LANGUAGE);
      int curIdx = 0;
      for(int i = 0; i < mscore->languages().size(); ++i) {
            language->addItem(mscore->languages().at(i).name, i);
            if (mscore->languages().at(i).key == lang)
                  curIdx = i;
            }
      language->setCurrentIndex(curIdx);
      language->blockSignals(false);

      oscServer->setChecked(preferences.getBool(PREF_IO_OSC_USEREMOTECONTROL));
      oscPort->setValue(preferences.getInt(PREF_IO_OSC_PORTNUMBER));

      styleName->setCurrentIndex(int(preferences.globalStyle()));
      defaultStyle->setText(preferences.getString(PREF_SCORE_STYLE_DEFAULTSTYLEFILE));
      partStyle->setText(preferences.getString(PREF_SCORE_STYLE_PARTSTYLEFILE));

      myScores->setText(preferences.getString(PREF_APP_PATHS_MYSCORES));
      myStyles->setText(preferences.getString(PREF_APP_PATHS_MYSTYLES));
      myImages->setText(preferences.getString(PREF_APP_PATHS_MYIMAGES));
      myTemplates->setText(preferences.getString(PREF_APP_PATHS_MYTEMPLATES));
      myPlugins->setText(preferences.getString(PREF_APP_PATHS_MYPLUGINS));
      mySoundfonts->setText(preferences.getString(PREF_APP_PATHS_MYSOUNDFONTS));
      myExtensions->setText(preferences.getString(PREF_APP_PATHS_MYEXTENSIONS));

      index = exportAudioSampleRate->findData(preferences.getInt(PREF_EXPORT_AUDIO_SAMPLERATE));
      exportAudioSampleRate->setCurrentIndex(index);

      index = exportMp3BitRate->findData(preferences.getInt(PREF_EXPORT_MP3_BITRATE));
      exportMp3BitRate->setCurrentIndex(index);

      exportPdfDpi->setValue(preferences.getInt(PREF_EXPORT_PDF_DPI));
      pageVertical->setChecked(MScore::verticalOrientation());

      if (useDefaultValues)
            preferences.setReturnDefaultValuesMode(false);
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
      for (Shortcut* s : localShortcuts) {
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
#ifdef NDEBUG
                            && !s->key().startsWith("debugger")
#endif
                            && !s->key().startsWith("edit_harmony")
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

void PreferenceDialog::saveShortcutListClicked()
      {
      QString saveFileName = QFileDialog::getSaveFileName(this, tr("Save Shortcuts"), preferences.getString(PREF_APP_PATHS_MYSHORTCUTS) + "/shortcuts.xml", tr("MuseScore Shortcuts File") + " (*.xml)", 0, preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS) ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
      preferences.setPreference(PREF_APP_PATHS_MYSHORTCUTS, saveFileName);
      Shortcut::saveToNewFile(saveFileName);
      }

void PreferenceDialog::loadShortcutListClicked()
      {
      QString loadFileName = QFileDialog::getOpenFileName(this, tr("Load Shortcuts"), preferences.getString(PREF_APP_PATHS_MYSHORTCUTS), tr("MuseScore Shortcuts File") +  " (*.xml)", 0, preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS) ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
      if (!loadFileName.isNull()) {
            preferences.setPreference(PREF_APP_PATHS_MYSHORTCUTS, loadFileName);
            Shortcut::loadFromNewFile(loadFileName);
            }
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

//--------------------------------------------------------
//   filterAdvancedPreferences
//--------------------------------------------------------

void PreferenceDialog::filterAdvancedPreferences(const QString& query)
      {
      QTreeWidgetItem *item;
      for(int i = 0; i < advancedWidget->topLevelItemCount(); i++) {
            item = advancedWidget->topLevelItem(i);

            if(item->text(0).toLower().contains(query.toLower()))
                  item->setHidden(false);
            else
                  item->setHidden(true);
            }
      }

//--------------------------------------------------------
//   resetAdvancedPreferenceToDefault
//--------------------------------------------------------

void PreferenceDialog::resetAdvancedPreferenceToDefault()
      {
      preferences.setReturnDefaultValuesMode(true);
      for (QTreeWidgetItem* item : advancedWidget->selectedItems()) {
            PreferenceItem* pref = static_cast<PreferenceItem*>(item);
            pref->setDefaultValue();
            }
      preferences.setReturnDefaultValuesMode(false);
      }

//---------------------------------------------------------
//   selectFgWallpaper
//---------------------------------------------------------

void PreferenceDialog::selectFgWallpaper()
      {
      QString s = mscore->getWallpaper(tr("Choose Notepaper"));
      if (!s.isNull()) {
            fgWallpaper->setText(s);
            updateFgView(false);
            }
      }

//---------------------------------------------------------
//   selectBgWallpaper
//---------------------------------------------------------

void PreferenceDialog::selectBgWallpaper()
      {
      QString s = mscore->getWallpaper(tr("Choose Background Wallpaper"));
      if (!s.isNull()) {
            bgWallpaper->setText(s);
            updateBgView(false);
            }
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
         preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS) ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog
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
         preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS) ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog
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
         preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS) ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog
         );
      if (!s.isNull())
            sessionScore->setText(s);
      }

//---------------------------------------------------------
//   fgClicked
//---------------------------------------------------------

void PreferenceDialog::updateFgView(bool useColor)
      {
      fgColorButton->setChecked(useColor);
      fgWallpaperButton->setChecked(!useColor);
      fgWallpaper->setEnabled(!useColor);
      fgWallpaperSelect->setEnabled(!useColor);

      if (useColor) {
            fgColorLabel->setColor(preferences.getColor(PREF_UI_CANVAS_FG_COLOR));
            fgColorLabel->setPixmap(0);
            }
      else {
            fgColorLabel->setPixmap(new QPixmap(fgWallpaper->text()));
            }
      }

//---------------------------------------------------------
//   bgClicked
//---------------------------------------------------------

void PreferenceDialog::updateBgView(bool useColor)
      {
      bgColorButton->setChecked(useColor);
      bgWallpaperButton->setChecked(!useColor);
      bgWallpaper->setEnabled(!useColor);
      bgWallpaperSelect->setEnabled(!useColor);

      if (useColor) {
            bgColorLabel->setColor(preferences.getColor(PREF_UI_CANVAS_BG_COLOR));
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
                  // intentional ??
                  // fall through
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
      advancedWidget->save();

      if (lastSession->isChecked())
            preferences.setCustomPreference<SessionStart>(PREF_APP_STARTUP_SESSIONSTART, SessionStart::LAST);
      else if (newSession->isChecked())
            preferences.setCustomPreference<SessionStart>(PREF_APP_STARTUP_SESSIONSTART, SessionStart::NEW);
      else if (scoreSession->isChecked())
            preferences.setCustomPreference<SessionStart>(PREF_APP_STARTUP_SESSIONSTART, SessionStart::SCORE);
      else if (emptySession->isChecked())
            preferences.setCustomPreference<SessionStart>(PREF_APP_STARTUP_SESSIONSTART, SessionStart::EMPTY);

      preferences.setPreference(PREF_APP_AUTOSAVE_AUTOSAVETIME, autoSaveTime->value());
      preferences.setPreference(PREF_APP_AUTOSAVE_USEAUTOSAVE, autoSave->isChecked());
      preferences.setPreference(PREF_APP_PATHS_INSTRUMENTLIST1, instrumentList1->text());
      preferences.setPreference(PREF_APP_PATHS_INSTRUMENTLIST2, instrumentList2->text());
      preferences.setPreference(PREF_APP_PATHS_MYIMAGES, myImages->text());
      preferences.setPreference(PREF_APP_PATHS_MYPLUGINS, myPlugins->text());
      preferences.setPreference(PREF_APP_PATHS_MYSCORES, myScores->text());
      preferences.setPreference(PREF_APP_PATHS_MYSOUNDFONTS, mySoundfonts->text());
      preferences.setPreference(PREF_APP_PATHS_MYSTYLES, myStyles->text());
      preferences.setPreference(PREF_APP_PATHS_MYTEMPLATES, myTemplates->text());
      preferences.setPreference(PREF_APP_PATHS_MYEXTENSIONS, myExtensions->text());
      preferences.setPreference(PREF_APP_STARTUP_STARTSCORE, sessionScore->text());
      preferences.setPreference(PREF_EXPORT_AUDIO_SAMPLERATE, exportAudioSampleRate->currentData().toInt());
      preferences.setPreference(PREF_EXPORT_MP3_BITRATE, exportMp3BitRate->currentData().toInt());
      preferences.setPreference(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, exportAllLayouts->isChecked());
      preferences.setPreference(PREF_EXPORT_PDF_DPI, exportPdfDpi->value());
      preferences.setPreference(PREF_EXPORT_PNG_RESOLUTION, pngResolution->value());
      preferences.setPreference(PREF_EXPORT_PNG_USETRANSPARENCY, pngTransparent->isChecked());
      preferences.setPreference(PREF_IMPORT_MUSICXML_IMPORTBREAKS, importBreaks->isChecked());
      preferences.setPreference(PREF_IMPORT_MUSICXML_IMPORTLAYOUT, importLayout->isChecked());
      preferences.setPreference(PREF_IO_MIDI_ADVANCEONRELEASE, advanceOnRelease->isChecked());
      preferences.setPreference(PREF_IO_MIDI_ENABLEINPUT, enableMidiInput->isChecked());
      preferences.setPreference(PREF_IO_MIDI_EXPANDREPEATS, expandRepeats->isChecked());
      preferences.setPreference(PREF_EXPORT_AUDIO_NORMALIZE, normalize->isChecked());
      preferences.setPreference(PREF_IO_MIDI_EXPORTRPNS, exportRPNs->isChecked());
      preferences.setPreference(PREF_IO_MIDI_REALTIMEDELAY, realtimeDelay->value());
      preferences.setPreference(PREF_IO_MIDI_USEREMOTECONTROL, rcGroup->isChecked());
      preferences.setPreference(PREF_IO_OSC_PORTNUMBER, oscPort->value());
      preferences.setPreference(PREF_IO_OSC_USEREMOTECONTROL, oscServer->isChecked());
      preferences.setPreference(PREF_SCORE_CHORD_PLAYONADDNOTE, playChordOnAddNote->isChecked());
      preferences.setPreference(PREF_SCORE_NOTE_DEFAULTPLAYDURATION, defaultPlayDuration->value());
      preferences.setPreference(PREF_SCORE_NOTE_PLAYONCLICK, playNotes->isChecked());
      preferences.setPreference(PREF_UI_APP_STARTUP_CHECKUPDATE, checkUpdateStartup->isChecked());
      preferences.setPreference(PREF_UI_APP_STARTUP_SHOWNAVIGATOR, navigatorShow->isChecked());
      preferences.setPreference(PREF_UI_APP_STARTUP_SHOWPLAYPANEL, playPanelShow->isChecked());
      preferences.setPreference(PREF_UI_APP_STARTUP_SHOWSPLASHSCREEN, showSplashScreen->isChecked());
      preferences.setPreference(PREF_UI_APP_STARTUP_SHOWSTARTCENTER, showStartcenter->isChecked());
      preferences.setPreference(PREF_UI_APP_STARTUP_SHOWTOURS, showTours->isChecked());
      preferences.setPreference(PREF_UI_CANVAS_BG_USECOLOR, bgColorButton->isChecked());
      preferences.setPreference(PREF_UI_CANVAS_BG_COLOR, bgColorLabel->color());
      preferences.setPreference(PREF_UI_CANVAS_FG_USECOLOR, fgColorButton->isChecked());
      preferences.setPreference(PREF_UI_CANVAS_FG_COLOR, fgColorLabel->color());
      preferences.setPreference(PREF_UI_CANVAS_BG_WALLPAPER, bgWallpaper->text());
      preferences.setPreference(PREF_UI_CANVAS_FG_WALLPAPER, fgWallpaper->text());
      preferences.setPreference(PREF_UI_CANVAS_MISC_ANTIALIASEDDRAWING, drawAntialiased->isChecked());
      preferences.setPreference(PREF_UI_CANVAS_MISC_SELECTIONPROXIMITY, proximity->value());
      preferences.setPreference(PREF_UI_CANVAS_SCROLL_LIMITSCROLLAREA, limitScrollArea->isChecked());
      preferences.setPreference(PREF_UI_THEME_ICONWIDTH, iconWidth->value());
      preferences.setPreference(PREF_UI_THEME_ICONHEIGHT, iconHeight->value());
      preferences.setPreference(PREF_UI_THEME_FONTFAMILY, fontFamily->currentFont().family());
      preferences.setPreference(PREF_UI_THEME_FONTSIZE, fontSize->value());

      bool wasJack = (preferences.getBool(PREF_IO_JACK_USEJACKMIDI) || preferences.getBool(PREF_IO_JACK_USEJACKAUDIO));
      bool wasJackAudio = preferences.getBool(PREF_IO_JACK_USEJACKAUDIO);
      bool wasJackMidi = preferences.getBool(PREF_IO_JACK_USEJACKMIDI);
      preferences.setPreference(PREF_IO_JACK_USEJACKAUDIO, jackDriver->isChecked() && useJackAudio->isChecked());
      preferences.setPreference(PREF_IO_JACK_USEJACKMIDI, jackDriver->isChecked() && useJackMidi->isChecked());
      bool nowJack = (preferences.getBool(PREF_IO_JACK_USEJACKMIDI) || preferences.getBool(PREF_IO_JACK_USEJACKAUDIO));
      bool jackParametersChanged = (preferences.getBool(PREF_IO_JACK_USEJACKAUDIO) != wasJackAudio
                  || preferences.getBool(PREF_IO_JACK_USEJACKMIDI) != wasJackMidi
                  || preferences.getBool(PREF_IO_JACK_REMEMBERLASTCONNECTIONS) != rememberLastMidiConnections->isChecked()
                  || preferences.getBool(PREF_IO_JACK_TIMEBASEMASTER) != becomeTimebaseMaster->isChecked())
                  && (wasJack && nowJack);

      preferences.setPreference(PREF_IO_JACK_TIMEBASEMASTER, becomeTimebaseMaster->isChecked());
      preferences.setPreference(PREF_IO_JACK_REMEMBERLASTCONNECTIONS, rememberLastMidiConnections->isChecked());
      preferences.setPreference(PREF_IO_JACK_USEJACKTRANSPORT, jackDriver->isChecked() && useJackTransport->isChecked());

      if (jackParametersChanged) {
            // Change parameters of JACK driver without unload
            if (seq) {
                  seq->driver()->init(true);
                  if (!seq->init(true))
                        qDebug("sequencer init failed");
                  }
            }
      else if (
         (wasJack != nowJack)
         || (preferences.getBool(PREF_IO_PORTAUDIO_USEPORTAUDIO) != portaudioDriver->isChecked())
         || (preferences.getBool(PREF_IO_PULSEAUDIO_USEPULSEAUDIO) != pulseaudioDriver->isChecked())
#ifdef USE_ALSA
         || (preferences.getString(PREF_IO_ALSA_DEVICE) != alsaDevice->text())
         || (preferences.getInt(PREF_IO_ALSA_SAMPLERATE) != alsaSampleRate->currentData().toInt())
         || (preferences.getInt(PREF_IO_ALSA_PERIODSIZE) != alsaPeriodSize->currentData().toInt())
         || (preferences.getInt(PREF_IO_ALSA_FRAGMENTS) != alsaFragments->value())
#endif
            ) {
            if (seq)
                  seq->exit();

            preferences.setPreference(PREF_IO_ALSA_USEALSAAUDIO, alsaDriver->isChecked());
            preferences.setPreference(PREF_IO_PORTAUDIO_USEPORTAUDIO, portaudioDriver->isChecked());
            preferences.setPreference(PREF_IO_PULSEAUDIO_USEPULSEAUDIO, pulseaudioDriver->isChecked());
            preferences.setPreference(PREF_IO_ALSA_DEVICE, alsaDevice->text());
            preferences.setPreference(PREF_IO_ALSA_SAMPLERATE, alsaSampleRate->currentData().toInt());
            preferences.setPreference(PREF_IO_ALSA_PERIODSIZE, alsaPeriodSize->currentData().toInt());
            preferences.setPreference(PREF_IO_ALSA_FRAGMENTS, alsaFragments->value());
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
      if (portAudioIsUsed && !noSeq) {
            Portaudio* audio = static_cast<Portaudio*>(seq->driver());
            preferences.setPreference(PREF_IO_PORTAUDIO_DEVICE, audio->deviceIndex(portaudioApi->currentIndex(), portaudioDevice->currentIndex()));
            }
#endif

#ifdef USE_PORTMIDI
      preferences.setPreference(PREF_IO_PORTMIDI_INPUTDEVICE, portMidiInput->currentText());
      preferences.setPreference(PREF_IO_PORTMIDI_OUTPUTDEVICE, portMidiOutput->currentText());
      if (seq->driver() && static_cast<PortMidiDriver*>(static_cast<Portaudio*>(seq->driver())->mididriver())->isSameCoreMidiIacBus(preferences.getString(PREF_IO_PORTMIDI_INPUTDEVICE), preferences.getString(PREF_IO_PORTMIDI_OUTPUTDEVICE))) {
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Possible MIDI Loopback"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Warning: You used the same CoreMIDI IAC bus for input and output.  This will cause problematic loopback, whereby MuseScore's output MIDI messages will be sent back to MuseScore as input, causing confusion.  To avoid this problem, access Audio MIDI Setup via Spotlight to create a dedicated virtual port for MuseScore's MIDI output, restart MuseScore, return to Preferences, and select your new virtual port for MuseScore's MIDI output.  Other programs may then use that dedicated virtual port to receive MuseScore's MIDI output."));
            msgBox.exec();
            }
      preferences.setPreference(PREF_IO_PORTMIDI_OUTPUTLATENCYMILLISECONDS, portMidiOutputLatencyMilliseconds->value());
#endif

      if (exportAllLayouts->isChecked() || exportAllBreaks->isChecked())
            preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::ALL);
      else if (exportManualBreaks->isChecked())
            preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::MANUAL);
      else if (exportNoBreaks->isChecked())
            preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::NO);

      if (preferences.getBool(PREF_UI_CANVAS_SCROLL_VERTICALORIENTATION) != pageVertical->isChecked()) {
            preferences.setPreference(PREF_UI_CANVAS_SCROLL_VERTICALORIENTATION, pageVertical->isChecked());
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
            for(const Shortcut* s : localShortcuts) {
                  Shortcut* os = Shortcut::getShortcut(s->key());
                  if (os) {
                        if (!os->compareKeys(*s))
                              os->setKeys(*s);
                        }
                  }
            Shortcut::dirty = true;
            }

      int lang = language->itemData(language->currentIndex()).toInt();
      QString l = lang == 0 ? "system" : mscore->languages().at(lang).key;
      bool languageChanged = l != preferences.getString(PREF_UI_APP_LANGUAGE);
      preferences.setPreference(PREF_UI_APP_LANGUAGE, l);

      preferences.setPreference(PREF_SCORE_MAGNIFICATION, scale->value()/100.0);

      if (showMidiControls->isChecked() != preferences.getBool(PREF_IO_MIDI_SHOWCONTROLSINMIXER)) {
            preferences.setPreference(PREF_IO_MIDI_SHOWCONTROLSINMIXER, showMidiControls->isChecked());
            emit mixerPreferencesChanged(preferences.getBool(PREF_IO_MIDI_SHOWCONTROLSINMIXER));
            }


      if (useImportStyleFile->isChecked())
            preferences.setPreference(PREF_IMPORT_STYLE_STYLEFILE, importStyleFile->text());
      else
            preferences.setPreference(PREF_IMPORT_STYLE_STYLEFILE, "");

      int ticks = MScore::division/4;
      switch(shortestNote->currentIndex()) {
            case 0: ticks = MScore::division;    break;
            case 1: ticks = MScore::division/2;  break;
            case 2: ticks = MScore::division/4;  break;
            case 3: ticks = MScore::division/8;  break;
            case 4: ticks = MScore::division/16; break;
            }
      preferences.setPreference(PREF_IO_MIDI_SHORTESTNOTE, ticks);

      preferences.setPreference(PREF_IMPORT_OVERTURE_CHARSET, importCharsetListOve->currentText());
      preferences.setPreference(PREF_IMPORT_GUITARPRO_CHARSET, importCharsetListGP->currentText());
      preferences.setPreference(PREF_SCORE_NOTE_WARNPITCHRANGE, warnPitchRange->isChecked());

      preferences.setCustomPreference<MuseScoreStyleType>(PREF_UI_APP_GLOBALSTYLE, MuseScoreStyleType(styleName->currentIndex()));

      if (languageChanged) {
            setMscoreLocale(preferences.getString(PREF_UI_APP_LANGUAGE));
            mscore->update();
            }

      if (defaultStyle->text() != preferences.getString(PREF_SCORE_STYLE_DEFAULTSTYLEFILE)) {
            preferences.setPreference(PREF_SCORE_STYLE_DEFAULTSTYLEFILE, defaultStyle->text());
            MScore::readDefaultStyle(preferences.getString(PREF_SCORE_STYLE_DEFAULTSTYLEFILE));
            }

      if (partStyle->text() != preferences.getString(PREF_SCORE_STYLE_PARTSTYLEFILE)) {
            preferences.setPreference(PREF_SCORE_STYLE_PARTSTYLEFILE, partStyle->text());
            MScore::defaultStyleForPartsHasChanged();
            }

      emit preferencesChanged();
      preferences.save();
      mscore->startAutoSave();
      }

//---------------------------------------------------------
//   resetAllValues
//---------------------------------------------------------

void PreferenceDialog::resetAllValues()
      {
      updateValues(true);

      shortcutsChanged = true;
      qDeleteAll(localShortcuts);
      localShortcuts.clear();
      Shortcut::resetToDefault();
      for (const Shortcut* s : Shortcut::shortcuts())
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
            preferences.clearMidiRemote(i);
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
            // If nothing is checked, prevent looping (run with -s, sequencer disabled)
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
         QFileDialog::ShowDirsOnly | (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS) ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog)
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
         QFileDialog::ShowDirsOnly | (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS) ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog)
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
         QFileDialog::ShowDirsOnly | (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS) ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog)
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
         QFileDialog::ShowDirsOnly | (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS) ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog)
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
         QFileDialog::ShowDirsOnly | (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS) ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog)
         );
      if (!s.isNull())
            myImages->setText(s);
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
//   selectExtensionsDirectory
//---------------------------------------------------------

void PreferenceDialog::selectExtensionsDirectory()
      {
      QString s = QFileDialog::getExistingDirectory(
         this,
         tr("Choose Extensions Folder"),
         myExtensions->text(),
         QFileDialog::ShowDirsOnly | (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS) ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog)
         );
      if (!s.isNull())
            myExtensions->setText(s);
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
//   printShortcutsClicked
//---------------------------------------------------------

void PreferenceDialog::printShortcutsClicked()
      {
#ifndef QT_NO_PRINTER
      QPrinter printer(QPrinter::HighResolution);
      const MStyle& s = MScore::defaultStyle();
      qreal pageW = s.value(Sid::pageWidth).toReal();
      qreal pageH = s.value(Sid::pageHeight).toReal();
      printer.setPaperSize(QSizeF(pageW, pageH), QPrinter::Inch);

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
      qreal y = 0.0;
      qreal lh = QFontMetricsF(p.font()).lineSpacing();

      // get max width for description
      QMapIterator<QString, Shortcut*> isc(localShortcuts);
      qreal col1Width = 0.0;
      while (isc.hasNext()) {
            isc.next();
            Shortcut* sc = isc.value();
            col1Width = qMax(col1Width, QFontMetricsF(p.font()).width(sc->descr()));
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
#endif
}


} // namespace Ms
