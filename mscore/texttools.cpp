//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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

#include "log.h"

#include "texttools.h"
#include "icons.h"
#include "libmscore/text.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "textpalette.h"
#include "libmscore/mscore.h"
#include "preferences.h"
#include "scoreview.h"

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
      setObjectName("text-tools");
      setAllowedAreas(Qt::DockWidgetAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea));
      setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

      text = nullptr;
      cursor = nullptr;

      toolbar = new QToolBar(tr("Text Edit"));
      toolbar->setIconSize(QSize(preferences.getInt(PREF_UI_THEME_ICONWIDTH) * guiScaling,
                                 preferences.getInt(PREF_UI_THEME_ICONHEIGHT) * guiScaling));

      showKeyboard = getAction("show-keys");
      showKeyboard->setCheckable(true);
      toolbar->addAction(showKeyboard);

      typefaceBold = toolbar->addAction(*icons[int(Icons::textBold_ICON)], "");
      typefaceBold->setCheckable(true);

      typefaceItalic = toolbar->addAction(*icons[int(Icons::textItalic_ICON)], "");
      typefaceItalic->setCheckable(true);

      typefaceUnderline = toolbar->addAction(*icons[int(Icons::textUnderline_ICON)], "");
      typefaceUnderline->setCheckable(true);

      toolbar->addSeparator();

      typefaceSubscript = toolbar->addAction(*icons[int(Icons::textSub_ICON)], "");
      typefaceSubscript->setCheckable(true);

      typefaceSuperscript = toolbar->addAction(*icons[int(Icons::textSuper_ICON)], "");
      typefaceSuperscript->setCheckable(true);

      toolbar->addSeparator();

      typefaceFamily = new QFontComboBox(this);
      typefaceFamily->setEditable(false);
      toolbar->addWidget(typefaceFamily);

      typefaceSize = new QDoubleSpinBox(this);
      typefaceSize->setFocusPolicy(Qt::ClickFocus);
      typefaceSize->setMinimum(1);
      toolbar->addWidget(typefaceSize);

      setWidget(toolbar);
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
      
      retranslate();
      }

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void TextTools::retranslate()
      {
      setWindowTitle(tr("Text Tools"));
      toolbar->setWindowTitle(tr("Text Edit"));
      typefaceBold->setToolTip(tr("Bold"));
      typefaceItalic->setToolTip(tr("Italic"));
      typefaceSubscript->setToolTip(tr("Subscript"));
      typefaceSuperscript->setToolTip(tr("Superscript"));
      }

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void TextTools::changeEvent(QEvent* event)
      {
      QDockWidget::changeEvent(event);
      if (event->type() == QEvent::LanguageChange)
            retranslate();
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

void TextTools::updateTools(EditData& ed)
      {
      text   = toTextBase(ed.element);
      IF_ASSERT_FAILED(text) {
            return;
            }

      cursor = text->cursor(ed);
      IF_ASSERT_FAILED(cursor) {
            return;
            }

      blockAllSignals(true);
      CharFormat* format = cursor->format();

      QFont f(format->fontFamily());
      typefaceFamily->setCurrentFont(f);
      typefaceFamily->setEnabled(true);
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
      if (!text)
            return;
      layoutText();
      }

//---------------------------------------------------------
//   layoutText
//---------------------------------------------------------

void TextTools::layoutText()
      {
      text->triggerLayout();
      text->score()->update();
      }

//---------------------------------------------------------
//   sizeChanged
//---------------------------------------------------------

void TextTools::sizeChanged(double value)
      {
      IF_ASSERT_FAILED(cursor) {
            return;
            }
      cursor->setFormat(FormatId::FontSize, value);
      cursor->format()->setFontSize(value);
      updateText();
      }

//---------------------------------------------------------
//   fontChanged
//---------------------------------------------------------

void TextTools::fontChanged(const QFont& f)
      {
      //! REVIEW An explanation is needed why only in this one method
      //! it is assumed that the cursor may not be,
      //! and that this is a normal situation, and not an exception (no assertion)
      if (cursor)
            cursor->setFormat(FormatId::FontFamily, f.family());
      if (textPalette)
            textPalette->setFont(f.family());
      updateText();
      }

//---------------------------------------------------------
//   boldClicked
//---------------------------------------------------------

void TextTools::boldClicked(bool val)
      {
      IF_ASSERT_FAILED(cursor) {
            return;
            }
      cursor->setFormat(FormatId::Bold, val);
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
      IF_ASSERT_FAILED(cursor) {
            return;
            }
      cursor->setFormat(FormatId::Underline, val);
      updateText();
      }

//---------------------------------------------------------
//   italicClicked
//---------------------------------------------------------

void TextTools::italicClicked(bool val)
      {
      IF_ASSERT_FAILED(cursor) {
            return;
            }
      cursor->setFormat(FormatId::Italic, val);
      updateText();
      }

//---------------------------------------------------------
//   subscriptClicked
//---------------------------------------------------------

void TextTools::subscriptClicked(bool val)
      {
      IF_ASSERT_FAILED(cursor) {
            return;
            }
      cursor->setFormat(FormatId::Valign, int(val ? VerticalAlignment::AlignSubScript : VerticalAlignment::AlignNormal));
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
      IF_ASSERT_FAILED(cursor) {
            return;
            }
      cursor->setFormat(FormatId::Valign, int(val ? VerticalAlignment::AlignSuperScript : VerticalAlignment::AlignNormal));
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
            IF_ASSERT_FAILED(cursor) {
                  return;
                  }
            if (textPalette == 0)
                  textPalette = new TextPalette(mscore);
            textPalette->setText(text);
            textPalette->setFont(cursor->format()->fontFamily());
            textPalette->show();
            }
      else {
            if (textPalette)
                  textPalette->hide();
            }
      }
}
