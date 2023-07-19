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
#include "selectdialog.h"

/**
 \file
 Implementation of class Selection plus other selection related functions.
*/

#include "engraving/libmscore/system.h"

#include "notationtypes.h"

#include "ui/view/widgetstatestore.h"

using namespace mu::notation;
using namespace mu::ui;

//---------------------------------------------------------
//   SelectDialog
//---------------------------------------------------------

SelectDialog::SelectDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("SelectDialog");
    setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    m_element = contextItem(globalContext()->currentNotation()->interaction());
    type->setText(m_element->translatedTypeUserName().toQString());

    if (m_element->type() == engraving::ElementType::ARTICULATION) {
        subtype->setText(m_element->translatedTypeUserName().toQString());
    } else {
        subtype->setText(m_element->translatedSubtypeUserName().toQString());
    }

    sameSubtype->setEnabled(m_element->subtype() != -1);
    subtype->setEnabled(m_element->subtype() != -1);

    const auto isSingleSelection = m_element->score()->selection().isSingle();
    inSelection->setCheckState(isSingleSelection ? Qt::CheckState::Unchecked : Qt::CheckState::Checked);
    inSelection->setEnabled(!isSingleSelection);

    sameDuration->setEnabled(m_element->isRest());

    connect(buttonBox, &QDialogButtonBox::clicked, this, &SelectDialog::buttonClicked);

    WidgetStateStore::restoreGeometry(this);

    //! NOTE: It is necessary for the correct start of navigation in the dialog
    setFocus();
}

SelectDialog::SelectDialog(const SelectDialog& other)
    : QDialog(other.parentWidget())
{
}

int SelectDialog::metaTypeId()
{
    return QMetaType::type("SelectDialog");
}

//---------------------------------------------------------
//   setPattern
//---------------------------------------------------------

FilterElementsOptions SelectDialog::elementOptions() const
{
    FilterElementsOptions options;
    options.elementType = m_element->type();
    options.subtype = m_element->subtype();
    if (m_element->isSlurSegment()) {
        const SlurSegment* slurSegment = dynamic_cast<const SlurSegment*>(m_element);
        options.subtype = static_cast<int>(slurSegment->spanner()->type());
    }

    if (sameStaff->isChecked()) {
        options.staffStart = static_cast<int>(m_element->staffIdx());
        options.staffEnd = static_cast<int>(m_element->staffIdx() + 1);
    } else if (inSelection->isChecked()) {
        options.staffStart = static_cast<int>(m_element->score()->selection().staffStart());
        options.staffEnd = static_cast<int>(m_element->score()->selection().staffEnd());
    } else {
        options.staffStart = -1;
        options.staffEnd = -1;
    }

    if (sameDuration->isChecked() && m_element->isRest()) {
        const Rest* rest = dynamic_cast<const Rest*>(m_element);
        options.durationTicks = rest->actualTicks();
    } else {
        options.durationTicks = Fraction(-1, 1);
    }

    options.voice = sameVoice->isChecked() ? static_cast<int>(m_element->voice()) : -1;
    options.bySubtype = sameSubtype->isChecked();

    if (sameSystem->isChecked()) {
        options.system = elementSystem(m_element);
    }

    return options;
}

mu::engraving::System* SelectDialog::elementSystem(const EngravingItem* element) const
{
    EngravingItem* _element = const_cast<EngravingItem*>(element);
    do {
        if (_element->type() == ElementType::SYSTEM) {
            return dynamic_cast<mu::engraving::System*>(_element);
        }
        _element = _element->parentItem();
    } while (element);

    return nullptr;
}

bool SelectDialog::doReplace() const
{
    return replace->isChecked();
}

bool SelectDialog::doAdd() const
{
    return add->isChecked();
}

bool SelectDialog::doSubtract() const
{
    return subtract->isChecked();
}

bool SelectDialog::doFromSelection() const
{
    return fromSelection->isChecked();
}

bool SelectDialog::isInSelection() const
{
    return inSelection->isChecked();
}

void SelectDialog::buttonClicked(QAbstractButton* button)
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

void SelectDialog::hideEvent(QHideEvent* event)
{
    WidgetStateStore::saveGeometry(this);
    QWidget::hideEvent(event);
}

std::shared_ptr<INotation> SelectDialog::currentNotation() const
{
    return globalContext()->currentNotation();
}

INotationInteractionPtr SelectDialog::currentNotationInteraction() const
{
    auto notation = currentNotation();
    if (!notation) {
        return nullptr;
    }

    return notation->interaction();
}

INotationElementsPtr SelectDialog::currentNotationElements() const
{
    auto notation = currentNotation();
    if (!notation) {
        return nullptr;
    }

    return notation->elements();
}

void SelectDialog::apply() const
{
    auto notationElements = currentNotationElements();
    auto interaction = currentNotationInteraction();
    if (!notationElements || !interaction) {
        return;
    }

    EngravingItem* selectedElement = contextItem(interaction);
    if (!selectedElement) {
        return;
    }

    FilterElementsOptions options = elementOptions();

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
