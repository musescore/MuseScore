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
#include "preferences.h"

namespace Ms {

TextPalette* textPalette;

//---------------------------------------------------------
//   textTools
//---------------------------------------------------------

TextTools* MuseScore::textTools()
      {
      if (!_textTools) {
            _textTools = new TextTools(this);
            addDockWidget(Qt::DockWidgetArea(Qt::BottomDockWidgetArea), _textTools);
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
      setAllowedAreas(Qt::DockWidgetAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea));

      QToolBar* tb = new QToolBar(tr("Text Edit"));
      tb->setIconSize(QSize(preferences.iconWidth, preferences.iconHeight));

      textStyles = new QComboBox;
      tb->addWidget(textStyles);

      showKeyboard = getAction("show-keys");
      showKeyboard->setCheckable(true);
      tb->addAction(showKeyboard);

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

      baselineAlign  = new QAction(*icons[textBaseline_ICON],  "", va);
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
      textStyles->addItem(tr("unstyled"), TEXT_STYLE_UNSTYLED);

      const QList<TextStyle>& ts = te->score()->style()->textStyles();

      int n = ts.size();
      for (int i = 0; i < n; ++i) {
            if ( !(ts.at(i).hidden() & TextStyle::HIDE_IN_LISTS) )
                  textStyles->addItem(ts.at(i).name(), i);
            }
      int comboIdx = 0;
      if (te->styled()) {
            int styleIdx = te->textStyleType();
            comboIdx = textStyles->findData(styleIdx);
            if (comboIdx < 0)
                  comboIdx = 0;
            }
      textStyles->setCurrentIndex(comboIdx);
      styleChanged(comboIdx);
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
      if (!_textElement->editMode()) {
            qDebug("TextTools::updateTools(): not in edit mode");
            abort();
            return;
            }
      blockAllSignals(true);

      QFont f(_textElement->curFont());
      typefaceFamily->setCurrentFont(f);
      double ps = f.pointSizeF();
      if (ps == -1.0)
            ps = f.pixelSize() * PPI / MScore::DPI;
      typefaceSize->setValue(ps);

      typefaceItalic->setChecked(_textElement->curItalic());
      typefaceBold->setChecked(_textElement->curBold());
      typefaceUnderline->setChecked(_textElement->curUnderline());
      typefaceSubscript->setChecked(_textElement->curSubscript());
      typefaceSuperscript->setChecked(_textElement->curSuperscript());

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
//      _textElement->layout();
      _textElement->score()->setLayoutAll(true);
//      _textElement->score()->addRefresh(_textElement->canvasBoundingRect() | r);
      _textElement->score()->end();
      }

//---------------------------------------------------------
//   sizeChanged
//---------------------------------------------------------

void TextTools::sizeChanged(double value)
      {
      _textElement->setCurFontPointSize(value);
      updateText();
      }

//---------------------------------------------------------
//   fontChanged
//---------------------------------------------------------

void TextTools::fontChanged(const QFont& f)
      {
      _textElement->setCurFontFamily(f.family());
      updateText();
      }

//---------------------------------------------------------
//   boldClicked
//---------------------------------------------------------

void TextTools::boldClicked(bool val)
      {
      _textElement->setCurBold(val);
      updateText();
      }

//---------------------------------------------------------
//   underlineClicked
//---------------------------------------------------------

void TextTools::underlineClicked(bool val)
      {
      _textElement->setCurUnderline(val);
      updateText();
      }

//---------------------------------------------------------
//   italicClicked
//---------------------------------------------------------

void TextTools::italicClicked(bool val)
      {
      _textElement->setCurItalic(val);
      updateText();
      }

//---------------------------------------------------------
//   unorderedListClicked
//---------------------------------------------------------

void TextTools::unorderedListClicked()
      {
      _textElement->unorderedList();
      updateText();
      }

//---------------------------------------------------------
//   orderedListClicked
//---------------------------------------------------------

void TextTools::orderedListClicked()
      {
      _textElement->orderedList();
      updateText();
      }

//---------------------------------------------------------
//   indentMoreClicked
//---------------------------------------------------------

void TextTools::indentMoreClicked()
      {
      _textElement->indentMore();
      updateText();
      }

//---------------------------------------------------------
//   indentLessClicked
//---------------------------------------------------------

void TextTools::indentLessClicked()
      {
      _textElement->indentLess();
      updateText();
      }

//---------------------------------------------------------
//   setHalign
//---------------------------------------------------------

void TextTools::setHalign(QAction* a)
      {
      _textElement->setCurHalign(a->data().toInt());
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
//      _textElement->setAlign((_textElement->align() & ~ALIGN_VMASK) | Align(a->data().toInt()));
      _textElement->setCurSubscript(val);
      typefaceSuperscript->blockSignals(true);
      typefaceSuperscript->setChecked(false);
      typefaceSuperscript->blockSignals(false);
      updateText();
      }

//---------------------------------------------------------
//   superscriptClicked
//---------------------------------------------------------

void TextTools::superscriptClicked(bool val)
      {
      _textElement->setCurSuperscript(val);
      typefaceSubscript->blockSignals(true);
      typefaceSubscript->setChecked(false);
      typefaceSubscript->blockSignals(false);
      updateText();
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void TextTools::styleChanged(int comboIdx)
      {
      blockAllSignals(true);
      int styleIdx = textStyles->itemData(comboIdx).toInt();
      if (styleIdx < 0)
            styleIdx = TEXT_STYLE_UNSTYLED;
      bool unstyled = (styleIdx == TEXT_STYLE_UNSTYLED);

      if (unstyled)
            _textElement->setUnstyled();
      else
            _textElement->setTextStyleType(styleIdx);
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
printf("showKeyboardClicked %d\n", val);
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
}

