//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: chordedit.cpp 4874 2011-10-21 12:18:42Z wschweer $
//
//  Copyright (C) 2008 Werner Schweer and others
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

#include <iostream>

#include "chordedit.h"
#include "libmscore/harmony.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/score.h"
#include "libmscore/chordlist.h"

//---------------------------------------------------------
//   ChordEdit
//---------------------------------------------------------

ChordEdit::ChordEdit(Score* s, QWidget* parent)
   : QDialog(parent)
      {
      _harmony = 0;
      score    = s;

      setupUi(this);
      // note that rootGroup button identifiers map conveniently
      // onto all possible tpc2step return values: don't change
      rootGroup = new QButtonGroup(this);
      rootGroup->addButton(rootC,   0);
      rootGroup->addButton(rootD,   1);
      rootGroup->addButton(rootE,   2);
      rootGroup->addButton(rootF,   3);
      rootGroup->addButton(rootG,   4);
      rootGroup->addButton(rootA,   5);
      rootGroup->addButton(rootB,   6);

      // note that accidentalsGroup button identifiers map conveniently
      // onto all possible tpc2alter return values: don't change
      accidentalsGroup = new QButtonGroup(this);
      accidentalsGroup->addButton(accDFlat,  -2 + 3);
      accidentalsGroup->addButton(accFlat,   -1 + 3);
      accidentalsGroup->addButton(accNone,    0 + 3);
      accidentalsGroup->addButton(accSharp,   1 + 3);
      accidentalsGroup->addButton(accDSharp,  2 + 3);

      extensionGroup = new QButtonGroup(this);
      extensionGroup->addButton(extMaj,    2);
      extensionGroup->addButton(ext2,     15);
      extensionGroup->addButton(extMaj7,   6);
      extensionGroup->addButton(extMaj9,   7);
      extensionGroup->addButton(ext6,      5);
      extensionGroup->addButton(ext69,    14);

      extensionGroup->addButton(extm,     16);
      extensionGroup->addButton(extm7,    19);
      extensionGroup->addButton(extm6,    23);
      extensionGroup->addButton(extm9,    20);
      extensionGroup->addButton(extmMaj7, 18);
      extensionGroup->addButton(extm7b5,  32);
      extensionGroup->addButton(extdim,   33);
      extensionGroup->addButton(ext7,     64);
      extensionGroup->addButton(ext9,     70);
      extensionGroup->addButton(ext13,    65);
      extensionGroup->addButton(ext7b9,   76);
      extensionGroup->addButton(extsus,  184);
      extensionGroup->addButton(ext7Sus, 128);
      extensionGroup->addButton(extOther,  0);

      extOtherCombo->clear();
      ChordList* cl = score->style()->chordList();
      foreach (const ChordDescription* cd, *cl) {
            QString p;
            if (cd->names.isEmpty())
                  p = "?";
            else
                  p = cd->names.front();
            extOtherCombo->addItem(p, cd->id);
            }
      connect(rootGroup, SIGNAL(buttonClicked(int)), SLOT(chordChanged()));
      connect(extensionGroup, SIGNAL(buttonClicked(int)), SLOT(chordChanged()));
      connect(extOtherCombo, SIGNAL(currentIndexChanged(int)), SLOT(chordChanged()));
      connect(accidentalsGroup, SIGNAL(buttonClicked(int)), SLOT(chordChanged()));
      connect(bassNote, SIGNAL(currentIndexChanged(int)), SLOT(chordChanged()));
      connect(extOther, SIGNAL(toggled(bool)), SLOT(otherToggled(bool)));
      connect(addButton, SIGNAL(clicked()), SLOT(addButtonClicked()));
      connect(deleteButton, SIGNAL(clicked()), SLOT(deleteButtonClicked()));

      extOtherCombo->setEnabled(false);

      model = new QStandardItemModel(0, 3);
      model->setHeaderData(0, Qt::Horizontal, tr("Type"));
      model->setHeaderData(1, Qt::Horizontal, tr("Value"));
      model->setHeaderData(2, Qt::Horizontal, tr("Alter"));

      connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                     SLOT(modelDataChanged(QModelIndex,QModelIndex)));

      degreeTable->setModel(model);
      delegate = new DegreeTabDelegate;
      degreeTable->setItemDelegate(delegate);
      degreeTable->setColumnWidth(0, 80);
      degreeTable->setColumnWidth(1, 71);
      degreeTable->setColumnWidth(2, 71);
      }

