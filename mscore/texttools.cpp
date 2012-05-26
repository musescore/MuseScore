//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: textpalette.cpp 3592 2010-10-18 17:24:18Z wschweer $
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

#include "texttools.h"
#include "icons.h"
#include "libmscore/text.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "textpalette.h"
#include "libmscore/mscore.h"

TextPalette* textPalette;

//---------------------------------------------------------
//   textTools
//---------------------------------------------------------

TextTools* MuseScore::textTools()
      {
      if (!_textTools) {
            _textTools = new TextTools(this);
            addDockWidget(Qt::BottomDockWidgetArea, _textTools);
            }
      setFocusPolicy(Qt::NoFocus);
      return _textTools;
      }

//---------------------------------------------------------
//   TextTools
//---------------------------------------------------------

TextTools::TextTools(QWidget* parent)
   : QDockWidget(parent)
      {
      _textElement = 0;
      setObjectName("text-tools");
      setWindowTitle(tr("Text Tools"));
      setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);

      QToolBar* tb = new QToolBar(tr("Text Edit"));

      textStyles = new QComboBox;
      tb->addWidget(textStyles);

      showKeyboard = getAction("show-keys");
      tb->addAction(showKeyboard);
      showKeyboard->setCheckable(true);

      typefaceBold = tb->addAction(*icons[textBold_ICON], "");
      typefaceBold->setToolTip(tr("bold"));
      typefaceBold->setCheckable(true);

      typefaceItalic = tb->addAction(*icons[textItalic_ICON], "");
      typefaceItalic->setToolTip(tr("italic"));
      typefaceItalic->setCheckable(true);

      typefaceUnderline = tb->addAction(*icons[textUnderline_ICON], "");
      typefaceUnderline->setToolTip(tr("underline"));
      typefaceUnderline->setCheckable(true);

      tb->addSeparator();

      leftAlign   = tb->addAction(*icons[textLeft_ICON],   "");
      leftAlign->setToolTip(tr("align left"));
      leftAlign->setCheckable(true);

      centerAlign = tb->addAction(*icons[textCenter_ICON], "");
      centerAlign->setToolTip(tr("align horizontal center"));
      centerAlign->setCheckable(true);

      rightAlign  = tb->addAction(*icons[textRight_ICON],  "");
      rightAlign->setToolTip(tr("align right"));
      rightAlign->setCheckable(true);

      topAlign  = tb->addAction(*icons[textTop_ICON],  "");
      topAlign->setToolTip(tr("align top"));
      topAlign->setCheckable(true);

      bottomAlign  = tb->addAction(*icons[textBottom_ICON],  "");
      bottomAlign->setToolTip(tr("align bottom"));
      bottomAlign->setCheckable(true);

      vcenterAlign  = tb->addAction(*icons[textVCenter_ICON],  "");
      vcenterAlign->setToolTip(tr("align vertical center"));
      vcenterAlign->setCheckable(true);

      typefaceSubscript   = tb->addAction(*icons[textSub_ICON], "");
      typefaceSubscript->setToolTip(tr("subscript"));
      typefaceSubscript->setCheckable(true);

      typefaceSuperscript = tb->addAction(*icons[textSuper_ICON], "");
      typefaceSuperscript->setToolTip(tr("superscript"));
      typefaceSuperscript->setCheckable(true);

      unorderedList = tb->addAction(*icons[formatListUnordered_ICON], "");
      unorderedList->setToolTip(tr("unordered list"));

      orderedList = tb->addAction(*icons[formatListOrdered_ICON], "");
      orderedList->setToolTip(tr("ordered list"));

      indentMore = tb->addAction(*icons[formatIndentMore_ICON], "");
      indentMore->setToolTip(tr("indent more"));

      indentLess = tb->addAction(*icons[formatIndentLess_ICON], "");
      indentLess->setToolTip(tr("indent less"));

      tb->addSeparator();

      typefaceFamily = new QFontComboBox(this);
      tb->addWidget(typefaceFamily);
      typefaceSize = new QDoubleSpinBox(this);
      tb->addWidget(typefaceSize);

      setWidget(tb);
      QWidget* w = new QWidget(this);
      setTitleBarWidget(w);
      titleBarWidget()->hide();

      connect(typefaceSize,        SIGNAL(valueChanged(double)), SLOT(sizeChanged(double)));
      connect(typefaceFamily,      SIGNAL(currentFontChanged(const QFont&)), SLOT(fontChanged(const QFont&)));
      connect(typefaceBold,        SIGNAL(triggered(bool)), SLOT(boldClicked(bool)));
      connect(typefaceItalic,      SIGNAL(triggered(bool)), SLOT(italicClicked(bool)));
      connect(typefaceUnderline,   SIGNAL(triggered(bool)), SLOT(underlineClicked(bool)));
      connect(typefaceSubscript,   SIGNAL(triggered(bool)), SLOT(subscriptClicked(bool)));
      connect(typefaceSuperscript, SIGNAL(triggered(bool)), SLOT(superscriptClicked(bool)));
      connect(typefaceFamily,      SIGNAL(currentFontChanged(const QFont&)), SLOT(fontChanged(const QFont&)));
      connect(leftAlign,           SIGNAL(triggered()),     SLOT(setLeftAlign()));
      connect(rightAlign,          SIGNAL(triggered()),     SLOT(setRightAlign()));
      connect(centerAlign,         SIGNAL(triggered()),     SLOT(setHCenterAlign()));
      connect(topAlign,            SIGNAL(triggered()),     SLOT(setTopAlign()));
      connect(bottomAlign,         SIGNAL(triggered()),     SLOT(setBottomAlign()));
      connect(vcenterAlign,        SIGNAL(triggered()),     SLOT(setVCenterAlign()));
      connect(showKeyboard,        SIGNAL(triggered(bool)), SLOT(showKeyboardClicked(bool)));
      connect(textStyles,          SIGNAL(currentIndexChanged(int)), SLOT(styleChanged(int)));
      connect(unorderedList,       SIGNAL(triggered()),     SLOT(unorderedListClicked()));
      connect(orderedList,         SIGNAL(triggered()),     SLOT(orderedListClicked()));
      connect(indentLess,          SIGNAL(triggered()),     SLOT(indentLessClicked()));
      connect(indentMore,          SIGNAL(triggered()),     SLOT(indentMoreClicked()));
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void TextTools::setText(Text* te)
      {
      textStyles->blockSignals(true);
      _textElement = te;
      textStyles->clear();
      textStyles->addItem(tr("unstyled"));
      foreach(const TextStyle& st, te->score()->style()->textStyles())
            textStyles->addItem(st.name());
      if (te->styled())
            textStyles->setCurrentIndex(te->textStyleType() + 1);
      else
            textStyles->setCurrentIndex(0);
      textStyles->blockSignals(false);
      }

