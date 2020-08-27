//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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
#include "notation/inotationelements.h"

using namespace mu::notation;

MeasurePropertiesDialog::MeasurePropertiesDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("MeasureProperties");
    setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    m_notation = context()->currentNotation();

    staves->verticalHeader()->hide();

    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(bboxClicked(QAbstractButton*)));
    connect(nextButton, SIGNAL(clicked()), SLOT(gotoNextMeasure()));
    connect(previousButton, SIGNAL(clicked()), SLOT(gotoPreviousMeasure()));

    if (qApp->layoutDirection() == Qt::LayoutDirection::RightToLeft) {
        horizontalLayout_2->removeWidget(nextButton);
        horizontalLayout_2->insertWidget(0, nextButton);
    }
}

MeasurePropertiesDialog::MeasurePropertiesDialog(const MeasurePropertiesDialog& dialog)
    : MeasurePropertiesDialog(dialog.parentWidget())
{
}

int MeasurePropertiesDialog::index() const
{
    return m_measureIndex;
}

void MeasurePropertiesDialog::setIndex(int measureIndex)
{
    if (m_measureIndex == measureIndex) {
        return;
    }

    Ms::Measure* measure = m_notation->elements()->measureByIndex(measureIndex);

    if (!measure) {
        return;
    }

    setMeasure(measure);

    m_measureIndex = measureIndex;
    emit indexChanged(m_measureIndex);
}

//---------------------------------------------------------
//   getNextMeasure
//    skip multi measure rests
//---------------------------------------------------------

Ms::Measure* getNextMeasure(Ms::Measure* m)
{
    Ms::Measure* mm = m->nextMeasureMM();
    while (mm && mm->isMMRest()) {
        mm = mm->nextMeasureMM();
    }
    return mm;
}

//---------------------------------------------------------
//   getPrevMeasure
//    skip multi measure rests
//---------------------------------------------------------

Ms::Measure* getPrevMeasure(Ms::Measure* m)
{
    Ms::Measure* mm = m->prevMeasureMM();
    while (mm && mm->isMMRest()) {
        mm = mm->prevMeasureMM();
    }
    return mm;
}

//---------------------------------------------------------
//   gotoNextMeasure
//---------------------------------------------------------

void MeasurePropertiesDialog::gotoNextMeasure()
{
    if (getNextMeasure(m_measure)) {
        setMeasure(getNextMeasure(m_measure));
    }
    nextButton->setEnabled(getNextMeasure(m_measure));
    previousButton->setEnabled(getPrevMeasure(m_measure));
    m_notation->notationChanged().notify();
}

//---------------------------------------------------------
//   gotoPreviousMeasure
//---------------------------------------------------------

void MeasurePropertiesDialog::gotoPreviousMeasure()
{
    if (getPrevMeasure(m_measure)) {
        setMeasure(getPrevMeasure(m_measure));
    }
    nextButton->setEnabled(getNextMeasure(m_measure));
    previousButton->setEnabled(getPrevMeasure(m_measure));
    m_notation->notationChanged().notify();
}

//---------------------------------------------------------
//   setMeasure
//---------------------------------------------------------

