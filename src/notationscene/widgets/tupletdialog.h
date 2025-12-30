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

#include "ui_tupletdialog.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "actions/iactionsdispatcher.h"

namespace mu::notation {
class TupletDialog : public QDialog, Ui::TupletDialog, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<context::IGlobalContext> globalContext = { this };
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };

    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;

public:
    TupletDialog(QWidget* parent = nullptr);

    void defaultToStyleSettings();

    TupletNumberType numberType() const;
    TupletBracketType bracketType() const;

    INotationStylePtr style() const;
    INotationPtr notation() const;

    void apply();

private slots:
    void bboxClicked(QAbstractButton* button);
};
}
