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

//---------------------------------------------------------
//   MeasureProperties
//---------------------------------------------------------

MeasureProperties::MeasureProperties(Measure* _m, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      setMeasure(_m);
      staves->verticalHeader()->hide();
      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(bboxClicked(QAbstractButton*)));
      connect(nextButton, SIGNAL(clicked()), SLOT(gotoNextMeasure()));
      connect(previousButton, SIGNAL(clicked()), SLOT(gotoPreviousMeasure()));
      }

//---------------------------------------------------------
//   gotoNextMeasure
//---------------------------------------------------------

void MeasureProperties::gotoNextMeasure()
      {
      if (m->nextMeasure())
            setMeasure(m->nextMeasure());
      }

//---------------------------------------------------------
//   gotoPreviousMeasure
//---------------------------------------------------------

void MeasureProperties::gotoPreviousMeasure()
      {
      if (m->prevMeasure())
            setMeasure(m->prevMeasure());
      }

//---------------------------------------------------------
//   setMeasure
//---------------------------------------------------------

void MeasureProperties::setMeasure(Measure* _m)
      {
      m = _m;
      setWindowTitle(QString(tr("MuseScore: Measure Properties for Measure %1")).arg(m->no()+1));
      actualZ->setValue(m->len().numerator());
      actualN->setValue(m->len().denominator());
      nominalZ->setNum(m->timesig().numerator());
      nominalN->setNum(m->timesig().denominator());

      irregular->setChecked(m->irregular());
      breakMultiMeasureRest->setChecked(m->getBreakMultiMeasureRest());
      int n  = m->repeatCount();
      count->setValue(n);
      count->setEnabled(m->repeatFlags() & RepeatEnd);
      layoutStretch->setValue(m->userStretch());
      measureNumberOffset->setValue(m->noOffset());

      Score* score = m->score();
      int rows = score->nstaves();
      staves->setRowCount(rows);
      staves->setColumnCount(3);

      for (int staffIdx = 0; staffIdx < rows; ++staffIdx) {
            QTableWidgetItem* item = new QTableWidgetItem(QString("%1").arg(staffIdx+1));
            staves->setItem(staffIdx, 0, item);
            MStaff* ms = m->mstaff(staffIdx);

            item = new QTableWidgetItem(tr("visible"));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            item->setCheckState(ms->_visible ? Qt::Checked : Qt::Unchecked);
            if (rows == 1)                // cannot be invisible if only one row
                  item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            staves->setItem(staffIdx, 1, item);

            item = new QTableWidgetItem(tr("stemless"));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            item->setCheckState(ms->_slashStyle ? Qt::Checked : Qt::Unchecked);
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
                  qDebug("EditStaff: unknown button %d\n", int(br));
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
      return Fraction(actualZ->value(), actualN->value());
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
            MStaff* ms = m->mstaff(staffIdx);
            bool v = visible(staffIdx);
            bool s = slashStyle(staffIdx);
            if (ms->visible() != v || ms->slashStyle() != s)
                  score->undo(new ChangeMStaffProperties(ms, v, s));
            }
      if (isIrregular() != m->irregular()
         || breakMultiMeasureRest->isChecked() != m->breakMultiMeasureRest()
         || repeatCount() != m->repeatCount()
         || layoutStretch->value() != m->userStretch()
         || measureNumberOffset->value() != m->noOffset()
         || m->len() != len()
         ) {
            score->undo(new ChangeMeasureProperties(
               m,
               breakMultiMeasureRest->isChecked(),
               repeatCount(),
               layoutStretch->value(),
               measureNumberOffset->value(),
               isIrregular())
               );
            if (m->len() != len()) {
                  m->adjustToLen(len());
                  score->select(m, SELECT_RANGE, 0);
                  }
            }
      score->select(0, SELECT_SINGLE, 0);
      score->end();
      }

