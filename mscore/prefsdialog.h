//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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

#ifndef __PREFSDIALOG_H__
#define __PREFSDIALOG_H__

#include "ui_prefsdialog.h"
#include "preferences.h"
#include "abstractdialog.h"
#include "preferenceslistwidget.h"

namespace Ms {

class Shortcut;

//---------------------------------------------------------
//   PreferenceDialog
//---------------------------------------------------------

class PreferenceDialog : public AbstractDialog, private Ui::PrefsDialogBase {
      Q_OBJECT

      QMap<QString, Shortcut*> localShortcuts;
      bool shortcutsChanged;
      QButtonGroup* recordButtons;
      std::vector<PreferenceItem*> normalWidgets;
      std::vector<PreferenceItem*> uiRelatedWidgets;
      std::vector<PreferenceItem*> audioRelatedWidgets;
      std::vector<PreferenceItem*> modifiedWidgets;
      std::vector<PreferenceItem*> modifiedUiWidgets;
      std::vector<PreferenceItem*> modifiedAudioWidgets;
      PreferencesListWidget* advancedWidget;

      virtual void hideEvent(QHideEvent*);
      void apply();
      void updateSCListView();
      void setUseMidiOutput(bool);
      void updateValues(bool useDefaultValues = false, bool setup = false);
      void checkApplyActivation();

      void applySetActive(bool active);
      void updateShortestNote();
      void applyShortestNote();
      void languageUpdate();
      void languageApply();
      void updateCharsetListGP();
      void updateCharsetListOve();
      void updateUseLocalAvsOmr();
      void applyPageVertical();

   private slots:
      void buttonBoxClicked(QAbstractButton*);
      void updateBgView(bool);
      void updateFgView(bool);
      void selectFgWallpaper();
      void selectBgWallpaper();
      void selectDefaultStyle();
      void selectPartStyle();
      void selectInstrumentList1();
      void selectInstrumentList2();
      void selectScoreOrderList1();
      void selectScoreOrderList2();
      void selectStartWith();
      void resetShortcutClicked();
      void saveShortcutListClicked();
      void loadShortcutListClicked();
      void clearShortcutClicked();
      void defineShortcutClicked();
      void portaudioApiActivated(int idx);
      void resetAllValues();
      void styleFileButtonClicked();
      void recordButtonClicked(int);
      void midiRemoteControlClearClicked();
      void exclusiveAudioDriver(bool on);
      void nonExclusiveJackDriver(bool on);
      void selectScoresDirectory();
      void selectStylesDirectory();
      void selectScoreFontsDirectory();
      void selectTemplatesDirectory();
      void selectPluginsDirectory();
      void selectImagesDirectory();
      void selectExtensionsDirectory();
      void zoomDefaultTypeChanged(int);
      void printShortcutsClicked();
      void filterShortcutsTextChanged(const QString &);
      void filterAdvancedPreferences(const QString&);
      void resetAdvancedPreferenceToDefault();
      void restartAudioEngine();
      void widgetModified();
      void uiWidgetModified();
      void audioWidgetModified();
      void applyActivate();

      void changeSoundfontPaths();
      void updateTranslationClicked();

   signals:
      void preferencesChanged(bool fromWorkspace, bool changeUI);
      void mixerPreferencesChanged(bool showMidiControls);

   protected:
      virtual void retranslate();

   public:
      PreferenceDialog(QWidget* parent);
      ~PreferenceDialog();
      void start();
      void updateRemote();
      };

//---------------------------------------------------------
//   ShortcutItem
//---------------------------------------------------------

class ShortcutItem : public QTreeWidgetItem {

      bool operator<(const QTreeWidgetItem&) const;

   public:
      ShortcutItem() : QTreeWidgetItem() {}
      };

} // namespace Ms
#endif

