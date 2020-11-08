//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  Copyright (C) 2017 Werner Schweer and others
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

#include "startupWizard.h"
#include "musescore.h"
#include "libmscore/mscore.h"
#include "shortcut.h"

namespace Ms {

const static QMap<QString, QString> layoutToShortcut = {
      {QT_TRANSLATE_NOOP("keyboard-layout", "US-QWERTY"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "UK-QWERTY"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "QWERTZ"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "AZERTY"), ":/data/shortcuts_AZERTY.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "BEPO"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "DVORAK"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "JIS - Standard Japanese"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "OYAYUBI SHIFUTO"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "T1 - Standard German"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "T2 - German"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "Swiss German"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "Belgian French"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "Canadian French"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "French French"), ":/data/shortcuts_AZERTY.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "Swiss French"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "Spanish"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "Italian"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "Brazilian Portuguese ABNT"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "Portuguese"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "US - International"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "Dutch"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "Korean - Dubeolsik"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "Korean - Sebeolsik"), ":/data/shortcuts.xml"},
      {QT_TRANSLATE_NOOP("keyboard-layout", "Other"), ":/data/shortcuts.xml"},
      };

const static QMap<QString, QString> langToLayout = {
      {"en", "US-QWERTY"},
      {"ja", "JIS - Standard Japanese"},
      {"de", "T1 - Standard German"},
      {"fr", "French French"},
      {"es", "Spanish"},
      {"it", "Italian"},
      {"pt", "Portuguese"},
      {"nl", "Dutch"},
      {"ko", "Korean - Dubeolsik"}
      };

StartupWizardIntroPage::StartupWizardIntroPage(QWidget* parent)
      : QWizardPage(parent)
      {
      setTitle(tr("Welcome to MuseScore!"));
      QLabel* label = new QLabel(tr("This wizard will help you choose settings for MuseScore based on your locale,\n music level, and personal preferences."), this);
      QVBoxLayout* layout = new QVBoxLayout(this);
      layout->addWidget(label);
      setLayout(layout);
      }

StartupWizardPage1::StartupWizardPage1(QWidget* parent)
      : QWizardPage(parent)
      {
      setTitle(tr("Language"));
      QLabel* label = new QLabel(tr("Choose your language"), this);
      _languages = new QComboBox(this);
      int index = 0;
      for (const auto &language : Ms::mscore->languages()) {
            _languages->addItem(qApp->translate("language", language.name.toUtf8().constData()));
            _languages->setItemData(index, language.key);
            index++;
            }
      QString systemText = _languages->itemText(0) + " (" + mscore->getLocaleISOCode() + ")";
      _languages->setItemText(0, systemText);
      QVBoxLayout* layout = new QVBoxLayout(this);
      layout->addWidget(label);
      layout->addWidget(_languages);
      setLayout(layout);
      QWizardPage::registerField("langIndex", _languages, "currentIndex", "currentIndexChanged()");
      }

QString StartupWizardPage1::language()
      {
      int index = _languages->currentIndex();
      return _languages->itemData(index).toString();
      }

QString StartupWizardPage1::getCurrentLangCode()
      {
      int index = QWizardPage::field("langIndex").toInt();
      if (_languages->itemData(index).toString().compare("system") == 0)
            return mscore->getLocaleISOCode();
      return _languages->itemData(index).toString();
      }

StartupWizardPage2::StartupWizardPage2(QWidget* parent)
      : QWizardPage(parent)
      {
      setTitle(tr("Keyboard Layout"));
      QLabel* label = new QLabel(tr("Enter your keyboard layout"), this);
      QStringList layoutList;
      QList<QString> keyboardLayouts = layoutToShortcut.keys();
      for (const auto &layout : qAsConst(keyboardLayouts))
            layoutList.append(qApp->translate("keyboard-layout", layout.toUtf8().constData()));
      _keyLayouts = new QComboBox(this);
      _keyLayouts->addItems(layoutList);
      int targetIndex = layoutList.indexOf("US - International", 0);
      _keyLayouts->setCurrentIndex(targetIndex);
      QVBoxLayout* layout = new QVBoxLayout(this);
      layout->addWidget(label);
      layout->addWidget(_keyLayouts);
      setLayout(layout);
      this->setCurrentLayout(mscore->getLocaleISOCode());
      }

void StartupWizardPage2::setCurrentLayout(QString langCode)
      {
      langCode = langCode.left(2);
      QString bestLayout = langToLayout.value(langCode, "US - International");
      QStringList layoutList;
      QList<QString> keyboardLayouts = layoutToShortcut.keys();
      for (const auto &layout : qAsConst(keyboardLayouts))
            layoutList.append(qApp->translate("keyboard-layout", layout.toUtf8().constData()));
      int targetIndex = layoutList.indexOf(bestLayout);
      _keyLayouts->setCurrentIndex(targetIndex);
      }

StartupWizardPage4::StartupWizardPage4(QWidget* parent)
      : QWizardPage(parent)
      {
      setTitle(tr("Tours"));
      QLabel* label = new QLabel(tr("Tours will help guide you through the functionality of MuseScore.\n\n Would you like to see these tours?"), this);
      QVBoxLayout* layout = new QVBoxLayout(this);
      layout->addWidget(label);
      yesButton = new QRadioButton(tr("Yes"), this);
      noButton  = new QRadioButton(tr("No"), this);

      yesButton->setChecked(true);
      QHBoxLayout* buttonLayout = new QHBoxLayout();
      buttonLayout->addWidget(yesButton);
      buttonLayout->addWidget(noButton);
      layout->addLayout(buttonLayout);
      setLayout(layout);
      }

StartupWizardFinalPage::StartupWizardFinalPage(QWidget* parent)
      : QWizardPage(parent)
      {
      setTitle(tr("Thank you!"));
      QLabel* label = new QLabel(tr("Your preferences have been successfully saved. Enjoy MuseScore!"), this);
      QVBoxLayout* layout = new QVBoxLayout(this);
      layout->addWidget(label);
      setLayout(layout);
      }

StartupWizard::StartupWizard(QWidget* parent)
      : QWizard(parent)
      {
      setObjectName("StartupWizard");
      auto wizardStyleValue = QWizard::ModernStyle; //Modern Windows look
#ifdef Q_OS_MAC
      wizardStyleValue = QWizard::MacStyle;
      setOption(QWizard::CancelButtonOnLeft, true);
#endif
      setWizardStyle(wizardStyleValue);
      setWindowTitle(tr("Startup Wizard"));

      p0 = new StartupWizardIntroPage(this);
      p1 = new StartupWizardPage1(this);
      p2 = new StartupWizardPage2(this);
      p4 = new StartupWizardPage4(this);
      p5 = new StartupWizardFinalPage(this);

      addPage(p0);
      addPage(p1);
      addPage(p2);
      addPage(p4);
      addPage(p5);

      connect(p1->getLanguages(), SIGNAL(currentIndexChanged(int)), SLOT(langChanged()));
      }

void StartupWizard::langChanged()
      {
      QString langCode = p1->getCurrentLangCode();
      p2->setCurrentLayout(langCode);
      }

void StartupWizard::autoSelectShortcuts(QString keyboardLayout)
      {
      QString fileLocation = layoutToShortcut.value(keyboardLayout, ":/data/shortcuts.xml");
      Shortcut::loadFromNewFile(fileLocation);
      }

}
