//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: metaedit.cpp 5290 2012-02-07 16:27:27Z wschweer $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#include "metaedit.h"
#include "icons.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"
#include "musescore.h"

namespace Ms {

//---------------------------------------------------------
//   MetaEditDialog
//---------------------------------------------------------

MetaEditDialog::MetaEditDialog(Score* s, QWidget* parent)
   : QDialog(parent),
     m_score(s),
     m_dirty(false)
      {
      if (objectName().isEmpty())
            setObjectName("MetaEditDialog");

      setupUi(this);
      QDialog::setWindowFlag(Qt::WindowContextHelpButtonHint, false);
      QDialog::setWindowFlag(Qt::WindowMinMaxButtonsHint);

      QString mscoreVrs = m_score->mscoreVersion();
      if (!mscoreVrs.isEmpty())
            version->setText(mscoreVrs);

      int scoreVrs = m_score->mscVersion();
      if (scoreVrs)
            level->setText(QString::number(scoreVrs, 10)); // in decimal

      int rev = m_score->mscoreRevision();
      if (rev > 99999)  // MuseScore 1.3 is decimal 5702, 2.0 and later uses a 7-digit hex SHA
            revision->setText(QString::number(rev, 16));
      else
            revision->setText(QString::number(rev, 10));

      QString sfilePath(m_score->importedFilePath());
      if (!sfilePath.isEmpty())
            filePath->setText(m_score->importedFilePath());

      QString sheet = "QLineEdit { background: transparent; }";

      QMapIterator<QString, QString> i(s->metaTags());
      while (i.hasNext()) {
            i.next();

            QString s = i.key();
            if (isSystemTag(s)) {
                  //QLabel* tag = new QLabel;
                  QLineEdit *tag = new QLineEdit;
                  tag->setStyleSheet(sheet); // Couldn't find any other way to remove the background.
                  tag->setFrame(false);
                  tag->setFocusPolicy(Qt::NoFocus);
                  tag->setReadOnly(true);
                  tag->setToolTip(tr("This is a system tag. It's name can't be modified."));

                  if      (s ==  "platform")       tag->setText(tr("Platform"));
                  else if (s ==  "movementNumber") tag->setText(tr("Movement number"));
                  else if (s ==  "movementTitle")  tag->setText(tr("Movement title"));
                  else if (s ==  "workNumber")     tag->setText(tr("Work number"));
                  else if (s ==  "workTitle")      tag->setText(tr("Work title"));
                  else if (s ==  "arranger")       tag->setText(tr("Arranger"));
                  else if (s ==  "composer")       tag->setText(tr("Composer"));
                  else if (s ==  "lyricist")       tag->setText(tr("Lyricist"));
                  else if (s ==  "poet")           tag->setText(tr("Poet"));
                  else if (s ==  "translator")     tag->setText(tr("Translator"));
                  else if (s ==  "source")         tag->setText(tr("Source"));
                  else if (s ==  "copyright")      tag->setText(tr("Copyright"));
                  else if (s ==  "creationDate")   tag->setText(tr("Creation date"));
                  else                             tag->setText(s); // For all other cases.

                  QLineEdit *value = new QLineEdit(i.value());
                  connect(value, SIGNAL(textChanged(const QString&)), SLOT(setDirty()));

                  QToolButton *deleteButton = new QToolButton;
                  deleteButton->setIcon(*icons[int (Icons::close_ICON)]);
                  deleteButton->setIconSize(QSize(12, 12));
                  deleteButton->setToolTip(tr("This is a system tag. It can't be deleted."));

                  // for now the connections aren't even made for deleteButton.
                  // If in the future the system tags can be removed,
                  // simply uncomment the following lines. It should do the job.
                  // Enable it, though.
                  deleteButton->setEnabled(false);
//                  connect(deleteButton, SIGNAL(clicked()), SLOT(setDirty()));
//                  connect(deleteButton, SIGNAL(clicked()), tag,          SLOT(deleteLater()));
//                  connect(deleteButton, SIGNAL(clicked()), value,        SLOT(deleteLater()));
//                  connect(deleteButton, SIGNAL(clicked()), deleteButton, SLOT(deleteLater()));

                  int numFlags = scrollAreaLayout->rowCount();
                  scrollAreaLayout->addWidget(tag,          numFlags, 0);
                  scrollAreaLayout->addWidget(value,        numFlags, 1);
                  scrollAreaLayout->addWidget(deleteButton, numFlags, 2);
                  }
            else {
                  QLineEdit* tag = new QLineEdit(s);
                  connect(tag, SIGNAL(textChanged(const QString&)), SLOT(setDirty()));

                  QLineEdit *value = new QLineEdit(i.value());
                  connect(value, SIGNAL(textChanged(const QString&)), SLOT(setDirty()));

                  QToolButton *deleteButton = new QToolButton;
                  deleteButton->setIcon(*icons[int (Icons::close_ICON)]);
                  deleteButton->setIconSize(QSize(12, 12));

                  connect(deleteButton, SIGNAL(clicked()), SLOT(setDirty()));
                  connect(deleteButton, SIGNAL(clicked()), tag,          SLOT(deleteLater()));
                  connect(deleteButton, SIGNAL(clicked()), value,        SLOT(deleteLater()));
                  connect(deleteButton, SIGNAL(clicked()), deleteButton, SLOT(deleteLater()));

                  int numFlags = scrollAreaLayout->rowCount();
                  scrollAreaLayout->addWidget(tag,          numFlags, 0);
                  scrollAreaLayout->addWidget(value,        numFlags, 1);
                  scrollAreaLayout->addWidget(deleteButton, numFlags, 2);
                  }
            }

      scrollAreaLayout->setColumnStretch(1, 1); // make the value column occupy all the place

      connect(newButton,  SIGNAL(clicked()),     SLOT(newClicked()));
      connect(saveButton, SIGNAL(clicked()),     SLOT(saveClicked()));
      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   isSystemTag
//---------------------------------------------------------

bool MetaEditDialog::isSystemTag(QString tag) const {
      return (tag ==  "platform"      || tag ==  "movementNumber" || tag ==  "movementTitle"
              || tag ==  "workNumber" || tag ==  "workTitle"      || tag ==  "arranger"
              || tag ==  "composer"   || tag ==  "lyricist"       || tag ==  "poet"
              || tag ==  "translator" || tag ==  "source"         || tag ==  "copyright"
              || tag ==  "creationDate");
      }

////---------------------------------------------------------
////   isSystemTag
////---------------------------------------------------------

//bool MetaEditDialog::isTransformedSystemTag(QString tag) const {
//      return (tag ==  tr("Platform")       || tag ==  tr("Movement number") || tag ==  tr("Movement title")
//              || tag ==  tr("Work number") || tag ==  tr("Work title")      || tag ==  tr("Arranger")
//              || tag ==  tr("Composer")    || tag ==  tr("Lyricist")        || tag ==  tr("Poet")
//              || tag ==  tr("Translator")  || tag ==  tr("Source")          || tag ==  tr("Copyright")
//              || tag ==  tr("Creation date"));
//      }

//---------------------------------------------------------
//   newClicked
//---------------------------------------------------------

void MetaEditDialog::newClicked()
      {
      QLineEdit* tag = new QLineEdit;
      tag->setPlaceholderText(tr("Tag name"));
      connect(tag, SIGNAL(textChanged(const QString&)), SLOT(setDirty()));

      QLineEdit *value = new QLineEdit;
      value->setPlaceholderText(tr("Tag value"));
      connect(value, SIGNAL(textChanged(const QString&)), SLOT(setDirty()));

      QToolButton *deleteButton = new QToolButton;
      deleteButton->setIcon(*icons[int (Icons::close_ICON)]);
      deleteButton->setIconSize(QSize(12, 12));

      connect(deleteButton, SIGNAL(clicked()), SLOT(setDirty()));
      connect(deleteButton, SIGNAL(clicked()), tag,          SLOT(deleteLater()));
      connect(deleteButton, SIGNAL(clicked()), value,        SLOT(deleteLater()));
      connect(deleteButton, SIGNAL(clicked()), deleteButton, SLOT(deleteLater()));

      int numFlags = scrollAreaLayout->rowCount();
      scrollAreaLayout->addWidget(tag,          numFlags, 0);
      scrollAreaLayout->addWidget(value,        numFlags, 1);
      scrollAreaLayout->addWidget(deleteButton, numFlags, 2);

      setDirty(true);
      tag->setFocus();
      // scroll down to see the newly created tag.
      // the following line doesn't work for some reason.
      //scrollArea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
      // and this one too:
      //scrollArea->ensureWidgetVisible(tag);
      QScrollBar* scrbar = scrollArea->verticalScrollBar();
      int max = 100000;
      scrbar->setMaximum(max);
      scrbar->setValue(max);
      }

//---------------------------------------------------------
//   setDirty
//---------------------------------------------------------

void MetaEditDialog::setDirty(bool dirty)
      {
      if (dirty == m_dirty)
            return;

      if (dirty) {
            saveButton->setEnabled(true);
            setWindowTitle("Score properties: " + m_score->title() + tr("*", "For unsaved changes. This is probably useless,"
                                                                             "but hey, I have no idea if there's asterisks in"
                                                                             "cyrillic and arabic."));
            }
      else {
            saveButton->setEnabled(false);
            setWindowTitle("Score properties: " + m_score->title());
            }

      m_dirty = dirty;
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

bool MetaEditDialog::save()
      {
      if (m_dirty) {
            int idx = scrollAreaLayout->rowCount();
            QMap<QString, QString> map;
            for (int i = 0; i < idx; ++i) {
                  QLayoutItem *tagItem   = scrollAreaLayout->itemAtPosition(i, 0);
                  QLayoutItem *valueItem = scrollAreaLayout->itemAtPosition(i, 1);
                  if (tagItem && valueItem) {
                        QLineEdit *tag   = static_cast<QLineEdit*>(tagItem->widget());
                        QLineEdit *value = static_cast<QLineEdit*>(valueItem->widget());

                        QString tagText = tag->text();
                        if (tagText.isEmpty()) {
                              QMessageBox::warning(this, tr("MuseScore"),
                                                   tr("Tags can't have empty names."),
                                                   QMessageBox::Ok, QMessageBox::Ok);
                              tag->setFocus();
                              return false;
                              }
                        if (isSystemTag(tagText)) {
                              QMessageBox::warning(this, tr("MuseScore"),
                                                   tagText + tr(" is a reserved system tag.\n"
                                                                "It can't be used."),
                                                   QMessageBox::Ok, QMessageBox::Ok);
                              tag->setFocus();
                              return false;
                              }
                        if      (tagText ==  tr("Platform"))        tagText = "platform";
                        else if (tagText ==  tr("Movement number")) tagText = "movementNumber";
                        else if (tagText ==  tr("Movement title"))  tagText = "movementTitle";
                        else if (tagText ==  tr("Work number"))     tagText = "workNumber";
                        else if (tagText ==  tr("Work title"))      tagText = "workTitle";
                        else if (tagText ==  tr("Arranger"))        tagText = "arranger";
                        else if (tagText ==  tr("Composer"))        tagText = "composer";
                        else if (tagText ==  tr("Lyricist"))        tagText = "lyricist";
                        else if (tagText ==  tr("Poet"))            tagText = "poet";
                        else if (tagText ==  tr("Translator"))      tagText = "translator";
                        else if (tagText ==  tr("Source"))          tagText = "source";
                        else if (tagText ==  tr("Copyright"))       tagText = "copyright";
                        else if (tagText ==  tr("Creation date"))   tagText = "creationDate";

                        if (map.contains(tagText)) {
                              QMessageBox::warning(this, tr("MuseScore"),
                                                   tr("You have multiple fields with the same name."),
                                                   QMessageBox::Ok, QMessageBox::Ok);
                              tag->setFocus();
                              return false;
                              }
                        map.insert(tagText, value->text());
                        }
                  }
            m_score->undo(new ChangeMetaTags(m_score, map));
            setDirty(false);
            }
      // should the file be saved (to file, not to the internal copy)
      // automatically when save is clicked? For now it's not.
//             mscore->cmd(getAction("file-save"));
      return true;
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void MetaEditDialog::accept()
      {
      if (!save())
            return;

      QDialog::accept();
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void MetaEditDialog::hideEvent(QHideEvent* event)
      {
      if (m_dirty) {
            QMessageBox::StandardButton button = QMessageBox::warning(this, tr("MuseScore"),
                                                                      tr("You have unsaved changes.\nSave?"),
                                                                      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                                                      QMessageBox::Save);
            if (button == QMessageBox::Save) {
                  if (!save()) {
                        event->ignore();
                        return;
                        }
                  }
            else if (button == QMessageBox::Cancel) {
                  event->ignore();
                  return;
                  }
            }
      MuseScore::saveGeometry(this);
      event->accept();
      }

}

