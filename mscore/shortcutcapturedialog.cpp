//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: shortcutcapturedialog.cpp 5537 2012-04-16 07:55:10Z wschweer $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
//  Copyright (C) 2003 Mathias Lundgren (lunar_shuttle@users.sourceforge.net)
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

#include "shortcutcapturedialog.h"
#include "musescore.h"
#include "shortcut.h"

//---------------------------------------------------------
//   ShortcutCaptureDialog
//---------------------------------------------------------

ShortcutCaptureDialog::ShortcutCaptureDialog(Shortcut* _s, QMap<QString, Shortcut*> ls, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      localShortcuts = ls;
      s = _s;

      addButton->setEnabled(false);
      replaceButton->setEnabled(false);
      oshrtLabel->setText(s->keysToString());
      connect(clearButton, SIGNAL(clicked()), SLOT(clearClicked()));
      connect(addButton, SIGNAL(clicked()), SLOT(addClicked()));
      connect(replaceButton, SIGNAL(clicked()), SLOT(replaceClicked()));
      clearClicked();
      grabKeyboard();

      oshrtLabel->installEventFilter(this);
      nshrtLabel->installEventFilter(this);
      }


//---------------------------------------------------------
//   addClicked
//---------------------------------------------------------

void ShortcutCaptureDialog::addClicked()
      {
      done(SC_ADD);
      }

//---------------------------------------------------------
//   replaceClicked
//---------------------------------------------------------

void ShortcutCaptureDialog::replaceClicked()
      {
      done(SC_REPLACE);
      }

//---------------------------------------------------------
//   ShortcutCaptureDialog
//---------------------------------------------------------

ShortcutCaptureDialog::~ShortcutCaptureDialog()
      {
      nshrtLabel->removeEventFilter(this);
      oshrtLabel->removeEventFilter(this);
      releaseKeyboard();
      }

//---------------------------------------------------------
//   eventFilter
//---------------------------------------------------------

bool ShortcutCaptureDialog::eventFilter(QObject* o, QEvent* e)
    {
    static const QSet<int> passthruKeys = QSet<int>()
          << Qt::Key_Backspace
          << Qt::Key_Delete
          << Qt::Key_Down
          << Qt::Key_Up
          << Qt::Key_Right
          << Qt::Key_Left;

    // Mac only, harmless on Win
    // Grab certain keys before one of the QLineEdit widgets gets them.
    // Otherwise Qt on mac swallows certain keys, e.g. Backspace, even if the field is read-only.
    if (e->type() == QEvent::KeyPress && passthruKeys.contains(static_cast<QKeyEvent*>(e)->key())) {
        keyPressEvent(static_cast<QKeyEvent*>(e));
        return true;
        }

    return false;
    }

//---------------------------------------------------------
//   extractKeycode
//---------------------------------------------------------

static int extractKeycode(QKeyEvent* e)
      {
      static QSet<int> shiftAllowed = QSet<int>()
            << Qt::Key_Backspace
            << Qt::Key_Delete
            << Qt::Key_Return
            << Qt::Key_Enter
            << Qt::Key_Escape
            << Qt::Key_PageUp
            << Qt::Key_PageDown
            << Qt::Key_Home
            << Qt::Key_End;

      Qt::KeyboardModifiers mods = e->modifiers();
      const int k = e->key();
      
      const QString displayedKey = QKeySequence(k).toString(QKeySequence::NativeText);
      const bool hasCase = displayedKey.toUpper() != displayedKey.toLower();
      const bool isNumpad = mods.testFlag(Qt::KeypadModifier);
      
      if (!isNumpad && !hasCase && !shiftAllowed.contains(k))
            mods &= ~Qt::ShiftModifier;
      
      return k | mods;
      }

//---------------------------------------------------------
//   keyPressEvent
//---------------------------------------------------------

void ShortcutCaptureDialog::keyPressEvent(QKeyEvent* e)
      {
      if (key.count() >= 4)
            return;
      int k = e->key();
      if (k == 0 || k == Qt::Key_Shift || k == Qt::Key_Control ||
         k == Qt::Key_Meta || k == Qt::Key_Alt || k == Qt::Key_AltGr
         || k == Qt::Key_CapsLock || k == Qt::Key_NumLock
         || k == Qt::Key_ScrollLock || k == Qt::Key_unknown)
            return;

      k = extractKeycode(e);
      
      switch(key.count()) {
            case 0: key = QKeySequence(k); break;
            case 1: key = QKeySequence(key[0], k); break;
            case 2: key = QKeySequence(key[0], key[1], k); break;
            case 3: key = QKeySequence(key[0], key[1], key[2], k); break;
            default:
                  qDebug("internal error: bad key count\n");
                  break;
            }

      // Check against conflicting shortcuts

      QList<Shortcut*> conflicts;
      
      foreach (Shortcut* ss, localShortcuts) {
            if (s == ss)
                  continue;
            if (! (s->state() & ss->state()))    // no conflict if states do not overlap
                  continue;
            foreach(const QKeySequence& ks, ss->keys()) {
                  if (ks == key) {
                        conflicts.append(ss);
                        break;
                        }
                  }
            }

      QString msgString;
      if (!conflicts.empty()) {
            QListIterator<Shortcut*> conflict(conflicts);
            msgString = tr("Shortcut conflicts with ") + conflict.next()->descr();
            while (conflict.hasNext())
                  msgString += ", " + conflict.next()->descr();
            }
            
      messageLabel->setText(msgString);
      addButton->setEnabled(conflicts.empty());
      replaceButton->setEnabled(conflicts.empty());
      nshrtLabel->setText(key.toString(QKeySequence::NativeText));

      QString A = key.toString(QKeySequence::NativeText);
      QString B = key.toString(QKeySequence::PortableText);
qDebug("capture key 0x%x  modifiers 0x%x virt 0x%x scan 0x%x <%s><%s>\n",
      k,
      int(e->modifiers()),
      int(e->nativeVirtualKey()),
      int(e->nativeScanCode()),
      qPrintable(A),
      qPrintable(B)
      );
      }

//---------------------------------------------------------
//   clearClicked
//---------------------------------------------------------

void ShortcutCaptureDialog::clearClicked()
      {
      nshrtLabel->setText(tr("Undefined"));
      messageLabel->setText("");
      key = 0;
      }

