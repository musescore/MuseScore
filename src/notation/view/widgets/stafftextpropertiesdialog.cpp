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
#include "stafftextpropertiesdialog.h"

#include "engraving/libmscore/masterscore.h"
#include "engraving/libmscore/score.h"
#include "engraving/libmscore/staff.h"
#include "engraving/libmscore/stafftextbase.h"

#include "ui/view/widgetstatestore.h"

#include "translation.h"

using namespace mu::notation;
using namespace mu::engraving;

static const QString STAFF_TEXT_PROPERTIES_DIALOG_NAME("StaffTextPropertiesDialog");

StaffTextPropertiesDialog::StaffTextPropertiesDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName(STAFF_TEXT_PROPERTIES_DIALOG_NAME);
    setupUi(this);

    const INotationPtr notation = globalContext()->currentNotation();
    const INotationInteractionPtr interaction = notation ? notation->interaction() : nullptr;
    EngravingItem* element = interaction ? interaction->hitElementContext().element : nullptr;
    StaffTextBase* st = element && element->isStaffTextBase() ? toStaffTextBase(element) : nullptr;

    if (!st) {
        return;
    }

    m_originStaffText = st;

    if (st->systemFlag()) {
        setWindowTitle(qtrc("notation/stafftextproperties", "System text properties"));
    } else {
        setWindowTitle(qtrc("notation/stafftextproperties", "Staff text properties"));
    }

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    m_staffText = static_cast<StaffTextBase*>(st->clone());

    if (m_staffText->swing()) {
        setSwingBox->setChecked(true);
        if (m_staffText->swingParameters().swingUnit == Constants::DIVISION / 2) {
            swingBox->setEnabled(true);
            swingEighth->setChecked(true);
            swingBox->setValue(m_staffText->swingParameters().swingRatio);
        } else if (m_staffText->swingParameters().swingUnit == Constants::DIVISION / 4) {
            swingBox->setEnabled(true);
            swingSixteenth->setChecked(true);
            swingBox->setValue(m_staffText->swingParameters().swingRatio);
        } else if (m_staffText->swingParameters().swingUnit == 0) {
            swingBox->setEnabled(false);
            swingOff->setChecked(true);
            swingBox->setValue(m_staffText->swingParameters().swingRatio);
        }
    }

    connect(swingOff, &QRadioButton::toggled, this, &StaffTextPropertiesDialog::setSwingControls);
    connect(swingEighth, &QRadioButton::toggled, this, &StaffTextPropertiesDialog::setSwingControls);
    connect(swingSixteenth, &QRadioButton::toggled, this, &StaffTextPropertiesDialog::setSwingControls);

    connect(this, &QDialog::accepted, this, &StaffTextPropertiesDialog::saveValues);

    ui::WidgetStateStore::restoreGeometry(this);
}

StaffTextPropertiesDialog::StaffTextPropertiesDialog(const StaffTextPropertiesDialog& other)
    : QDialog(other.parentWidget())
{
}

StaffTextPropertiesDialog::~StaffTextPropertiesDialog()
{
    delete m_staffText;
}

void StaffTextPropertiesDialog::hideEvent(QHideEvent* event)
{
    ui::WidgetStateStore::saveGeometry(this);
    QDialog::hideEvent(event);
}

int StaffTextPropertiesDialog::static_metaTypeId()
{
    return QMetaType::type(STAFF_TEXT_PROPERTIES_DIALOG_NAME.toStdString().c_str());
}

//---------------------------------------------------------
//   setSwingControls
//---------------------------------------------------------

void StaffTextPropertiesDialog::setSwingControls(bool checked)
{
    if (!checked) {
        return;
    }
    if (swingOff->isChecked()) {
        swingBox->setEnabled(false);
    } else if (swingEighth->isChecked()) {
        swingBox->setEnabled(true);
    } else if (swingSixteenth->isChecked()) {
        swingBox->setEnabled(true);
    }
}

void StaffTextPropertiesDialog::saveValues()
{
    if (setSwingBox->isChecked()) {
        m_staffText->setSwing(true);
        if (swingOff->isChecked()) {
            m_staffText->setSwingParameters(0, swingBox->value());
            swingBox->setEnabled(false);
        } else if (swingEighth->isChecked()) {
            m_staffText->setSwingParameters(Constants::DIVISION / 2, swingBox->value());
            swingBox->setEnabled(true);
        } else if (swingSixteenth->isChecked()) {
            m_staffText->setSwingParameters(Constants::DIVISION / 4, swingBox->value());
            swingBox->setEnabled(true);
        }
    }

    INotationUndoStackPtr stack = undoStack();
    IF_ASSERT_FAILED(stack) {
        return;
    }

    Score* score = m_originStaffText->score();
    StaffTextBase* nt = toStaffTextBase(m_staffText->clone());
    nt->setScore(score);

    stack->prepareChanges();
    score->undoChangeElement(m_originStaffText, nt);
    score->masterScore()->updateChannel();
    score->updateSwing();
    score->setPlaylistDirty();
    stack->commitChanges();
}

INotationUndoStackPtr StaffTextPropertiesDialog::undoStack() const
{
    INotationPtr notation = globalContext()->currentNotation();
    return notation ? notation->undoStack() : nullptr;
}
