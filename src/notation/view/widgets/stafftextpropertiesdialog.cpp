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
        tabWidget->removeTab(tabWidget->indexOf(tabAeolusStops));     // Aeolus settings  for staff text only
        tabWidget->removeTab(tabWidget->indexOf(tabCapoSettings));     // Capos for staff text only
    } else {
        setWindowTitle(qtrc("notation/stafftextproperties", "Staff text properties"));
#ifndef AEOLUS
        tabWidget->removeTab(tabWidget->indexOf(tabAeolusStops));
#endif
    }

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    m_staffText = static_cast<StaffTextBase*>(st->clone());

    if (m_staffText->swing()) {
        setSwingBox->setChecked(true);
        if (m_staffText->swingParameters().swingUnit == Constants::division / 2) {
            swingBox->setEnabled(true);
            swingEighth->setChecked(true);
            swingBox->setValue(m_staffText->swingParameters().swingRatio);
        } else if (m_staffText->swingParameters().swingUnit == Constants::division / 4) {
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

    //---------------------------------------------------
    //    setup capo
    //      Note that capo is stored as an int, where 0 = no change,
    //      1 = remove capo, and everyother number (n) = pitch increase
    //      of n-1 semitones.
    //---------------------------------------------------

    if (m_staffText->capo() != 0) {
        setCapoBox->setChecked(true);
        fretList->setCurrentIndex(m_staffText->capo() - 1);
    }

    connect(this, &QDialog::accepted, this, &StaffTextPropertiesDialog::saveValues);

    //---------------------------------------------------
    //    setup aeolus m_stops
    //---------------------------------------------------

    changeStops->setChecked(m_staffText->setAeolusStops());

    for (int i = 0; i < 4; ++i) {
        for (int k = 0; k < 16; ++k) {
            m_stops[i][k] = 0;
        }
    }
    m_stops[0][0]  = stop_3_0;
    m_stops[0][1]  = stop_3_1;
    m_stops[0][2]  = stop_3_2;
    m_stops[0][3]  = stop_3_3;
    m_stops[0][4]  = stop_3_4;
    m_stops[0][5]  = stop_3_5;
    m_stops[0][6]  = stop_3_6;
    m_stops[0][7]  = stop_3_7;
    m_stops[0][8]  = stop_3_8;
    m_stops[0][9]  = stop_3_9;
    m_stops[0][10] = stop_3_10;
    m_stops[0][11] = stop_3_11;

    m_stops[1][0]  = stop_2_0;
    m_stops[1][1]  = stop_2_1;
    m_stops[1][2]  = stop_2_2;
    m_stops[1][3]  = stop_2_3;
    m_stops[1][4]  = stop_2_4;
    m_stops[1][5]  = stop_2_5;
    m_stops[1][6]  = stop_2_6;
    m_stops[1][7]  = stop_2_7;
    m_stops[1][8]  = stop_2_8;
    m_stops[1][9]  = stop_2_9;
    m_stops[1][10] = stop_2_10;
    m_stops[1][11] = stop_2_11;
    m_stops[1][12] = stop_2_12;

    m_stops[2][0]  = stop_1_0;
    m_stops[2][1]  = stop_1_1;
    m_stops[2][2]  = stop_1_2;
    m_stops[2][3]  = stop_1_3;
    m_stops[2][4]  = stop_1_4;
    m_stops[2][5]  = stop_1_5;
    m_stops[2][6]  = stop_1_6;
    m_stops[2][7]  = stop_1_7;
    m_stops[2][8]  = stop_1_8;
    m_stops[2][9]  = stop_1_9;
    m_stops[2][10] = stop_1_10;
    m_stops[2][11] = stop_1_11;
    m_stops[2][12] = stop_1_12;
    m_stops[2][13] = stop_1_13;
    m_stops[2][14] = stop_1_14;
    m_stops[2][15] = stop_1_15;

    m_stops[3][0]  = stop_p_0;
    m_stops[3][1]  = stop_p_1;
    m_stops[3][2]  = stop_p_2;
    m_stops[3][3]  = stop_p_3;
    m_stops[3][4]  = stop_p_4;
    m_stops[3][5]  = stop_p_5;
    m_stops[3][6]  = stop_p_6;
    m_stops[3][7]  = stop_p_7;
    m_stops[3][8]  = stop_p_8;
    m_stops[3][9]  = stop_p_9;
    m_stops[3][10] = stop_p_10;
    m_stops[3][11] = stop_p_11;
    m_stops[3][12] = stop_p_12;
    m_stops[3][13] = stop_p_13;
    m_stops[3][14] = stop_p_14;
    m_stops[3][15] = stop_p_15;

    m_curTabIndex = tabWidget->currentIndex();
    connect(tabWidget, &QTabWidget::currentChanged, this, &StaffTextPropertiesDialog::tabChanged);

    WidgetStateStore::restoreGeometry(this);
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
    WidgetStateStore::saveGeometry(this);
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

//---------------------------------------------------------
//   tabChanged
//---------------------------------------------------------

void StaffTextPropertiesDialog::tabChanged(int tab)
{
    if (tab == 2) {
        for (int i = 0; i < 4; ++i) {
            for (int k = 0; k < 16; ++k) {
                if (m_stops[i][k]) {
                    m_stops[i][k]->setChecked(m_staffText->getAeolusStop(i, k));
                }
            }
        }
    }
    if (m_curTabIndex == 2) {
        m_staffText->setSetAeolusStops(changeStops->isChecked());
        for (int i = 0; i < 4; ++i) {
            for (int k = 0; k < 16; ++k) {
                if (m_stops[i][k]) {
                    m_staffText->setAeolusStop(i, k, m_stops[i][k]->isChecked());
                }
            }
        }
    }
    m_curTabIndex = tab;
}

void StaffTextPropertiesDialog::saveValues()
{
    //
    // save Aeolus m_stops
    //
    m_staffText->setSetAeolusStops(changeStops->isChecked());
    if (changeStops->isChecked()) {
        for (int i = 0; i < 4; ++i) {
            for (int k = 0; k < 16; ++k) {
                if (m_stops[i][k]) {
                    m_staffText->setAeolusStop(i, k, m_stops[i][k]->isChecked());
                }
            }
        }
    }
    if (setSwingBox->isChecked()) {
        m_staffText->setSwing(true);
        if (swingOff->isChecked()) {
            m_staffText->setSwingParameters(0, swingBox->value());
            swingBox->setEnabled(false);
        } else if (swingEighth->isChecked()) {
            m_staffText->setSwingParameters(Constants::division / 2, swingBox->value());
            swingBox->setEnabled(true);
        } else if (swingSixteenth->isChecked()) {
            m_staffText->setSwingParameters(Constants::division / 4, swingBox->value());
            swingBox->setEnabled(true);
        }
    }

    if (setCapoBox->isChecked()) {
        m_staffText->setCapo(fretList->currentIndex() + 1);
    } else {
        m_staffText->setCapo(0);
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
    score->updateCapo();
    score->updateSwing();
    score->setPlaylistDirty();
    stack->commitChanges();
}

INotationUndoStackPtr StaffTextPropertiesDialog::undoStack() const
{
    INotationPtr notation = globalContext()->currentNotation();
    return notation ? notation->undoStack() : nullptr;
}
