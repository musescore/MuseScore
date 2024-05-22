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
#ifndef MU_NOTATION_ALIGNSELECT_H
#define MU_NOTATION_ALIGNSELECT_H

#include "modularity/ioc.h"
#include "ui_align_select.h"
#include "engraving/types/types.h"

namespace mu::notation {
class AlignSelect : public QWidget, public Ui::AlignSelect, public muse::Injectable
{
    Q_OBJECT

public:
    AlignSelect(QWidget* parent);
    mu::engraving::Align align() const;
    void setAlign(mu::engraving::Align);

signals:
    void alignChanged(mu::engraving::Align);

private:
    QButtonGroup* g1;
    QButtonGroup* g2;

    void blockAlign(bool val);

private slots:
    void _alignChanged();
};
}

#endif // MU_NOTATION_ALIGNSELECT_H
