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
#include "pathlistdialog.h"
#include "preferences.h"
#include "prefsdialog.h"
#include "resourceManager.h"
#include "seq.h"
#include "scoreview.h"
#include "shortcut.h"
#include "shortcutcapturedialog.h"
#include "timeline.h"
#include "workspace.h"

#include "audio/drivers/pa.h"
#ifdef USE_PORTMIDI
#include "audio/drivers/pm.h"
#endif

#ifdef AVSOMR
#include "avsomr/avsomrlocal.h"
#endif

#ifdef Q_OS_MAC
#include "macos/cocoabridge.h"
#endif

namespace Ms {

//---------------------------------------------------------
//   startPreferenceDialog
//---------------------------------------------------------

void MuseScore::startPreferenceDialog()
      {
      if (!preferenceDialog) {
            preferenceDialog = new PreferenceDialog(this);
            connect(preferenceDialog, &PreferenceDialog::preferencesChanged, this,
               &MuseScore::preferencesChanged);
            connect(preferenceDialog, &PreferenceDialog::mixerPreferencesChanged, this,
               &MuseScore::mixerPreferencesChanged);
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
      shortcutsChanged = false;

      styleName->clear();
      styleName->addItem(tr("Light"));
      styleName->addItem(tr("Dark"));
#ifdef Q_OS_MAC // On Mac, we have a theme option to follow the system's Dark Mode
      if (CocoaBridge::isSystemDarkModeSupported())
            styleName->addItem(tr("System"));
#endif

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
      connect(fgColorButton, &QRadioButton::toggled, this, &PreferenceDialog::updateFgView);

      QButtonGroup* bgButtons = new QButtonGroup(this);
      bgButtons->setExclusive(true);
      bgButtons->addButton(bgColorButton);
      bgButtons->addButton(bgWallpaperButton);
      connect(bgColorButton, &QRadioButton::toggled, this, &PreferenceDialog::updateBgView);

      zoomDefaultType->clear();
      zoomDefaultType->addItem(tr("Percentage"), 0);
      zoomDefaultType->addItem(tr("Page Width"), 1);
      zoomDefaultType->addItem(tr("Whole Page"), 2);
      zoomDefaultType->addItem(tr("Two Pages"), 3);

      zoomPrecisionKeyboard->setRange(ZOOM_PRECISION_MIN, ZOOM_PRECISION_MAX);
      zoomPrecisionMouse->setRange(ZOOM_PRECISION_MIN, ZOOM_PRECISION_MAX);

      connect(buttonBox,            &QDialogButtonBox::clicked, this, &PreferenceDialog::buttonBoxClicked);
      connect(fgWallpaperSelect,    &QToolButton::clicked, this, &PreferenceDialog::selectFgWallpaper);
      connect(bgWallpaperSelect,    &QToolButton::clicked, this, &PreferenceDialog::selectBgWallpaper);

      bgWallpaperSelect->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      fgWallpaperSelect->setIcon(*icons[int(Icons::fileOpen_ICON)]);

      connect(myScoresButton, &QToolButton::clicked, this, &PreferenceDialog::selectScoresDirectory);
      connect(myStylesButton, &QToolButton::clicked, this, &PreferenceDialog::selectStylesDirectory);
      connect(myScoreFontsButton, &QToolButton::clicked, this, &PreferenceDialog::selectScoreFontsDirectory);
      connect(myTemplatesButton, &QToolButton::clicked, this, &PreferenceDialog::selectTemplatesDirectory);
      connect(myPluginsButton, &QToolButton::clicked, this, &PreferenceDialog::selectPluginsDirectory);
      connect(mySoundfontsButton, &QToolButton::clicked, this, &PreferenceDialog::changeSoundfontPaths);
      connect(myImagesButton, &QToolButton::clicked, this, &PreferenceDialog::selectImagesDirectory);
      connect(myExtensionsButton, &QToolButton::clicked, this, &PreferenceDialog::selectExtensionsDirectory);

      myScoresButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      myStylesButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      myScoreFontsButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      myTemplatesButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      myPluginsButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      mySoundfontsButton->setIcon(*icons[int(Icons::edit_ICON)]);
      myImagesButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      myExtensionsButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);

      connect(updateTranslation, &QToolButton::clicked, this, &PreferenceDialog::updateTranslationClicked);

      connect(defaultStyleButton,     &QToolButton::clicked, this, &PreferenceDialog::selectDefaultStyle);
      connect(partStyleButton,        &QToolButton::clicked, this, &PreferenceDialog::selectPartStyle);
      connect(styleFileButton,        &QToolButton::clicked, this, &PreferenceDialog::styleFileButtonClicked);
      connect(instrumentList1Button,  &QToolButton::clicked, this, &PreferenceDialog::selectInstrumentList1);
      connect(instrumentList2Button,  &QToolButton::clicked, this, &PreferenceDialog::selectInstrumentList2);
      connect(scoreOrderList1Button,  &QToolButton::clicked, this, &PreferenceDialog::selectScoreOrderList1);
      connect(scoreOrderList2Button,  &QToolButton::clicked, this, &PreferenceDialog::selectScoreOrderList2);
      connect(startWithButton,        &QToolButton::clicked, this, &PreferenceDialog::selectStartWith);

      defaultStyleButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      partStyleButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      styleFileButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      instrumentList1Button->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      instrumentList2Button->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      scoreOrderList1Button->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      scoreOrderList2Button->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      startWithButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);

      connect(shortcutList,   &QTreeWidget::itemActivated, this, &PreferenceDialog::defineShortcutClicked);
      connect(resetShortcut,  &QToolButton::clicked, this, &PreferenceDialog::resetShortcutClicked);
      connect(saveShortcutList,  &QToolButton::clicked, this, &PreferenceDialog::saveShortcutListClicked);
      connect(loadShortcutList,  &QToolButton::clicked, this, &PreferenceDialog::loadShortcutListClicked);
      connect(clearShortcut, &QToolButton::clicked, this, &PreferenceDialog::clearShortcutClicked);
      connect(defineShortcut, &QToolButton::clicked, this, &PreferenceDialog::defineShortcutClicked);
      connect(resetToDefault, &QToolButton::clicked, this, &PreferenceDialog::resetAllValues);
      connect(filterShortcuts, &QLineEdit::textChanged, this, &PreferenceDialog::filterShortcutsTextChanged);
      connect(printShortcuts, &QToolButton::clicked, this, &PreferenceDialog::printShortcutsClicked);
      connect(zoomDefaultType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PreferenceDialog::zoomDefaultTypeChanged);

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

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
      connect(recordButtons,              QOverload<int>::of(&QButtonGroup::idClicked), this, &PreferenceDialog::recordButtonClicked);
#else
      connect(recordButtons,              QOverload<int>::of(&QButtonGroup::buttonClicked), this, &PreferenceDialog::recordButtonClicked);
#endif
      connect(midiRemoteControlClear,     &QToolButton::clicked, this, &PreferenceDialog::midiRemoteControlClearClicked);
      connect(portaudioDriver,            &QGroupBox::toggled, this, &PreferenceDialog::exclusiveAudioDriver);
      connect(pulseaudioDriver,           &QGroupBox::toggled, this, &PreferenceDialog::exclusiveAudioDriver);
      connect(alsaDriver,                 &QGroupBox::toggled, this, &PreferenceDialog::exclusiveAudioDriver);
      connect(jackDriver,                 &QGroupBox::toggled, this, &PreferenceDialog::exclusiveAudioDriver);
      connect(useJackAudio,               &QRadioButton::toggled, this, &PreferenceDialog::nonExclusiveJackDriver);
      connect(useJackMidi,                &QRadioButton::toggled, this, &PreferenceDialog::nonExclusiveJackDriver);
      connect(rescanDrivers,              &QToolButton::clicked, this, &PreferenceDialog::restartAudioEngine);
      updateRemote();