//---------------------------------------------------------
//   ChordEdit
//---------------------------------------------------------

ChordEdit::~ChordEdit()
      {
      delete _harmony;
      }

//---------------------------------------------------------
//   setHarmony
//---------------------------------------------------------

void ChordEdit::setHarmony(const Harmony* h)
      {
      _harmony = h->clone();
      setRoot(h->rootTpc());
      setBase(h->baseTpc());
      setExtension(h->id());

      for (int i = 0; i < h->numberOfDegrees(); ++i)
            addDegree(h->degree(i));
      }


// Set the chord root to val (in tpc)

//---------------------------------------------------------
//   setRoot
//---------------------------------------------------------

void ChordEdit::setRoot(int val)
      {
//      qDebug("ChordEdit::setRoot(val=%d) tpc2step=%d tpc2stepname=%s tpc2alter=%d\n",
//             val, tpc2step(val), qPrintable(tpc2stepName(val)), tpc2alter(val));

      QAbstractButton* button = NULL;
      int id = 0;

      // catch INVALID_TPC
      if (val == INVALID_TPC) {
            qDebug("ChordEdit::setRoot(val=INVALID_TPC)\n");
            // default to C, no accidentals
            button = rootGroup->button(0);
            button->setChecked(true);
            button = accidentalsGroup->button(0 + 3);
            button->setChecked(true);
            return;
            }

      // translate tpc to button nr
      id = tpc2step(val);
      button = rootGroup->button(id);
      if (button)
            button->setChecked(true);
      else
            qDebug("root button %d not found\n", val);

      id = tpc2alter(val);
      button = accidentalsGroup->button(id + 3);
      if (button)
            button->setChecked(true);
      else
            qDebug("root button %d not found\n", id);

      chordChanged();
      }

//---------------------------------------------------------
//   setExtension
//---------------------------------------------------------

void ChordEdit::setExtension(int val)
      {
      QAbstractButton* button = extensionGroup->button(val);
      if (button)
            button->setChecked(true);
      else {
            extOther->setChecked(true);
            int idx = extOtherCombo->findData(val);
            if (idx != -1)
                  extOtherCombo->setCurrentIndex(idx);
            }
      chordChanged();
      }

//---------------------------------------------------------
//   setBase
//---------------------------------------------------------

void ChordEdit::setBase(int val)
      {
      if (val == INVALID_TPC)
            return;

      // translate tpc to button nr
      int idx = tpc2pitch(val) + 1;
      bassNote->setCurrentIndex(idx);
      chordChanged();
      }

//---------------------------------------------------------
//   extension
//---------------------------------------------------------

const ChordDescription* ChordEdit::extension()
      {
      int id = extensionGroup->checkedId();
      if (id == -1)
            return 0;
      else if (id == 0) {
            int idx = extOtherCombo->currentIndex();
            return score->style()->chordDescription(extOtherCombo->itemData(idx).toInt());
            }
      else
            return score->style()->chordDescription(id);
      }

//---------------------------------------------------------
//   root
//---------------------------------------------------------

// return root as tpc

int ChordEdit::root()
      {
      int tpc = step2tpc(rootGroup->checkedId(), accidentalsGroup->checkedId() - 3);
//      qDebug("ChordEdit::root() rootid=%d accid=%d -> tpc=%d\n",
//             rootGroup->checkedId(), accidentalsGroup->checkedId() - 3, tpc);
      return tpc;
      }

//---------------------------------------------------------
//   base
//---------------------------------------------------------

// convert base (which is the current index in the bassNote combobox)
// to a tpc value.
// Note that the labels for each index are defined in chordedit.ui
// Note that the the combobox item text could also be used, but
// by using tpc2name all note name knowledge is in one location

