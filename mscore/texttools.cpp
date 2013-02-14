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

      QActionGroup* ha = new QActionGroup(tb);
      leftAlign   = new QAction(*icons[textLeft_ICON],   "", ha);
      leftAlign->setToolTip(tr("align left"));
      leftAlign->setCheckable(true);
      leftAlign->setData(ALIGN_LEFT);
      hcenterAlign = new QAction(*icons[textCenter_ICON], "", ha);
      hcenterAlign->setToolTip(tr("align horizontal center"));
      hcenterAlign->setCheckable(true);
      hcenterAlign->setData(ALIGN_HCENTER);
      rightAlign  = new QAction(*icons[textRight_ICON],  "", ha);
      rightAlign->setToolTip(tr("align right"));
      rightAlign->setCheckable(true);
      rightAlign->setData(ALIGN_RIGHT);
      tb->addActions(ha->actions());

      QActionGroup* va = new QActionGroup(tb);
      topAlign  = new QAction(*icons[textTop_ICON],  "", va);
      topAlign->setToolTip(tr("align top"));
      topAlign->setCheckable(true);
      topAlign->setData(ALIGN_TOP);

      bottomAlign  = new QAction(*icons[textBottom_ICON],  "", va);
      bottomAlign->setToolTip(tr("align bottom"));
      bottomAlign->setCheckable(true);
      bottomAlign->setData(ALIGN_BOTTOM);

      baselineAlign  = new QAction(*icons[textVCenter_ICON],  "", va);
      baselineAlign->setToolTip(tr("align vertical baseline"));
      baselineAlign->setCheckable(true);
      baselineAlign->setData(ALIGN_BASELINE);

      vcenterAlign  = new QAction(*icons[textVCenter_ICON],  "", va);
      vcenterAlign->setToolTip(tr("align vertical center"));
      vcenterAlign->setCheckable(true);
      vcenterAlign->setData(ALIGN_VCENTER);
      tb->addActions(va->actions());

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
      connect(ha,                  SIGNAL(triggered(QAction*)), SLOT(setHalign(QAction*)));
      connect(va,                  SIGNAL(triggered(QAction*)), SLOT(setValign(QAction*)));
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

      const QList<TextStyle>& ts = te->score()->style()->textStyles();

      int n = ts.size();
      for (int i = 1; i < n; ++i)                     // skip default style
            textStyles->addItem(ts[i].name());
      int idx = 0;
      if (te->styled())
            idx = te->textStyleType();
      textStyles->setCurrentIndex(idx);
      styleChanged(idx);
      Align align = _textElement->align();
      if (align & ALIGN_HCENTER)
            hcenterAlign->setChecked(true);
      else if (align & ALIGN_RIGHT)
            rightAlign->setChecked(true);
      else
            leftAlign->setChecked(true);

      if (align & ALIGN_BOTTOM)
            bottomAlign->setChecked(true);
      else if (align & ALIGN_BASELINE)
            baselineAlign->setChecked(true);
      else if (align & ALIGN_VCENTER)
            vcenterAlign->setChecked(true);
      else
            topAlign->setChecked(true);
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
      showKeyboard->blockSignals(val);
      textStyles->blockSignals(val);
      }

//---------------------------------------------------------
//   updateTools
//---------------------------------------------------------

void TextTools::updateTools()
      {
      if (_textElement->styled())   // TODO
            return;

      blockAllSignals(true);
      QTextCursor* cursor      = _textElement->cursor();
      QTextCharFormat format   = cursor->charFormat();
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

      blockAllSignals(false);
      }

//---------------------------------------------------------
//   updateText
//---------------------------------------------------------

void TextTools::updateText()
      {
      if (_textElement->type() == Element::LYRICS) {
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
//   setHalign
//---------------------------------------------------------

void TextTools::setHalign(QAction* a)
      {
      QTextBlockFormat bformat;

      Qt::Alignment qa = bformat.alignment() & ~Qt::AlignHorizontal_Mask;
      switch(a->data().toInt()) {
            case ALIGN_HCENTER:
                  qa  |= Qt::AlignHCenter;
                  break;
            case ALIGN_RIGHT:
                  qa |= Qt::AlignRight;
                  break;
            case ALIGN_LEFT:
                  qa |= Qt::AlignLeft;
                  break;
            }

      bformat.setAlignment(qa);
      cursor()->mergeBlockFormat(bformat);
      _textElement->setAlign((_textElement->align() & ~ ALIGN_HMASK) | Align(a->data().toInt()));
      updateTools();
      layoutText();
      }

//---------------------------------------------------------
//   setValign
//---------------------------------------------------------

void TextTools::setValign(QAction* a)
      {
      _textElement->setAlign((_textElement->align() & ~ALIGN_VMASK) | Align(a->data().toInt()));
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
      updateText();
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
      updateText();
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void TextTools::styleChanged(int idx)
      {
      blockAllSignals(true);
      bool styled = idx != 0;

      if (styled)
            _textElement->setTextStyleType(idx);
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
      hcenterAlign->setEnabled(unstyled);
      topAlign->setEnabled(unstyled);
      bottomAlign->setEnabled(unstyled);
      baselineAlign->setEnabled(unstyled);
      vcenterAlign->setEnabled(unstyled);
      blockAllSignals(false);
      updateText();
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

