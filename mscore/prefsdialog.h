//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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
#include "advancedpreferenceswidget.h"

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
      int _currentTabIndex;

      virtual void hideEvent(QHideEvent*);
      void apply();
      void updateSCListView();
      void setUseMidiOutput(bool);
      void updateValues(bool useDefaultValues = false);

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
      void selectTemplatesDirectory();
      void selectPluginsDirectory();
      void selectImagesDirectory();
      void selectExtensionsDirectory();
      void printShortcutsClicked();
      void filterShortcutsTextChanged(const QString&);

      void changeSoundfontPaths();
      void updateTranslationClicked();

      void tabAboutToChange(int index);

   signals:
      void preferencesChanged();
      void mixerPreferencesChanged(bool showMidiControls);

   protected:
      virtual void retranslate() { retranslateUi(this); updateValues(); }

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