//---------------------------------------------------------
//   blockAllSignals
//---------------------------------------------------------

void TextTools::blockAllSignals(bool val)
      {
      typefaceSize->blockSignals(val);
      typefaceFamily->blockSignals(val);
      typefaceBold->blockSignals(val);
      typefaceItalic->blockSignals(val);
      typefaceUnderline->blockSignals(val);
      typefaceSubscript->blockSignals(val);
      typefaceSuperscript->blockSignals(val);
      typefaceFamily->blockSignals(val);
      leftAlign->blockSignals(val);
      rightAlign->blockSignals(val);
      centerAlign->blockSignals(val);
      topAlign->blockSignals(val);
      bottomAlign->blockSignals(val);
      vcenterAlign->blockSignals(val);
      showKeyboard->blockSignals(val);
      textStyles->blockSignals(val);
      }

//---------------------------------------------------------
//   updateTools
//---------------------------------------------------------

void TextTools::updateTools()
      {
      blockAllSignals(true);
      QTextCursor* cursor = _textElement->cursor();

      QTextCharFormat format = cursor->charFormat();
      QTextBlockFormat bformat = cursor->blockFormat();

      QFont f(format.font());
      typefaceFamily->setCurrentFont(f);
      double ps = f.pointSizeF();
      if (ps == -1.0)
            ps = f.pixelSize() * PPI / MScore::DPI;
      typefaceSize->setValue(ps);
      typefaceItalic->setChecked(format.fontItalic());
      typefaceBold->setChecked(format.fontWeight() == QFont::Bold);
      typefaceUnderline->setChecked(format.fontUnderline());
      typefaceSubscript->setChecked(format.verticalAlignment() == QTextCharFormat::AlignSubScript);
      typefaceSuperscript->setChecked(format.verticalAlignment() == QTextCharFormat::AlignSuperScript);

      centerAlign->setChecked(bformat.alignment() & Qt::AlignHCenter);
      leftAlign->setChecked  (bformat.alignment() & Qt::AlignLeft);
      rightAlign->setChecked (bformat.alignment() & Qt::AlignRight);
      Align align = _textElement->align();
      if (align & ALIGN_BOTTOM) {
            topAlign->setChecked(false);
            bottomAlign->setChecked(true);
            vcenterAlign->setChecked(false);
            }
      else if (align & ALIGN_VCENTER) {
            topAlign->setChecked(false);
            bottomAlign->setChecked(false);
            vcenterAlign->setChecked(true);
            }
      else {
            topAlign->setChecked(true);
            bottomAlign->setChecked(false);
            vcenterAlign->setChecked(false);
            }
      blockAllSignals(false);
      }