void MeasurePropertiesDialog::setMeasure(Ms::Measure* measure)
{
    m_measure = measure;
    nextButton->setEnabled(m_measure->nextMeasure() != 0);
    previousButton->setEnabled(m_measure->prevMeasure() != 0);

    setWindowTitle(tr("Measure Properties for Measure %1").arg(m_measure->no() + 1));
    m_notation->interaction()->clearSelection();
    m_notation->interaction()->select(m_measure, Ms::SelectType::ADD, 0);

    actualZ->setValue(m_measure->ticks().numerator());
    int index = actualN->findText(QString::number(m_measure->ticks().denominator()));
    if (index == -1) {
        index = 2;
    }
    actualN->setCurrentIndex(index);
    nominalZ->setNum(m_measure->timesig().numerator());
    nominalN->setNum(m_measure->timesig().denominator());

    irregular->setChecked(m_measure->irregular());
    breakMultiMeasureRest->setChecked(m_measure->breakMultiMeasureRest());
    int n  = m_measure->repeatCount();
    count->setValue(n);
    bool enableCount = m_measure->repeatEnd();
    count->setEnabled(enableCount);
    count->setVisible(enableCount);
    labelCount->setVisible(enableCount);
    layoutStretch->setValue(m_measure->userStretch());
    measureNumberMode->setCurrentIndex(int(m_measure->measureNumberMode()));
    measureNumberOffset->setValue(m_measure->noOffset());

    Ms::Score* score = m_measure->score();
    int rows = score->nstaves();
    staves->setRowCount(rows);
    staves->setColumnCount(3);

    for (int staffIdx = 0; staffIdx < rows; ++staffIdx) {
        QTableWidgetItem* item = new QTableWidgetItem(QString("%1").arg(staffIdx + 1));
        staves->setItem(staffIdx, 0, item);

        item = new QTableWidgetItem(tr("visible"));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setCheckState(m_measure->visible(staffIdx) ? Qt::Checked : Qt::Unchecked);
        if (rows == 1) {                  // cannot be invisible if only one row
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        }
        staves->setItem(staffIdx, 1, item);

        item = new QTableWidgetItem(tr("stemless"));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setCheckState(m_measure->stemless(staffIdx) ? Qt::Checked : Qt::Unchecked);
        staves->setItem(staffIdx, 2, item);
    }
}

//---------------------------------------------------------
//   bboxClicked
//---------------------------------------------------------

void MeasurePropertiesDialog::bboxClicked(QAbstractButton* button)
{
    QDialogButtonBox::ButtonRole br = buttonBox->buttonRole(button);
    switch (br) {
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

bool MeasurePropertiesDialog::visible(int staffIdx)
{
    QTableWidgetItem* item = staves->item(staffIdx, 1);
    return item->checkState() == Qt::Checked;
}

//---------------------------------------------------------
//   stemless
//---------------------------------------------------------

bool MeasurePropertiesDialog::stemless(int staffIdx)
{
    QTableWidgetItem* item = staves->item(staffIdx, 2);
    return item->checkState() == Qt::Checked;
}

//---------------------------------------------------------
//   sig
//---------------------------------------------------------

Ms::Fraction MeasurePropertiesDialog::len() const
{
    return Ms::Fraction(actualZ->value(), 1 << actualN->currentIndex());
}

//---------------------------------------------------------
//   isIrregular
//---------------------------------------------------------

bool MeasurePropertiesDialog::isIrregular() const
{
    return irregular->isChecked();
}

//---------------------------------------------------------
//   repeatCount
//---------------------------------------------------------

int MeasurePropertiesDialog::repeatCount() const
{
    return count->value();
}

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void MeasurePropertiesDialog::apply()
{
    Ms::Score* score = m_measure->score();

    bool propertiesChanged = false;
    for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        bool v = visible(staffIdx);
        bool s = stemless(staffIdx);
        if (m_measure->visible(staffIdx) != v || m_measure->stemless(staffIdx) != s) {
            score->undo(new Ms::ChangeMStaffProperties(m_measure, staffIdx, v, s));
            propertiesChanged = true;
        }
    }

    m_measure->undoChangeProperty(Ms::Pid::REPEAT_COUNT, repeatCount());
    m_measure->undoChangeProperty(Ms::Pid::BREAK_MMR, breakMultiMeasureRest->isChecked());
    m_measure->undoChangeProperty(Ms::Pid::USER_STRETCH, layoutStretch->value());
    m_measure->undoChangeProperty(Ms::Pid::MEASURE_NUMBER_MODE, measureNumberMode->currentIndex());
    m_measure->undoChangeProperty(Ms::Pid::NO_OFFSET, measureNumberOffset->value());
    m_measure->undoChangeProperty(Ms::Pid::IRREGULAR, isIrregular());

    if (m_measure->ticks() != len()) {
        Ms::ScoreRange range;
        range.read(m_measure->first(), m_measure->last());
        m_measure->adjustToLen(len());
    }

    if (propertiesChanged) {
        m_measure->triggerLayout();
    }

    m_notation->interaction()->select(m_measure, Ms::SelectType::SINGLE, 0);
    m_notation->notationChanged().notify();
}
