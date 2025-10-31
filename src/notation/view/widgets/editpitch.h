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

#include "ui_editpitch.h"
#include "engraving/iengravingconfiguration.h"
#include "modularity/ioc.h"

namespace mu::notation {
//---------------------------------------------------------
//   EditPitch
//---------------------------------------------------------

class EditPitch : public QDialog, private Ui::EditPitchBase, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<mu::engraving::IEngravingConfiguration> engravingConfiguration = { this };

public:
    EditPitch(QWidget* parent);
    EditPitch(QWidget* parent, int midiCode);
    ~EditPitch() {}

private slots:
    void on_tableWidget_cellDoubleClicked(int row, int column);
    void accept() override;
    void reject() override { done(-1); }                               // return an invalid pitch MIDI code

private:
    virtual void showEvent(QShowEvent*) override;
    virtual void hideEvent(QHideEvent*) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

    void setup();
};
}
