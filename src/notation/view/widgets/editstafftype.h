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

#ifndef MU_NOTATION_EDITSTAFFTYPE_H
#define MU_NOTATION_EDITSTAFFTYPE_H

#include "ui_editstafftype.h"

#include "modularity/ioc.h"
#include "engraving/iengravingconfiguration.h"

#include "engraving/dom/stafftype.h"

#include "notation/notationtypes.h"

namespace mu::notation {
//---------------------------------------------------------
//   EditStaffType
//---------------------------------------------------------

class EditStaffType : public QDialog, private Ui::EditStaffType, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<engraving::IEngravingConfiguration> engravingConfiguration = { this };

    mu::engraving::StaffType staffType;

    virtual void hideEvent(QHideEvent*);
    void blockSignals(bool block);

    void setFromDlg();

    void tabStemsCompatibility(bool checked);
    void tabMinimShortCompatibility(bool checked);
    void tabStemThroughCompatibility(bool checked);
    QString createUniqueStaffTypeName(mu::engraving::StaffGroup group);
    void setValues();

private slots:
    void nameEdited(const QString&);
    void durFontNameChanged(int idx);
    void fretFontNameChanged(int idx);
    void tabStemThroughToggled(bool checked);
    void tabMinimShortToggled(bool checked);
    void tabStemsToggled(bool checked);
    void updatePreview();

    void savePresets();
    void loadPresets();
    void resetToTemplateClicked();
    void addToTemplatesClicked();
//      void staffGroupChanged(int);

public:
    EditStaffType(QWidget* parent = nullptr);
    ~EditStaffType() {}
    void setStaffType(const mu::engraving::StaffType* staffType);
    mu::engraving::StaffType getStaffType() const { return staffType; }

    void setInstrument(const Instrument& instrument);

private:
    muse::Ret loadScore(mu::engraving::MasterScore* score, const muse::io::path_t& path);
};
}
#endif
