//=============================================================================
//  MuseScore
//  Linux Music Score Editor
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

/**
 \file
 Implementation of class Selection plus other selection related functions.
*/

#include "libmscore/select.h"
#include "selectnotedialog.h"
#include "libmscore/element.h"
#include "libmscore/system.h"
#include "libmscore/score.h"
#include "libmscore/chord.h"
#include "libmscore/sym.h"
#include "libmscore/segment.h"
#include "libmscore/note.h"

#include "widgetstatestore.h"

using namespace mu::notation;

//---------------------------------------------------------
//   SelectDialog
//---------------------------------------------------------

SelectNoteDialog::SelectNoteDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("SelectNoteDialog");
    setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    m_note = dynamic_cast<Ms::Note*>(globalContext()->currentNotation()->interaction()->selection()->element());

    notehead->setText(Ms::NoteHead::group2userName(m_note->headGroup()));
    pitch->setText(m_note->tpcUserName());
    string->setText(QString::number(m_note->string() + 1));
    type->setText(m_note->noteTypeUserName());
    durationType->setText(tr("%1 Note").arg(m_note->chord()->durationType().durationTypeUserName()));
    durationTicks->setText(m_note->chord()->durationUserName());
    name->setText(tpc2name(m_note->tpc(), Ms::NoteSpellingType::STANDARD, Ms::NoteCaseType::AUTO, false));
    inSelection->setEnabled(m_note->score()->selection().isRange());

    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(buttonClicked(QAbstractButton*)));

    WidgetStateStore::restoreGeometry(this);
}

SelectNoteDialog::SelectNoteDialog(const SelectNoteDialog& other)
    : QDialog(other.parentWidget())
{
}

int SelectNoteDialog::metaTypeId()
{
    return QMetaType::type("SelectNoteDialog");
}

FilterNotesOptions SelectNoteDialog::noteOptions() const
{
    FilterNotesOptions options;
    options.elementType = ElementType::NOTE;
    if (sameNotehead->isChecked()) {
        options.notehead = m_note->headGroup();
    }
    if (samePitch->isChecked()) {
        options.pitch = m_note->pitch();
    }
    if (sameString->isChecked()) {
        options.string = m_note->string();
    }
    if (sameName->isChecked()) {
        options.tpc = m_note->tpc();
    }
    if (sameType->isChecked()) {
        options.noteType = m_note->noteType();
    }
    if (sameDurationType->isChecked()) {
        options.durationType = m_note->chord()->actualDurationType();
    }

    if (sameDurationTicks->isChecked()) {
        options.durationTicks = m_note->chord()->actualTicks();
    } else {
        options.durationTicks = Ms::Fraction(-1, 1);
    }

    if (sameStaff->isChecked()) {
        options.staffStart = m_note->staffIdx();
        options.staffEnd = m_note->staffIdx() + 1;
    } else if (inSelection->isChecked()) {
        options.staffStart = m_note->score()->selection().staffStart();
        options.staffEnd = m_note->score()->selection().staffEnd();
    } else {
        options.staffStart = -1;
        options.staffEnd = -1;
    }

    options.voice = sameVoice->isChecked() ? m_note->voice() : -1;
    options.system = nullptr;
    if (sameSystem->isChecked()) {
        options.system = m_note->chord()->segment()->system();
    }

    return options;
}

bool SelectNoteDialog::doReplace() const
{
    return replace->isChecked();
}

bool SelectNoteDialog::doAdd() const
{
    return add->isChecked();
}

bool SelectNoteDialog::doSubtract() const
{
    return subtract->isChecked();
}

bool SelectNoteDialog::doFromSelection() const
{
    return fromSelection->isChecked();
}

bool SelectNoteDialog::isInSelection() const
{
    return inSelection->isChecked();
}

void SelectNoteDialog::setSameStringVisible(bool v)
{
    sameString->setVisible(v);
    string->setVisible(v);
}

void SelectNoteDialog::buttonClicked(QAbstractButton* button)
{
    switch (buttonBox->standardButton(button)) {
    case QDialogButtonBox::Ok:
        apply();
        done(1);
        break;
    case QDialogButtonBox::Cancel:
        done(0);
        break;
    default:
        break;
    }
}

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void SelectNoteDialog::hideEvent(QHideEvent* event)
{
    WidgetStateStore::saveGeometry(this);
    QWidget::hideEvent(event);
}

std::shared_ptr<INotation> SelectNoteDialog::currentNotation() const
{
    return globalContext()->currentNotation();
}

INotationInteractionPtr SelectNoteDialog::currentNotationInteraction() const
{
    auto notation = currentNotation();
    if (!notation) {
        return nullptr;
    }

    return notation->interaction();
}

INotationElementsPtr SelectNoteDialog::currentNotationElements() const
{
    auto notation = currentNotation();
    if (!notation) {
        return nullptr;
    }

    return notation->elements();
}

void SelectNoteDialog::apply() const
{
    auto notationElements = currentNotationElements();
    auto interaction = currentNotationInteraction();
    if (!notationElements || !interaction) {
        return;
    }

    Element* selectedElement = interaction->selection()->element();
    if (!selectedElement) {
        return;
    }

    FilterElementsOptions options = noteOptions();

    std::vector<Element*> elements = notationElements->elements(options);
    if (elements.empty()) {
        return;
    }

    if (doReplace()) {
        interaction->clearSelection();
        interaction->select(elements, SelectType::ADD);
    } else if (doSubtract()) {
        std::vector<Element*> selesctionElements = interaction->selection()->elements();
        for (Element* element: elements) {
            selesctionElements.erase(std::remove(selesctionElements.begin(), selesctionElements.end(), element), selesctionElements.end());
        }

        interaction->clearSelection();
        interaction->select(selesctionElements, SelectType::ADD);
    } else if (doAdd()) {
        std::vector<Element*> selesctionElements = interaction->selection()->elements();

        std::vector<Element*> elementsToSelect;
        for (Element* element: elements) {
            if (std::find(selesctionElements.begin(), selesctionElements.end(), element) == selesctionElements.end()) {
                elementsToSelect.push_back(element);
            }
        }

        interaction->select(elementsToSelect, SelectType::ADD);
    }
}
