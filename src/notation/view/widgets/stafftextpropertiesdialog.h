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
#ifndef MU_NOTATION_STAFFTEXTPROPERTIESDIALOG_H
#define MU_NOTATION_STAFFTEXTPROPERTIESDIALOG_H

#include "ui_stafftextpropertiesdialog.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

namespace mu::engraving {
class StaffTextBase;
}

namespace mu::notation {
class StaffTextPropertiesDialog : public QDialog, public Ui::StaffTextPropertiesDialog
{
    Q_OBJECT

    INJECT(context::IGlobalContext, globalContext)

public:
    StaffTextPropertiesDialog(QWidget* parent = nullptr);
    StaffTextPropertiesDialog(const StaffTextPropertiesDialog& other);
    ~StaffTextPropertiesDialog() override;

    static int static_metaTypeId();

private slots:
    void saveValues();
    void setSwingControls(bool);

private:
    void hideEvent(QHideEvent*) override;

    INotationUndoStackPtr undoStack() const;

    engraving::StaffTextBase* m_originStaffText = nullptr;
    engraving::StaffTextBase* m_staffText = nullptr;
};
}

Q_DECLARE_METATYPE(mu::notation::StaffTextPropertiesDialog)

#endif // MU_NOTATION_STAFFTEXTPROPERTIESDIALOG_H
