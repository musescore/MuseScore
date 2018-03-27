//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: measureproperties.cpp 5628 2012-05-15 07:46:43Z wschweer $
//
//  Copyright (C) 2007 Werner Schweer and others
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

#include "measureproperties.h"
#include "libmscore/measure.h"
#include "libmscore/sig.h"
#include "libmscore/score.h"
#include "libmscore/repeat.h"
#include "libmscore/undo.h"
#include "libmscore/range.h"
#include "musescore.h"
#include "timeline.h"

namespace Ms {

//---------------------------------------------------------
//   MeasureProperties
//---------------------------------------------------------

MeasureProperties::MeasureProperties(Measure* _m, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("MeasureProperties");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      setMeasure(_m);
      staves->verticalHeader()->hide();

      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(bboxClicked(QAbstractButton*)));
      connect(nextButton, SIGNAL(clicked()), SLOT(gotoNextMeasure()));
      connect(previousButton, SIGNAL(clicked()), SLOT(gotoPreviousMeasure()));

      nextButton->setEnabled(_m->nextMeasure() != 0);
      previousButton->setEnabled(_m->prevMeasure() != 0);
      if (qApp->layoutDirection() == Qt::LayoutDirection::RightToLeft) {
            horizontalLayout_2->removeWidget(nextButton);
            horizontalLayout_2->insertWidget(0, nextButton);
            }

      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   getNextMeasure
//    skip multi measure rests
//---------------------------------------------------------

Measure* getNextMeasure(Measure* m)
      {
      Measure* mm = m->nextMeasureMM();
      while (mm && mm->isMMRest())
            mm = mm->nextMeasureMM();
      return mm;
      }

//---------------------------------------------------------
//   getPrevMeasure
//    skip multi measure rests
//---------------------------------------------------------

Measure* getPrevMeasure(Measure* m)
      {
      Measure* mm = m->prevMeasureMM();
      while (mm && mm->isMMRest())
            mm = mm->prevMeasureMM();
      return mm;
      }

//---------------------------------------------------------
//   gotoNextMeasure
//---------------------------------------------------------

void MeasureProperties::gotoNextMeasure()
      {
      if (getNextMeasure(m))
            setMeasure(getNextMeasure(m));
      nextButton->setEnabled(getNextMeasure(m));
      previousButton->setEnabled(getPrevMeasure(m));
      m->score()->update();
      }

//---------------------------------------------------------
//   gotoPreviousMeasure
//---------------------------------------------------------

void MeasureProperties::gotoPreviousMeasure()
      {
      if (getPrevMeasure(m))
            setMeasure(getPrevMeasure(m));
      nextButton->setEnabled(getNextMeasure(m));
      previousButton->setEnabled(getPrevMeasure(m));
      m->score()->update();
      }

//---------------------------------------------------------
//   setMeasure
//---------------------------------------------------------

void MeasureProperties::setMeasure(Measure* _m)
      {
      m = _m;
      setWindowTitle(tr("Measure Properties for Measure %1").arg(m->no()+1));
      m->score()->deselectAll();
      m->score()->select(m, SelectType::ADD, 0);

      actualZ->setValue(m->len().numerator());
      int index = actualN->findText(QString::number(m->len().denominator()));
      if (index == -1)
            index = 2;
      actualN->setCurrentIndex(index);
      nominalZ->setNum(m->timesig().numerator());
      nominalN->setNum(m->timesig().denominator());

      irregular->setChecked(m->irregular());
      breakMultiMeasureRest->setChecked(m->breakMultiMeasureRest());
      int n  = m->repeatCount();
      count->setValue(n);
      bool enableCount = m->repeatEnd();
      count->setEnabled(enableCount);
      count->setVisible(enableCount);
      labelCount->setVisible(enableCount);
      layoutStretch->setValue(m->userStretch());
      measureNumberMode->setCurrentIndex(int(m->measureNumberMode()));
      measureNumberOffset->setValue(m->noOffset());

      Score* score = m->score();
      int rows = score->nstaves();
      staves->setRowCount(rows);
      staves->setColumnCount(3);

      for (int staffIdx = 0; staffIdx < rows; ++staffIdx) {
            QTableWidgetItem* item = new QTableWidgetItem(QString("%1").arg(staffIdx+1));
            staves->setItem(staffIdx, 0, item);

            item = new QTableWidgetItem(tr("visible"));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            item->setCheckState(m->visible(staffIdx) ? Qt::Checked : Qt::Unchecked);
            if (rows == 1)                // cannot be invisible if only one row
                  item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            staves->setItem(staffIdx, 1, item);

            item = new QTableWidgetItem(tr("stemless"));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            item->setCheckState(m->slashStyle(staffIdx) ? Qt::Checked : Qt::Unchecked);
            staves->setItem(staffIdx, 2, item);
            }
      }

//---------------------------------------------------------
//   bboxClicked
//---------------------------------------------------------

void MeasureProperties::bboxClicked(QAbstractButton* button)
      {
      QDialogButtonBox::ButtonRole br = buttonBox->buttonRole(button);
      switch(br) {
            case QDialogButtonBox::ApplyRole:
                  apply();
                  break;

            case QDialogButtonBox::AcceptRole:
                  apply();
                  // fall through

            case QDialogButtonBox::RejectRole:
                  close();
                  break;

            default:
                  qDebug("EditStaff: unknown button %d", int(br));
                  break;
            }
      }

//---------------------------------------------------------
//   visible
//---------------------------------------------------------

bool MeasureProperties::visible(int staffIdx)
      {
      QTableWidgetItem* item = staves->item(staffIdx, 1);
      return item->checkState() == Qt::Checked;
      }

//---------------------------------------------------------
//   slashStyle
//---------------------------------------------------------

bool MeasureProperties::slashStyle(int staffIdx)
      {
      QTableWidgetItem* item = staves->item(staffIdx, 2);
      return item->checkState() == Qt::Checked;
      }

//---------------------------------------------------------
//   sig
//---------------------------------------------------------

Fraction MeasureProperties::len() const
      {
      return Fraction(actualZ->value(), 1 << actualN->currentIndex());
      }

//---------------------------------------------------------
//   isIrregular
//---------------------------------------------------------

bool MeasureProperties::isIrregular() const
      {
      return irregular->isChecked();
      }

//---------------------------------------------------------
//   repeatCount
//---------------------------------------------------------

int MeasureProperties::repeatCount() const
      {
      return count->value();
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void MeasureProperties::apply()
      {
      Score* score = m->score();

      for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            bool v = visible(staffIdx);
            bool s = slashStyle(staffIdx);
            if (m->visible(staffIdx) != v || m->slashStyle(staffIdx) != s)
                  score->undo(new ChangeMStaffProperties(m, staffIdx, v, s));
            }

      m->undoChangeProperty(Pid::REPEAT_COUNT, repeatCount());
      m->undoChangeProperty(Pid::BREAK_MMR, breakMultiMeasureRest->isChecked());
      m->undoChangeProperty(Pid::USER_STRETCH, layoutStretch->value());
      m->undoChangeProperty(Pid::MEASURE_NUMBER_MODE, measureNumberMode->currentIndex());
      m->undoChangeProperty(Pid::NO_OFFSET, measureNumberOffset->value());
      m->undoChangeProperty(Pid::IRREGULAR, isIrregular());

      if (m->len() != len()) {
            ScoreRange range;
            range.read(m->first(), m->last());
            m->adjustToLen(len());
#if 0
            // handled by endCmd():
            else if (!MScore::noGui) {
                  QMessageBox::warning(0,
                     QT_TRANSLATE_NOOP("MeasureProperties", "MuseScore"),
                     QT_TRANSLATE_NOOP("MeasureProperties", "Cannot change measure length:\n"
                     "tuplet would cross measure")
                     );
                  }
#endif
            }
      score->select(m, SelectType::SINGLE, 0);
      score->update();
      mscore->timeline()->updateGrid();
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void MeasureProperties::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }

}

