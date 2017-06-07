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
QString StartupWizard::layoutInput()
      {
      QString lang = Ms::mscore->getLocaleISOCode();
      lang = lang.left(2);

      QMap<QString, QStringList> *langToLayout = new QMap<QString, QStringList>();
      QStringList layoutList;
      //QMap<QString, QString> *systemLang;

      layoutList << QObject::tr("US-QWERTY") << QObject::tr("UK-QWERTY") << QObject::tr("QWERTZ") << QObject::tr("AZERTY") << QObject::tr("BEPO") << QObject::tr("DVORAK") << QObject::tr("OTHER");
      langToLayout->insert("en", layoutList);
      layoutList.clear();

      layoutList << QObject::tr("JIS - Standard Japanese") << QObject::tr("OYAYUBI SHIFUTO") << QObject::tr("OTHER");
      langToLayout->insert("ja", layoutList);
      layoutList.clear();

      layoutList << QObject::tr("T1 - Standard German") << QObject::tr("T2") << QObject::tr("Swiss German") << QObject::tr("OTHER");
      langToLayout->insert("de", layoutList);
      layoutList.clear();

      layoutList << QObject::tr("Belgian French") << QObject::tr("Canadian French") << QObject::tr("French French") << QObject::tr("Swiss French") << QObject::tr("OTHER");
      langToLayout->insert("fr", layoutList);
      layoutList.clear();

      layoutList << QObject::tr("Spanish") << QObject::tr("OTHER");
      langToLayout->insert("es", layoutList);
      layoutList.clear();

      layoutList << QObject::tr("Italian") << QObject::tr("OTHER");
      langToLayout->insert("it", layoutList);
      layoutList.clear();

      layoutList << QObject::tr("Portuguese Brazilian ABNT") << QObject::tr("Portuguese") << QObject::tr("OTHER");
      langToLayout->insert("pt", layoutList);
      layoutList.clear();

      layoutList << QObject::tr("US-International") << QObject::tr("Dutch Layout") << QObject::tr("OTHER");
      langToLayout->insert("nl", layoutList);
      layoutList.clear();

      layoutList << QObject::tr("Dubeolsik (두벌식)") << QObject::tr("Sebeolsik (세벌식)") << QObject::tr("OTHER");
      langToLayout->insert("ko", layoutList);
      layoutList.clear();

      layoutList << QObject::tr("US-QWERTY") << QObject::tr("UK-QWERTY") << QObject::tr("QWERTZ") << QObject::tr("AZERTY") << QObject::tr("BEPO") << QObject::tr("DVORAK") << QObject::tr("JIS - Standard Japanese") << QObject::tr("OYAYUBI SHIFUTO") << QObject::tr("T1 - Standard German") << QObject::tr("T2") << QObject::tr("Swiss German") << QObject::tr("Belgian French") << QObject::tr("Canadian French") << QObject::tr("French French") << QObject::tr("Swiss French") << QObject::tr("Spanish") << QObject::tr("Italian") << QObject::tr("Portuguese Brazilian ABNT") << QObject::tr("Portuguese") << QObject::tr("US-International") << QObject::tr("Dutch Layout") << QObject::tr("Dubeolsik (두벌식)") << QObject::tr("Sebeolsik (세벌식)") << QObject::tr("OTHER");
      langToLayout->insert("OTHER", layoutList);
      layoutList.clear();

      /*systemLang->insert("en", "English");
      systemLang->insert("ja", "Japanese");
      systemLang->insert("de", "German");
      systemLang->insert("fr", "French");
      systemLang->insert("it", "Italian");
      systemLang->insert("pt", "Portuguese");
      systemLang->insert("nl", "Dutch");
      systemLang->insert("ko", "Korean");*/

      //QString labelMessage = "Detected Language: " + systemLang->values(lang)[0] + ". Enter your keyboard layout:";
      QString labelMessage = QObject::tr("Enter your keyboard layout");
      QString result = QInputDialog::getItem(new QWidget(), "Startup Wizard", labelMessage, langToLayout->values(lang)[0]);
      if (result == "OTHER") {
            labelMessage = QObject::tr("Choose keyboard layout:");
            QString result = QInputDialog::getItem(new QWidget(), "Startup Wizard", labelMessage, langToLayout->values(result)[0]);
            }
      return result;
      }

void StartupWizard::autoSelectShortcuts(QString keyboardLayout)
      {
      QMap<QString, QString> *layoutToShortcut = new QMap<QString, QString>();
      layoutToShortcut->insert("US-QWERTY", ":/data/shortcuts.xml");
      layoutToShortcut->insert("UK-QWERTY", ":/data/shortcuts.xml");
      layoutToShortcut->insert("QWERTZ", ":/data/shortcuts.xml");
      layoutToShortcut->insert("AZERTY", ":/data/shortcuts_AZERTY.xml");
      layoutToShortcut->insert("BEPO", ":/data/shortcuts.xml");
      layoutToShortcut->insert("DVORAK", ":/data/shortcuts.xml");
      layoutToShortcut->insert("JIS - Standard Japanese", ":/data/shortcuts.xml");
      layoutToShortcut->insert("OYAYUBI SHIFUTO", ":/data/shortcuts.xml");
      layoutToShortcut->insert("T1 - Standard German", ":/data/shortcuts.xml");
      layoutToShortcut->insert("T2", ":/data/shortcuts.xml");
      layoutToShortcut->insert("Swiss German", ":/data/shortcuts.xml");
      layoutToShortcut->insert("Belgian French", ":/data/shortcuts.xml");
      layoutToShortcut->insert("Canadian French", ":/data/shortcuts.xml");
      layoutToShortcut->insert("French French", ":/data/shortcuts_AZERTY.xml");
      layoutToShortcut->insert("Swiss French", ":/data/shortcuts.xml");
      layoutToShortcut->insert("Spanish", ":/data/shortcuts.xml");
      layoutToShortcut->insert("Italian", ":/data/shortcuts.xml");
      layoutToShortcut->insert("Portuguese Brazilian ABNT", ":/data/shortcuts.xml");
      layoutToShortcut->insert("Portuguese", ":/data/shortcuts.xml");
      layoutToShortcut->insert("US-International", ":/data/shortcuts.xml");
      layoutToShortcut->insert("Dutch Layout", ":/data/shortcuts.xml");
      layoutToShortcut->insert("Dubeolsik (두벌식)", ":/data/shortcuts.xml");
      layoutToShortcut->insert("Sebeolsik (세벌식)", ":/data/shortcuts.xml");
      layoutToShortcut->insert("OTHER", ":/data/shortcuts.xml");

      QString fileLocation = layoutToShortcut->value(keyboardLayout);
      Shortcut::loadFromNewFile(fileLocation);
      }
}