int ChordEdit::base()
      {
      int baseTpc;
      switch (bassNote->currentIndex()) {
            case  1: baseTpc = 14; break; // C
            case  2: baseTpc =  9; break; // Db
            case  3: baseTpc = 16; break; // D
            case  4: baseTpc = 11; break; // Eb
            case  5: baseTpc = 18; break; // E
            case  6: baseTpc = 13; break; // F
            case  7: baseTpc = 20; break; // F#
            case  8: baseTpc = 15; break; // G
            case  9: baseTpc = 10; break; // Ab
            case 10: baseTpc = 17; break; // A
            case 11: baseTpc = 12; break; // Bb
            case 12: baseTpc = 19; break; // B
            default: baseTpc = INVALID_TPC;
            }
      return baseTpc;
      }

//---------------------------------------------------------
//   otherToggled
//---------------------------------------------------------

void ChordEdit::otherToggled(bool val)
      {
      extOtherCombo->setEnabled(val);
      }

//---------------------------------------------------------
//   chordChanged
//---------------------------------------------------------

void ChordEdit::chordChanged()
      {
//      qDebug("ChordEdit::chordChanged() root=%d ext=%d base=%d ndeg=%d\n",
//             root(), extension(), base(), numberOfDegrees());
      _harmony->clearDegrees();
      for (int i = 0; i < numberOfDegrees(); i++)
            _harmony->addDegree(degree(i));
      _harmony->setRootTpc(root());
      _harmony->setBaseTpc(base());
      _harmony->setId(extension() ? extension()->id : 0);
      QString s = _harmony->harmonyName();
      chordLabel->setText(s);
      }

//---------------------------------------------------------
//   addButtonClicked
//---------------------------------------------------------

// The "add degree" button was clicked: add a new row to the model
// As value is zero, the row does not yet contain an valid degree

void ChordEdit::addButtonClicked()
      {
      int rowCount = model->rowCount();
      if (model->insertRow(model->rowCount())) {
            QModelIndex index = model->index(rowCount, 0, QModelIndex());
            model->setData(index, QVariant("add"));
            index = model->index(rowCount, 1, QModelIndex());
            model->setData(index, QVariant(0));
            index = model->index(rowCount, 2, QModelIndex());
            model->setData(index, QVariant(0));
            }
      }

//---------------------------------------------------------
//   deleteButtonClicked
//---------------------------------------------------------

// The "delete degree" button was clicked: delete the current row from the model

void ChordEdit::deleteButtonClicked()
      {
      if (degreeTable->currentIndex().isValid()) {
            model->removeRow(degreeTable->currentIndex().row());
            }
      chordChanged();
      }

//---------------------------------------------------------
//   modelDataChanged
//---------------------------------------------------------

// Call-back, called when the model was changed. Happens three times in a row
// when a new degree is added and once when an individual cell is changed.
// Doesn't happen when a row is deleted.

void ChordEdit::modelDataChanged(const QModelIndex & /* topLeft */, const QModelIndex & /* bottomRight */)
      {
      chordChanged();
/*
      std::cout << "ChordEdit::modelDataChanged()" << std::endl;
      for (int row = 0; row < model->rowCount(); ++row) {
            for (int column = 0; column < model->columnCount(); ++column) {
                  QModelIndex index = model->index(row, column, QModelIndex());
                  if (index.isValid()) {
                        std::cout << "r=" << row << " c=" << column << " ";
                        if (column == 0)
                              std::cout << qPrintable(index.data().toString());
                        else
                              std::cout << index.data().toInt();
                        std::cout << std::endl;
                        }
                  }
            }
*/
      }

//---------------------------------------------------------
//   addDegree
//---------------------------------------------------------

// Add degree d to the chord