//---------------------------------------------------------
//   updateText
//---------------------------------------------------------

void TextTools::updateText()
      {
      if (_textElement->type() == LYRICS) {
            _textElement->score()->setLayoutAll(true);
            _textElement->score()->end();
            }
      else
            layoutText();
      }

//---------------------------------------------------------
//   layoutText
//---------------------------------------------------------

void TextTools::layoutText()
      {
      QRectF r(_textElement->canvasBoundingRect());
      _textElement->layout();
      _textElement->score()->addRefresh(_textElement->canvasBoundingRect() | r);
      _textElement->score()->end();
      }

//---------------------------------------------------------
//   cursor
//---------------------------------------------------------

QTextCursor* TextTools::cursor()
      {
      return _textElement->cursor();
      }

//---------------------------------------------------------
//   sizeChanged
//---------------------------------------------------------

void TextTools::sizeChanged(double value)
      {
      QTextCharFormat format;
      format.setFontPointSize(value);
      cursor()->mergeCharFormat(format);
      updateText();
      }

//---------------------------------------------------------
//   fontChanged
//---------------------------------------------------------

void TextTools::fontChanged(const QFont& f)
      {
      QTextCharFormat format;
      format.setFontFamily(f.family());
      cursor()->mergeCharFormat(format);
      updateText();
      }

//---------------------------------------------------------
//   boldClicked
//---------------------------------------------------------

void TextTools::boldClicked(bool val)
      {
      QTextCharFormat format;
      format.setFontWeight(val ? QFont::Bold : QFont::Normal);
      cursor()->mergeCharFormat(format);
      updateText();
      }

//---------------------------------------------------------
//   underlineClicked
//---------------------------------------------------------

void TextTools::underlineClicked(bool val)
      {
      QTextCharFormat format;
      format.setFontUnderline(val);
      cursor()->mergeCharFormat(format);
      updateText();
      }

//---------------------------------------------------------
//   italicClicked
//---------------------------------------------------------

void TextTools::italicClicked(bool val)
      {
      QTextCharFormat format;
      format.setFontItalic(val);
      cursor()->mergeCharFormat(format);
      updateText();
      }

//---------------------------------------------------------
//   unorderedListClicked
//---------------------------------------------------------

void TextTools::unorderedListClicked()
      {
      QTextCharFormat format = cursor()->charFormat();
      QTextListFormat listFormat;
      QTextList* list = cursor()->currentList();
      if (list) {
            listFormat = list->format();
            int indent = listFormat.indent();
            listFormat.setIndent(indent + 1);
            }
      listFormat.setStyle(QTextListFormat::ListDisc);
      cursor()->insertList(listFormat);
      cursor()->setCharFormat(format);
      updateText();
      }

//---------------------------------------------------------
//   orderedListClicked
//---------------------------------------------------------

void TextTools::orderedListClicked()
      {
      QTextCharFormat format = cursor()->charFormat();
      QTextListFormat listFormat;
      QTextList* list = cursor()->currentList();
      if (list) {
            listFormat = list->format();
            int indent = listFormat.indent();
            listFormat.setIndent(indent + 1);
            }
      listFormat.setStyle(QTextListFormat::ListDecimal);
      cursor()->insertList(listFormat);
      cursor()->setCharFormat(format);
      updateText();
      }

//---------------------------------------------------------
//   indentMoreClicked
//---------------------------------------------------------

void TextTools::indentMoreClicked()
      {
      QTextList* list = cursor()->currentList();
      if (list == 0) {
            QTextBlockFormat format = cursor()->blockFormat();
            format.setIndent(format.indent() + 1);
            cursor()->insertBlock(format);
            updateText();
            return;
            }
      unorderedListClicked();
      }

//---------------------------------------------------------
//   indentLessClicked
//---------------------------------------------------------

void TextTools::indentLessClicked()
      {
      QTextList* list = cursor()->currentList();
      if (list == 0) {
            QTextBlockFormat format = cursor()->blockFormat();
            int indent = format.indent();
            if (indent) {
                  indent--;
                  format.setIndent(indent);
                  cursor()->insertBlock(format);
                  updateText();
                  }
            return;
            }
      QTextCharFormat format = cursor()->blockCharFormat();
      QTextListFormat listFormat = list->format();
      QTextBlock block = cursor()->block();
      if (block.next().isValid())
            block = block.next();
      else {
            block = QTextBlock();
            }
      cursor()->insertBlock(block.blockFormat());
      cursor()->setCharFormat(block.charFormat());
      updateText();
      }

