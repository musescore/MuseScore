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

//---------------------------------------------------------
//   TextProperties
//---------------------------------------------------------

TextProperties::TextProperties(Text* t, QWidget* parent)
   : QDialog(parent)
      {
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setWindowTitle(tr("MuseScore: Text Properties"));
      QGridLayout* layout = new QGridLayout;

      tp = new TextProp;
      tp->setScore(false, t->score());

      layout->addWidget(tp, 0, 1);
      QLabel* l = new QLabel;
      l->setPixmap(QPixmap(":/data/bg1.jpg"));
      l->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);

      layout->addWidget(l, 0, 0, 2, 1);
      QHBoxLayout* hb = new QHBoxLayout;
      QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
      hb->addWidget(bb);
      layout->addLayout(hb, 1, 1);
      setLayout(layout);

      text = t;

      tp->setTextStyle(t->textStyle());
      tp->setStyled(t->styled());
      if (t->styled())
            tp->setTextStyleType(t->textStyleType());
      else
            tp->setTextStyleType(0);

      connect(bb, SIGNAL(accepted()), SLOT(accept()));
      connect(bb, SIGNAL(rejected()), SLOT(reject()));
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void TextProperties::accept()
      {
      if (tp->isStyled()) {
            if (!text->styled() || (text->textStyleType() != tp->textStyleType())) {
                  text->setTextStyleType(tp->textStyleType());
                  text->styleChanged();
                  }
            }
      else {
            text->setUnstyled();
            text->setTextStyle(tp->textStyle());
            }

      QDialog::accept();
      }

