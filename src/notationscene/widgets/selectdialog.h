/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "ui/view/widgetdialog.h"

#include "ui_selectdialog.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

#include "notation/notationtypes.h"

namespace mu::engraving {
class EngravingItem;
class System;
}

namespace mu::notation {
//---------------------------------------------------------
//   SelectDialog
//---------------------------------------------------------

class SelectDialog : public muse::ui::WidgetDialog, private Ui::SelectDialog
{
    Q_OBJECT

    muse::ContextInject<context::IGlobalContext> globalContext = { this };

public:
    SelectDialog(QWidget* parent = nullptr);

    void componentComplete() override;

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

    engraving::System* elementSystem(const engraving::EngravingItem* element) const;

    const engraving::EngravingItem* m_element = nullptr;
};
}
