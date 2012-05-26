//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: textstyle.cpp 5427 2012-03-07 12:41:34Z wschweer $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "libmscore/style.h"
#include "textstyle.h"
#include "globals.h"
#include "libmscore/score.h"
#include "scoreview.h"
#include "textprop.h"
#include "musescore.h"
#include "libmscore/undo.h"

//---------------------------------------------------------
//   TextStyleDialog
//---------------------------------------------------------

TextStyleDialog::TextStyleDialog(QWidget* parent, Score* score)
   : QDialog(parent)
      {
      setupUi(this);

      cs     = score;
      styles = cs->style()->textStyles();
      tp->setScore(true, cs);

      textNames->clear();
      for (int i = 0; i < styles.size(); ++i) {
            const TextStyle& s = styles.at(i);
            textNames->addItem(qApp->translate("MuseScore", s.name().toAscii().data()));
            }

      connect(bb, SIGNAL(clicked(QAbstractButton*)), SLOT(buttonClicked(QAbstractButton*)));
      connect(textNames, SIGNAL(currentRowChanged(int)), SLOT(nameSelected(int)));
      connect(newButton, SIGNAL(clicked()), SLOT(newClicked()));

      current   = -1;
      textNames->setCurrentItem(textNames->item(0));
      }

//---------------------------------------------------------
//   ~TextStyleDialog
//---------------------------------------------------------

TextStyleDialog::~TextStyleDialog()
      {
      }

//---------------------------------------------------------
//   nameSelected
//---------------------------------------------------------

void TextStyleDialog::nameSelected(int n)
      {
      if (current != -1)
            saveStyle(current);
      current = n;
      tp->setTextStyle(styles[current]);
      }

//---------------------------------------------------------
//   saveStyle
//---------------------------------------------------------

void TextStyleDialog::saveStyle(int n)
      {
      TextStyle st = tp->textStyle();
      // set data members not set by TextProp::textStyle()
      st.setName(styles[n].name());
// TextProp::textStyle() now deals with sizeIsSpatiumDependent
// Following statement defeats additions to TextProp::textStyle()
//      st.setSizeIsSpatiumDependent(styles[n].sizeIsSpatiumDependent());
      styles[n] = st;
      }

//---------------------------------------------------------
//   buttonClicked
//---------------------------------------------------------

void TextStyleDialog::buttonClicked(QAbstractButton* b)
      {
      switch (bb->standardButton(b)) {
            case QDialogButtonBox::Apply:
                  apply();
                  break;
            case QDialogButtonBox::Ok:
                  apply();
                  done(1);
                  break;
            default:
                  if (cs->undo()->current()) {
                        cs->undo()->current()->unwind();
                        cs->setLayoutAll(true);
                        }
                  done(0);
                  break;
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void TextStyleDialog::apply()
      {
      saveStyle(current);                 // update local copy of style list

      int n = cs->style()->textStyles().size();
      for (int i = 0; i < n; ++i) {
            const TextStyle& os = cs->textStyle(i);
            const TextStyle& ns = styles[i];
            if (os != ns) {
                  cs->undo(new ChangeTextStyle(cs, ns));
                  }
            }
      for (int i = n; i < styles.size(); ++i)
            cs->undo(new AddTextStyle(cs, styles[i]));
      cs->end2();
      cs->end();
      }

//---------------------------------------------------------
//   newClicked
//---------------------------------------------------------

void TextStyleDialog::newClicked()
      {
      QString s = QInputDialog::getText(this, tr("MuseScore: Read Style Name"),
         tr("Text Style Name:"));
      if (s.isEmpty())
            return;
      for (;;) {
            bool notFound = true;
            for (int i = 0; i < styles.size(); ++i) {
                  const TextStyle& style = styles.at(i);
                  if (style.name() == s) {
                        notFound = false;
                        break;
                        }
                  }
            if (!notFound) {
                  s = QInputDialog::getText(this,
                     tr("MuseScore: Read Style Name"),
                     QString(tr("'%1' does already exist,\nplease choose a different name:")).arg(s)
                     );
                  if (s.isEmpty())
                        return;
                  }
            else
                  break;
            }
      //
      // use current selected style as template
      //
      QString name = textNames->currentItem()->text();
      TextStyle newStyle = cs->textStyle(name);

      textNames->addItem(s);
      newStyle.setName(s);
      styles.append(newStyle);
      textNames->setCurrentItem(textNames->item(textNames->count()-1));
      cs->setDirty(true);
      mscore->endCmd();
      }

