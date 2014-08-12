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
#include "libmscore/excerpt.h"

namespace Ms {

//---------------------------------------------------------
//   TextStyleDialog
//---------------------------------------------------------

TextStyleDialog::TextStyleDialog(QWidget* parent, Score* score)
   : QDialog(parent)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      cs = score;
      buttonApplyToAllParts = bb->addButton(tr("Apply to all Parts"), QDialogButtonBox::ApplyRole);
      buttonApplyToAllParts->setEnabled(cs->parentScore() != nullptr);

      styles = cs->style()->textStyles();
      tp->setScore(true, cs);

      textNames->clear();
      for (int i = 0, n = styles.size(); i < n; ++i) {
//            const TextStyle& s = styles.at(i);
            if ( (styles.at(i).hidden()& TextStyleHidden::IN_EDITOR) == 0) {
                  int count = textNames->count();
                  textNames->addItem(qApp->translate("TextStyle", styles.at(i).name().toLatin1().data()));
                  textNames->item(count)->setData(Qt::UserRole, i);
                  }
            }

      connect(bb, SIGNAL(clicked(QAbstractButton*)), SLOT(buttonClicked(QAbstractButton*)));
      connect(textNames, SIGNAL(currentRowChanged(int)), SLOT(nameSelected(int)));
      connect(newButton, SIGNAL(clicked()), SLOT(newClicked()));

      current = -1;
      textNames->setCurrentItem(textNames->item(0));
      }

//---------------------------------------------------------
//   ~TextStyleDialog
//---------------------------------------------------------

TextStyleDialog::~TextStyleDialog()
      {
      }

//---------------------------------------------------------
//   setPage
//---------------------------------------------------------

void TextStyleDialog::setPage(QString name)
      {
      for (int i = 0; i < styles.size(); ++i) {
            if (styles.at(i).name() == name) {
                  for (int j = 0; j < textNames->count(); ++j) {
                        if (textNames->item(j)->data(Qt::UserRole).toInt() == i) {
                              textNames->setCurrentItem(textNames->item(j));
                              return;
                              }
                        }
                  return;
                  }
            }
      }

//---------------------------------------------------------
//   nameSelected
//---------------------------------------------------------

void TextStyleDialog::nameSelected(int n)
      {
      if (current != -1)
            saveStyle(current);
      current = n;
      int listIdx = textNames->item(n)->data(Qt::UserRole).toInt();
      tp->setTextStyle(styles[listIdx]);
      }

//---------------------------------------------------------
//   saveStyle
//---------------------------------------------------------

void TextStyleDialog::saveStyle(int n)
      {
      int listIdx = textNames->item(n)->data(Qt::UserRole).toInt();
      TextStyle st = tp->textStyle();
      st._hidden = styles.at(listIdx).hidden();
      st.setName(styles.at(listIdx).name());    // set data members not set by TextProp::textStyle()
      styles[listIdx] = st;                     // store style into local style list
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
            case QDialogButtonBox::Cancel:
                  if (cs->undo()->current()) {
                        cs->undo()->current()->unwind();
                        cs->setLayoutAll(true);
                        }
                  done(0);
                  break;
            case QDialogButtonBox::NoButton:
            default:
                  if (b == buttonApplyToAllParts)
                        applyToAllParts();
                  break;
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void TextStyleDialog::apply()
      {
      saveStyle(current);     // update local copy of style list
      applyToScore(cs);
      }

//---------------------------------------------------------
//   applyToScore
//---------------------------------------------------------

void TextStyleDialog::applyToScore(Score* s)
      {
      int count = textNames->count();
      int numOfStyles = s->style()->textStyles().size();
      for (int i = 0; i < count; ++i) {
            int listIdx = textNames->item(i)->data(Qt::UserRole).toInt();
            if (listIdx < numOfStyles) {         // style already exists in score text styles
                  const TextStyle& os = s->textStyle(TextStyleType(listIdx));
                  const TextStyle& ns = styles[listIdx];
                  if (os != ns)
                        cs->undo(new ChangeTextStyle(s, ns));
                  }
            else  // style does not exist in score text styles yet
                  s->undo(new AddTextStyle(s, styles[listIdx]));
            }
      s->update();
      }

//---------------------------------------------------------
//   applyToAllParts
//---------------------------------------------------------

void TextStyleDialog::applyToAllParts()
      {
      saveStyle(current);     // update local copy of style list
      QList<Excerpt*>& el = cs->rootScore()->excerpts();
      for (Excerpt* e : el)
            applyToScore(e->score());
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
      newStyle.setName(s);

      int count = textNames->count();
      int listIdx = styles.count();
      styles.append(newStyle);
      textNames->addItem(s);
      textNames->item(count)->setData(Qt::UserRole, listIdx);
      textNames->setCurrentRow(count);
      cs->setDirty(true);
      mscore->endCmd();
      }
}

