//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  Copyright (C) 2009-2017 Werner Schweer and others
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

#ifndef __STARTUPWIZARD_H__
#define __STARTUPWIZARD_H__

#include <QWizard>

namespace Ms {

class StartupWizardIntroPage : public QWizardPage {
      Q_OBJECT

   public:
      StartupWizardIntroPage(QWidget* parent = 0);
      void init();
      };

class StartupWizardPage1 : public QWizardPage {
      Q_OBJECT

      QComboBox* _languages;

   public:
      StartupWizardPage1(QWidget* parent = 0);
      QString language();
      void init();
      QComboBox* getLanguages()     { return _languages; }
      QString getCurrentLangCode();
      };

class StartupWizardPage2 : public QWizardPage {
      Q_OBJECT

      QComboBox* _keyLayouts;

   public:
      StartupWizardPage2(QWidget* parent = 0);
      QString keyboardLayout()    { return _keyLayouts->currentText(); }
      void init();
      void setCurrentLayout(QString langCode);
      };

class StartupWizardPage3 : public QWizardPage {
      Q_OBJECT

      QComboBox* _workspaces;

   public:
      StartupWizardPage3(QWidget* parent = 0);
      QString workspace()   { return _workspaces->currentText(); }
      void init();
      };

class StartupWizardPage4 : public QWizardPage {
      Q_OBJECT

      QRadioButton* yesButton;
      QRadioButton* noButton;

   public:
      StartupWizardPage4(QWidget* parent = 0);
      bool showTours()  { return yesButton->isChecked(); }
      void init();
      };

class StartupWizardFinalPage : public QWizardPage {
      Q_OBJECT

   public:
      StartupWizardFinalPage(QWidget* parent = 0);
      void init();
      };

class StartupWizard : public QWizard {
      Q_OBJECT

      StartupWizardIntroPage* p0;
      StartupWizardPage1* p1;
      StartupWizardPage2* p2;
      StartupWizardPage3* p3;
      StartupWizardPage4* p4;
      StartupWizardFinalPage* p5;

   public:
      StartupWizard(QWidget* parent = 0);
      static void autoSelectShortcuts(QString keyboardLayout);
      QString keyboardLayout()      { return p2->keyboardLayout(); }
      QString language()     { return p1->language(); }
      QString workspace()    { return p3->workspace(); }
      bool showTours()       { return p4->showTours(); }

   private slots:
      void langChanged();
      };
}
#endif