      advancedWidget = new PreferencesListWidget();
      QVBoxLayout* l = static_cast<QVBoxLayout*> (tabAdvanced->layout());
      l->insertWidget(0, advancedWidget);
      advancedWidget->loadPreferences();
      connect(advancedSearch, &QLineEdit::textChanged, this, &PreferenceDialog::filterAdvancedPreferences);
      connect(resetPreference, &QPushButton::clicked, this, &PreferenceDialog::resetAdvancedPreferenceToDefault);
      connect(this, &PreferenceDialog::preferencesChanged, mscore->timeline(),  &Timeline::updateTimelineTheme); // this should probably be moved to updateUiStyleAndTheme
      MuseScore::restoreGeometry(this);
#if !defined(Q_OS_MAC) && (!defined(Q_OS_WIN) || defined(FOR_WINSTORE)) || !0
      General->removeTab(General->indexOf(tabUpdate)); // updateTab not needed on Linux and not wanted in Windows Store
#endif
      }

//---------------------------------------------------------
//   start
//---------------------------------------------------------

void PreferenceDialog::start()
      {

      // Each entry to one of the vectors controls one widget (and usually one preference).
      // Each entry is a *PreferenceItem(preference_key, widget, optional applyFunction, optional updateFunction).
      // A *PreferenceItem with only the first 2 parameters saves its value on apply and loads it when the dialog starts or when reset is pressed.
      //
      // The default apply/update(load) behavior can be overriden by providing an applyFunction and/or updateFunction (the default value is nullptr).
      // The apply function (either the default or the provided one) is called in PreferenceDialog::Apply.
      // The update function (either the default or the provided one) is called in PreferenceDialog::Update.
      //
      // If the logic is too complicated to be implemented like this (for example for multiple interconnected preferences),
      // doNothing can be used in place of a function to disable the call to that function.
      // Then the related functionality must be called directly from PrefsDialog::Apply (or PrefsDialog::Update).

      const auto doNothing = []() { }; // used to disable the default apply/update functions (in contrast to nullptr which instructs the use of the defaults)

      // Most widgets go here.
      normalWidgets = std::vector<PreferenceItem*>{
                  new IntPreferenceItem(PREF_APP_AUTOSAVE_AUTOSAVETIME, autoSaveTime),
                  new BoolPreferenceItem(PREF_APP_AUTOSAVE_USEAUTOSAVE, autoSave),
                  new StringPreferenceItem(PREF_APP_PATHS_INSTRUMENTLIST1, instrumentList1),
                  new StringPreferenceItem(PREF_APP_PATHS_INSTRUMENTLIST2, instrumentList2),
                  new StringPreferenceItem(PREF_APP_PATHS_SCOREORDERLIST1, scoreOrderList1),
                  new StringPreferenceItem(PREF_APP_PATHS_SCOREORDERLIST2, scoreOrderList2),
                  new StringPreferenceItem(PREF_APP_PATHS_MYIMAGES, myImages),
                  new StringPreferenceItem(PREF_APP_PATHS_MYPLUGINS, myPlugins),
                  new StringPreferenceItem(PREF_APP_PATHS_MYSCOREFONTS, myScoreFonts),
                  new StringPreferenceItem(PREF_APP_PATHS_MYSCORES, myScores),
                  new StringPreferenceItem(PREF_APP_PATHS_MYSOUNDFONTS, mySoundfonts),
                  new StringPreferenceItem(PREF_APP_PATHS_MYSTYLES, myStyles),
                  new StringPreferenceItem(PREF_APP_PATHS_MYTEMPLATES, myTemplates),
                  new StringPreferenceItem(PREF_APP_PATHS_MYEXTENSIONS, myExtensions),
                  new StringPreferenceItem(PREF_APP_STARTUP_STARTSCORE, sessionScore),
                  new IntPreferenceItem(PREF_EXPORT_PDF_DPI, exportPdfDpi),
                  new DoublePreferenceItem(PREF_EXPORT_PNG_RESOLUTION, pngResolution),
                  new BoolPreferenceItem(PREF_EXPORT_PNG_USETRANSPARENCY, pngTransparent),
                  new BoolPreferenceItem(PREF_IMPORT_MUSICXML_IMPORTBREAKS, importBreaks),
                  new BoolPreferenceItem(PREF_IMPORT_MUSICXML_IMPORTLAYOUT, importLayout),
                  new BoolPreferenceItem(PREF_MIGRATION_APPLY_EDWIN_FOR_XML_FILES, applyDefaultTypeFaceToImportedScores,
                                      [this]() { preferences.setPreference(PREF_MIGRATION_APPLY_EDWIN_FOR_XML_FILES, applyDefaultTypeFaceToImportedScores->isChecked()); }, // apply function
                                      [this]() {
                                            bool value = preferences.getBool(PREF_MIGRATION_DO_NOT_ASK_ME_AGAIN_XML) && preferences.getBool(PREF_MIGRATION_APPLY_EDWIN_FOR_XML_FILES);
                                            applyDefaultTypeFaceToImportedScores->setChecked(value);
                                            }), // update function
                  new BoolPreferenceItem(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE, inferTextTypes),
            #ifdef AVSOMR
                  new BoolPreferenceItem(PREF_IMPORT_AVSOMR_USELOCAL, useLocalAvsOmr, [&](){ updateUseLocalAvsOmr(); }),
            #endif
                  new BoolPreferenceItem(PREF_IO_MIDI_ADVANCEONRELEASE, advanceOnRelease),
                  new BoolPreferenceItem(PREF_IO_MIDI_ENABLEINPUT, enableMidiInput),
                  new BoolPreferenceItem(PREF_IO_MIDI_EXPANDREPEATS, expandRepeats),
                  new BoolPreferenceItem(PREF_EXPORT_AUDIO_NORMALIZE, normalize),
                  new BoolPreferenceItem(PREF_IO_MIDI_EXPORTRPNS, exportRPNs),
                  // TODO? new BoolPreferenceItem(PREF_IO_MIDI_SPACELYRICS, spaceLyrics),
                  new IntPreferenceItem(PREF_IO_MIDI_REALTIMEDELAY, realtimeDelay),
                  new BoolPreferenceItem(PREF_IO_MIDI_USEREMOTECONTROL, rcGroup),
                  new IntPreferenceItem(PREF_IO_OSC_PORTNUMBER, oscPort),
                  new BoolPreferenceItem(PREF_IO_OSC_USEREMOTECONTROL, oscServer),
                  new BoolPreferenceItem(PREF_SCORE_CHORD_PLAYONADDNOTE, playChordOnAddNote),
                  new BoolPreferenceItem(PREF_SCORE_HARMONY_PLAY_ONEDIT, playHarmonyOnEdit),
                  new IntPreferenceItem(PREF_SCORE_NOTE_DEFAULTPLAYDURATION, defaultPlayDuration),
                  new BoolPreferenceItem(PREF_SCORE_NOTE_PLAYONCLICK, playNotes),
                  new BoolPreferenceItem(PREF_UI_APP_STARTUP_CHECKUPDATE, checkUpdateStartup),
                  new BoolPreferenceItem(PREF_UI_APP_STARTUP_SHOWNAVIGATOR, navigatorShow),
                  new BoolPreferenceItem(PREF_UI_APP_STARTUP_SHOWPLAYPANEL, playPanelShow),
                  new BoolPreferenceItem(PREF_UI_APP_STARTUP_SHOWSPLASHSCREEN, showSplashScreen),
                  new BoolPreferenceItem(PREF_UI_APP_STARTUP_SHOWSTARTCENTER, showStartcenter),
                  new BoolPreferenceItem(PREF_UI_APP_STARTUP_SHOWTOURS, showTours),
                  new BoolPreferenceItem(PREF_APP_TELEMETRY_ALLOWED, collectTelemetry),
                  new BoolPreferenceItem(PREF_SCORE_NOTE_WARNPITCHRANGE, warnPitchRange),
                  new StringPreferenceItem(PREF_IMPORT_OVERTURE_CHARSET, importCharsetListOve, nullptr, [&](){ updateCharsetListOve(); }),      // keep the default apply
                  new StringPreferenceItem(PREF_IMPORT_GUITARPRO_CHARSET, importCharsetListGP, nullptr, [&](){ updateCharsetListGP(); }),       // keep the default apply
                  new IntPreferenceItem(PREF_EXPORT_AUDIO_SAMPLERATE, exportAudioSampleRate),
                  new IntPreferenceItem(PREF_EXPORT_MP3_BITRATE, exportMp3BitRate),
                  new StringPreferenceItem(PREF_SCORE_STYLE_DEFAULTSTYLEFILE, defaultStyle,
                                          [this]() { // apply function
                                                preferences.setPreference(PREF_SCORE_STYLE_DEFAULTSTYLEFILE, defaultStyle->text());
                                                MScore::readDefaultStyle(preferences.getString(PREF_SCORE_STYLE_DEFAULTSTYLEFILE));
                                                }),
                  new StringPreferenceItem(PREF_SCORE_STYLE_PARTSTYLEFILE, partStyle,
                                          [this]() { // apply function
                                                preferences.setPreference(PREF_SCORE_STYLE_PARTSTYLEFILE, partStyle->text());
                                                MScore::defaultStyleForPartsHasChanged();
                                                }),
                  new StringPreferenceItem(PREF_IMPORT_STYLE_STYLEFILE, importStyleFile,
                                          [this]() { // apply function
                                                preferences.setPreference(PREF_IMPORT_STYLE_STYLEFILE, useImportStyleFile->isChecked() ? importStyleFile->text() : "");
                                                },
                                          [this]() { // update function
                                                QString styleFile = preferences.getString(PREF_IMPORT_STYLE_STYLEFILE);
                                                importStyleFile->setText(styleFile);
                                                useImportBuiltinStyle->setChecked(styleFile.isEmpty());
                                                useImportStyleFile->setChecked(!styleFile.isEmpty());
                                                }),
                  new BoolPreferenceItem(PREF_IO_MIDI_SHOWCONTROLSINMIXER, showMidiControls,
                                          [this]() { // apply function
                                                preferences.setPreference(PREF_IO_MIDI_SHOWCONTROLSINMIXER, showMidiControls->isChecked());
                                                emit mixerPreferencesChanged(preferences.getBool(PREF_IO_MIDI_SHOWCONTROLSINMIXER));
                                                }),
                  new BoolPreferenceItem(PREF_UI_CANVAS_SCROLL_VERTICALORIENTATION, pageVertical,
                                          [this]() { applyPageVertical(); },  // apply function
                                          [this]() { pageVertical->setChecked(MScore::verticalOrientation()); }), // update function
                  new IntPreferenceItem(PREF_IO_MIDI_SHORTESTNOTE, shortestNote,
                                          [this]() { applyShortestNote();  },  // apply function
                                          [this]() { updateShortestNote(); }), // update function
                  new BoolPreferenceItem(PREF_MIGRATION_DO_NOT_ASK_ME_AGAIN, scoreMigrationEnabled,
                                          [this]() { preferences.setPreference(PREF_MIGRATION_DO_NOT_ASK_ME_AGAIN, !scoreMigrationEnabled->isChecked()); }, // apply function
                                          [this]() { scoreMigrationEnabled->setChecked(!preferences.getBool(PREF_MIGRATION_DO_NOT_ASK_ME_AGAIN)); }), // update function
                  new BoolPreferenceItem(PREF_MIGRATION_DO_NOT_ASK_ME_AGAIN_XML),
                  new StringPreferenceItem(PREF_UI_APP_LANGUAGE, language, [&](){ languageApply(); }, [&](){ languageUpdate(); }),
                  new CustomPreferenceItem(PREF_APP_STARTUP_SESSIONSTART, lastSession,
                                          [this]() { // apply function
                                                if (lastSession->isChecked())
                                                      preferences.setCustomPreference<SessionStart>(PREF_APP_STARTUP_SESSIONSTART, SessionStart::LAST);
                                                },
                                          [this]() { // update function
                                                lastSession->setChecked(preferences.sessionStart() == SessionStart::LAST);
                                                }),
                  new CustomPreferenceItem(PREF_APP_STARTUP_SESSIONSTART, newSession,
                                          [this]() { // apply function
                                                if (newSession->isChecked())
                                                      preferences.setCustomPreference<SessionStart>(PREF_APP_STARTUP_SESSIONSTART, SessionStart::NEW);
                                                },
                                          [this]() { // update function
                                                newSession->setChecked(preferences.sessionStart() == SessionStart::NEW);
                                                }),
                  new CustomPreferenceItem(PREF_APP_STARTUP_SESSIONSTART, scoreSession,
                                          [this]() { // apply function
                                                if (scoreSession->isChecked())
                                                      preferences.setCustomPreference<SessionStart>(PREF_APP_STARTUP_SESSIONSTART, SessionStart::SCORE);
                                                },
                                          [this]() { // update function
                                                scoreSession->setChecked(preferences.sessionStart() == SessionStart::SCORE);
                                                }),
                  new CustomPreferenceItem(PREF_APP_STARTUP_SESSIONSTART, emptySession,
                                          [this]() { // apply function
                                               if (emptySession->isChecked())
                                                     preferences.setCustomPreference<SessionStart>(PREF_APP_STARTUP_SESSIONSTART, SessionStart::EMPTY);
                                                },
                                          [this]() { // update function
                                                emptySession->setChecked(preferences.sessionStart() == SessionStart::EMPTY);
                                                }),
                  new BoolPreferenceItem(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, exportAllLayouts),
                  new CustomPreferenceItem(PREF_EXPORT_MUSICXML_EXPORTBREAKS, exportAllLayouts,
                                          [this]() { // apply function
                                                if (exportAllLayouts->isChecked())
                                                      preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::ALL);
                                                },
                                          []()  { // update function
                                                ;
                                                }),
                  new CustomPreferenceItem(PREF_EXPORT_MUSICXML_EXPORTBREAKS, exportAllBreaks,
                                          [this]() { // apply function
                                                if (exportAllBreaks->isChecked())
                                                      preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::ALL);
                                                },
                                          [this]() { // update function
                                                if (!preferences.getBool(PREF_EXPORT_MUSICXML_EXPORTLAYOUT))
                                                      exportAllBreaks->setChecked(preferences.musicxmlExportBreaks() == MusicxmlExportBreaks::ALL);
                                                }),
                  new CustomPreferenceItem(PREF_EXPORT_MUSICXML_EXPORTBREAKS, exportManualBreaks,
                                          [this]() { // apply function
                                                if (exportManualBreaks->isChecked())
                                                      preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::MANUAL);
                                                },
                                          [this]() { // update function
                                                if (!preferences.getBool(PREF_EXPORT_MUSICXML_EXPORTLAYOUT))
                                                      exportManualBreaks->setChecked(preferences.musicxmlExportBreaks() == MusicxmlExportBreaks::MANUAL);
                                                }),
                  new CustomPreferenceItem(PREF_EXPORT_MUSICXML_EXPORTBREAKS, exportNoBreaks,
                                          [this]() { // apply function
                                                if (exportNoBreaks->isChecked())
                                                      preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::NO);
                                                },
                                          [this]() { // update function
                                                if (!preferences.getBool(PREF_EXPORT_MUSICXML_EXPORTLAYOUT))
                                                      exportNoBreaks->setChecked(preferences.musicxmlExportBreaks() == MusicxmlExportBreaks::NO);
                                                }),

      };

      // Contains widgets that change the appearance of MuseScore (style/theme) and/or are saved in Workspaces.
      uiRelatedWidgets = std::vector<PreferenceItem*>{
                  new BoolPreferenceItem(PREF_UI_CANVAS_BG_USECOLOR, bgColorButton, nullptr, [&](){ updateBgView(preferences.getBool(PREF_UI_CANVAS_BG_USECOLOR)); }),
                  new ColorPreferenceItem(PREF_UI_CANVAS_BG_COLOR, bgColorLabel, nullptr, doNothing),
                  new BoolPreferenceItem(PREF_UI_CANVAS_FG_USECOLOR, fgColorButton, nullptr, [&](){ updateFgView(preferences.getBool(PREF_UI_CANVAS_FG_USECOLOR)); }),
                  new BoolPreferenceItem(PREF_UI_CANVAS_FG_USECOLOR_IN_PALETTES, fgUseColorInPalettes, nullptr, doNothing),
                  new ColorPreferenceItem(PREF_UI_CANVAS_FG_COLOR, fgColorLabel, nullptr, doNothing),
                  new StringPreferenceItem(PREF_UI_CANVAS_BG_WALLPAPER, bgWallpaper, nullptr, doNothing),
                  new StringPreferenceItem(PREF_UI_CANVAS_FG_WALLPAPER, fgWallpaper, nullptr, doNothing),
                  new IntPreferenceItem(PREF_UI_CANVAS_ZOOM_DEFAULT_TYPE, zoomDefaultType, nullptr,
                                          [this]() { // update function
                                                zoomDefaultType->setCurrentIndex(preferences.getInt(PREF_UI_CANVAS_ZOOM_DEFAULT_TYPE));
                                                zoomDefaultTypeChanged(preferences.getInt(PREF_UI_CANVAS_ZOOM_DEFAULT_TYPE));
                                                }),
                  new IntPreferenceItem(PREF_UI_CANVAS_ZOOM_DEFAULT_LEVEL, zoomDefaultLevel,
                                          [this]() { preferences.setPreference(PREF_UI_CANVAS_ZOOM_DEFAULT_LEVEL, zoomDefaultLevel->value()); }, // apply function
                                          [this]() { zoomDefaultLevel->setValue(preferences.getInt(PREF_UI_CANVAS_ZOOM_DEFAULT_LEVEL)); }        // update function
                                                ),
                  new IntPreferenceItem(PREF_UI_CANVAS_ZOOM_PRECISION_KEYBOARD, zoomPrecisionKeyboard),
                  new IntPreferenceItem(PREF_UI_CANVAS_ZOOM_PRECISION_MOUSE, zoomPrecisionMouse),
                  new BoolPreferenceItem(PREF_UI_CANVAS_MISC_ANTIALIASEDDRAWING, drawAntialiased),
                  new BoolPreferenceItem(PREF_UI_CANVAS_SCROLL_LIMITSCROLLAREA, limitScrollArea),
                  new IntPreferenceItem(PREF_UI_CANVAS_MISC_SELECTIONPROXIMITY, proximity),
                  new IntPreferenceItem(PREF_UI_THEME_ICONWIDTH, iconWidth),
                  new IntPreferenceItem(PREF_UI_THEME_ICONHEIGHT, iconHeight),
                  new StringPreferenceItem(PREF_UI_THEME_FONTFAMILY, fontFamily, nullptr,
                                          [&]() { // update function
                                                auto currFontFamily = preferences.getString(PREF_UI_THEME_FONTFAMILY);
                                                if (-1 == fontFamily->findText(currFontFamily))
                                                      fontFamily->addItem(currFontFamily);
                                                fontFamily->setCurrentIndex(fontFamily->findText(currFontFamily));
                                                }),
                  new DoublePreferenceItem(PREF_UI_THEME_FONTSIZE, fontSize),
                  new CustomPreferenceItem(PREF_UI_APP_GLOBALSTYLE, styleName,
                                          [&]() { // apply function
                                                preferences.setCustomPreference<MuseScorePreferredStyleType>(PREF_UI_APP_GLOBALSTYLE, MuseScorePreferredStyleType(styleName->currentIndex()));
                                                },
                                          [&]() { // update function
                                                styleName->setCurrentIndex(int(preferences.preferredGlobalStyle()));
                                                }),
      };

      // All these widgets are interconnected and are used together to set their preferences. Don't add unrelated, or not immediately related widgets.
      audioRelatedWidgets = std::vector<PreferenceItem*>{
                  new BoolPreferenceItem(PREF_IO_ALSA_USEALSAAUDIO, alsaDriver, doNothing),
                  new BoolPreferenceItem(PREF_IO_JACK_USEJACKAUDIO, useJackAudio, doNothing),
#ifdef USE_PORTAUDIO
                  new BoolPreferenceItem(PREF_IO_PORTAUDIO_USEPORTAUDIO, portaudioDriver, doNothing),
#endif
                  new BoolPreferenceItem(PREF_IO_PULSEAUDIO_USEPULSEAUDIO, pulseaudioDriver, doNothing),
                  new BoolPreferenceItem(PREF_IO_JACK_USEJACKMIDI, useJackMidi, doNothing),
                  new BoolPreferenceItem(PREF_IO_JACK_USEJACKTRANSPORT, useJackTransport, doNothing),
                  new BoolPreferenceItem(PREF_IO_JACK_TIMEBASEMASTER, becomeTimebaseMaster, doNothing),
                  new BoolPreferenceItem(PREF_IO_JACK_REMEMBERLASTCONNECTIONS, rememberLastMidiConnections, doNothing),
#ifdef USE_ALSA
                  new StringPreferenceItem(PREF_IO_ALSA_DEVICE, alsaDevice, doNothing),
                  new IntPreferenceItem(PREF_IO_ALSA_SAMPLERATE, alsaSampleRate, doNothing),
                  new IntPreferenceItem(PREF_IO_ALSA_PERIODSIZE, alsaPeriodSize, doNothing),
                  new IntPreferenceItem(PREF_IO_ALSA_FRAGMENTS, alsaFragments, doNothing),
#endif
#ifdef USE_PORTAUDIO
                  new IntPreferenceItem(PREF_IO_PORTAUDIO_DEVICE, portaudioApi, doNothing, doNothing),
                  new IntPreferenceItem(PREF_IO_PORTAUDIO_DEVICE, portaudioDevice, doNothing, doNothing),
#endif
#ifdef USE_PORTMIDI
                  new StringPreferenceItem(PREF_IO_PORTMIDI_INPUTDEVICE, portMidiInput, doNothing, doNothing),
                  new StringPreferenceItem(PREF_IO_PORTMIDI_OUTPUTDEVICE, portMidiOutput, doNothing, doNothing),
                  new IntPreferenceItem(PREF_IO_PORTMIDI_OUTPUTLATENCYMILLISECONDS, portMidiOutputLatencyMilliseconds),
            #endif
      };

      // These connections are used to enable the Apply button and to save only the changed preferences.

      for (auto& x : normalWidgets)
            connect(x, &PreferenceItem::editorValueModified, this, &PreferenceDialog::widgetModified);

      for (auto& x : uiRelatedWidgets)
            connect(x, &PreferenceItem::editorValueModified, this, &PreferenceDialog::uiWidgetModified);

      for (auto& x : audioRelatedWidgets)
            connect(x, &PreferenceItem::editorValueModified, this, &PreferenceDialog::audioWidgetModified);

      for (auto& x : advancedWidget->preferenceItems())
            connect(x, &PreferenceItem::editorValueModified, this, &PreferenceDialog::applyActivate);

      updateValues(false, true);
      modifiedWidgets.clear();
      modifiedUiWidgets.clear();
      modifiedAudioWidgets.clear();
      applySetActive(false);
      show();
      }

