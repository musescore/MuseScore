//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: text.cpp -1   $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "globals.h"
#include "libmscore/text.h"
#include "libmscore/xml.h"
#include "libmscore/style.h"
#include "musescore.h"
#include "scoreview.h"
#include "libmscore/score.h"
#include "libmscore/utils.h"
#include "libmscore/page.h"
#include "textpalette.h"
#include "libmscore/sym.h"
#include "libmscore/symbol.h"
#include "libmscore/textline.h"
#include "preferences.h"
#include "libmscore/system.h"
#include "libmscore/measure.h"
#include "textproperties.h"
#include "textprop.h"
#include "libmscore/box.h"
#include "libmscore/segment.h"
#include "texttools.h"

namespace Ms {

//---------------------------------------------------------
//   TextProperties
//---------------------------------------------------------

TextProperties::TextProperties(Text* t, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("TextProperties");
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setWindowTitle(tr("Text Properties"));
      QGridLayout* layout = new QGridLayout;

      tp = new TextProp;
      tp->setScore(false, t->score());

      layout->addWidget(tp, 0, 1);

      QHBoxLayout* hb = new QHBoxLayout;
      QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
      hb->addWidget(bb);
      layout->addLayout(hb, 1, 1);
      setLayout(layout);

      text = t;

      tp->setStyle(t->textStyleType(), t->textStyle());

      connect(bb, SIGNAL(accepted()), SLOT(accept()));
      connect(bb, SIGNAL(rejected()), SLOT(reject()));
      connect(tp, SIGNAL(resetToStyleClicked()), SLOT(resetToStyle()));

      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   resetToStyle
//---------------------------------------------------------

void TextProperties::resetToStyle()
      {
      text->setXmlText(text->plainText());
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void TextProperties::accept()
      {
      text->setTextStyleType(tp->textStyleType());
      text->setTextStyle(tp->textStyle());
      QDialog::accept();
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void TextProperties::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }
}

