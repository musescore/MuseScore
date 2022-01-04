/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "editstringdata.h"

#include <QKeyEvent>

#include "translation.h"
#include "ui/view/widgetstatestore.h"
#include "ui/view/widgetnavigationfix.h"
#include "editpitch.h"

static const int OPEN_ACCESSIBLE_TITLE_ROLE = Qt::UserRole + 1;

using namespace mu::notation;
using namespace mu::ui;

//---------------------------------------------------------
//   EditStringData
//    To edit the string data (tuning and number of frets) for an instrument
//---------------------------------------------------------

EditStringData::EditStringData(QWidget* parent, QList<Ms::instrString>* strings, int* frets)
    : QDialog(parent)
{
    setObjectName("EditStringData");
    setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    _strings = strings;
    QStringList hdrLabels;
    int numOfStrings = _strings->size();
    hdrLabels << tr("Open", "string data") << tr("Pitch", "string data");
    stringList->setHorizontalHeaderLabels(hdrLabels);
    stringList->setRowCount(numOfStrings);
    // if any string, insert into string list control and select the first one

    if (numOfStrings > 0) {
        int i;
        Ms::instrString strg;
        // insert into local working copy and into string list dlg control
        // IN REVERSED ORDER
        for (i=0; i < numOfStrings; i++) {
            strg = (*_strings)[numOfStrings - i - 1];
            _stringsLoc.append(strg);
            QTableWidgetItem* newCheck = new QTableWidgetItem();
            newCheck->setFlags(Qt::ItemFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled));
            newCheck->setCheckState(strg.open ? Qt::Checked : Qt::Unchecked);

            newCheck->setData(OPEN_ACCESSIBLE_TITLE_ROLE, qtrc("notation", "Open"));
            newCheck->setData(Qt::AccessibleTextRole, openColumnAccessibleText(newCheck));

            stringList->setItem(i, 0, newCheck);
            QTableWidgetItem* newPitch = new QTableWidgetItem(midiCodeToStr(strg.pitch));
            stringList->setItem(i, 1, newPitch);
        }
        stringList->setCurrentCell(0, 1);
    }
    // if no string yet, disable buttons acting on individual string
    else {
        editString->setEnabled(false);
        deleteString->setEnabled(false);
    }

    connect(stringList, &QTableWidget::itemChanged, this, [this](QTableWidgetItem* item){
        if (item->column() == 0) {
            item->setData(Qt::AccessibleTextRole, openColumnAccessibleText(item));
        }
    });

    _frets = frets;
    numOfFrets->setValue(*_frets);

    connect(deleteString, &QPushButton::clicked, this, &EditStringData::deleteStringClicked);
    connect(editString,   &QPushButton::clicked, this, &EditStringData::editStringClicked);
    connect(newString,    &QPushButton::clicked, this, &EditStringData::newStringClicked);

    connect(stringList,   &QTableWidget::itemClicked,       this, &EditStringData::listItemClicked);
    connect(stringList,   &QTableWidget::itemDoubleClicked, this, &EditStringData::editStringClicked);

    _modified = false;

    WidgetStateStore::restoreGeometry(this);

    //! NOTE: It is necessary for the correct start of navigation in the dialog
    setFocus();

    qApp->installEventFilter(this);
}

EditStringData::~EditStringData()
{
}

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void EditStringData::hideEvent(QHideEvent* ev)
{
    WidgetStateStore::saveGeometry(this);
    QWidget::hideEvent(ev);
}

bool EditStringData::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent
            && WidgetNavigationFix::fixNavigationForTableWidget(
                new WidgetNavigationFix::NavigationChain { stringList, newString, buttonBox },
                keyEvent->key())) {
            return true;
        }
    }

    return QDialog::eventFilter(obj, event);
}

QString EditStringData::openColumnAccessibleText(const QTableWidgetItem* item) const
{
    QString accessibleText = item->data(OPEN_ACCESSIBLE_TITLE_ROLE).toString() + ": "
                             + (item->checkState() == Qt::Checked ? tr("checked") : tr("unchecked"));
    return accessibleText;
}

//---------------------------------------------------------
//   deleteStringClicked
//---------------------------------------------------------

void EditStringData::deleteStringClicked()
{
    int i = stringList->currentRow();

    // remove item from local string list and from dlg list control
    _stringsLoc.removeAt(i);
    stringList->model()->removeRow(i);
    // if no more items, disable buttons acting on individual string
    if (stringList->rowCount() == 0) {
        editString->setEnabled(false);
        deleteString->setEnabled(false);
    }
    _modified = true;
}