//---------------------------------------------------------
//   ~PreferenceDialog
//---------------------------------------------------------

PreferenceDialog::~PreferenceDialog()
      {
      for (size_t i = 0; i < normalWidgets.size(); i++)
            delete normalWidgets.at(i);
      normalWidgets.clear();

      for (size_t i = 0; i < uiRelatedWidgets.size(); i++)
            delete uiRelatedWidgets.at(i);
      uiRelatedWidgets.clear();

      for (size_t i = 0; i < audioRelatedWidgets.size(); i++)
            delete audioRelatedWidgets.at(i);
      audioRelatedWidgets.clear();

      qDeleteAll(localShortcuts);
      }

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void PreferenceDialog::retranslate()
      {
      retranslateUi(this);
      updateValues();
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
      for (QAbstractButton*& b : recordButtons->buttons()) {
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

void PreferenceDialog::updateValues(bool useDefaultValues, bool setup)
      {
      if (useDefaultValues)
            preferences.setReturnDefaultValuesMode(true);

      styleName->clear();
      styleName->addItem(tr("Light"));
      styleName->addItem(tr("Dark"));
#ifdef Q_OS_MAC // On Mac, we have a theme option to follow the system's Dark Mode
      if (CocoaBridge::isSystemDarkModeSupported())
            styleName->addItem(tr("System"));
#endif

      advancedWidget->updatePreferences();

      //macOS default fonts are not in QFontCombobox because they are "private":
      //https://code.woboq.org/qt5/qtbase/src/widgets/widgets/qfontcombobox.cpp.html#329

      jackDriver->setChecked(preferences.getBool(PREF_IO_JACK_USEJACKAUDIO) || preferences.getBool(PREF_IO_JACK_USEJACKMIDI));

#ifndef AVSOMR
    groupBox_omr->setVisible(false);
#endif

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

                  connect(portaudioApi, QOverload<int>::of(&QComboBox::activated), this, &PreferenceDialog::portaudioApiActivated);
#ifdef USE_PORTMIDI
                  PortMidiDriver* midiDriver = static_cast<PortMidiDriver*>(audio->mididriver());
                  if (midiDriver) {
                        QStringList midiInputs = midiDriver->deviceInList();
                        int curMidiInIdx = 0;
                        portMidiInput->clear();
                        portMidiInput->insertItem(0," "); // note: a space to be different from the default empty string.
                        // The current input device can be different from the saved preference if the preference was empty
                        // because of automatic grabbing of the default input device if not explicitly told otherwise.
                        // Therefore, comparison must be done with respect to the actual current input device name.
                        const PmDeviceInfo* info = Pm_GetDeviceInfo(midiDriver->getInputId());
                        QString portmidiInputDevice;
                        if(info && (info->input))
                              portmidiInputDevice = QString(info->interf) + "," + QString(info->name);
                        for(int i = 0; i < midiInputs.size(); ++i) {
                              portMidiInput->insertItem(i+1, midiInputs.at(i));
                              if (midiInputs.at(i) == portmidiInputDevice)
                                    curMidiInIdx = i + 1;
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
                        }
#endif
                  }
            }
#endif

#ifndef HAS_MIDI
      enableMidiInput->setEnabled(false);
      rcGroup->setEnabled(false);
#endif

      for (auto& x : normalWidgets)
            x->update(setup);

      for (auto& x : uiRelatedWidgets)
            x->update(setup);

      for (auto& x : audioRelatedWidgets)
            x->update(setup);

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
      for (Shortcut*& s : localShortcuts) {
            if (!s)
                  continue;
            ShortcutItem* newItem = new ShortcutItem;
            newItem->setText(0, s->descr());
            if (s->icon() != Icons::Invalid_ICON)
                  newItem->setIcon(0, *icons[int(s->icon())]);
            newItem->setText(1, s->keysToString());
            newItem->setData(0, Qt::UserRole, s->key());
            QString accessibleInfo = tr("Action: %1; Shortcut: %2")
               .arg(newItem->text(0), newItem->text(1).isEmpty()
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
      applySetActive(true);
      }

//---------------------------------------------------------
//   saveShortcutListClicked
//---------------------------------------------------------

void PreferenceDialog::saveShortcutListClicked()
      {
      QString saveFileName = QFileDialog::getSaveFileName(this, tr("Save Shortcuts"), preferences.getString(PREF_APP_PATHS_MYSHORTCUTS) + "/shortcuts.xml", tr("MuseScore Shortcuts File") + " (*.xml)", 0, preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS) ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
      preferences.setPreference(PREF_APP_PATHS_MYSHORTCUTS, saveFileName);
      Shortcut::saveToNewFile(saveFileName);
      }

//---------------------------------------------------------
//   loadShortcutListClicked
//---------------------------------------------------------

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
      applySetActive(true);
      }

//--------------------------------------------------------
//   filterShortcutsTextChanged
//--------------------------------------------------------

void  PreferenceDialog::filterShortcutsTextChanged(const QString &query )
      {
      QTreeWidgetItem *item;
      for(int i = 0; i < shortcutList->topLevelItemCount(); i++) {
          item = shortcutList->topLevelItem(i);
          item->setHidden(!(item->text(0).contains(query, Qt::CaseInsensitive) || item->text(1).contains(query, Qt::CaseInsensitive)));
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
      for (QTreeWidgetItem*& item : advancedWidget->selectedItems()) {
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
//   selectScoreOrderList1
//---------------------------------------------------------

void PreferenceDialog::selectScoreOrderList1()
      {
      QString s = QFileDialog::getOpenFileName(
         this,
         tr("Choose Score Order List"),
         scoreOrderList1->text(),
         tr("Score Order List") + " (*.xml)",
         0,
         preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS) ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog
         );
      if (!s.isNull())
            scoreOrderList1->setText(s);
      }

//---------------------------------------------------------
//   selectScoreOrderList2
//---------------------------------------------------------

void PreferenceDialog::selectScoreOrderList2()
      {
      QString s = QFileDialog::getOpenFileName(
         this,
         tr("Choose Score Order List"),
         scoreOrderList2->text(),
         tr("Score Order List") + " (*.xml)",
         0,
         preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS) ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog
         );
      if (!s.isNull())
            scoreOrderList2->setText(s);
      }

//---------------------------------------------------------
//   zoomDefaultTypeChanged
//---------------------------------------------------------

void PreferenceDialog::zoomDefaultTypeChanged(const int index)
      {
      // Only enable editing of [zoom-percentage spinner widget] if [Percentage] is selected
      zoomDefaultLevel->setEnabled(static_cast<ZoomType>(index) == ZoomType::PERCENTAGE);
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
      fgUseColorInPalettes->setChecked(preferences.getBool(PREF_UI_CANVAS_FG_USECOLOR_IN_PALETTES));
      fgWallpaper->setEnabled(!useColor);
      fgWallpaperSelect->setEnabled(!useColor);
      fgUseColorInPalettes->setEnabled(useColor);

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
//   languageUpdate
//---------------------------------------------------------

void PreferenceDialog::languageUpdate()
      {
      language->blockSignals(true);
      language->clear();
      QString lang = preferences.getString(PREF_UI_APP_LANGUAGE);
      int curIdx = 0;
      for(int i = 0; i < mscore->languages().size(); ++i) {
            language->addItem(mscore->languages().at(i).name, i);
            if (mscore->languages().at(i).key == lang)
                  curIdx = i;
            }
      language->blockSignals(false);
      language->setCurrentIndex(curIdx);
      }

//---------------------------------------------------------
//   languageApply
//---------------------------------------------------------

void PreferenceDialog::languageApply()
      {
      int lang = language->itemData(language->currentIndex()).toInt();
      QString l = lang == 0 ? "system" : mscore->languages().at(lang).key;
      bool languageChanged = l != preferences.getString(PREF_UI_APP_LANGUAGE);

      if (languageChanged) {
            preferences.setPreference(PREF_UI_APP_LANGUAGE, l);
            setMscoreLocale(preferences.getString(PREF_UI_APP_LANGUAGE));
            mscore->update();
            }
      }


//---------------------------------------------------------
//   updateCharsetListGP
//---------------------------------------------------------

void PreferenceDialog::updateCharsetListGP()
      {
      QList<QByteArray> charsets = QTextCodec::availableCodecs();
      std::sort(charsets.begin(), charsets.end());
      int idx = 0;
      importCharsetListGP->clear();
      for (QByteArray& charset : charsets) {
            importCharsetListGP->addItem(charset);
            if (charset == preferences.getString(PREF_IMPORT_GUITARPRO_CHARSET))
                  importCharsetListGP->setCurrentIndex(idx);
            idx++;
            }
      }

//---------------------------------------------------------
//   updateCharsetListOve
//---------------------------------------------------------

void PreferenceDialog::updateCharsetListOve()
      {
      QList<QByteArray> charsets = QTextCodec::availableCodecs();
      std::sort(charsets.begin(), charsets.end());
      int idx = 0;
      importCharsetListOve->clear();
      for (QByteArray& charset : charsets) {
            importCharsetListOve->addItem(charset);
            if (charset == preferences.getString(PREF_IMPORT_OVERTURE_CHARSET))
                  importCharsetListOve->setCurrentIndex(idx);
            idx++;
            }
      }

#ifdef AVSOMR
//---------------------------------------------------------
//   updateUseLocalAvsOmr
//---------------------------------------------------------

void PreferenceDialog::updateUseLocalAvsOmr()
      {
      useLocalAvsOmr->setChecked(preferences.getBool(PREF_IMPORT_AVSOMR_USELOCAL));
      Avs::AvsOmrLocal::instance()->isInstalledAsync([this](bool isInstalled) {
            QString text = QObject::tr("Use local OMR engine");
            if (isInstalled)
                  text += " (" + QObject::tr("Installed") + ")";
            else
                  text += " (" + QObject::tr("Not installed, needs internet connection for installing") + ")";

            useLocalAvsOmr->setText(text);
            });
      }
#else
void PreferenceDialog::updateUseLocalAvsOmr()
      {
      ;
      }
#endif

//---------------------------------------------------------
//   applyPageVertical
//---------------------------------------------------------

void PreferenceDialog::applyPageVertical()
      {
      const auto cv = mscore->currentScoreView();
      preferences.setPreference(PREF_UI_CANVAS_SCROLL_VERTICALORIENTATION, pageVertical->isChecked());
      MScore::setVerticalOrientation(pageVertical->isChecked());
      for (Score* s : qAsConst(mscore->scores())) {
            s->doLayout();
            for (Score*& ss : s->scoreList())
                  ss->doLayout();
            }
      if (cv)
            cv->pageTop();
      mscore->scorePageLayoutChanged();
      mscore->update();
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
//   applySetActive
//---------------------------------------------------------

void PreferenceDialog::applySetActive(bool active)
      {
      buttonBox->button(QDialogButtonBox::Apply)->setEnabled(active);
      }


//---------------------------------------------------------
//   applyActivate
//---------------------------------------------------------

void PreferenceDialog::applyActivate()
      {
      applySetActive(true);
      }

//---------------------------------------------------------
//   applyActivate
//---------------------------------------------------------

void PreferenceDialog::checkApplyActivation()
      {
      qDebug() << modifiedWidgets.size() << " " << modifiedUiWidgets.size() << " " << modifiedAudioWidgets.size();
      if (modifiedWidgets.size() == 0 && modifiedUiWidgets.size() == 0 && modifiedAudioWidgets.size() == 0)
            applySetActive(false);
      }

//---------------------------------------------------------
//   widgetModified
//---------------------------------------------------------

void PreferenceDialog::widgetModified()
      {
      PreferenceItem* item = static_cast<PreferenceItem*>(sender());
      const auto itemIter = std::find(modifiedWidgets.begin(), modifiedWidgets.end(), item);
      if (itemIter == modifiedWidgets.end()) {
            modifiedWidgets.push_back(item);
            applySetActive(true);
            }
      else {
            if (!item->isModified()) {
                  modifiedWidgets.erase(itemIter);
                  checkApplyActivation();
                  }
            }
      }

//---------------------------------------------------------
//   uiWidgetModified
//---------------------------------------------------------

void PreferenceDialog::uiWidgetModified()
      {
      PreferenceItem* item = static_cast<PreferenceItem*>(sender());
      const auto itemIter = std::find(modifiedUiWidgets.begin(), modifiedUiWidgets.end(), item);
      if (itemIter == modifiedUiWidgets.end()) {
            modifiedUiWidgets.push_back(item);
            applySetActive(true);
            }
      else {
            if (!item->isModified()) {
                  modifiedUiWidgets.erase(itemIter);
                  checkApplyActivation();
                  }
            }
      }


//---------------------------------------------------------
//   audioWidgetModified
//---------------------------------------------------------

void PreferenceDialog::audioWidgetModified()
      {
      PreferenceItem* item = static_cast<PreferenceItem*>(sender());
      const auto itemIter = std::find(modifiedAudioWidgets.begin(), modifiedAudioWidgets.end(), item);
      if (itemIter == modifiedAudioWidgets.end()) {
            modifiedAudioWidgets.push_back(item);
            applySetActive(true);
            }
      else {
            if (!item->isModified()) {
                  modifiedAudioWidgets.erase(itemIter);
                  checkApplyActivation();
                  }
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void PreferenceDialog::apply()
      {
      const auto cv = mscore->currentScoreView();

      QElapsedTimer timer;
      timer.start();

      if(buttonBox->button(QDialogButtonBox::Apply)->isEnabled() == false)
            return;

      bool uiStyleThemeChanged = false;
      bool audioModified = false;

      applySetActive(false);
      buttonBox->button(QDialogButtonBox::Apply)->setText(tr("Applying…"));
      buttonBox->repaint();

      std::vector<QString> changedAdvancedProperties = advancedWidget->save();
      for (auto& x : changedAdvancedProperties)
            if (x.startsWith("ui"))
                  uiStyleThemeChanged = true;

      for (auto& x : modifiedWidgets)
            x->apply();

      for (auto& x : modifiedUiWidgets) {
            x->apply();
            uiStyleThemeChanged = true;
            }

      if (modifiedAudioWidgets.size() > 0)
            audioModified = true;

#ifdef AVSOMR
      preferences.setPreference(PREF_IMPORT_AVSOMR_USELOCAL, useLocalAvsOmr->isChecked());
#endif

      if (audioModified) {
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
                        if (seq->driver() == nullptr) {
                              qDebug("sequencer driver is null");
                              restartAudioEngine();
                              }
                        seq->driver()->init(true);
                        if (!seq->init(true))
                              qDebug("sequencer init failed");
                        }
                  }
            else if (
               (wasJack != nowJack)
               || (preferences.getBool(PREF_IO_ALSA_USEALSAAUDIO) != alsaDriver->isChecked())
#ifdef USE_PORTAUDIO
               || (preferences.getBool(PREF_IO_PORTAUDIO_USEPORTAUDIO) != portaudioDriver->isChecked())
#endif
               || (preferences.getBool(PREF_IO_PULSEAUDIO_USEPULSEAUDIO) != pulseaudioDriver->isChecked())
#ifdef USE_ALSA
               || (preferences.getString(PREF_IO_ALSA_DEVICE) != alsaDevice->text())
               || (preferences.getInt(PREF_IO_ALSA_SAMPLERATE) != alsaSampleRate->currentData().toInt())
               || (preferences.getInt(PREF_IO_ALSA_PERIODSIZE) != alsaPeriodSize->currentData().toInt())
               || (preferences.getInt(PREF_IO_ALSA_FRAGMENTS) != alsaFragments->value())
#endif
                  ) {
                  preferences.setPreference(PREF_IO_ALSA_USEALSAAUDIO, alsaDriver->isChecked());
#ifdef USE_PORTAUDIO
                  preferences.setPreference(PREF_IO_PORTAUDIO_USEPORTAUDIO, portaudioDriver->isChecked());
#endif
                  preferences.setPreference(PREF_IO_PULSEAUDIO_USEPULSEAUDIO, pulseaudioDriver->isChecked());
#ifdef USE_ALSA
                  preferences.setPreference(PREF_IO_ALSA_DEVICE, alsaDevice->text());
                  preferences.setPreference(PREF_IO_ALSA_SAMPLERATE, alsaSampleRate->currentData().toInt());
                  preferences.setPreference(PREF_IO_ALSA_PERIODSIZE, alsaPeriodSize->currentData().toInt());
                  preferences.setPreference(PREF_IO_ALSA_FRAGMENTS, alsaFragments->value());
#endif

                  restartAudioEngine();
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
            preferences.setPreference(PREF_IO_PORTMIDI_OUTPUTLATENCYMILLISECONDS, portMidiOutputLatencyMilliseconds->value());
            if (seq->driver() && static_cast<PortMidiDriver*>(static_cast<Portaudio*>(seq->driver())->mididriver())->isSameCoreMidiIacBus(preferences.getString(PREF_IO_PORTMIDI_INPUTDEVICE), preferences.getString(PREF_IO_PORTMIDI_OUTPUTDEVICE))) {
                  QMessageBox msgBox;
                  msgBox.setWindowTitle(tr("Possible MIDI Loopback"));
                  msgBox.setIcon(QMessageBox::Warning);
                  msgBox.setText(tr("Warning: You used the same CoreMIDI IAC bus for input and output. This will cause problematic loopback, whereby MuseScore's output MIDI messages will be sent back to MuseScore as input, causing confusion. To avoid this problem, access Audio MIDI Setup via Spotlight to create a dedicated virtual port for MuseScore's MIDI output, restart MuseScore, return to Preferences, and select your new virtual port for MuseScore's MIDI output. Other programs may then use that dedicated virtual port to receive MuseScore's MIDI output."));
                  msgBox.exec();
                  }
#endif
            }

      if (shortcutsChanged) {
            shortcutsChanged = false;
            for(Shortcut*& s : localShortcuts) {
                  Shortcut* os = Shortcut::getShortcut(s->key());
                  if (os) {
                        if (!os->compareKeys(*s))
                              os->setKeys(*s);
                        }
                  }
            Shortcut::dirty = true;
            }

      if(uiStyleThemeChanged) {
            WorkspacesManager::retranslateAll();
            preferences.setPreference(PREF_APP_WORKSPACE, WorkspacesManager::currentWorkspace()->id());
            WorkspacesManager::currentWorkspace()->save();
            emit mscore->workspacesChanged();
            }

      emit preferencesChanged(false, uiStyleThemeChanged);
      //uiStyleThemeChanged = false;
      preferences.save();
      mscore->startAutoSave();

      // Smooth panning
      if (cv) {
            cv->panSettings().loadFromPreferences();
            cv->setControlCursorVisible(preferences.getBool(PREF_PAN_CURSOR_VISIBLE));
            }

      modifiedWidgets.clear();
      modifiedUiWidgets.clear();
      modifiedAudioWidgets.clear();

      buttonBox->button(QDialogButtonBox::Apply)->setText(tr("Apply"));
      qDebug() << "Final: " << timer.elapsed();
      }

//---------------------------------------------------------
//   resetAllValues
//---------------------------------------------------------

void PreferenceDialog::resetAllValues()
      {
      updateValues(true);

      shortcutsChanged = true;
      applySetActive(true);
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
//   selectScoreFontsDirectory
//---------------------------------------------------------

void PreferenceDialog::selectScoreFontsDirectory()
      {
      QString s = QFileDialog::getExistingDirectory(
         this,
         tr("Choose Score Fonts Folder"),
         myScoreFonts->text(),
         QFileDialog::ShowDirsOnly | (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS) ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog)
         );
      if (!s.isNull())
            myScoreFonts->setText(s);
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
      ResourceManager r(this);
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
      applySetActive(true);
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
      printer.setPageSize(QPageSize(QSizeF(pageW, pageH), QPageSize::Inch));

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

//---------------------------------------------------------
//   rebuildAudioDrivers
//---------------------------------------------------------

void PreferenceDialog::restartAudioEngine()
      {
      mscore->restartAudioEngine();
      }

//---------------------------------------------------------
//   updateShortestNote
//---------------------------------------------------------

void PreferenceDialog::updateShortestNote()
      {
      int shortestNoteIndex;
      int nn = preferences.getInt(PREF_IO_MIDI_SHORTESTNOTE);
      if (nn == MScore::division)
            shortestNoteIndex = 0;           // Quarter
      else if (nn == MScore::division / 2)
            shortestNoteIndex = 1;  // Eighth
      else if (nn == MScore::division / 4)
            shortestNoteIndex = 2;  // etc.
      else if (nn == MScore::division / 8)
            shortestNoteIndex = 3;
      else if (nn == MScore::division / 16)
            shortestNoteIndex = 4;
      else if (nn == MScore::division / 32)
            shortestNoteIndex = 5;
      else if (nn == MScore::division / 64)
            shortestNoteIndex = 6;
      else if (nn == MScore::division / 128)
            shortestNoteIndex = 7;
      else if (nn == MScore::division / 256)
            shortestNoteIndex = 8;
      else {
            qDebug("Unknown shortestNote value of %d, defaulting to 16th", nn);
            shortestNoteIndex = 2;
            }
      shortestNote->setCurrentIndex(shortestNoteIndex);
      }

//---------------------------------------------------------
//   applyShortestNote
//---------------------------------------------------------

void PreferenceDialog::applyShortestNote()
      {
      int ticks;
      switch (shortestNote->currentIndex()) {
            case 0: ticks = MScore::division;       break;
            case 1: ticks = MScore::division / 2;   break;
            case 2: ticks = MScore::division / 4;   break;
            case 3: ticks = MScore::division / 8;   break;
            case 4: ticks = MScore::division / 16;  break;
            case 5: ticks = MScore::division / 32;  break;
            case 6: ticks = MScore::division / 64;  break;
            case 7: ticks = MScore::division / 128; break;
            case 8: ticks = MScore::division / 256; break;
            default: {
                  qDebug("Unknown index for shortestNote: %d, defaulting to 16th",
                         shortestNote->currentIndex());
                  ticks = MScore::division / 4;
                  }
            }
      preferences.setPreference(PREF_IO_MIDI_SHORTESTNOTE, ticks);
      }

} // namespace Ms
