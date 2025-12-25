/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

/**
 \file
 Implementation of class Selection plus other selection related functions.
*/

#include "selectnotedialog.h"

#include "translation.h"

#include "engraving/types/typesconv.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/note.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/select.h"
#include "engraving/dom/system.h"

#include "ui/view/widgetstatestore.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace muse::ui;

//---------------------------------------------------------
//   SelectDialog
//---------------------------------------------------------

SelectNoteDialog::SelectNoteDialog(QWidget* parent)
    : QDialog(parent), muse::Injectable(muse::iocCtxForQWidget(this))
{
    setObjectName("SelectNoteDialog");
    setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    const INotationInteractionPtr interaction = globalContext()->currentNotation()->interaction();
    IF_ASSERT_FAILED(interaction) {
        return;
    }

    m_note = dynamic_cast<mu::engraving::Note*>(interaction->contextItem());
    IF_ASSERT_FAILED(m_note) {
        return;
    }

    notehead->setText(TConv::translatedUserName(m_note->headGroup()));
    sameNotehead->setAccessibleName(sameNotehead->text() + notehead->text());

    pitch->setText(m_note->tpcUserName());
    samePitch->setAccessibleName(samePitch->text() + pitch->text());

    string->setText(QString::number(m_note->string() + 1));
    sameString->setAccessibleName(sameString->text() + string->text());

    type->setText(m_note->noteTypeUserName());
    sameType->setAccessibleName(sameType->text() + type->text());

    //: %1 is a note duration. If your language does not have different terms for
    //: "quarter note" and "quarter" (for example), or if the translations for the
    //: durations as separate strings are not suitable to be used as adjectives here,
    //: translate this string with "%1", so that just the duration will be shown.
    durationType->setText(muse::qtrc("notation", "%1 note").arg(TConv::translatedUserName(m_note->chord()->durationType().type())));
    sameDurationType->setAccessibleName(sameDurationType->text() + durationType->text());

    durationTicks->setText(m_note->chord()->durationUserName());
    sameDurationTicks->setAccessibleName(sameDurationTicks->text() + durationTicks->text());

    name->setText(tpc2name(m_note->tpc(), mu::engraving::NoteSpellingType::STANDARD, mu::engraving::NoteCaseType::AUTO, false));
    sameName->setAccessibleName(sameName->text() + name->text());

    const auto isSingleSelection = m_note->score()->selection().isSingle();
    inSelection->setCheckState(isSingleSelection ? Qt::CheckState::Unchecked : Qt::CheckState::Checked);
    inSelection->setEnabled(!isSingleSelection);

    connect(buttonBox, &QDialogButtonBox::clicked, this, &SelectNoteDialog::buttonClicked);

    //! NOTE: It is necessary for the correct start of navigation in the dialog
    setFocus();
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
        options.durationTicks = mu::engraving::Fraction(-1, 1);
    }

    if (sameBeat->isChecked()) {
        options.beat = m_note->beat();
    } else {
        options.beat = mu::engraving::Fraction(0, 0);
    }

    if (sameMeasure->isChecked()) {
        options.measure = m_note->findMeasure();
    } else {
        options.measure = nullptr;
    }

    if (sameStaff->isChecked()) {
        options.staffStart = static_cast<int>(m_note->staffIdx());
        options.staffEnd = static_cast<int>(m_note->staffIdx() + 1);
    } else if (inSelection->isChecked()) {
        options.staffStart = static_cast<int>(m_note->score()->selection().staffStart());
        options.staffEnd = static_cast<int>(m_note->score()->selection().staffEnd());
    } else {
        options.staffStart = -1;
        options.staffEnd = -1;
    }

    options.voice = sameVoice->isChecked() ? static_cast<int>(m_note->voice()) : -1;
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
//   showEvent
//---------------------------------------------------------

void SelectNoteDialog::showEvent(QShowEvent* event)
{
    WidgetStateStore::restoreGeometry(this);
    QDialog::showEvent(event);
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

    EngravingItem* selectedElement = interaction->contextItem();
    if (!selectedElement) {
        return;
    }

    FilterNotesOptions options = noteOptions();

    std::vector<EngravingItem*> elements = notationElements->elements(options);
    if (elements.empty()) {
        return;
    }
    if (isInSelection()) {
        const auto& selectedElements = interaction->selection()->elements();
        elements.erase(std::remove_if(elements.begin(), elements.end(), [selectedElements](const auto& e) {
            return std::find(selectedElements.begin(), selectedElements.end(), e) == selectedElements.end();
        }), elements.end());
    }

    if (doReplace()) {
        interaction->clearSelection();
        interaction->select(elements, SelectType::ADD);
    } else if (doSubtract()) {
        std::vector<EngravingItem*> selectionElements = interaction->selection()->elements();
        for (EngravingItem* element: elements) {
            selectionElements.erase(std::remove(selectionElements.begin(), selectionElements.end(), element), selectionElements.end());
        }

        interaction->clearSelection();
        interaction->select(selectionElements, SelectType::ADD);
    } else if (doAdd()) {
        std::vector<EngravingItem*> selectionElements = interaction->selection()->elements();
        std::copy(selectionElements.begin(), selectionElements.end(), back_inserter(elements));
        interaction->select(elements, SelectType::ADD);
    }
}
