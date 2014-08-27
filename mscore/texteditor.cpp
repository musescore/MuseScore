//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: editstyle.cpp 3537 2010-10-01 10:52:51Z wschweer $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include "texteditor.h"
#include "icons.h"

namespace Ms {

//---------------------------------------------------------
//   TextEditor
//---------------------------------------------------------

TextEditor::TextEditor(QWidget* parent)
   : QDialog(parent)
      {
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      QVBoxLayout* vl = new QVBoxLayout;
      setLayout(vl);
      QFrame* f = new QFrame;
      QHBoxLayout* hl = new QHBoxLayout;
      hl->setSpacing(0);
      f->setLayout(hl);

      typefaceBold = new QToolButton;
      typefaceBold->setIcon(*icons[int(Icons::textBold_ICON)]);
      typefaceBold->setToolTip(tr("bold"));
      typefaceBold->setCheckable(true);

      typefaceItalic = new QToolButton;
      typefaceItalic->setIcon(*icons[int(Icons::textItalic_ICON)]);
      typefaceItalic->setToolTip(tr("italic"));
      typefaceItalic->setCheckable(true);

      typefaceUnderline = new QToolButton;
      typefaceUnderline->setIcon(*icons[int(Icons::textUnderline_ICON)]);
      typefaceUnderline->setToolTip(tr("underline"));
      typefaceUnderline->setCheckable(true);

      leftAlign   = new QToolButton;
      leftAlign->setIcon(*icons[int(Icons::textLeft_ICON)]);
      leftAlign->setToolTip(tr("align left"));
      leftAlign->setCheckable(true);

      centerAlign = new QToolButton;
      centerAlign->setIcon(*icons[int(Icons::textCenter_ICON)]);
      centerAlign->setToolTip(tr("align center"));
      centerAlign->setCheckable(true);

      rightAlign  = new QToolButton;
      rightAlign->setIcon(*icons[int(Icons::textRight_ICON)]);
      rightAlign->setToolTip(tr("align right"));
      rightAlign->setCheckable(true);

      typefaceSubscript = new QToolButton;
      typefaceSubscript->setIcon(*icons[int(Icons::textSub_ICON)]);
      typefaceSubscript->setToolTip(tr("subscript"));
      typefaceSubscript->setCheckable(true);

      typefaceSuperscript = new QToolButton;
      typefaceSuperscript->setIcon(*icons[int(Icons::textSuper_ICON)]);
      typefaceSuperscript->setToolTip(tr("superscript"));
      typefaceSuperscript->setCheckable(true);

      typefaceSize = new QDoubleSpinBox(this);
      typefaceSize->setRange(0.0, 100.0);
      typefaceFamily = new QFontComboBox(this);

      hl->addWidget(typefaceBold);
      hl->addWidget(typefaceItalic);
      hl->addWidget(typefaceUnderline);
      hl->addStretch(10);
      hl->addWidget(leftAlign);
      hl->addWidget(centerAlign);
      hl->addWidget(rightAlign);
      hl->addStretch(10);
      hl->addWidget(typefaceSubscript);
      hl->addWidget(typefaceSuperscript);
      hl->addStretch(10);
      hl->addWidget(typefaceFamily);
      hl->addWidget(typefaceSize);
      hl->addStretch(10);

      vl->addWidget(f);
      edit = new QTextEdit;
      vl->addWidget(edit);

      dialogButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
      vl->addWidget(dialogButtons);

      charFormatChanged(edit->currentCharFormat());
      cursorPositionChanged();

      connect(typefaceBold,        SIGNAL(clicked(bool)), SLOT(toggleBold(bool)));
      connect(typefaceItalic,      SIGNAL(clicked(bool)), SLOT(toggleItalic(bool)));
      connect(typefaceUnderline,   SIGNAL(clicked(bool)), SLOT(toggleUnderline(bool)));
      connect(leftAlign,           SIGNAL(clicked(bool)), SLOT(toggleLeftAlign(bool)));
      connect(centerAlign,         SIGNAL(clicked(bool)), SLOT(toggleCenterAlign(bool)));
      connect(rightAlign,          SIGNAL(clicked(bool)), SLOT(toggleRightAlign(bool)));
      connect(typefaceSubscript,   SIGNAL(clicked(bool)), SLOT(toggleTypefaceSubscript(bool)));
      connect(typefaceSuperscript, SIGNAL(clicked(bool)), SLOT(toggleTypefaceSuperscript(bool)));
      connect(edit, SIGNAL(currentCharFormatChanged(const QTextCharFormat&)), SLOT(charFormatChanged(const QTextCharFormat&)));
      connect(edit, SIGNAL(cursorPositionChanged()), SLOT(cursorPositionChanged()));
      connect(typefaceSize,        SIGNAL(valueChanged(double)), SLOT(sizeChanged(double)));
      connect(typefaceFamily,      SIGNAL(currentFontChanged(const QFont&)), SLOT(fontChanged(const QFont&)));
      connect(dialogButtons,       SIGNAL(accepted()), this, SLOT(okClicked()));
      connect(dialogButtons,       SIGNAL(rejected()), this, SLOT(cancelClicked()));
      connect(this,                SIGNAL(rejected()), SLOT(cancelEditing()));
      }

//---------------------------------------------------------
//   setHtml
//---------------------------------------------------------

void TextEditor::setHtml(const QString& txt)
      {
      originalText = txt;
      edit->setHtml(originalText);
      editAsHtml = true;
      }

//---------------------------------------------------------
//   toHtml
//---------------------------------------------------------

QString TextEditor::toHtml() const
      {
      return edit->toHtml();
      }

//---------------------------------------------------------
//   setPlainText
//---------------------------------------------------------

void TextEditor::setPlainText(const QString& txt)
      {
      originalText = txt;
      edit->setPlainText(originalText);
      editAsHtml = false;
      }

//---------------------------------------------------------
//   toPlainText
//---------------------------------------------------------

QString TextEditor::toPlainText() const
      {
      return edit->toPlainText();
      }

//---------------------------------------------------------
//   editText
//---------------------------------------------------------

QString editPlainText(const QString& s, const QString& title)
      {
      TextEditor editor;
      editor.setWindowModality(Qt::WindowModal);
      editor.setWindowTitle(title);
      editor.setPlainText(s);
      editor.exec();
      return editor.toPlainText();
      }

//---------------------------------------------------------
//   editHtml
//---------------------------------------------------------

QString editHtml(const QString& s, const QString& title)
      {
      TextEditor editor;
      editor.setWindowModality(Qt::WindowModal);
      editor.setWindowTitle(title);
      editor.setHtml(s);
      editor.exec();
      return editor.toHtml();
      }

//---------------------------------------------------------
//   okClicked
//---------------------------------------------------------

void TextEditor::okClicked()
      {
      accept();
      }

//---------------------------------------------------------
//   cancelClicked
//---------------------------------------------------------

void TextEditor::cancelClicked()
      {
      reject();
      }

//---------------------------------------------------------
//   cancel
//---------------------------------------------------------

void TextEditor::cancelEditing()
      {
      if (editAsHtml)
            edit->setHtml(originalText);
      else
            edit->setPlainText(originalText);
      }

//---------------------------------------------------------
//   toggleBold
//---------------------------------------------------------

void TextEditor::toggleBold(bool val)
      {
      edit->setFontWeight(val ? 75 : 50);
      }

//---------------------------------------------------------
//   toggleUnderline
//---------------------------------------------------------

void TextEditor::toggleUnderline(bool val)
      {
      edit->setFontUnderline(val);
      }

//---------------------------------------------------------
//   toggleItalic
//---------------------------------------------------------

void TextEditor::toggleItalic(bool val)
      {
      edit->setFontItalic(val);
      }

//---------------------------------------------------------
//   toggleLeftAlign
//---------------------------------------------------------

void TextEditor::toggleLeftAlign(bool val)
      {
      QTextCursor cursor = edit->textCursor();
      QTextBlockFormat f = cursor.blockFormat();
      f.setAlignment(Qt::AlignLeft);
      cursor.setBlockFormat(f);
      edit->setTextCursor(cursor);
      if (val) {
            //leftAlign->setChecked(false);
            centerAlign->setChecked(false);
            rightAlign->setChecked(false);
            }
      }

//---------------------------------------------------------
//   toggleCenterAlign
//---------------------------------------------------------

void TextEditor::toggleCenterAlign(bool val)
      {
      QTextCursor cursor = edit->textCursor();
      QTextBlockFormat f = cursor.blockFormat();
      f.setAlignment(val ? Qt::AlignHCenter : Qt::AlignLeft);
      cursor.setBlockFormat(f);
      edit->setTextCursor(cursor);

      if (val) {
            leftAlign->setChecked(false);
            // centerAlign
            rightAlign->setChecked(false);
            }
      }

//---------------------------------------------------------
//   toggleRightAlign
//---------------------------------------------------------

void TextEditor::toggleRightAlign(bool val)
      {
      QTextCursor cursor = edit->textCursor();
      QTextBlockFormat f = cursor.blockFormat();
      f.setAlignment(val ? Qt::AlignRight : Qt::AlignLeft);
      cursor.setBlockFormat(f);
      edit->setTextCursor(cursor);
      if (val) {
            leftAlign->setChecked(false);
            centerAlign->setChecked(false);
            // rightAlign->setChecked(false);
            }
      }

//---------------------------------------------------------
//   toggleTypefaceSubscript
//---------------------------------------------------------

void TextEditor::toggleTypefaceSubscript(bool val)
      {
      QTextCharFormat cf = edit->currentCharFormat();
      cf.setVerticalAlignment(val ? QTextCharFormat::AlignSubScript : QTextCharFormat::AlignNormal);
      edit->setCurrentCharFormat(cf);
      }

//---------------------------------------------------------
//   toggleTypefaceSuperscript
//---------------------------------------------------------

void TextEditor::toggleTypefaceSuperscript(bool val)
      {
      QTextCharFormat cf = edit->currentCharFormat();
      cf.setVerticalAlignment(val ? QTextCharFormat::AlignSuperScript : QTextCharFormat::AlignNormal);
      edit->setCurrentCharFormat(cf);
      }

//---------------------------------------------------------
//   sizeChanged
//---------------------------------------------------------

void TextEditor::sizeChanged(double size)
      {
      QTextCharFormat cf = edit->currentCharFormat();
      cf.setFontPointSize(size);
      edit->setCurrentCharFormat(cf);
      }

//---------------------------------------------------------
//   fontChanged
//---------------------------------------------------------

void TextEditor::fontChanged(const QFont& f)
      {
      QTextCharFormat cf = edit->currentCharFormat();
      cf.setFontFamily(f.family());
      edit->setCurrentCharFormat(cf);
      }

//---------------------------------------------------------
//   charFormatChanged
//---------------------------------------------------------

void TextEditor::charFormatChanged(const QTextCharFormat& f)
      {
      typefaceItalic->setChecked(f.fontItalic());
      typefaceUnderline->setChecked(f.fontUnderline());
      typefaceBold->setChecked(f.fontWeight() >= 75);
      if (f.verticalAlignment() == QTextCharFormat::AlignSuperScript) {
            typefaceSuperscript->setChecked(true);
            typefaceSubscript->setChecked(false);
            }
      else if (f.verticalAlignment() == QTextCharFormat::AlignSubScript) {
            typefaceSuperscript->setChecked(false);
            typefaceSubscript->setChecked(true);
            }
      else {
            typefaceSuperscript->setChecked(false);
            typefaceSubscript->setChecked(false);
            }
      typefaceSize->setValue(f.fontPointSize());
      typefaceFamily->setCurrentFont(f.font());
      }

//---------------------------------------------------------
//   cursorPositionChanged
//---------------------------------------------------------

void TextEditor::cursorPositionChanged()
      {
      QTextCursor cursor = edit->textCursor();
      QTextBlockFormat f = cursor.blockFormat();
      if (f.alignment() & Qt::AlignLeft) {
            leftAlign->setChecked(true);
            centerAlign->setChecked(false);
            rightAlign->setChecked(false);
            }
      else if (f.alignment() & Qt::AlignHCenter) {
            leftAlign->setChecked(false);
            centerAlign->setChecked(true);
            rightAlign->setChecked(false);
            }
      else if (f.alignment() & Qt::AlignRight) {
            leftAlign->setChecked(false);
            centerAlign->setChecked(false);
            rightAlign->setChecked(true);
            }
      }

}

