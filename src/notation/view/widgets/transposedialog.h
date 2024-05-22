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

#ifndef MU_NOTATION_TRANSPOSEDIALOG_H
#define MU_NOTATION_TRANSPOSEDIALOG_H

#include "ui_transposedialog.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

#include "notationtypes.h"

namespace mu::notation {
class TransposeDialog : public QDialog, Ui::TransposeDialogBase, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<context::IGlobalContext> context = { this };

public:
    TransposeDialog(QWidget* parent = 0);

private slots:
    void transposeByKeyToggled(bool);
    void transposeByIntervalToggled(bool);
    void chromaticBoxToggled(bool val);
    void diatonicBoxToggled(bool val);
    void apply();

private:
    void hideEvent(QHideEvent*) override;

    INotationPtr notation() const;
    INotationSelectionPtr selection() const;
    INotationInteractionPtr interaction() const;

    Key firstPitchedStaffKey() const;
    void setEnableTransposeKeys(bool val);
    void setEnableTransposeToKey(bool val);
    void setEnableTransposeChordNames(bool val);
    bool getTransposeKeys() const;
    bool getTransposeChordNames() const;
    Key transposeKey() const;
    int transposeInterval() const;
    TransposeDirection direction() const;
    TransposeMode mode() const;
    void setKey(Key k);
    bool useDoubleSharpsFlats() const;

    bool m_allSelected = false;
};
}

#endif // MU_NOTATION_TRANSPOSEDIALOG_H
