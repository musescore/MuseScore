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

StartupWizardPage1::StartupWizardPage1(QWidget* parent)
    : QWizardPage(parent)
      {
      setTitle(QObject::tr("Keyboard Layout"));
      QLabel *label = new QLabel(QObject::tr("Enter your keyboard layout"), this);
      QStringList layoutList;
      QList<QString> keyboardLayouts = layoutToShortcut.keys();
      for (auto layout : keyboardLayouts) {
            layoutList.append(qApp->translate("keyboard-layout", layout.toStdString().c_str()));
            }
      _keyLayouts = new QComboBox(this);
      _keyLayouts->addItems(layoutList);
      QVBoxLayout *layout = new QVBoxLayout(this);
      QString lang = Ms::mscore->getLocaleISOCode().left(2);
      QString bestLayout = langToLayout.value(lang, "US-QWERTY");
      int targetIndex = layoutList.indexOf(bestLayout);
      _keyLayouts->setCurrentIndex(targetIndex);
      layout->addWidget(label);
      layout->addWidget(_keyLayouts);
      setLayout(layout);
      }

StartupWizard::StartupWizard(QWidget* parent)
    : QWizard(parent)
      {
      setObjectName("StartupWizard");
      setWizardStyle(QWizard::ClassicStyle);
      setWindowTitle(tr("Startup Wizard"));

      p1 = new StartupWizardPage1(this);

      addPage(p1);
      }

void StartupWizard::autoSelectShortcuts(QString keyboardLayout)
      {
      QString fileLocation = layoutToShortcut.value(keyboardLayout, ":/data/shortcuts.xml");
      Shortcut::loadFromNewFile(fileLocation);
      }

}