//---------------------------------------------------------
//   editStringClicked
//---------------------------------------------------------

void EditStringData::editStringClicked()
{
    int i = stringList->currentRow();
    int newCode;

    EditPitch* ep = new EditPitch(this, _stringsLoc[i].pitch);
    if ((newCode=ep->exec()) != -1) {
        // update item value in local string list and item text in dlg list control
        _stringsLoc[i].pitch = newCode;
        QTableWidgetItem* item = stringList->item(i, 1);
        item->setText(midiCodeToStr(newCode));
        _modified = true;
    }
}

//---------------------------------------------------------
//   listItemClicked
//---------------------------------------------------------

void EditStringData::listItemClicked(QTableWidgetItem* item)
{
    int col = item->column();
    if (col != 0) {                 // ignore clicks not on check boxes
        return;
    }
    int row = item->row();

    // flip openness in local string list, then sync dlg list ctrl
    bool open = !_stringsLoc[row].open;
    _stringsLoc[row].open = open;
    stringList->item(row, col)->setCheckState(open ? Qt::Checked : Qt::Unchecked);
    _modified = true;
}

//---------------------------------------------------------
//   newStringClicked
//---------------------------------------------------------

void EditStringData::newStringClicked()
{
    int i, newCode;

    EditPitch* ep = new EditPitch(this);
    if ((newCode=ep->exec()) != -1) {
        // add below selected string or at the end if no selected string
        i = stringList->currentRow() + 1;
        if (i <= 0) {
            i = stringList->rowCount();
        }

        // insert in local string list and in dlg list control
        Ms::instrString strg = { newCode, 0 };
        _stringsLoc.insert(i, strg);
        stringList->insertRow(i);

        QTableWidgetItem* newCheck = new QTableWidgetItem();
        newCheck->setFlags(Qt::ItemFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled));
        newCheck->setCheckState(strg.open ? Qt::Checked : Qt::Unchecked);
        newCheck->setData(Qt::AccessibleTextRole, openColumnAccessibleText(newCheck));
        stringList->setItem(i, 0, newCheck);

        QTableWidgetItem* newPitch = new QTableWidgetItem(midiCodeToStr(strg.pitch));
        stringList->setItem(i, 1, newPitch);

        // select last added item and ensure buttons are active
        stringList->setCurrentCell(i, 1);
        editString->setEnabled(true);
        deleteString->setEnabled(true);
        _modified = true;
    }
}

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void EditStringData::accept()
{
    // store data back into original variables
    // string tunings are copied in reversed order (from lowest to highest)
    if (_modified) {
        _strings->clear();
        for (int i=_stringsLoc.size() - 1; i >= 0; i--) {
            _strings->append(_stringsLoc[i]);
        }
    }
    if (*_frets != numOfFrets->value()) {
        *_frets = numOfFrets->value();
        _modified = true;
    }

    if (_modified) {
        QDialog::accept();
    } else {
        QDialog::reject();                // if no data change, no need to trigger changes downward the caller chain
    }
}

//---------------------------------------------------------
//   midiCodeToStr
//    Converts a MIDI numeric pitch code to human-readable note name
//---------------------------------------------------------
static const char* s_esd_noteNames[] = {
    QT_TRANSLATE_NOOP("editstringdata", "C"),
    QT_TRANSLATE_NOOP("editstringdata", "C♯"),
    QT_TRANSLATE_NOOP("editstringdata", "D"),
    QT_TRANSLATE_NOOP("editstringdata", "E♭"),
    QT_TRANSLATE_NOOP("editstringdata", "E"),
    QT_TRANSLATE_NOOP("editstringdata", "F"),
    QT_TRANSLATE_NOOP("editstringdata", "F♯"),
    QT_TRANSLATE_NOOP("editstringdata", "G"),
    QT_TRANSLATE_NOOP("editstringdata", "A♭"),
    QT_TRANSLATE_NOOP("editstringdata", "A"),
    QT_TRANSLATE_NOOP("editstringdata", "B♭"),
    QT_TRANSLATE_NOOP("editstringdata", "B")
};

QString EditStringData::midiCodeToStr(int midiCode)
{
    return QString("%1 %2").arg(qApp->translate("editstringdata", s_esd_noteNames[midiCode % 12])).arg(midiCode / 12 - 1);
}