//---------------------------------------------------------
//   setHCenterAlign
//---------------------------------------------------------

void TextTools::setHCenterAlign()
      {
      QTextBlockFormat bformat;
      bformat.setAlignment((bformat.alignment() & ~Qt::AlignHorizontal_Mask) | Qt::AlignHCenter);
      cursor()->mergeBlockFormat(bformat);
      updateTools();
      }

//---------------------------------------------------------
//   setLeftAlign
//---------------------------------------------------------

void TextTools::setLeftAlign()
      {
      QTextBlockFormat bformat;
      bformat.setAlignment((bformat.alignment() & ~Qt::AlignHorizontal_Mask) | Qt::AlignLeft);
      cursor()->mergeBlockFormat(bformat);
      updateTools();
      }

//---------------------------------------------------------
//   setRightAlign
//---------------------------------------------------------

void TextTools::setRightAlign()
      {
      QTextBlockFormat bformat;
      bformat.setAlignment((bformat.alignment() & ~Qt::AlignHorizontal_Mask) | Qt::AlignRight);
      cursor()->mergeBlockFormat(bformat);
      updateTools();
      }

//---------------------------------------------------------
//   setTopAlign
//---------------------------------------------------------

void TextTools::setTopAlign()
      {
      Align align = (_textElement->align() & ~ALIGN_VMASK) | ALIGN_TOP;
      _textElement->setAlign(align);
      updateTools();
      layoutText();
      }

//---------------------------------------------------------
//   setBottomAlign
//---------------------------------------------------------

void TextTools::setBottomAlign()
      {
      Align align = (_textElement->align() & ~ALIGN_VMASK) | ALIGN_BOTTOM;
      _textElement->setAlign(align);
      updateTools();
      layoutText();
      }

//---------------------------------------------------------
//   setVCenterAlign
//---------------------------------------------------------

void TextTools::setVCenterAlign()
      {
      Align align = (_textElement->align() & ~ALIGN_VMASK) | ALIGN_VCENTER;
      _textElement->setAlign(align);
      updateTools();
      layoutText();
      }

//---------------------------------------------------------
//   subscriptClicked
//---------------------------------------------------------

void TextTools::subscriptClicked(bool val)
      {
      typefaceSuperscript->blockSignals(true);
      typefaceSuperscript->setChecked(false);
      typefaceSuperscript->blockSignals(false);

      QTextCharFormat format;
      format.setVerticalAlignment(val ? QTextCharFormat::AlignSubScript : QTextCharFormat::AlignNormal);
      cursor()->mergeCharFormat(format);
      }

//---------------------------------------------------------
//   superscriptClicked
//---------------------------------------------------------

void TextTools::superscriptClicked(bool val)
      {
      typefaceSubscript->blockSignals(true);
      typefaceSubscript->setChecked(false);
      typefaceSubscript->blockSignals(false);

      QTextCharFormat format;
      format.setVerticalAlignment(val ? QTextCharFormat::AlignSuperScript : QTextCharFormat::AlignNormal);
      cursor()->mergeCharFormat(format);
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void TextTools::styleChanged(int idx)
      {
      blockAllSignals(true);
      bool styled = idx != 0;

      if (styled)
            _textElement->setTextStyleType(idx - 1);
      else
            _textElement->setUnstyled();
      bool unstyled = !styled;
      typefaceSize->setEnabled(unstyled);
      typefaceFamily->setEnabled(unstyled);
      typefaceBold->setEnabled(unstyled);
      typefaceItalic->setEnabled(unstyled);
      typefaceUnderline->setEnabled(unstyled);
      typefaceSubscript->setEnabled(unstyled);
      typefaceSuperscript->setEnabled(unstyled);
      typefaceFamily->setEnabled(unstyled);
      leftAlign->setEnabled(unstyled);
      rightAlign->setEnabled(unstyled);
      centerAlign->setEnabled(unstyled);
      topAlign->setEnabled(unstyled);
      bottomAlign->setEnabled(unstyled);
      vcenterAlign->setEnabled(unstyled);

      blockAllSignals(false);
      }

//---------------------------------------------------------
//   showKeyboardClicked
//---------------------------------------------------------

void TextTools::showKeyboardClicked(bool val)
      {
      if (val) {
            if (textPalette == 0)
                  textPalette = new TextPalette(mscore);
            textPalette->setText(_textElement);
            textPalette->show();
            }
      else {
            if (textPalette)
                  textPalette->hide();
            }
      }

