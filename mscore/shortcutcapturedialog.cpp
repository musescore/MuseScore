//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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

namespace Ms {

//---------------------------------------------------------
//   ShortcutCaptureDialog
//---------------------------------------------------------

ShortcutCaptureDialog::ShortcutCaptureDialog(Shortcut* _s, QMap<QString, Shortcut*> ls, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("ShortcutCaptureDialog");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      localShortcuts = ls;
      s = _s;

      addButton->setEnabled(false);
      replaceButton->setEnabled(false);
      oshrtLabel->setText(s->keysToString());
      oshrtTextLabel->setAccessibleDescription(s->keysToString());
      oshrtLabel->setEnabled(false);
      connect(clearButton, SIGNAL(clicked()), SLOT(clearClicked()));
      connect(addButton, SIGNAL(clicked()), SLOT(addClicked()));
      connect(replaceButton, SIGNAL(clicked()), SLOT(replaceClicked()));
      clearClicked();

      nshrtLabel->installEventFilter(this);
      MuseScore::restoreGeometry(this);

      // Force the focus onto this dialog. This is required in osx to get command+backspace
      setFocus();
      }

// We don't allow the dialog to loose focus as on osx. if we would allow, on osx command-backspace
// couldn't be tracked.
void ShortcutCaptureDialog::focusOutEvent (QFocusEvent* /*event*/)
      {
      setFocus();
      }

//---------------------------------------------------------
//   addClicked
//---------------------------------------------------------

void ShortcutCaptureDialog::addClicked()
      {
      done(1);
      }

//---------------------------------------------------------
//   replaceClicked
//---------------------------------------------------------

void ShortcutCaptureDialog::replaceClicked()
      {
      done(2);
      }

//---------------------------------------------------------
//   ShortcutCaptureDialog
//---------------------------------------------------------

ShortcutCaptureDialog::~ShortcutCaptureDialog()
      {
      nshrtLabel->removeEventFilter(this);
      releaseKeyboard();
      }

//---------------------------------------------------------
//   eventFilter
//---------------------------------------------------------

bool ShortcutCaptureDialog::eventFilter(QObject* /*o*/, QEvent* e)
      {
      if (e->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
            if(keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab){
                  QWidget::keyPressEvent(keyEvent);
                  return true;
                  }
            keyPress(keyEvent);
            return true;
            }
      return false;
      }


//---------------------------------------------------------
//   keyPressEvent
//---------------------------------------------------------

void ShortcutCaptureDialog::keyPress(QKeyEvent* e)
      {
      if (key.count() >= 4)
            return;
      int k = e->key();
      if (k == 0 || k == Qt::Key_Shift || k == Qt::Key_Control ||
         k == Qt::Key_Meta || k == Qt::Key_Alt || k == Qt::Key_AltGr
         || k == Qt::Key_CapsLock || k == Qt::Key_NumLock
         || k == Qt::Key_ScrollLock || k == Qt::Key_unknown)
            return;

      k += e->modifiers();
      // remove shift-modifier for keys that don't need it: letters and special keys
      if ((k & Qt::ShiftModifier) && ((e->key() < 0x41) || (e->key() > 0x5a) || (e->key() >= 0x01000000))) {
            qDebug() << k;
            k -= Qt::ShiftModifier;
            qDebug() << k;
            }

      switch(key.count()) {
            case 0: key = QKeySequence(k); break;
            case 1: key = QKeySequence(key[0], k); break;
            case 2: key = QKeySequence(key[0], key[1], k); break;
            case 3: key = QKeySequence(key[0], key[1], key[2], k); break;
            default:
                  qDebug("Internal error: bad key count");
                  break;
            }

      // Check against conflicting shortcuts
      bool conflict = false;
      QString msgString;

      for (Shortcut* ss : localShortcuts) {
            if (s == ss)
                  continue;
            if (!(s->state() & ss->state()))    // no conflict if states do not overlap
                  continue;

            QList<QKeySequence> skeys = QKeySequence::keyBindings(ss->standardKey());

            for (const QKeySequence& ks : skeys) {
                  if (ks == key) {
                        msgString = tr("Shortcut conflicts with %1").arg(ss->descr());
                        conflict = true;
                        break;
                        }
                  }

            for (const QKeySequence& ks : ss->keys()) {
                  if (ks == key) {
                        msgString = tr("Shortcut conflicts with %1").arg(ss->descr());
                        conflict = true;
                        break;
                        }
                  }
            if (conflict)
                  break;
            }

      messageLabel->setText(msgString);

      if (conflict) {
            if (!nshrtLabel->accessibleName().contains(tr("Shortcut conflicts with")))
                  nshrtLabel->setAccessibleName(msgString);
            }
      else {
            if (!nshrtLabel->accessibleName().contains("New shortcut"))
                  nshrtLabel->setAccessibleName(tr("New shortcut"));
            }
      addButton->setEnabled(conflict == false);
      replaceButton->setEnabled(conflict == false);

//      nshrtLabel->setText(key.toString(QKeySequence::NativeText));
      QString keyStr = Shortcut::keySeqToString(key, QKeySequence::NativeText);
      nshrtLabel->setText(keyStr);
//      QString A = key.toString(QKeySequence::NativeText);
      QString A = keyStr;
      QString B = Shortcut::keySeqToString(key, QKeySequence::PortableText);
      qDebug("capture key 0x%x  modifiers 0x%x virt 0x%x scan 0x%x <%s><%s>",
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
      if (!nshrtLabel->accessibleName().contains("New shortcut"))
            nshrtLabel->setAccessibleName(tr("New shortcut"));

      nshrtLabel->setAccessibleName(tr("New shortcut"));
      messageLabel->setText("");
      addButton->setEnabled(false);
      replaceButton->setEnabled(false);
      nshrtLabel->setText("");
      key = 0;
      nshrtLabel->setFocus();
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void ShortcutCaptureDialog::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }

} // namespace Ms

