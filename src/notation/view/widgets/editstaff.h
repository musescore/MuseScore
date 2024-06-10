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

#ifndef MU_NOTATION_EDITSTAFF_H
#define MU_NOTATION_EDITSTAFF_H

#include <QDialog>

#include "ui_editstaff.h"
#include "engraving/dom/stafftype.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "global/iinteractive.h"
#include "engraving/iengravingconfiguration.h"
#include "iselectinstrumentscenario.h"

namespace mu::notation {
class EditStaffType;

class EditStaff : public QDialog, private Ui::EditStaffBase, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<context::IGlobalContext> globalContext = { this };
    muse::Inject<muse::IInteractive> interactive = { this };
    muse::Inject<ISelectInstrumentsScenario> selectInstrumentsScenario = { this };
    muse::Inject<engraving::IEngravingConfiguration> engravingConfiguration = { this };

public:
    EditStaff(QWidget* parent = nullptr);

private:
    void hideEvent(QHideEvent*) override;
    void apply();
    void setStaff(mu::engraving::Staff*, const mu::engraving::Fraction& tick);
    void updateInterval(const mu::engraving::Interval&);
    void updateStaffType(const mu::engraving::StaffType& staffType);
    void updateInstrument();
    void updateNextPreviousButtons();

private slots:
    void bboxClicked(QAbstractButton* button);
    void editStringDataClicked();
    void showReplaceInstrumentDialog();
    void showStaffTypeDialog();
    void minPitchAClicked();
    void maxPitchAClicked();
    void minPitchPClicked();
    void maxPitchPClicked();
    void lineDistanceChanged();
    void numOfLinesChanged();
    void showClefChanged();
    void showTimeSigChanged();
    void showBarlinesChanged();
    void gotoNextStaff();
    void gotoPreviousStaff();
    void invisibleChanged();
    void colorChanged();
    void magChanged(double newValue);
    void isSmallChanged();
    void transpositionChanged();

signals:
    void instrumentChanged();

private:
    INotationPtr notation() const;
    IMasterNotationPtr masterNotation() const;
    INotationPartsPtr notationParts() const;
    INotationPartsPtr masterNotationParts() const;

    void initStaff();

    Staff* staff(int staffIndex) const;
    Instrument instrument() const;

    void applyStaffProperties();
    void applyPartProperties();

    QString midiCodeToStr(int midiCode);

    mu::engraving::Staff* m_staff = nullptr;
    mu::engraving::Staff* m_orgStaff = nullptr;
    Instrument m_instrument;
    Instrument m_orgInstrument;
    InstrumentKey m_instrumentKey;
    int m_minPitchA, m_maxPitchA, m_minPitchP, m_maxPitchP;
    mu::engraving::Fraction m_tickStart, m_tickEnd;

    EditStaffType* editStaffTypeDialog = nullptr;
};
}

#endif