void ChordEdit::addDegree(HDegree d)
      {
      if (model->insertRow(model->rowCount())
          && (d.type() == ADD || d.type() == ALTER || d.type() == SUBTRACT)) {
            int rowCount = model->rowCount();
            QModelIndex index = model->index(rowCount - 1, 0, QModelIndex());
            switch (d.type()) {
                  case ADD:      model->setData(index, QVariant("add"));      break;
                  case ALTER:    model->setData(index, QVariant("alter"));    break;
                  case SUBTRACT: model->setData(index, QVariant("subtract")); break;
                  default:       /* do nothing */                             break;
                  }
            index = model->index(rowCount - 1, 1, QModelIndex());
            model->setData(index, QVariant(d.value()));
            index = model->index(rowCount - 1, 2, QModelIndex());
            model->setData(index, QVariant(d.alter()));
            }
      }

//---------------------------------------------------------
//   isValidDegree
//---------------------------------------------------------

// determine if row r in the model contains a valid degree
// all degrees with value > 0 are considered valid
// notes:
// - addDegree() and the "type" delegate make sure the type is always valid
// - alter is considered "don't care" here and ignored

bool ChordEdit::isValidDegree(int r)
      {
      QModelIndex index = model->index(r, 1, QModelIndex());
      if (index.isValid()) {
            if (index.data().toInt() > 0)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   numberOfDegrees
//---------------------------------------------------------

// return number of valid degrees in the model
// note: may be lower than the number of rows

int ChordEdit::numberOfDegrees()
      {
      int count = 0;
      for (int row = 0; row < model->rowCount(); ++row) {
            if (isValidDegree(row)) {
                  count++;
            }
      }
      return count;
      }

//---------------------------------------------------------
//   degree
//---------------------------------------------------------

// return valid degree i, where i = 0 corresponds to the first one
// note: must skips rows with invalid data

HDegree ChordEdit::degree(int i)
      {
      int count = -1;
      for (int row = 0; row < model->rowCount(); ++row) {
            if (isValidDegree(row)) {
                  count++;
                  if (count == i) {
                        QModelIndex index = model->index(row, 0, QModelIndex());
                        QString strType = index.data().toString();
                        int iType = 0;
                        if (strType == "add")           iType = ADD;
                        else if (strType == "alter")    iType = ALTER;
                        else if (strType == "subtract") iType = SUBTRACT;
                        index = model->index(row, 1, QModelIndex());
                        int value = index.data().toInt();
                        index = model->index(row, 2, QModelIndex());
                        int alter = index.data().toInt();
                        return HDegree(value, alter, iType);
                        }
                  }
            }
      return HDegree();
      }

//---------------------------------------------------------
//   DegreeTabDelegate constructor
//---------------------------------------------------------

DegreeTabDelegate::DegreeTabDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

//---------------------------------------------------------
//   createEditor
//---------------------------------------------------------

// Create a combobox with add, alter and subtract as editor for column 0,
// a spinbox for all others

QWidget *DegreeTabDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex & index) const
{
    if (index.column() == 0) {
        QComboBox *editor = new QComboBox(parent);
        editor->insertItem(0, "add");
        editor->insertItem(1, "alter");
        editor->insertItem(2, "subtract");
        return editor;
    } else if (index.column() == 1) {
        QSpinBox *editor = new QSpinBox(parent);
        editor->setMinimum(1);
        editor->setMaximum(13);
        return editor;
    } else {
        QSpinBox *editor = new QSpinBox(parent);
        editor->setMinimum(-2);
        editor->setMaximum( 2);
        return editor;
    }
    return 0;
}

//---------------------------------------------------------
//   setEditorData
//---------------------------------------------------------

void DegreeTabDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    int value = index.model()->data(index, Qt::DisplayRole).toInt();

    if (index.column() == 0) {
        QComboBox *spinBox = static_cast<QComboBox*>(editor);
        spinBox->setCurrentIndex(value);
    } else {
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->setValue(value);
    }
}

//---------------------------------------------------------
//   setModelData
//---------------------------------------------------------

void DegreeTabDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    if (index.column() == 0) {
        QComboBox *spinBox = static_cast<QComboBox*>(editor);
        model->setData(index, spinBox->currentText());
    } else {
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->interpretText();
        model->setData(index, spinBox->value());
    }
}

//---------------------------------------------------------
//   updateEditorGeometry
//---------------------------------------------------------

void DegreeTabDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
