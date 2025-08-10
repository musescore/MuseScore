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
#pragma once

#include "ui_selectdialog.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

namespace mu::engraving {
class System;
}

namespace mu::notation {
//---------------------------------------------------------
//   SelectDialog
//---------------------------------------------------------

class SelectDialog : public QDialog, Ui::SelectDialog, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<context::IGlobalContext> globalContext = { this };

public:
    SelectDialog(QWidget* parent = nullptr);

    bool doReplace() const;
    bool doAdd() const;
    bool doSubtract() const;
    bool doFromSelection() const;
    bool isInSelection() const;

private slots:
    void buttonClicked(QAbstractButton* button);

private:
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;

    INotationPtr currentNotation() const;
    INotationInteractionPtr currentNotationInteraction() const;
    INotationElementsPtr currentNotationElements() const;

    void apply() const;
    FilterElementsOptions elementOptions() const;

    mu::engraving::System* elementSystem(const EngravingItem* element) const;

    const EngravingItem* m_element = nullptr;
};
}
