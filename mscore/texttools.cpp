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
      tb->setIconSize(QSize(preferences.iconWidth * guiScaling, preferences.iconHeight * guiScaling));

      showKeyboard = getAction("show-keys");
      showKeyboard->setCheckable(true);
      tb->addAction(showKeyboard);

      typefaceBold = tb->addAction(*icons[int(Icons::textBold_ICON)], "");
      typefaceBold->setToolTip(tr("Bold"));
      typefaceBold->setCheckable(true);

      typefaceItalic = tb->addAction(*icons[int(Icons::textItalic_ICON)], "");
      typefaceItalic->setToolTip(tr("Italic"));
      typefaceItalic->setCheckable(true);

      typefaceUnderline = tb->addAction(*icons[int(Icons::textUnderline_ICON)], "");
      typefaceUnderline->setToolTip(tr("Underline"));
      typefaceUnderline->setCheckable(true);

      tb->addSeparator();

      typefaceSubscript   = tb->addAction(*icons[int(Icons::textSub_ICON)], "");
      typefaceSubscript->setToolTip(tr("Subscript"));
      typefaceSubscript->setCheckable(true);

      typefaceSuperscript = tb->addAction(*icons[int(Icons::textSuper_ICON)], "");
      typefaceSuperscript->setToolTip(tr("Superscript"));
      typefaceSuperscript->setCheckable(true);

      tb->addSeparator();

      typefaceFamily = new QFontComboBox(this);
      tb->addWidget(typefaceFamily);

      typefaceSize = new QDoubleSpinBox(this);
      typefaceSize->setFocusPolicy(Qt::ClickFocus);
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
      connect(showKeyboard,        SIGNAL(toggled(bool)),   SLOT(showKeyboardClicked(bool)));
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void TextTools::setText(Text* te)
      {
      _textElement = te;
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
      }

//---------------------------------------------------------
//   updateTools
//---------------------------------------------------------

void TextTools::updateTools()
      {
      if (!_textElement->editMode())
            qFatal("TextTools::updateTools(): not in edit mode");
      blockAllSignals(true);
      TextCursor* cursor = _textElement->cursor();
      CharFormat* format = cursor->format();

      QFont f(format->fontFamily());
      typefaceFamily->setCurrentFont(f);
      typefaceFamily->setEnabled(cursor->format()->type() == CharFormatType::TEXT);
      typefaceSize->setValue(format->fontSize());

      typefaceItalic->setChecked(format->italic());
      typefaceBold->setChecked(format->bold());
      typefaceUnderline->setChecked(format->underline());
      typefaceSubscript->setChecked(format->valign() == VerticalAlignment::AlignSubScript);
      typefaceSuperscript->setChecked(format->valign() == VerticalAlignment::AlignSuperScript);

      blockAllSignals(false);
      }

//---------------------------------------------------------
//   updateText
//---------------------------------------------------------

void TextTools::updateText()
      {
      if (!_textElement)
            return;
      if (_textElement->type() == Element::Type::LYRICS) {
            _textElement->score()->setLayoutAll();
            _textElement->score()->update();
            }
      else
            layoutText();
      }

//---------------------------------------------------------
//   layoutText
//---------------------------------------------------------

void TextTools::layoutText()
      {
      _textElement->score()->setLayoutAll();
      _textElement->score()->update();
      }

//---------------------------------------------------------
//   sizeChanged
//---------------------------------------------------------

void TextTools::sizeChanged(double value)
      {
      _textElement->setFormat(FormatId::FontSize, value);
      _textElement->cursor()->format()->setFontSize(value);
      updateText();
      }

//---------------------------------------------------------
//   fontChanged
//---------------------------------------------------------

void TextTools::fontChanged(const QFont& f)
      {
      if (_textElement)
            _textElement->setFormat(FormatId::FontFamily, f.family());
      if (textPalette)
            textPalette->setFont(f.family());
      updateText();
      }

//---------------------------------------------------------
//   boldClicked
//---------------------------------------------------------

void TextTools::boldClicked(bool val)
      {
      _textElement->setFormat(FormatId::Bold, val);
      updateText();
      }

//---------------------------------------------------------
//   toggleBold
//---------------------------------------------------------

void TextTools::toggleBold()
      {
      typefaceBold->toggle();
      boldClicked(typefaceBold->isChecked());
      }

//---------------------------------------------------------
//   toggleItalic
//---------------------------------------------------------

void TextTools::toggleItalic()
      {
      typefaceItalic->toggle();
      italicClicked(typefaceItalic->isChecked());
      }

//---------------------------------------------------------
//   toggleUnderline
//---------------------------------------------------------

void TextTools::toggleUnderline()
      {
      typefaceUnderline->toggle();
      underlineClicked(typefaceUnderline->isChecked());
      }

//---------------------------------------------------------
//   underlineClicked
//---------------------------------------------------------

void TextTools::underlineClicked(bool val)
      {
      _textElement->setFormat(FormatId::Underline, val);
      updateText();
      }

//---------------------------------------------------------
//   italicClicked
//---------------------------------------------------------

void TextTools::italicClicked(bool val)
      {
      _textElement->setFormat(FormatId::Italic, val);
      updateText();
      }

//---------------------------------------------------------
//   subscriptClicked
//---------------------------------------------------------

void TextTools::subscriptClicked(bool val)
      {
      _textElement->setFormat(FormatId::Valign, int(val ? VerticalAlignment::AlignSubScript : VerticalAlignment::AlignNormal));
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
      _textElement->setFormat(FormatId::Valign, int(val ? VerticalAlignment::AlignSuperScript : VerticalAlignment::AlignNormal));
      typefaceSubscript->blockSignals(true);
      typefaceSubscript->setChecked(false);
      typefaceSubscript->blockSignals(false);
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
            textPalette->setFont(_textElement->cursor()->format()->fontFamily());
            textPalette->show();
            }
      else {
            if (textPalette)
                  textPalette->hide();
            }
      }
}

